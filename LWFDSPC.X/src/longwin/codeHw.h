#ifndef CODEHW_H
#define CODEHW_H

// 包含 userparms.h 以獲取 IO 接腳的別名定義
#include "../userparms.h"
// 包含 codeSw.h 以取得 CODESW_DEBUG_ISR_PROFILE_ENABLE 開關 (不受包含順序影響)
#include "codeSw.h"

// NOTE: userparms.h 已經包含了 <xc.h>，而 mcc_generated_files/pin_manager.h
// 中的定義 (如 _LATD3) 最終也來自 <xc.h>。
// 因此，直接包含 userparms.h 是最直接且符合專案結構的做法。

// =============================================================================
// == 硬體 IO 控制宏 (寫入) ==
// =============================================================================

/**
 * @brief 設定 RS485 為發送模式
 */
#define HW_RS485_SetTxMode()   (O_RS485_RE_LAT = 1)

/**
 * @brief 設定 RS485 為接收模式
 */
#define HW_RS485_SetRxMode()   (O_RS485_RE_LAT = 0)

/**
 * @brief 檢查 RS485 當前是否為發送模式
 */
#define HW_RS485_IsTxMode()    (O_RS485_RE_LAT == 1)

/**
 * @brief 檢查 RS485 當前是否為接收模式
 */
#define HW_RS485_IsRxMode()    (O_RS485_RE_LAT == 0)

/**
 * @brief 開啟頭燈
 */
#define HW_Headlight_On()      (O_HEAD_LIGHT_LAT = 1)

/**
 * @brief 關閉頭燈
 */
#define HW_Headlight_Off()     (O_HEAD_LIGHT_LAT = 0)

/**
 * @brief 開啟方向指示燈 (黃色 LED)
 */
#define HW_DirectionLed_On()   (O_LED_YELLOW_LAT = 1)

/**
 * @brief 關閉方向指示燈 (黃色 LED)
 */
#define HW_DirectionLed_Off()  (O_LED_YELLOW_LAT = 0)

/**
 * @brief 開啟煞車燈
 */
#define HW_BrakeLight_On()     (O_BRAKE_LIGHT_LAT = 1)

/**
 * @brief 關閉煞車燈
 */
#define HW_BrakeLight_Off()    (O_BRAKE_LIGHT_LAT = 0)

/**
 * @brief 開啟故障指示燈 (紅色 LED)
 */
#define HW_FaultLed_On()       (O_LED_RED_LAT = 1)

/**
 * @brief 關閉故障指示燈 (紅色 LED)
 */
#define HW_FaultLed_Off()      (O_LED_RED_LAT = 0)

/**
 * @brief 切換故障指示燈 (紅色 LED)
 */
#define HW_FaultLed_Toggle()   (O_LED_RED_LAT = !O_LED_RED_LAT)

/**
 * @brief 開啟運行指示燈 (綠色 LED)
 */
#define HW_RunLed_On()         (O_LED_GREEN_LAT = 1)

/**
 * @brief 關閉運行指示燈 (綠色 LED)
 */
#define HW_RunLed_Off()        (O_LED_GREEN_LAT = 0)

/**
 * @brief 切換運行指示燈 (綠色 LED)
 */
#define HW_RunLed_Toggle()     (O_LED_GREEN_LAT = !O_LED_GREEN_LAT)


// =============================================================================
// == 硬體 IO 讀取宏 (讀取) ==
// =============================================================================

/**
 * @brief 讀取 Hall U 感測器狀態
 */
#define HW_Hall_U_Read()       (I_HALL_U_PIN)

/**
 * @brief 讀取 Hall V 感測器狀態
 */
#define HW_Hall_V_Read()       (I_HALL_V_PIN)

/**
 * @brief 讀取 Hall W 感測器狀態
 */
#define HW_Hall_W_Read()       (I_HALL_W_PIN)

/**
 * @brief 檢查煞車訊號是否有效 (低電位有效)
 */
#define HW_BrakeSignal_IsActive()  (I_BRAKE_PIN == 0)


// =============================================================================
// == 除錯量測宏 (示波器量測執行時間) ==
// == RC13: ADC ISR 執行期間拉高；RD13: 速度命令處理期間拉高。               ==
// == 由 CODESW_DEBUG_ISR_PROFILE_ENABLE 控制；停用時展開為空，無任何開銷。   ==
// =============================================================================
#if CODESW_DEBUG_ISR_PROFILE_ENABLE == 1
#define HW_DbgAdcIsr_High()          (O_DBG_ADC_ISR_LAT = 1)
#define HW_DbgAdcIsr_Low()           (O_DBG_ADC_ISR_LAT = 0)
#define HW_DbgSpeedProfile_High()    (O_DBG_SPEED_PROFILE_LAT = 1)
#define HW_DbgSpeedProfile_Low()     (O_DBG_SPEED_PROFILE_LAT = 0)
#else
#define HW_DbgAdcIsr_High()
#define HW_DbgAdcIsr_Low()
#define HW_DbgSpeedProfile_High()
#define HW_DbgSpeedProfile_Low()
#endif


#endif // CODEHW_H 