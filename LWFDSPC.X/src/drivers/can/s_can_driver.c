/*******************************************************************************
  Longwin CAN 驅動模組實作檔 (CAN Driver Implementation File)

  檔案名稱:
    s_can_driver.c

  摘要:
    此檔案包含Longwin控制器的CAN通訊驅動模組實作
    基於dsPIC33CK256MP506微控制器的CAN1模組

  描述:
    此檔案實作了CAN通訊的介面函式、訊息處理邏輯和回調機制
    包含電池電壓、再生煞車、輔助模式、馬達控制等CAN訊息處理

*******************************************************************************/
#include "s_can_driver.h"
#include "../../longwin/devices/s_pin_definitions.h"

// *****************************************************************************
// 靜態變數 (Static Variables)
// *****************************************************************************
static S_CAN_DRIVER_STATUS_T sstDriverStatus = {0};
static uint8_t su8TxBuffer[8] = {0};  // 傳送緩衝區

// 回調函式指標 (Callback Function Pointers)
static CAN_BATTERY_VOLTAGE_CALLBACK_T spfnBatteryVoltageCallback = NULL;
static CAN_REGEN_MODE_CALLBACK_T spfnRegenModeCallback = NULL;
static CAN_ASSISTANCE_MODE_CALLBACK_T spfnAssistanceModeCallback = NULL;
static CAN_DRIVE_TEMP_CALLBACK_T spfnDriveTempCallback = NULL;
static CAN_MOTOR_RPM_CALLBACK_T spfnMotorRpmCallback = NULL;
static CAN_DRIVE_STATUS_CALLBACK_T spfnDriveStatusCallback = NULL;
static CAN_REGEN_SET_CALLBACK_T spfnRegenSetCallback = NULL;
static CAN_ASSIST_SET_CALLBACK_T spfnAssistSetCallback = NULL;
static CAN_MOTOR_ENABLE_CALLBACK_T spfnMotorEnableCallback = NULL;

// *****************************************************************************
// 內部函式宣告 (Internal Function Declarations)
// *****************************************************************************
static bool _canDriver_handleBatteryVoltageQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleRegenModeQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleAssistanceModeQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleDriveTempQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleMotorRpmQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleDriveStatusQuery(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleRegenModeSet(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleAssistModeSet(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleMotorEnableSet(const CAN_MSG_OBJ* psRxMsg);
static bool _canDriver_handleUnknownCommand(const CAN_MSG_OBJ* psRxMsg);

// *****************************************************************************
// CAN 驅動器初始化函式 (CAN Driver Initialization Functions)
// *****************************************************************************

/**
 * @brief 初始化 CAN 驅動器模組
 * @return bool true=成功, false=失敗
 */
bool canDriver_init(void)
{
    // 重置狀態結構
    memset(&sstDriverStatus, 0, sizeof(S_CAN_DRIVER_STATUS_T));
    
    // 初始化 CAN1 模組 (使用 MCC 生成的函式)
    CAN1_Initialize();
    
    // 設定收發器為正常模式
    pinDef_setCANMode(true);
    
    // 更新狀態
    sstDriverStatus.bInitialized = true;
    sstDriverStatus.bTransceiverEnabled = true;
    
    return true;
}

/**
 * @brief 啟用 CAN 收發器
 * @return 無
 */
void canDriver_enableTransceiver(void)
{
    pinDef_setCANMode(true);
    sstDriverStatus.bTransceiverEnabled = true;
}

/**
 * @brief 停用 CAN 收發器
 * @return 無
 */
void canDriver_disableTransceiver(void)
{
    pinDef_setCANMode(false);
    sstDriverStatus.bTransceiverEnabled = false;
}

// *****************************************************************************
// CAN 訊息處理函式 (CAN Message Processing Functions)
// *****************************************************************************

/**
 * @brief 處理接收到的 CAN 訊息
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=訊息已處理, false=訊息未處理
 * @note 對應 HallFOC_Regen.c 中的 CAN 訊息處理邏輯：
 *       if (CAN1_Receive(&My_CAN_RXMSG) == true) { ... }
 */
bool canDriver_processReceivedMessage(const CAN_MSG_OBJ* psRxMsg)
{
    bool bProcessed = false;
    
    // 參數檢查
    if (psRxMsg == NULL || !sstDriverStatus.bInitialized) {
        return false;
    }
    
    // =========================================================================
    // 對應原始程式碼：
    // CANRXCntr++;
    // My_CAN_TXMSG.data = CANTXBuffer;
    // My_CAN_TXMSG.msgId = 0x200;
    // My_CAN_TXMSG.field.dlc = 5;
    // =========================================================================
    sstDriverStatus.u32RxMessageCount++;  // 對應 CANRXCntr++
    
    // 準備回應訊息結構 (對應 My_CAN_TXMSG 設定)
    // 注意：實際的訊息傳送在各個處理函式中進行
    
    // =========================================================================
    // 對應原始程式碼的 switch 邏輯：
    // if (*CANRXPtr == 0x10) // Battery voltage
    // else if (*CANRXPtr == 0x12) // REGEN Mode
    // else if (*CANRXPtr == 0x13) // Assistance mode
    // else if (*CANRXPtr == 0x14) // Drive Temp
    // else if (*CANRXPtr == 0x15) // motor RPM
    // else if (*CANRXPtr == 0x1f) // Drive status
    // else if (*CANRXPtr == 0x22) // ReGen mode
    // else if (*CANRXPtr == 0x23) // Drive/Assist mode
    // else if (*CANRXPtr == 0x24) // Motor drive enable/disable
    // else if (*CANRXPtr == 0x30) // None or Error
    // else { CANTXBuffer[0] = 0x55; }
    // =========================================================================
    
    // 根據訊息 ID 分發處理
    switch (psRxMsg->msgId) {
        case CAN_MSG_ID_BATTERY_VOLTAGE:
            bProcessed = _canDriver_handleBatteryVoltageQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_REGEN_MODE:
            bProcessed = _canDriver_handleRegenModeQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_ASSISTANCE_MODE:
            bProcessed = _canDriver_handleAssistanceModeQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_DRIVE_TEMP:
            bProcessed = _canDriver_handleDriveTempQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_MOTOR_RPM:
            bProcessed = _canDriver_handleMotorRpmQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_DRIVE_STATUS:
            bProcessed = _canDriver_handleDriveStatusQuery(psRxMsg);
            break;
            
        case CAN_MSG_ID_REGEN_SET:
            bProcessed = _canDriver_handleRegenModeSet(psRxMsg);
            break;
            
        case CAN_MSG_ID_DRIVE_ASSIST_SET:
            bProcessed = _canDriver_handleAssistModeSet(psRxMsg);
            break;
            
        case CAN_MSG_ID_MOTOR_ENABLE:
            bProcessed = _canDriver_handleMotorEnableSet(psRxMsg);
            break;
            
        default:
            bProcessed = _canDriver_handleUnknownCommand(psRxMsg);
            break;
    }
    
    // 如果處理失敗，增加錯誤計數
    if (!bProcessed) {
        sstDriverStatus.u32ErrorCount++;
    }
    
    return bProcessed;
}

/**
 * @brief 傳送 CAN 回應訊息
 * @param u8CommandId 指令 ID
 * @param pu8Data 資料緩衝區指標
 * @param u8DataLength 資料長度
 * @return bool true=傳送成功, false=傳送失敗
 */
bool canDriver_sendResponse(uint8_t u8CommandId, const uint8_t* pu8Data, uint8_t u8DataLength)
{
    CAN_MSG_OBJ sTxMsg;
    CAN_TX_MSG_REQUEST_STATUS eTxStatus;
    
    // 參數檢查
    if (pu8Data == NULL || u8DataLength > 8 || !sstDriverStatus.bInitialized) {
        return false;
    }
    
    // 準備傳送訊息
    sTxMsg.msgId = 0x200;  // 回應 ID
    sTxMsg.field.dlc = 5;  // 資料長度
    sTxMsg.data = su8TxBuffer;
    
    // 設定回應資料
    su8TxBuffer[0] = u8CommandId;
    for (uint8_t i = 0; i < u8DataLength && i < 4; i++) {
        su8TxBuffer[i + 1] = pu8Data[i];
    }
    
    // 傳送訊息
    eTxStatus = CAN1_Transmit(CAN1_FIFO_CH1, &sTxMsg);
    
    if (eTxStatus == CAN_TX_MSG_REQUEST_SUCCESS) {
        sstDriverStatus.u32TxMessageCount++;
        return true;
    } else {
        sstDriverStatus.u32ErrorCount++;
        return false;
    }
}

// *****************************************************************************
// 內部訊息處理函式 (Internal Message Processing Functions)
// *****************************************************************************

/**
 * @brief 處理電池電壓查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的電池電壓查詢處理邏輯：
 *       if (*CANRXPtr == 0x10) // Battery voltage
 *       {
 *           CANTXBuffer[0] = 0x10;
 *           // TempVar = FracMpy (faultOvervoltage.monitor+1, Q15(0.051625));
 *           TempVar = faultOvervoltage.monitor + 1;
 *           CANTXBuffer[4] = TempVar;
 *           CANTXBuffer[3] = TempVar >> 8;
 *           CANTXBuffer[2] = 0;
 *           CANTXBuffer[1] = 0;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleBatteryVoltageQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x10;
    // TempVar = faultOvervoltage.monitor + 1;
    // CANTXBuffer[4] = TempVar;
    // CANTXBuffer[3] = TempVar >> 8;
    // CANTXBuffer[2] = 0;
    // CANTXBuffer[1] = 0;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint16_t u16BatteryVoltage = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0;  // 對應 CANTXBuffer[1] = 0
    u8ResponseData[1] = 0;  // 對應 CANTXBuffer[2] = 0
    
    // 取得電池電壓值 (對應 faultOvervoltage.monitor + 1)
    if (spfnBatteryVoltageCallback != NULL) {
        u16BatteryVoltage = spfnBatteryVoltageCallback();
    } else {
        // 如果沒有回調函式，使用預設值
        u16BatteryVoltage = 1;  // 對應 faultOvervoltage.monitor + 1
    }
    
    // 設定電壓值 (對應 TempVar = faultOvervoltage.monitor + 1)
    u8ResponseData[2] = (uint8_t)(u16BatteryVoltage >> 8);  // 對應 CANTXBuffer[3] = TempVar >> 8
    u8ResponseData[3] = (uint8_t)(u16BatteryVoltage & 0xFF); // 對應 CANTXBuffer[4] = TempVar
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x10, u8ResponseData, 4);
}

/**
 * @brief 處理再生模式查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的再生模式查詢處理邏輯：
 *       else if (*CANRXPtr == 0x12) // REGEN Mode
 *       {
 *           CANTXBuffer[0] = 0x12;
 *           CANTXBuffer[4] = uGF.ReGenMode;
 *           CANTXBuffer[3] = 0xFF;
 *           CANTXBuffer[2] = 0xFF;
 *           CANTXBuffer[1] = 0xFF;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleRegenModeQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x12;
    // CANTXBuffer[4] = uGF.ReGenMode;
    // CANTXBuffer[3] = 0xFF;
    // CANTXBuffer[2] = 0xFF;
    // CANTXBuffer[1] = 0xFF;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8RegenMode = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0xFF;  // 對應 CANTXBuffer[1] = 0xFF
    u8ResponseData[1] = 0xFF;  // 對應 CANTXBuffer[2] = 0xFF
    u8ResponseData[2] = 0xFF;  // 對應 CANTXBuffer[3] = 0xFF
    
    // 取得再生模式值 (對應 uGF.ReGenMode)
    if (spfnRegenModeCallback != NULL) {
        u8RegenMode = spfnRegenModeCallback();
    } else {
        // 如果沒有回調函式，使用預設值
        u8RegenMode = 0;  // 對應 uGF.ReGenMode 預設值
    }
    
    // 設定再生模式值 (對應 CANTXBuffer[4] = uGF.ReGenMode)
    u8ResponseData[3] = u8RegenMode;
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x12, u8ResponseData, 4);
}

/**
 * @brief 處理輔助模式查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的輔助模式查詢處理邏輯：
 *       else if (*CANRXPtr == 0x13) // Assistance mode
 *       {
 *           CANTXBuffer[0] = 0x13;
 *           CANTXBuffer[4] = uGF.DriveMode;
 *           CANTXBuffer[3] = 0xFF;
 *           CANTXBuffer[2] = 0xFF;
 *           CANTXBuffer[1] = 0xFF;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleAssistanceModeQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x13;
    // CANTXBuffer[4] = uGF.DriveMode;
    // CANTXBuffer[3] = 0xFF;
    // CANTXBuffer[2] = 0xFF;
    // CANTXBuffer[1] = 0xFF;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8DriveMode = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0xFF;  // 對應 CANTXBuffer[1] = 0xFF
    u8ResponseData[1] = 0xFF;  // 對應 CANTXBuffer[2] = 0xFF
    u8ResponseData[2] = 0xFF;  // 對應 CANTXBuffer[3] = 0xFF
    
    // 取得驅動模式值 (對應 uGF.DriveMode)
    if (spfnAssistanceModeCallback != NULL) {
        u8DriveMode = spfnAssistanceModeCallback();
    } else {
        // 如果沒有回調函式，使用預設值
        u8DriveMode = 0;  // 對應 uGF.DriveMode 預設值
    }
    
    // 設定驅動模式值 (對應 CANTXBuffer[4] = uGF.DriveMode)
    u8ResponseData[3] = u8DriveMode;
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x13, u8ResponseData, 4);
}

/**
 * @brief 處理驅動溫度查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的驅動溫度查詢處理邏輯：
 *       else if (*CANRXPtr == 0x14) // Drive Temp
 *       {
 *           CANTXBuffer[0] = 0x14;
 *           CANTXBuffer[4] = faultOverTempMOSFET.monitor;
 *           CANTXBuffer[3] = faultOverTempMOSFET.monitor >> 8;
 *           CANTXBuffer[2] = 0;
 *           CANTXBuffer[1] = 0;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleDriveTempQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x14;
    // CANTXBuffer[4] = faultOverTempMOSFET.monitor;
    // CANTXBuffer[3] = faultOverTempMOSFET.monitor >> 8;
    // CANTXBuffer[2] = 0;
    // CANTXBuffer[1] = 0;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint16_t u16DriveTemp = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0;  // 對應 CANTXBuffer[1] = 0
    u8ResponseData[1] = 0;  // 對應 CANTXBuffer[2] = 0
    
    // 取得驅動溫度值 (對應 faultOverTempMOSFET.monitor)
    if (spfnDriveTempCallback != NULL) {
        u16DriveTemp = spfnDriveTempCallback();
    } else {
        // 如果沒有回調函式，使用預設值
        u16DriveTemp = 0;  // 對應 faultOverTempMOSFET.monitor 預設值
    }
    
    // 設定溫度值 (對應 faultOverTempMOSFET.monitor)
    u8ResponseData[2] = (uint8_t)(u16DriveTemp >> 8);  // 對應 CANTXBuffer[3] = faultOverTempMOSFET.monitor >> 8
    u8ResponseData[3] = (uint8_t)(u16DriveTemp & 0xFF); // 對應 CANTXBuffer[4] = faultOverTempMOSFET.monitor
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x14, u8ResponseData, 4);
}

/**
 * @brief 處理馬達轉速查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的馬達轉速查詢處理邏輯：
 *       else if (*CANRXPtr == 0x15) // motor RPM
 *       {
 *           CANTXBuffer[0] = 0x15;
 *           // TempVar = FracMpy (FilteredSpeed, Q15(0.30517));
 *           // TempVar = FracMpy(HallPulsesLatch, Q15(0.83333));
 *           TempVar = HallPulsesLatch * 10;
 *           CANTXBuffer[4] = TempVar;
 *           CANTXBuffer[3] = TempVar >> 8;
 *           CANTXBuffer[2] = 0;
 *           CANTXBuffer[1] = 0;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleMotorRpmQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x15;
    // // TempVar = FracMpy (FilteredSpeed, Q15(0.30517));
    // // TempVar = FracMpy(HallPulsesLatch, Q15(0.83333));
    // TempVar = HallPulsesLatch * 10;
    // CANTXBuffer[4] = TempVar;
    // CANTXBuffer[3] = TempVar >> 8;
    // CANTXBuffer[2] = 0;
    // CANTXBuffer[1] = 0;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint16_t u16MotorRpm = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0;  // 對應 CANTXBuffer[1] = 0
    u8ResponseData[1] = 0;  // 對應 CANTXBuffer[2] = 0
    
    // 取得馬達轉速值 (對應 HallPulsesLatch * 10)
    if (spfnMotorRpmCallback != NULL) {
        u16MotorRpm = spfnMotorRpmCallback();
    } else {
        // 如果沒有回調函式，使用預設值
        u16MotorRpm = 0;  // 對應 HallPulsesLatch * 10 預設值
    }
    
    // 設定轉速值 (對應 TempVar = HallPulsesLatch * 10)
    u8ResponseData[2] = (uint8_t)(u16MotorRpm >> 8);  // 對應 CANTXBuffer[3] = TempVar >> 8
    u8ResponseData[3] = (uint8_t)(u16MotorRpm & 0xFF); // 對應 CANTXBuffer[4] = TempVar
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x15, u8ResponseData, 4);
}

/**
 * @brief 處理驅動狀態查詢
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的驅動狀態查詢處理邏輯：
 *       else if (*CANRXPtr == 0x1f) // Drive status
 *       {
 *           CANTXBuffer[0] = 0x1f;
 *           CANTXBuffer[4] = FaultFlags.MOSOverHeat +
 *                            (FaultFlags.Overvoltage << 1) +
 *                            (FaultFlags.Undervoltage << 2);
 *           CANTXBuffer[3] = 0;
 *           CANTXBuffer[2] = 0;
 *           CANTXBuffer[1] = uGF.RunMotor;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleDriveStatusQuery(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x1f;
    // CANTXBuffer[4] = FaultFlags.MOSOverHeat +
    //                  (FaultFlags.Overvoltage << 1) +
    //                  (FaultFlags.Undervoltage << 2);
    // CANTXBuffer[3] = 0;
    // CANTXBuffer[2] = 0;
    // CANTXBuffer[1] = uGF.RunMotor;
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8RunMotor = 0;
    uint8_t u8FaultStatus = 0;
    
    // 準備回應資料
    u8ResponseData[0] = 0;  // 對應 CANTXBuffer[2] = 0
    u8ResponseData[1] = 0;  // 對應 CANTXBuffer[3] = 0
    
    // 取得驅動狀態值 (對應 uGF.RunMotor 和 FaultFlags)
    if (spfnDriveStatusCallback != NULL) {
        S_DRIVE_STATUS_T sDriveStatus = {0};
        if (spfnDriveStatusCallback(&sDriveStatus)) {
            u8RunMotor = sDriveStatus.bRunMotor;  // 對應 uGF.RunMotor
            u8FaultStatus = sDriveStatus.u8FaultStatus;  // 對應 FaultFlags 組合
        }
    } else {
        // 如果沒有回調函式，使用預設值
        u8RunMotor = 0;  // 對應 uGF.RunMotor 預設值
        u8FaultStatus = 0;  // 對應 FaultFlags 預設值
    }
    
    // 設定狀態值
    u8ResponseData[0] = u8RunMotor;  // 對應 CANTXBuffer[1] = uGF.RunMotor
    u8ResponseData[3] = u8FaultStatus;  // 對應 CANTXBuffer[4] = FaultFlags 組合
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x1f, u8ResponseData, 4);
}

/**
 * @brief 處理再生煞車模式設定
 * @param psRxMsg 接收訊息指標
 * @return bool true=處理成功, false=處理失敗
 */
static bool _canDriver_handleRegenModeSet(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x22;
    // if (*(CANRXPtr + 4) == 0) { uGF.ReGenSet = 0; uGF.ReGenMode = 0; ReGenTorq = 0; }
    // else if (*(CANRXPtr + 4) == 1) { uGF.ReGenSet = 1; uGF.ReGenMode = 1; ReGenTorq = 2000; }
    // else if (*(CANRXPtr + 4) == 2) { uGF.ReGenSet = 1; uGF.ReGenMode = 2; ReGenTorq = 6553; }
    // else if (*(CANRXPtr + 4) == 3) { uGF.ReGenSet = 1; uGF.ReGenMode = 3; ReGenTorq = 9000; }
    // CANTXBuffer[4] = *(CANRXPtr + 4);
    // CANTXBuffer[3] = *(CANRXPtr + 3);
    // CANTXBuffer[2] = *(CANRXPtr + 2);
    // CANTXBuffer[1] = *(CANRXPtr + 1);
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8RegenMode = 0;
    bool bSuccess = false;
    
    // 檢查資料長度
    if (psRxMsg->field.dlc < 5) {
        return false;
    }
    
    // 取得再生模式值 (對應 *(CANRXPtr + 4))
    u8RegenMode = psRxMsg->data[4];
    
    // 處理再生模式設定 (對應原始程式碼的 if-else 邏輯)
    if (spfnRegenSetCallback != NULL) {
        bSuccess = spfnRegenSetCallback(u8RegenMode);
    } else {
        // 如果沒有回調函式，預設成功
        bSuccess = true;
    }
    
    // 準備回應資料 (對應原始程式碼的 CANTXBuffer 設定)
    u8ResponseData[0] = psRxMsg->data[1];  // 對應 CANTXBuffer[1] = *(CANRXPtr + 1)
    u8ResponseData[1] = psRxMsg->data[2];  // 對應 CANTXBuffer[2] = *(CANRXPtr + 2)
    u8ResponseData[2] = psRxMsg->data[3];  // 對應 CANTXBuffer[3] = *(CANRXPtr + 3)
    u8ResponseData[3] = u8RegenMode;       // 對應 CANTXBuffer[4] = *(CANRXPtr + 4)
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x22, u8ResponseData, 4);
}

/**
 * @brief 處理輔助模式設定
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的輔助模式設定處理邏輯：
 *       else if (*CANRXPtr == 0x23) // Drive/Assist mode
 *       {
 *           CANTXBuffer[0] = 0x23;
 *           uGF.DriveMode = *(CANRXPtr + 4);
 *           CANTXBuffer[4] = *(CANRXPtr + 4);
 *           CANTXBuffer[3] = *(CANRXPtr + 3);
 *           CANTXBuffer[2] = *(CANRXPtr + 2);
 *           CANTXBuffer[1] = *(CANRXPtr + 1);
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleAssistModeSet(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x23;
    // uGF.DriveMode = *(CANRXPtr + 4);
    // CANTXBuffer[4] = *(CANRXPtr + 4);
    // CANTXBuffer[3] = *(CANRXPtr + 3);
    // CANTXBuffer[2] = *(CANRXPtr + 2);
    // CANTXBuffer[1] = *(CANRXPtr + 1);
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8DriveMode = 0;
    bool bSuccess = false;
    
    // 檢查資料長度
    if (psRxMsg->field.dlc < 5) {
        return false;
    }
    
    // 取得驅動模式值 (對應 *(CANRXPtr + 4))
    u8DriveMode = psRxMsg->data[4];
    
    // 處理驅動模式設定 (對應 uGF.DriveMode = *(CANRXPtr + 4))
    if (spfnAssistSetCallback != NULL) {
        bSuccess = spfnAssistSetCallback(u8DriveMode);
    } else {
        // 如果沒有回調函式，預設成功
        bSuccess = true;
    }
    
    // 準備回應資料 (對應原始程式碼的 CANTXBuffer 設定)
    u8ResponseData[0] = psRxMsg->data[1];  // 對應 CANTXBuffer[1] = *(CANRXPtr + 1)
    u8ResponseData[1] = psRxMsg->data[2];  // 對應 CANTXBuffer[2] = *(CANRXPtr + 2)
    u8ResponseData[2] = psRxMsg->data[3];  // 對應 CANTXBuffer[3] = *(CANRXPtr + 3)
    u8ResponseData[3] = u8DriveMode;       // 對應 CANTXBuffer[4] = *(CANRXPtr + 4)
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x23, u8ResponseData, 4);
}

/**
 * @brief 處理馬達啟用設定
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的馬達啟用設定處理邏輯：
 *       else if (*CANRXPtr == 0x24) // Motor drive enable/disable
 *       {
 *           CANTXBuffer[0] = 0x24;
 *           uGF.CAN_Runmotor = *(CANRXPtr + 4);
 *           CANTXBuffer[4] = *(CANRXPtr + 4);
 *           CANTXBuffer[3] = *(CANRXPtr + 3);
 *           CANTXBuffer[2] = *(CANRXPtr + 2);
 *           CANTXBuffer[1] = *(CANRXPtr + 1);
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleMotorEnableSet(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // CANTXBuffer[0] = 0x24;
    // uGF.CAN_Runmotor = *(CANRXPtr + 4);
    // CANTXBuffer[4] = *(CANRXPtr + 4);
    // CANTXBuffer[3] = *(CANRXPtr + 3);
    // CANTXBuffer[2] = *(CANRXPtr + 2);
    // CANTXBuffer[1] = *(CANRXPtr + 1);
    // CAN1_Transmit(1, &My_CAN_TXMSG);
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8MotorEnable = 0;
    bool bSuccess = false;
    
    // 檢查資料長度
    if (psRxMsg->field.dlc < 5) {
        return false;
    }
    
    // 取得馬達啟用值 (對應 *(CANRXPtr + 4))
    u8MotorEnable = psRxMsg->data[4];
    
    // 處理馬達啟用設定 (對應 uGF.CAN_Runmotor = *(CANRXPtr + 4))
    if (spfnMotorEnableCallback != NULL) {
        bSuccess = spfnMotorEnableCallback(u8MotorEnable);
    } else {
        // 如果沒有回調函式，預設成功
        bSuccess = true;
    }
    
    // 準備回應資料 (對應原始程式碼的 CANTXBuffer 設定)
    u8ResponseData[0] = psRxMsg->data[1];  // 對應 CANTXBuffer[1] = *(CANRXPtr + 1)
    u8ResponseData[1] = psRxMsg->data[2];  // 對應 CANTXBuffer[2] = *(CANRXPtr + 2)
    u8ResponseData[2] = psRxMsg->data[3];  // 對應 CANTXBuffer[3] = *(CANRXPtr + 3)
    u8ResponseData[3] = u8MotorEnable;     // 對應 CANTXBuffer[4] = *(CANRXPtr + 4)
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(0x24, u8ResponseData, 4);
}

/**
 * @brief 處理未知指令
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=處理成功, false=處理失敗
 * @note 對應 HallFOC_Regen.c 中的未知指令處理邏輯：
 *       else if (*CANRXPtr == 0x30) // None or Error
 *       {
 *           CANTXBuffer[0] = 0x30;
 *           CANTXBuffer[4] = 1;
 *           CANTXBuffer[3] = 0;
 *           CANTXBuffer[2] = 0;
 *           CANTXBuffer[1] = 0;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 *       else
 *       {
 *           CANTXBuffer[0] = 0x55;
 *           CAN1_Transmit(1, &My_CAN_TXMSG);
 *       }
 */
static bool _canDriver_handleUnknownCommand(const CAN_MSG_OBJ* psRxMsg)
{
    // =========================================================================
    // 對應原始程式碼：
    // else if (*CANRXPtr == 0x30) // None or Error
    // {
    //     CANTXBuffer[0] = 0x30;
    //     CANTXBuffer[4] = 1;
    //     CANTXBuffer[3] = 0;
    //     CANTXBuffer[2] = 0;
    //     CANTXBuffer[1] = 0;
    //     CAN1_Transmit(1, &My_CAN_TXMSG);
    // }
    // else
    // {
    //     CANTXBuffer[0] = 0x55;
    //     CAN1_Transmit(1, &My_CAN_TXMSG);
    // }
    // =========================================================================
    
    uint8_t u8ResponseData[4] = {0};
    uint8_t u8CommandId = 0;
    
    // 檢查是否為錯誤指令 (對應 0x30 處理)
    if (psRxMsg->data[0] == 0x30) {
        u8CommandId = 0x30;
        // 設定錯誤回應資料 (對應 CANTXBuffer[1-4] 設定)
        u8ResponseData[0] = 0;  // 對應 CANTXBuffer[1] = 0
        u8ResponseData[1] = 0;  // 對應 CANTXBuffer[2] = 0
        u8ResponseData[2] = 0;  // 對應 CANTXBuffer[3] = 0
        u8ResponseData[3] = 1;  // 對應 CANTXBuffer[4] = 1
    } else {
        // 其他未知指令 (對應 else 分支)
        u8CommandId = 0x55;
        // 不需要額外資料，只傳送指令 ID
    }
    
    // 傳送回應 (對應 CAN1_Transmit(1, &My_CAN_TXMSG))
    return canDriver_sendResponse(u8CommandId, u8ResponseData, 4);
}

// *****************************************************************************
// CAN 回調函式註冊函式 (CAN Callback Registration Functions)
// *****************************************************************************

void canDriver_registerBatteryVoltageCallback(CAN_BATTERY_VOLTAGE_CALLBACK_T pfnCallback)
{
    spfnBatteryVoltageCallback = pfnCallback;
}

void canDriver_registerRegenModeCallback(CAN_REGEN_MODE_CALLBACK_T pfnCallback)
{
    spfnRegenModeCallback = pfnCallback;
}

void canDriver_registerAssistanceModeCallback(CAN_ASSISTANCE_MODE_CALLBACK_T pfnCallback)
{
    spfnAssistanceModeCallback = pfnCallback;
}

void canDriver_registerDriveTempCallback(CAN_DRIVE_TEMP_CALLBACK_T pfnCallback)
{
    spfnDriveTempCallback = pfnCallback;
}

void canDriver_registerMotorRpmCallback(CAN_MOTOR_RPM_CALLBACK_T pfnCallback)
{
    spfnMotorRpmCallback = pfnCallback;
}

void canDriver_registerDriveStatusCallback(CAN_DRIVE_STATUS_CALLBACK_T pfnCallback)
{
    spfnDriveStatusCallback = pfnCallback;
}

void canDriver_registerRegenSetCallback(CAN_REGEN_SET_CALLBACK_T pfnCallback)
{
    spfnRegenSetCallback = pfnCallback;
}

void canDriver_registerAssistSetCallback(CAN_ASSIST_SET_CALLBACK_T pfnCallback)
{
    spfnAssistSetCallback = pfnCallback;
}

void canDriver_registerMotorEnableCallback(CAN_MOTOR_ENABLE_CALLBACK_T pfnCallback)
{
    spfnMotorEnableCallback = pfnCallback;
}

// *****************************************************************************
// CAN 狀態查詢函式 (CAN Status Query Functions)
// *****************************************************************************

bool canDriver_getStatus(S_CAN_DRIVER_STATUS_T* psStatus)
{
    if (psStatus == NULL) {
        return false;
    }
    
    *psStatus = sstDriverStatus;
    return true;
}

bool canDriver_isInitialized(void)
{
    return sstDriverStatus.bInitialized;
}

bool canDriver_isTransceiverEnabled(void)
{
    return sstDriverStatus.bTransceiverEnabled;
}

// *****************************************************************************
// CAN 統計資訊函式 (CAN Statistics Functions)
// *****************************************************************************

void canDriver_resetStatistics(void)
{
    sstDriverStatus.u32RxMessageCount = 0;
    sstDriverStatus.u32TxMessageCount = 0;
    sstDriverStatus.u32ErrorCount = 0;
}

uint32_t canDriver_getRxMessageCount(void)
{
    return sstDriverStatus.u32RxMessageCount;
}

uint32_t canDriver_getTxMessageCount(void)
{
    return sstDriverStatus.u32TxMessageCount;
}

uint32_t canDriver_getErrorCount(void)
{
    return sstDriverStatus.u32ErrorCount;
} 