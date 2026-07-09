/*******************************************************************************
  Longwin CAN 驅動模組標頭檔 (CAN Driver Header File)

  檔案名稱:
    s_can_driver.h

  摘要:
    此檔案包含Longwin控制器的CAN通訊驅動模組定義
    基於dsPIC33CK256MP506微控制器的CAN1模組

  描述:
    此檔案定義了CAN通訊的介面函式、資料結構和訊息處理邏輯
    包含電池電壓、再生煞車、輔助模式、馬達控制等CAN訊息處理

*******************************************************************************/
#ifndef _S_CAN_DRIVER_H
#define _S_CAN_DRIVER_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/can_types.h"
#include "mcc_generated_files/can1.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// CAN 訊息 ID 定義 (CAN Message ID Definitions)
// *****************************************************************************
#define CAN_MSG_ID_BATTERY_VOLTAGE     0x10    // 電池電壓查詢/回應
#define CAN_MSG_ID_REGEN_MODE          0x12    // 再生煞車模式查詢/回應
#define CAN_MSG_ID_ASSISTANCE_MODE     0x13    // 輔助模式查詢/回應
#define CAN_MSG_ID_DRIVE_TEMP          0x14    // 驅動器溫度查詢/回應
#define CAN_MSG_ID_MOTOR_RPM           0x15    // 馬達轉速查詢/回應
#define CAN_MSG_ID_DRIVE_STATUS        0x1f    // 驅動器狀態查詢/回應
#define CAN_MSG_ID_REGEN_SET           0x22    // 再生煞車模式設定
#define CAN_MSG_ID_DRIVE_ASSIST_SET    0x23    // 驅動/輔助模式設定
#define CAN_MSG_ID_MOTOR_ENABLE        0x24    // 馬達啟用/停用設定
#define CAN_MSG_ID_ERROR_RESPONSE      0x30    // 錯誤回應
#define CAN_MSG_ID_UNKNOWN_COMMAND     0x55    // 未知指令回應

// *****************************************************************************
// CAN 訊息資料結構 (CAN Message Data Structures)
// *****************************************************************************
/**
 * @brief CAN 電池電壓訊息資料結構
 */
typedef struct {
    uint16_t u16Voltage;           // 電池電壓值
    uint8_t u8Reserved[3];         // 保留位元組
} S_CAN_BATTERY_VOLTAGE_T;

/**
 * @brief CAN 再生煞車模式訊息資料結構
 */
typedef struct {
    uint8_t u8RegenMode;           // 再生煞車模式 (0-3)
    uint8_t u8Reserved[3];         // 保留位元組
} S_CAN_REGEN_MODE_T;

/**
 * @brief CAN 輔助模式訊息資料結構
 */
typedef struct {
    uint8_t u8AssistMode;          // 輔助模式
    uint8_t u8Reserved[3];         // 保留位元組
} S_CAN_ASSISTANCE_MODE_T;

/**
 * @brief CAN 驅動器溫度訊息資料結構
 */
typedef struct {
    uint16_t u16Temperature;       // 溫度值
    uint8_t u8Reserved[2];         // 保留位元組
} S_CAN_DRIVE_TEMP_T;

/**
 * @brief CAN 馬達轉速訊息資料結構
 */
typedef struct {
    uint16_t u16MotorRPM;          // 馬達轉速
    uint8_t u8Reserved[2];         // 保留位元組
} S_CAN_MOTOR_RPM_T;

/**
 * @brief CAN 驅動器狀態訊息資料結構
 */
typedef struct {
    uint8_t u8RunMotor;            // 馬達運行狀態
    uint8_t u8FaultFlags;          // 故障標誌
    uint8_t u8Reserved[2];         // 保留位元組
} S_CAN_DRIVE_STATUS_T;

/**
 * @brief CAN 馬達控制訊息資料結構
 */
typedef struct {
    uint8_t u8MotorEnable;         // 馬達啟用狀態
    uint8_t u8Reserved[3];         // 保留位元組
} S_CAN_MOTOR_ENABLE_T;

// *****************************************************************************
// CAN 驅動器狀態結構 (CAN Driver Status Structure)
// *****************************************************************************
/**
 * @brief CAN 驅動器狀態結構
 */
typedef struct {
    uint32_t u32RxMessageCount;    // 接收訊息計數
    uint32_t u32TxMessageCount;    // 傳送訊息計數
    uint32_t u32ErrorCount;        // 錯誤計數
    bool bInitialized;             // 初始化狀態
    bool bTransceiverEnabled;      // 收發器啟用狀態
} S_CAN_DRIVER_STATUS_T;

// *****************************************************************************
// 外部變數宣告 (External Variable Declarations)
// *****************************************************************************
extern uint8_t* CANRXPtr;          // CAN 接收資料指標
extern unsigned int CANCntr;       // CAN 計數器

// *****************************************************************************
// CAN 驅動器初始化函式 (CAN Driver Initialization Functions)
// *****************************************************************************

/**
 * @brief 初始化 CAN 驅動器模組
 * @return bool true=成功, false=失敗
 * @note 此函式會初始化 CAN1 模組和相關硬體設定
 * 
 * @example
 * // 初始化 CAN 驅動器
 * if (canDriver_init()) {
 *     // 初始化成功
 * } else {
 *     // 初始化失敗
 * }
 */
bool canDriver_init(void);

/**
 * @brief 啟用 CAN 收發器
 * @return 無
 * @note 設定收發器為正常模式，開始 CAN 通訊
 */
void canDriver_enableTransceiver(void);

/**
 * @brief 停用 CAN 收發器
 * @return 無
 * @note 設定收發器為待機模式，停止 CAN 通訊
 */
void canDriver_disableTransceiver(void);

// *****************************************************************************
// CAN 訊息處理函式 (CAN Message Processing Functions)
// *****************************************************************************

/**
 * @brief 處理接收到的 CAN 訊息
 * @param psRxMsg 指向接收訊息結構的指標
 * @return bool true=訊息已處理, false=訊息未處理
 * @note 此函式會根據訊息 ID 分發到對應的處理函式
 * 
 * @example
 * CAN_MSG_OBJ rxMsg;
 * if (CAN1_Receive(&rxMsg)) {
 *     canDriver_processReceivedMessage(&rxMsg);
 * }
 */
bool canDriver_processReceivedMessage(const CAN_MSG_OBJ* psRxMsg);

/**
 * @brief 傳送 CAN 回應訊息
 * @param u8CommandId 指令 ID
 * @param pu8Data 資料緩衝區指標
 * @param u8DataLength 資料長度
 * @return bool true=傳送成功, false=傳送失敗
 * @note 此函式會傳送標準的回應訊息格式
 */
bool canDriver_sendResponse(uint8_t u8CommandId, const uint8_t* pu8Data, uint8_t u8DataLength);

// *****************************************************************************
// CAN 訊息處理回調函式 (CAN Message Processing Callback Functions)
// *****************************************************************************

/**
 * @brief 電池電壓查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供電池電壓資料
 */
typedef bool (*CAN_BATTERY_VOLTAGE_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 再生煞車模式查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供再生煞車模式資料
 */
typedef bool (*CAN_REGEN_MODE_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 輔助模式查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供輔助模式資料
 */
typedef bool (*CAN_ASSISTANCE_MODE_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 驅動器溫度查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供驅動器溫度資料
 */
typedef bool (*CAN_DRIVE_TEMP_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 馬達轉速查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供馬達轉速資料
 */
typedef bool (*CAN_MOTOR_RPM_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 驅動器狀態查詢處理回調函式
 * @param pu8ResponseData 回應資料緩衝區
 * @param pu8DataLength 回應資料長度指標
 * @return bool true=處理成功, false=處理失敗
 * @note 此函式需要由外部實作，提供驅動器狀態資料
 */
typedef bool (*CAN_DRIVE_STATUS_CALLBACK_T)(uint8_t* pu8ResponseData, uint8_t* pu8DataLength);

/**
 * @brief 再生煞車模式設定處理回調函式
 * @param u8RegenMode 再生煞車模式值
 * @return bool true=設定成功, false=設定失敗
 * @note 此函式需要由外部實作，處理再生煞車模式設定
 */
typedef bool (*CAN_REGEN_SET_CALLBACK_T)(uint8_t u8RegenMode);

/**
 * @brief 驅動/輔助模式設定處理回調函式
 * @param u8AssistMode 輔助模式值
 * @return bool true=設定成功, false=設定失敗
 * @note 此函式需要由外部實作，處理輔助模式設定
 */
typedef bool (*CAN_ASSIST_SET_CALLBACK_T)(uint8_t u8AssistMode);

/**
 * @brief 馬達啟用/停用設定處理回調函式
 * @param u8MotorEnable 馬達啟用狀態
 * @return bool true=設定成功, false=設定失敗
 * @note 此函式需要由外部實作，處理馬達啟用/停用設定
 */
typedef bool (*CAN_MOTOR_ENABLE_CALLBACK_T)(uint8_t u8MotorEnable);

// *****************************************************************************
// CAN 回調函式註冊函式 (CAN Callback Registration Functions)
// *****************************************************************************

/**
 * @brief 註冊電池電壓查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerBatteryVoltageCallback(CAN_BATTERY_VOLTAGE_CALLBACK_T pfnCallback);

/**
 * @brief 註冊再生煞車模式查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerRegenModeCallback(CAN_REGEN_MODE_CALLBACK_T pfnCallback);

/**
 * @brief 註冊輔助模式查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerAssistanceModeCallback(CAN_ASSISTANCE_MODE_CALLBACK_T pfnCallback);

/**
 * @brief 註冊驅動器溫度查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerDriveTempCallback(CAN_DRIVE_TEMP_CALLBACK_T pfnCallback);

/**
 * @brief 註冊馬達轉速查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerMotorRpmCallback(CAN_MOTOR_RPM_CALLBACK_T pfnCallback);

/**
 * @brief 註冊驅動器狀態查詢處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerDriveStatusCallback(CAN_DRIVE_STATUS_CALLBACK_T pfnCallback);

/**
 * @brief 註冊再生煞車模式設定處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerRegenSetCallback(CAN_REGEN_SET_CALLBACK_T pfnCallback);

/**
 * @brief 註冊驅動/輔助模式設定處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerAssistSetCallback(CAN_ASSIST_SET_CALLBACK_T pfnCallback);

/**
 * @brief 註冊馬達啟用/停用設定處理回調函式
 * @param pfnCallback 回調函式指標
 * @return 無
 */
void canDriver_registerMotorEnableCallback(CAN_MOTOR_ENABLE_CALLBACK_T pfnCallback);

// *****************************************************************************
// CAN 狀態查詢函式 (CAN Status Query Functions)
// *****************************************************************************

/**
 * @brief 取得 CAN 驅動器狀態
 * @param psStatus 指向狀態結構的指標
 * @return bool true=成功, false=失敗
 * @note 此函式會回傳 CAN 驅動器的當前狀態資訊
 */
bool canDriver_getStatus(S_CAN_DRIVER_STATUS_T* psStatus);

/**
 * @brief 檢查 CAN 驅動器是否已初始化
 * @return bool true=已初始化, false=未初始化
 */
bool canDriver_isInitialized(void);

/**
 * @brief 檢查 CAN 收發器是否已啟用
 * @return bool true=已啟用, false=未啟用
 */
bool canDriver_isTransceiverEnabled(void);

// *****************************************************************************
// CAN 統計資訊函式 (CAN Statistics Functions)
// *****************************************************************************

/**
 * @brief 重置 CAN 統計資訊
 * @return 無
 * @note 此函式會重置所有計數器
 */
void canDriver_resetStatistics(void);

/**
 * @brief 取得接收訊息計數
 * @return uint32_t 接收訊息計數
 */
uint32_t canDriver_getRxMessageCount(void);

/**
 * @brief 取得傳送訊息計數
 * @return uint32_t 傳送訊息計數
 */
uint32_t canDriver_getTxMessageCount(void);

/**
 * @brief 取得錯誤計數
 * @return uint32_t 錯誤計數
 */
uint32_t canDriver_getErrorCount(void);

#ifdef __cplusplus
}
#endif

#endif // _S_CAN_DRIVER_H 