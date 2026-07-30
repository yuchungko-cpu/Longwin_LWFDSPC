#ifndef S_MODBUS_MASTER_H
#define S_MODBUS_MASTER_H

// --- 編譯開關: 用於啟用 Modbus 測試從站 (ID: 0xFE) ---
#define ENABLE_MODBUS_TEST_SLAVE

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 *  類型定義 (Type Definitions)
 * ========================================================================== */

// --- 公開的資料結構 (根據協議文件定義) ---

// --- 電池暫存器結構定義 ---
typedef struct
{
    uint16_t u16Regs[0x0A]; // Address-indexed: valid Modbus addresses 0x0001-0x0009.
} S_MODBUS_BATTERY_DATA_RAW;

typedef union
{
    S_MODBUS_BATTERY_DATA_RAW stRaw;
    uint16_t u16Regs[sizeof(S_MODBUS_BATTERY_DATA_RAW) / sizeof(uint16_t)];
} U_MODBUS_BATTERY_DATA;

// --- LCD/APP 暫存器結構定義 ---

typedef struct 
{
    uint16_t u16ToDeviceBlock1[6];   // Addr 0x0001 - 0x0006
    uint16_t u16FromDeviceBlock1[4]; // Addr 0x0007 - 0x000A
    uint16_t u16ToDeviceBlock2[2];   // Addr 0x000B - 0x000C
    uint16_t u16FromDeviceBlock2[5]; // Addr 0x000D - 0x0011
} S_MODBUS_LCD_APP_DATA_RAW;
typedef union
{
    S_MODBUS_LCD_APP_DATA_RAW stRaw;
    uint16_t u16Regs[0x12]; // Address-indexed: valid Modbus addresses 0x0001-0x0011.
} U_MODBUS_LCD_APP_DATA;

// --- PC GUI 暫存器結構定義 ---
typedef struct
{
    uint16_t u16ToDeviceBlock1[6];   // Addr 0x0001 - 0x0006
    uint16_t u16ToDeviceBlock2;      // Addr 0x000B
    uint16_t u16ToDeviceBlock3;      // Addr 0x0013
    uint16_t u16ToDeviceBlock4;      // Addr 0x0015
    uint16_t u16ToDeviceBlock5[6];   // Addr 0x0016 - 0x001B
    uint16_t u16ToDeviceBlock6;      // Addr 0x001C
    uint16_t u16ToDeviceBlock7;      // Addr 0x001D
    uint16_t u16FromDeviceBlock1[4]; // Addr 0x0007 - 0x000A
    uint16_t u16FromDeviceBlock2[3]; // Addr 0x000C - 0x000E
    uint16_t u16FromDeviceBlock3[3]; // Addr 0x000F - 0x0011
    uint16_t u16FromDeviceBlock4;    // Addr 0x0012
    uint16_t u16FromDeviceBlock5;    // Addr 0x0014
} S_MODBUS_PC_GUI_DATA_RAW;
typedef union
{
    S_MODBUS_PC_GUI_DATA_RAW stRaw;
    uint16_t u16Regs[0x1E]; // Address-indexed: valid Modbus addresses 0x0001-0x001D.
} U_MODBUS_PC_GUI_DATA;

// --- 包含所有裝置資料的聯合體 ---
typedef struct
{
    U_MODBUS_BATTERY_DATA uBatteryData;
    U_MODBUS_LCD_APP_DATA uLcdData;
    U_MODBUS_PC_GUI_DATA uPcGuiData;
} S_MODBUS_ALL_DATA;

/* ==========================================================================
 *  服務層 API (Service Layer - For Simple Main)
 * ========================================================================== */

// --- 從站 ID 定義 ---
#define SLAVE_ID_BATTERY 1
#define SLAVE_ID_LCD 2
#define SLAVE_ID_PC_GUI 3

#ifdef ENABLE_MODBUS_TEST_SLAVE
#define SLAVE_ID_TEST 0xFE
#endif

// --- 表驅動調度器結構 ---
typedef enum
{
    E_MODBUS_DIR_WRITE, // Master 寫入到 Slave
    E_MODBUS_DIR_READ   // Master 讀取從 Slave
} E_MODBUS_DIRECTION;

typedef struct
{
    uint8_t u8SlaveId;
    uint16_t u16StartAddr;
    uint16_t u16RegCount;
    E_MODBUS_DIRECTION eDirection;
    void *pvDataPtr;
} S_MODBUS_REG_MAP_ITEM;

/** @brief 初始化 Modbus 服務及內部排程器 */
void modbusService_init(uint32_t (*pfnGetTime_ms)(void), uint32_t u32TimeoutMs);

/** @brief 驅動 Modbus 自動排程器，必須在主迴圈中持續呼叫 */
void modbusService_process(void);

/** @brief 啟用或禁用自動排程器 */
void modbusService_enable(bool bEnable);

/** @brief 啟用或禁用指定的從站通訊任務 */
void modbusService_setSlaveEnabled(uint8_t u8SlaveId, bool bIsEnabled);

// --- Getters / Setters ---
const U_MODBUS_BATTERY_DATA *modbusService_getBatteryData(void);

const U_MODBUS_LCD_APP_DATA *modbusService_getLcdData(void);
void modbusService_setLcdData(const U_MODBUS_LCD_APP_DATA *pData);

const U_MODBUS_PC_GUI_DATA *modbusService_getGuiData(void);
void modbusService_setGuiData(const U_MODBUS_PC_GUI_DATA *pData);

const S_MODBUS_ALL_DATA *modbusService_getAllData(void);
void modbusService_setAllData(const S_MODBUS_ALL_DATA *pData);

/** @brief 取得 Modbus 資料結構的直接指標（方案二：受控全域化）
 *  @return 指向內部資料結構的指標，可直接存取，無需再透過函式
 *  @note 一次取得指標後，可直接進行多次高效能存取
 */
S_MODBUS_ALL_DATA* modbusService_getDataPtr(void);

/** @brief 檢查 LCD 初始寫入是否已發送
 *  @return 如果初始寫入已發送，則為 true，否則為 false
 */
bool modbusService_isLcdInitialWriteSent(void);

/** @brief 檢查是否有來自 LCD 的新資料
 *  @return 如果有新資料，則為 true，否則為 false
 */
bool modbusService_hasNewLcdData(void);

/** @brief 檢查是否有來自 GUI 的新資料
 *  @return 如果有新資料，則為 true，否則為 false
 */
bool modbusService_hasNewGuiData(void);

/** @brief 清除 LCD 新資料旗標
 */
void modbusService_clearNewLcdDataFlag(void);

/** @brief 清除 GUI 新資料旗標
 */
void modbusService_clearNewGuiDataFlag(void);

/* ==========================================================================
 *  引擎層 API (Engine Layer - For Main-Controlled Flow)
 * ========================================================================== */

/** @brief 建立讀取請求封包 */
uint8_t modbusEngine_buildReadReq(uint8_t u8SlaveId,
                                  uint16_t u16StartAddr,
                                  uint16_t u16RegCount,
                                  uint8_t *pu8TxBuffer);

/** @brief 建立多重寫入請求封包 */
uint8_t modbusEngine_buildWriteMultiReq(uint8_t u8SlaveId,
                                        uint16_t u16StartAddr,
                                        uint16_t u16RegCount,
                                        const uint16_t *pu16WriteData,
                                        uint8_t *pu8TxBuffer);

/** @brief 解析回應封包 */
bool modbusEngine_parseResponse(uint8_t u8SlaveId,
                                uint8_t u8FuncCode,
                                const uint8_t *pu8RxBuffer,
                                uint8_t u8RxLen,
                                uint16_t *pu16DestBuffer,
                                uint16_t u16DestCount);

#endif // S_MODBUS_MASTER_H
