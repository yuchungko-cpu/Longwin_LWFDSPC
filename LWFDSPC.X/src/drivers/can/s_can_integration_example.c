/*******************************************************************************
  Longwin CAN 整合範例 (CAN Integration Example)

  檔案名稱:
    s_can_integration_example.c

  摘要:
    此檔案展示如何在主程式中整合新的 CAN 驅動模組
    包含回調函式實作和主程式整合範例

  描述:
    此檔案提供了完整的 CAN 整合範例，展示如何：
    1. 初始化 CAN 驅動器
    2. 註冊回調函式
    3. 在主迴圈中處理 CAN 訊息
    4. 與現有系統整合

*******************************************************************************/
#include "s_can_driver.h"
#include "s_can_integration_example.h"
#include "../../general.h"
#include "../../userparms.h"

// *****************************************************************************
// 外部變數宣告 (External Variable Declarations)
// *****************************************************************************
// 這些變數來自 HallFOC_Regen.c，需要在此宣告
extern UGF_T uGF;
extern volatile FAULT_DATA_T faultOvervoltage;
extern volatile FAULT_DATA_T faultOverTempMOSFET;
extern volatile FaultFlags_T FaultFlags;
extern signed int HallPulsesLatch;
extern signed int ReGenTorq;

// *****************************************************************************
// CAN 回調函式實作 (CAN Callback Function Implementations)
// *****************************************************************************

/**
 * @brief 電池電壓查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canBatteryVoltageCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 從 faultOvervoltage.monitor 取得電池電壓值
    uint16_t u16Voltage = faultOvervoltage.monitor + 1;
    
    // 設定回應資料
    pu8ResponseData[0] = u16Voltage & 0xFF;
    pu8ResponseData[1] = (u16Voltage >> 8) & 0xFF;
    pu8ResponseData[2] = 0;
    pu8ResponseData[3] = 0;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 再生煞車模式查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canRegenModeCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 設定回應資料
    pu8ResponseData[0] = uGF.ReGenMode;
    pu8ResponseData[1] = 0xFF;
    pu8ResponseData[2] = 0xFF;
    pu8ResponseData[3] = 0xFF;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 輔助模式查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canAssistanceModeCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 設定回應資料
    pu8ResponseData[0] = uGF.DriveMode;
    pu8ResponseData[1] = 0xFF;
    pu8ResponseData[2] = 0xFF;
    pu8ResponseData[3] = 0xFF;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 驅動器溫度查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canDriveTempCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 從 faultOverTempMOSFET.monitor 取得溫度值
    uint16_t u16Temperature = faultOverTempMOSFET.monitor;
    
    // 設定回應資料
    pu8ResponseData[0] = u16Temperature & 0xFF;
    pu8ResponseData[1] = (u16Temperature >> 8) & 0xFF;
    pu8ResponseData[2] = 0;
    pu8ResponseData[3] = 0;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 馬達轉速查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canMotorRpmCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 從 HallPulsesLatch 計算馬達轉速
    uint16_t u16MotorRPM = HallPulsesLatch * 10;
    
    // 設定回應資料
    pu8ResponseData[0] = u16MotorRPM & 0xFF;
    pu8ResponseData[1] = (u16MotorRPM >> 8) & 0xFF;
    pu8ResponseData[2] = 0;
    pu8ResponseData[3] = 0;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 驅動器狀態查詢回調函式實作
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canDriveStatusCallback(uint8_t* pu8ResponseData, uint8_t* pu8DataLength)
{
    if (pu8ResponseData == NULL || pu8DataLength == NULL) {
        return false;
    }
    
    // 計算故障標誌
    uint8_t u8FaultFlags = FaultFlags.MOSOverHeat +
                          (FaultFlags.Overvoltage << 1) +
                          (FaultFlags.Undervoltage << 2);
    
    // 設定回應資料
    pu8ResponseData[0] = uGF.RunMotor;
    pu8ResponseData[1] = 0;
    pu8ResponseData[2] = 0;
    pu8ResponseData[3] = u8FaultFlags;
    
    *pu8DataLength = 4;
    
    return true;
}

/**
 * @brief 再生煞車模式設定回調函式實作
 * @param u8RegenMode 再生煞車模式值
 * @return bool true=設定成功, false=設定失敗
 */
static bool _canRegenSetCallback(uint8_t u8RegenMode)
{
    // 根據模式設定再生煞車參數
    switch (u8RegenMode) {
        case 0:
            uGF.ReGenSet = 0;
            uGF.ReGenMode = 0;
            ReGenTorq = 0;
            break;
            
        case 1:
            uGF.ReGenSet = 1;
            uGF.ReGenMode = 1;
            ReGenTorq = 2000;
            break;
            
        case 2:
            uGF.ReGenSet = 1;
            uGF.ReGenMode = 2;
            ReGenTorq = 6553;
            break;
            
        case 3:
            uGF.ReGenSet = 1;
            uGF.ReGenMode = 3;
            ReGenTorq = 9000;
            break;
            
        default:
            return false;
    }
    
    return true;
}

/**
 * @brief 輔助模式設定回調函式實作
 * @param u8AssistMode 輔助模式值
 * @return bool true=設定成功, false=設定失敗
 */
static bool _canAssistSetCallback(uint8_t u8AssistMode)
{
    // 設定輔助模式
    uGF.DriveMode = u8AssistMode;
    return true;
}

/**
 * @brief 馬達啟用/停用設定回調函式實作
 * @param u8MotorEnable 馬達啟用狀態
 * @return bool true=設定成功, false=設定失敗
 */
static bool _canMotorEnableCallback(uint8_t u8MotorEnable)
{
    // 設定馬達啟用狀態
    uGF.CAN_Runmotor = u8MotorEnable;
    return true;
}

// *****************************************************************************
// CAN 整合函式 (CAN Integration Functions)
// *****************************************************************************

/**
 * @brief 初始化 CAN 整合模組
 * @return bool true=成功, false=失敗
 * @note 此函式會初始化 CAN 驅動器並註冊所有回調函式
 */
bool canIntegration_init(void)
{
    // 初始化 CAN 驅動器
    if (!canDriver_init()) {
        return false;
    }
    
    // 註冊所有回調函式
    canDriver_registerBatteryVoltageCallback(_canBatteryVoltageCallback);
    canDriver_registerRegenModeCallback(_canRegenModeCallback);
    canDriver_registerAssistanceModeCallback(_canAssistanceModeCallback);
    canDriver_registerDriveTempCallback(_canDriveTempCallback);
    canDriver_registerMotorRpmCallback(_canMotorRpmCallback);
    canDriver_registerDriveStatusCallback(_canDriveStatusCallback);
    canDriver_registerRegenSetCallback(_canRegenSetCallback);
    canDriver_registerAssistSetCallback(_canAssistSetCallback);
    canDriver_registerMotorEnableCallback(_canMotorEnableCallback);
    
    return true;
}

/**
 * @brief 處理 CAN 訊息 (主迴圈中呼叫)
 * @return 無
 * @note 此函式應在主迴圈中定期呼叫，處理接收到的 CAN 訊息
 */
void canIntegration_processMessages(void)
{
    CAN_MSG_OBJ sRxMsg;
    
    // 檢查是否有新的 CAN 訊息
    if (CAN1_Receive(&sRxMsg)) {
        // 處理接收到的訊息
        canDriver_processReceivedMessage(&sRxMsg);
    }
}

/**
 * @brief 取得 CAN 統計資訊
 * @param pu32RxCount 接收計數指標
 * @param pu32TxCount 傳送計數指標
 * @param pu32ErrorCount 錯誤計數指標
 * @return bool true=成功, false=失敗
 */
bool canIntegration_getStatistics(uint32_t* pu32RxCount, uint32_t* pu32TxCount, uint32_t* pu32ErrorCount)
{
    if (pu32RxCount == NULL || pu32TxCount == NULL || pu32ErrorCount == NULL) {
        return false;
    }
    
    *pu32RxCount = canDriver_getRxMessageCount();
    *pu32TxCount = canDriver_getTxMessageCount();
    *pu32ErrorCount = canDriver_getErrorCount();
    
    return true;
}

/**
 * @brief 重置 CAN 統計資訊
 * @return 無
 */
void canIntegration_resetStatistics(void)
{
    canDriver_resetStatistics();
}

// *****************************************************************************
// 主程式整合範例 (Main Program Integration Example)
// *****************************************************************************

/**
 * @brief 主程式 CAN 整合範例
 * @note 此函式展示如何在主程式中整合 CAN 功能
 * 
 * 使用方式：
 * 1. 在 main() 函式中呼叫 canIntegration_init()
 * 2. 在主迴圈中呼叫 canIntegration_processMessages()
 * 3. 可選擇性地呼叫統計資訊函式
 */
void canIntegration_mainExample(void)
{
    // 範例：在主程式中使用 CAN 整合模組
    
    // 1. 初始化 CAN 整合模組
    if (!canIntegration_init()) {
        // 處理初始化失敗
        return;
    }
    
    // 2. 主迴圈範例
    while (1) {
        // 處理 CAN 訊息
        canIntegration_processMessages();
        
        // 其他主迴圈邏輯...
        
        // 可選擇性地檢查統計資訊
        static uint32_t su32LastStatsCheck = 0;
        if (++su32LastStatsCheck >= 1000) {  // 每1000次迴圈檢查一次
            su32LastStatsCheck = 0;
            
            uint32_t u32RxCount, u32TxCount, u32ErrorCount;
            if (canIntegration_getStatistics(&u32RxCount, &u32TxCount, &u32ErrorCount)) {
                // 處理統計資訊 (例如：記錄到日誌、顯示等)
                // printf("CAN Stats: RX=%lu, TX=%lu, Errors=%lu\n", 
                //        u32RxCount, u32TxCount, u32ErrorCount);
            }
        }
    }
} 