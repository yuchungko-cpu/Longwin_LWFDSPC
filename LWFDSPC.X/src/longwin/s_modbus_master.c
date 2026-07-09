#include "s_modbus_master.h"
#include "s_hal_rs485.h"
#include <stddef.h>
#include <string.h>

//=================================================================================================
// 內部共享變數 (Internal Shared Variables)
//=================================================================================================
static uint32_t (*s_pfnGetTime_ms)(void) = NULL;
static uint32_t su32DefaultTimeoutMs = 100;
static bool sbServiceEnabled = true;

//=================================================================================================
// 靜態輔助函式宣告 (Static Helper Function Declarations)
//=================================================================================================
static uint16_t _modMst_calculateCrc16(const uint8_t *pu8Msg, uint16_t u16DataLen);

/* ==========================================================================
 *  服務層 (Service Layer) 實作
 * ========================================================================== */

// --- 服務層狀態 ---
typedef enum
{
    E_SVC_STATE_READY_TO_SEND,
    E_SVC_STATE_WAITING_FOR_RESPONSE,
    E_SVC_STATE_DELAY
} E_SVC_STATE;

// --- 狀態變數 ---
static E_SVC_STATE seSvcState = E_SVC_STATE_READY_TO_SEND;
static uint32_t su32SvcTimestamp = 0;
static int siCurrentMapIndex = 0;

// Index 0 is unused. IDs 1-4 map to indices 1-4. {BATT, LCD, GUI, APP}
static bool sabSlaveEnabled[5] = {false, true, true, true, false};

// --- 首次通訊旗標 ---
static bool sbLcdInitialContactMade = false; // 是否已與 LCD 首次通訊
static bool sbLcdInitialWriteSent = false;   // 是否已發送首次寫入

// --- 新資料旗標 ---
static bool sbNewDataFromLcd = false;
static bool sbNewDataFromGui = false;

// --- 資料儲存 ---
// 方案二：受控全域化 - 靜態全域變數，只透過函式介面存取
static S_MODBUS_ALL_DATA g_stModbusAllData;

#ifdef ENABLE_MODBUS_TEST_SLAVE
static bool sbTestSlaveEnabled = true;
#endif

// --- Modbus 註冊表 ---
static const S_MODBUS_REG_MAP_ITEM scstRegisterMap[] = {
    // --- 主要裝置任務 (ID 1-3) ---
    // ID01: Battery
    {SLAVE_ID_BATTERY, 0x0001, 9, E_MODBUS_DIR_READ, &g_stModbusAllData.uBatteryData.u16Regs},
    // ID02: LCD - 根據實際通訊序列調整
    {SLAVE_ID_LCD, 0x0001, 17, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uLcdData.u16Regs[0x01]},  // 寫入 17 個暫存器 (0x0001-0x0011)
    {SLAVE_ID_LCD, 0x0007, 11, E_MODBUS_DIR_READ, &g_stModbusAllData.uLcdData.u16Regs[0x07]},   // 讀取 11 個暫存器 (0x0007-0x0011)
    // ID03: PC-GUI
    {SLAVE_ID_PC_GUI, 0x0001, 6, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x01]},
    {SLAVE_ID_PC_GUI, 0x000B, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x0B]},
    {SLAVE_ID_PC_GUI, 0x0013, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x13]},
    {SLAVE_ID_PC_GUI, 0x0015, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x15]},
    {SLAVE_ID_PC_GUI, 0x0016, 6, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x16]},
    {SLAVE_ID_PC_GUI, 0x001C, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x1C]},
    {SLAVE_ID_PC_GUI, 0x001D, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x1D]},
    {SLAVE_ID_PC_GUI, 0x0007, 4, E_MODBUS_DIR_READ, &g_stModbusAllData.uPcGuiData.u16Regs[0x07]},
    {SLAVE_ID_PC_GUI, 0x000C, 3, E_MODBUS_DIR_READ, &g_stModbusAllData.uPcGuiData.u16Regs[0x0C]},
    {SLAVE_ID_PC_GUI, 0x000F, 3, E_MODBUS_DIR_READ, &g_stModbusAllData.uPcGuiData.u16Regs[0x0F]},
    {SLAVE_ID_PC_GUI, 0x0012, 1, E_MODBUS_DIR_READ, &g_stModbusAllData.uPcGuiData.u16Regs[0x12]},
    {SLAVE_ID_PC_GUI, 0x0014, 1, E_MODBUS_DIR_READ, &g_stModbusAllData.uPcGuiData.u16Regs[0x14]},

#ifdef ENABLE_MODBUS_TEST_SLAVE
    // --- 測試鏡像任務 (ID 0xFE) ---
    // (這些任務在所有主要裝置任務完成後執行)
    // Mirror for ID01
    {SLAVE_ID_TEST, 0x0101, 9, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uBatteryData.u16Regs[0x01]},
    // Mirror for ID02 - 根據實際通訊序列調整
    {SLAVE_ID_TEST, 0x0201, 17, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uLcdData.u16Regs[0x01]},  // 鏡像寫入 17 個暫存器
    {SLAVE_ID_TEST, 0x0207, 11, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uLcdData.u16Regs[0x07]},   // 鏡像讀取 11 個暫存器
    // Mirror for ID03
    {SLAVE_ID_TEST, 0x0301, 6, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x01]},
    {SLAVE_ID_TEST, 0x030B, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x0B]},
    {SLAVE_ID_TEST, 0x0313, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x13]},
    {SLAVE_ID_TEST, 0x0315, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x15]},
    {SLAVE_ID_TEST, 0x0316, 6, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x16]},
    {SLAVE_ID_TEST, 0x031C, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x1C]},
    {SLAVE_ID_TEST, 0x031D, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x1D]},

    {SLAVE_ID_TEST, 0x0307, 4, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x07]},
    {SLAVE_ID_TEST, 0x030C, 3, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x0C]},
    {SLAVE_ID_TEST, 0x030F, 3, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x0F]},
    {SLAVE_ID_TEST, 0x0312, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x12]},
    {SLAVE_ID_TEST, 0x0314, 1, E_MODBUS_DIR_WRITE, &g_stModbusAllData.uPcGuiData.u16Regs[0x14]},
#endif
};

static const int siRegisterMapSize = sizeof(scstRegisterMap) / sizeof(scstRegisterMap[0]);

static bool _modMst_isSlaveEnabled(uint8_t u8SlaveId)
{
#ifdef ENABLE_MODBUS_TEST_SLAVE
    if (u8SlaveId == SLAVE_ID_TEST)
    {
        return sbTestSlaveEnabled;
    }
#endif
    if (u8SlaveId > 0 && u8SlaveId <= 4)
    {
        return sabSlaveEnabled[u8SlaveId];
    }
    return false;
}

void modbusService_init(uint32_t (*pfnGetTime)(void), uint32_t u32Timeout)
{
    s_pfnGetTime_ms = pfnGetTime;
    su32DefaultTimeoutMs = u32Timeout;
    hal_rs485_init(pfnGetTime, 20);

    seSvcState = E_SVC_STATE_READY_TO_SEND;
    memset(&g_stModbusAllData, 0, sizeof(g_stModbusAllData));

    // 初始化首次同步旗標
    sbLcdInitialContactMade = false;
    sbLcdInitialWriteSent = false;

    // 初始化新資料旗標
    sbNewDataFromLcd = false;
    sbNewDataFromGui = false;
}

void modbusService_enable(bool bEnable)
{
    sbServiceEnabled = bEnable;
    if (!bEnable)
        seSvcState = E_SVC_STATE_READY_TO_SEND;
}

void modbusService_setSlaveEnabled(uint8_t u8SlaveId, bool bIsEnabled)
{
#ifdef ENABLE_MODBUS_TEST_SLAVE
    if (u8SlaveId == SLAVE_ID_TEST)
    {
        sbTestSlaveEnabled = bIsEnabled;
        return;
    }
#endif
    if (u8SlaveId > 0 && u8SlaveId <= 4)
        sabSlaveEnabled[u8SlaveId] = bIsEnabled;
}

void modbusService_process(void)
{
    if (!sbServiceEnabled || s_pfnGetTime_ms == NULL)
        return;

    hal_rs485_process();

    switch (seSvcState)
    {
    case E_SVC_STATE_READY_TO_SEND:
    {
        // --- Longwin: 新增首次寫入邏輯 ---
        if (!sbLcdInitialWriteSent) {
            for (int i = 0; i < siRegisterMapSize; ++i) {
                const S_MODBUS_REG_MAP_ITEM* pstTask = &scstRegisterMap[i];
                if (pstTask->u8SlaveId == SLAVE_ID_LCD && pstTask->eDirection == E_MODBUS_DIR_WRITE) {
                    uint8_t au8TxBuf[256];
                    uint8_t u8Len = modbusEngine_buildWriteMultiReq(pstTask->u8SlaveId,
                                                                    pstTask->u16StartAddr,
                                                                    pstTask->u16RegCount,
                                                                    (const uint16_t*)pstTask->pvDataPtr,
                                                                    au8TxBuf);
                    if (u8Len > 0) {
                        hal_rs485_send(au8TxBuf, u8Len);
                        seSvcState = E_SVC_STATE_WAITING_FOR_RESPONSE;
                        su32SvcTimestamp = s_pfnGetTime_ms();
                        sbLcdInitialWriteSent = true; // 標記為已發送
                    }
                    return; // 強制任務完成後，直接返回，等待回應
                }
            }
        }
        // --- 結束 ---

        int iInitialIndex = siCurrentMapIndex;
        do
        {
            const S_MODBUS_REG_MAP_ITEM *pstStartTask = &scstRegisterMap[siCurrentMapIndex];
            if (_modMst_isSlaveEnabled(pstStartTask->u8SlaveId))
            {
                uint16_t u16MergedRegCount = pstStartTask->u16RegCount;
                int iItemsMerged = 1;

                for (int i = 1; (siCurrentMapIndex + i) < siRegisterMapSize; ++i)
                {
                    const S_MODBUS_REG_MAP_ITEM *pstNextTask = &scstRegisterMap[siCurrentMapIndex + i];
                    if (pstNextTask->u8SlaveId == pstStartTask->u8SlaveId &&
                        pstNextTask->eDirection == pstStartTask->eDirection &&
                        pstNextTask->u16StartAddr == (pstStartTask->u16StartAddr + u16MergedRegCount))
                    {
                        u16MergedRegCount += pstNextTask->u16RegCount;
                        iItemsMerged++;
                    }
                    else
                    {
                        break;
                    }
                }

                uint8_t au8TxBuf[256];
                uint8_t u8Len = 0;
                if (pstStartTask->eDirection == E_MODBUS_DIR_READ)
                {
                    u8Len = modbusEngine_buildReadReq(pstStartTask->u8SlaveId,
                                                      pstStartTask->u16StartAddr,
                                                      u16MergedRegCount,
                                                      au8TxBuf);
                }
                else
                {
                    uint16_t au16WriteBuf[64];
                    uint16_t u16CurrentOffset = 0;
                    for (int i = 0; i < iItemsMerged; ++i)
                    {
                        const S_MODBUS_REG_MAP_ITEM *pstItem = &scstRegisterMap[siCurrentMapIndex + i];
                        memcpy(&au16WriteBuf[u16CurrentOffset], pstItem->pvDataPtr, pstItem->u16RegCount * sizeof(uint16_t));
                        u16CurrentOffset += pstItem->u16RegCount;
                    }
                    u8Len = modbusEngine_buildWriteMultiReq(pstStartTask->u8SlaveId,
                                                            pstStartTask->u16StartAddr,
                                                            u16MergedRegCount,
                                                            au16WriteBuf,
                                                            au8TxBuf);
                }

                if (u8Len > 0)
                {
                    hal_rs485_send(au8TxBuf, u8Len);
                    seSvcState = E_SVC_STATE_WAITING_FOR_RESPONSE;
                    su32SvcTimestamp = s_pfnGetTime_ms();
                }
                return;
            }
            siCurrentMapIndex = (siCurrentMapIndex + 1) % siRegisterMapSize;
        } while (siCurrentMapIndex != iInitialIndex);

        seSvcState = E_SVC_STATE_DELAY;
        su32SvcTimestamp = s_pfnGetTime_ms();
        break;
    }

    case E_SVC_STATE_WAITING_FOR_RESPONSE:
    {
        const S_MODBUS_REG_MAP_ITEM *pstStartTask = &scstRegisterMap[siCurrentMapIndex];
        uint16_t u16MergedRegCount = pstStartTask->u16RegCount;
        int iItemsMerged = 1;
        for (int i = 1; (siCurrentMapIndex + i) < siRegisterMapSize; ++i)
        {
            const S_MODBUS_REG_MAP_ITEM *pstNextTask = &scstRegisterMap[siCurrentMapIndex + i];
            if (pstNextTask->u8SlaveId == pstStartTask->u8SlaveId &&
                pstNextTask->eDirection == pstStartTask->eDirection &&
                pstNextTask->u16StartAddr == (pstStartTask->u16StartAddr + u16MergedRegCount))
            {
                u16MergedRegCount += pstNextTask->u16RegCount;
                iItemsMerged++;
            }
            else
            {
                break;
            }
        }

        if (hal_rs485_is_rx_complete())
        {
            // --- Longwin: 檢查是否為 LCD 讀取回應，且初始值尚未發送完成 ---
            if (pstStartTask->u8SlaveId == SLAVE_ID_LCD && 
                pstStartTask->eDirection == E_MODBUS_DIR_READ && 
                !sbLcdInitialWriteSent)
            {
                // 忽略來自 LCD 的讀取回應，直到初始值發送完成
                hal_rs485_receive_reset();
                siCurrentMapIndex = (siCurrentMapIndex + iItemsMerged) % siRegisterMapSize;
                seSvcState = E_SVC_STATE_DELAY;
                su32SvcTimestamp = s_pfnGetTime_ms();
                break;
            }
            // --- 結束 ---

            uint8_t au8RxBuf[256];
            uint8_t u8Len = hal_rs485_get_rx_length();
            if (u8Len > sizeof(au8RxBuf))
            {
                u8Len = (uint8_t)sizeof(au8RxBuf);
            }
            memcpy(au8RxBuf, hal_rs485_get_rx_data(), u8Len);
            hal_rs485_receive_reset();

            if (pstStartTask->eDirection == E_MODBUS_DIR_READ)
            {
                uint16_t au16TempReadBuf[64];
                bool bSuccess = modbusEngine_parseResponse(pstStartTask->u8SlaveId,
                                                           0x03,
                                                           au8RxBuf,
                                                           u8Len,
                                                           au16TempReadBuf,
                                                           u16MergedRegCount);
                if (bSuccess)
                {
                    // --- Longwin: 新增首次通訊偵測 ---
                    if (pstStartTask->u8SlaveId == SLAVE_ID_LCD)
                    {
                        sbLcdInitialContactMade = true;
                        sbNewDataFromLcd = true; // 設定 LCD 新資料旗標
                    }
                    else if (pstStartTask->u8SlaveId == SLAVE_ID_PC_GUI)
                    {
                        sbNewDataFromGui = true; // 設定 GUI 新資料旗標
                    }
                    // --- 結束 ---

                    // 驗證回應的暫存器範圍是否正確
                    uint8_t u8ByteCount = au8RxBuf[2];
                    uint16_t u16WordCount = u8ByteCount / 2;
                    
                    // ID02 特殊處理：檢查是否為 LCD 設備的回應異常
                    if (pstStartTask->u8SlaveId == SLAVE_ID_LCD)
                    {
                        // LCD 設備可能回應錯誤的暫存器範圍，需要特殊處理
                        if (u16WordCount == u16MergedRegCount)
                        {
                            // 正常情況：回應數量匹配
                            uint16_t u16CurrentOffset = 0;
                            for (int i = 0; i < iItemsMerged; ++i)
                            {
                                const S_MODBUS_REG_MAP_ITEM *pstItem = &scstRegisterMap[siCurrentMapIndex + i];
                                memcpy(pstItem->pvDataPtr, &au16TempReadBuf[u16CurrentOffset], pstItem->u16RegCount * sizeof(uint16_t));
                                u16CurrentOffset += pstItem->u16RegCount;
                            }
                        }
                        else
                        {
                            // LCD 回應異常：嘗試從回應中提取有效資料
                            // 根據實際通訊序列，LCD 可能回應 0x000D 開始的資料
                            uint16_t u16ValidCount = (u16WordCount < u16MergedRegCount) ? u16WordCount : u16MergedRegCount;
                            
                            // 只複製有效的資料
                            uint16_t u16CurrentOffset = 0;
                            for (int i = 0; i < iItemsMerged && u16CurrentOffset < u16ValidCount; ++i)
                            {
                                const S_MODBUS_REG_MAP_ITEM *pstItem = &scstRegisterMap[siCurrentMapIndex + i];
                                uint16_t u16CopyCount = (pstItem->u16RegCount <= (u16ValidCount - u16CurrentOffset)) ? 
                                                       pstItem->u16RegCount : (u16ValidCount - u16CurrentOffset);
                                
                                if (u16CopyCount > 0)
                                {
                                    memcpy(pstItem->pvDataPtr, &au16TempReadBuf[u16CurrentOffset], u16CopyCount * sizeof(uint16_t));
                                    u16CurrentOffset += u16CopyCount;
                                }
                            }
                        }
                    }
                    else
                    {
                        // 其他設備的正常處理
                        if (u16WordCount == u16MergedRegCount)
                        {
                            uint16_t u16CurrentOffset = 0;
                            for (int i = 0; i < iItemsMerged; ++i)
                            {
                                const S_MODBUS_REG_MAP_ITEM *pstItem = &scstRegisterMap[siCurrentMapIndex + i];
                                memcpy(pstItem->pvDataPtr, &au16TempReadBuf[u16CurrentOffset], pstItem->u16RegCount * sizeof(uint16_t));
                                u16CurrentOffset += pstItem->u16RegCount;
                            }
                        }
                    }
                }
            }
            else
            {
                modbusEngine_parseResponse(pstStartTask->u8SlaveId, 0x10, au8RxBuf, u8Len, NULL, 0);
            }

            // --- 處理完畢，進入下一個狀態 ---
            siCurrentMapIndex = (siCurrentMapIndex + iItemsMerged) % siRegisterMapSize;
            seSvcState = E_SVC_STATE_DELAY;
            su32SvcTimestamp = s_pfnGetTime_ms();
            break;
        }

        // --- 通訊超時處理 ---
        if (s_pfnGetTime_ms() - su32SvcTimestamp > su32DefaultTimeoutMs)
        {
            hal_rs485_receive_reset(); // 重置硬體層接收
            siCurrentMapIndex = (siCurrentMapIndex + iItemsMerged) % siRegisterMapSize;
            seSvcState = E_SVC_STATE_DELAY; // 即使超時也要進入延遲，避免連續發送
            su32SvcTimestamp = s_pfnGetTime_ms();
        }
        break;
    }

    case E_SVC_STATE_DELAY:
    {
        // 減少延遲時間，適應 LCD 的快速回應
        // 原本 50ms 延遲對於 LCD 來說太長了
        if (s_pfnGetTime_ms() - su32SvcTimestamp > 10)  // 減少到 10ms
        {
            seSvcState = E_SVC_STATE_READY_TO_SEND;
        }
        break;
    }
    }
}

// --- Getters / Setters 實作 ---
const U_MODBUS_BATTERY_DATA *modbusService_getBatteryData(void)
{
    return &g_stModbusAllData.uBatteryData;
}
const U_MODBUS_LCD_APP_DATA *modbusService_getLcdData(void)
{
    return &g_stModbusAllData.uLcdData;
}
void modbusService_setLcdData(const U_MODBUS_LCD_APP_DATA *pData)
{
    if (pData)
    {
        memcpy(&g_stModbusAllData.uLcdData, pData, sizeof(U_MODBUS_LCD_APP_DATA));
    }
}

const U_MODBUS_PC_GUI_DATA *modbusService_getGuiData(void)
{
    return &g_stModbusAllData.uPcGuiData;
}

void modbusService_setGuiData(const U_MODBUS_PC_GUI_DATA *pData)
{
    if (pData)
    {
        memcpy(&g_stModbusAllData.uPcGuiData, pData, sizeof(U_MODBUS_PC_GUI_DATA));
    }
}

const S_MODBUS_ALL_DATA *modbusService_getAllData(void)
{
    return &g_stModbusAllData;
}

void modbusService_setAllData(const S_MODBUS_ALL_DATA *pData)
{
    if (pData)
    {
        memcpy(&g_stModbusAllData, pData, sizeof(S_MODBUS_ALL_DATA));
    }
}

S_MODBUS_ALL_DATA* modbusService_getDataPtr(void)
{
    return &g_stModbusAllData;
}

bool modbusService_isLcdInitialWriteSent(void)
{
    return sbLcdInitialWriteSent;
}

bool modbusService_hasNewLcdData(void)
{
    return sbNewDataFromLcd;
}

bool modbusService_hasNewGuiData(void)
{
    return sbNewDataFromGui;
}

void modbusService_clearNewLcdDataFlag(void)
{
    sbNewDataFromLcd = false;
}

void modbusService_clearNewGuiDataFlag(void)
{
    sbNewDataFromGui = false;
}

/**
 * @brief Modbus RTU CRC-16 靜態查詢表
 */
static const uint16_t s_au16Crc16Table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

/**
 * @brief 使用查表法計算 Modbus RTU CRC-16
 * @param pu8Msg 指向要計算的數據的指標
 * @param u16DataLen 數據長度
 * @return 16位元的 CRC 校驗碼
 */
static uint16_t _modMst_calculateCrc16(const uint8_t *pu8Msg, uint16_t u16DataLen)
{
    uint16_t u16CRC = 0xFFFF;
    for (uint16_t i = 0; i < u16DataLen; i++)
    {
        uint8_t u8Index = (u16CRC ^ pu8Msg[i]) & 0xFF;
        u16CRC = (u16CRC >> 8) ^ s_au16Crc16Table[u8Index];
    }
    return u16CRC;
}

/* ==========================================================================
 *  引擎層 API (Engine Layer) 實作
 * ========================================================================== */

uint8_t modbusEngine_buildReadReq(uint8_t u8SlaveId,
                                  uint16_t u16StartAddr,
                                  uint16_t u16RegCount,
                                  uint8_t *pu8TxBuffer)
{
    if (pu8TxBuffer == NULL)
        return 0;

    pu8TxBuffer[0] = u8SlaveId;
    pu8TxBuffer[1] = 0x03;
    pu8TxBuffer[2] = (u16StartAddr >> 8) & 0xFF;
    pu8TxBuffer[3] = u16StartAddr & 0xFF;
    pu8TxBuffer[4] = (u16RegCount >> 8) & 0xFF;
    pu8TxBuffer[5] = u16RegCount & 0xFF;

    uint16_t u16Crc = _modMst_calculateCrc16(pu8TxBuffer, 6);
    pu8TxBuffer[6] = u16Crc & 0xFF;
    pu8TxBuffer[7] = (u16Crc >> 8) & 0xFF;

    return 8;
}

uint8_t modbusEngine_buildWriteMultiReq(uint8_t u8SlaveId,
                                        uint16_t u16StartAddr,
                                        uint16_t u16RegCount,
                                        const uint16_t *pu16WriteData,
                                        uint8_t *pu8TxBuffer)
{
    if (pu8TxBuffer == NULL || pu16WriteData == NULL)
        return 0;

    uint8_t u8ByteCount = u16RegCount * 2;
    uint8_t u8TotalLen = 7 + u8ByteCount + 2;

    pu8TxBuffer[0] = u8SlaveId;
    pu8TxBuffer[1] = 0x10;
    pu8TxBuffer[2] = (u16StartAddr >> 8) & 0xFF;
    pu8TxBuffer[3] = u16StartAddr & 0xFF;
    pu8TxBuffer[4] = (u16RegCount >> 8) & 0xFF;
    pu8TxBuffer[5] = u16RegCount & 0xFF;
    pu8TxBuffer[6] = u8ByteCount;

    for (uint16_t i = 0; i < u16RegCount; i++)
    {
        pu8TxBuffer[7 + i * 2] = (pu16WriteData[i] >> 8) & 0xFF;
        pu8TxBuffer[8 + i * 2] = pu16WriteData[i] & 0xFF;
    }

    uint16_t u16Crc = _modMst_calculateCrc16(pu8TxBuffer, u8TotalLen - 2);
    pu8TxBuffer[u8TotalLen - 2] = u16Crc & 0xFF;
    pu8TxBuffer[u8TotalLen - 1] = (u16Crc >> 8) & 0xFF;

    return u8TotalLen;
}

bool modbusEngine_parseResponse(uint8_t u8SlaveId,
                                uint8_t u8FuncCode,
                                const uint8_t *pu8RxBuffer,
                                uint8_t u8RxLen,
                                uint16_t *pu16DestBuffer,
                                uint16_t u16DestCount)
{
    if (pu8RxBuffer == NULL || u8RxLen < 4)
        return false;

    // CRC 驗證
    uint16_t u16ReceivedCrc = (pu8RxBuffer[u8RxLen - 1] << 8) | pu8RxBuffer[u8RxLen - 2];
    uint16_t u16CalculatedCrc = _modMst_calculateCrc16(pu8RxBuffer, u8RxLen - 2);
    if (u16ReceivedCrc != u16CalculatedCrc)
        return false;

    // 基本驗證
    if (pu8RxBuffer[0] != u8SlaveId)
        return false;

    if (pu8RxBuffer[1] & 0x80)  // 檢查錯誤回應
        return false;

    if (pu8RxBuffer[1] != u8FuncCode)
        return false;

    // 處理讀取回應 (FC 0x03)
    if (u8FuncCode == 0x03 && pu16DestBuffer != NULL)
    {
        uint8_t u8ByteCount = pu8RxBuffer[2];
        uint16_t u16WordCount = u8ByteCount / 2;

        // 驗證回應長度
        if ((3 + u8ByteCount + 2) != u8RxLen)
            return false;

        // 驗證暫存器數量
        if (u16WordCount > u16DestCount)
            u16WordCount = u16DestCount;

        // 複製資料
        for (uint16_t i = 0; i < u16WordCount; i++)
        {
            pu16DestBuffer[i] = (pu8RxBuffer[3 + i * 2] << 8) | pu8RxBuffer[4 + i * 2];
        }
    }
    // 處理寫入回應 (FC 0x10)
    else if (u8FuncCode == 0x10)
    {
        if (u8RxLen != 8)
            return false;
    }

    return true;
}