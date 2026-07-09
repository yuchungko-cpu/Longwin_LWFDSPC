#ifndef S_LOGIC_VR_H
#define S_LOGIC_VR_H

#include <stdint.h>
#include <stdbool.h>
#include "s_logic_motor.h"  // 包含 S_MOTOR_STEP_TIME_T 結構體定義

// --- ADC 與分壓電路相關配置 ---
#define LOGIC_VR_ADC_RESOLUTION_BITS 12
#define LOGIC_VR_VREF_MV 3300
#define LOGIC_VR_R1_OHMS 51000UL
#define LOGIC_VR_R2_OHMS 100000UL

// --- VR 配置常數 (根據文檔要求) ---

// 開機檢測電壓 (mV)
#define LOGIC_VR_POWER_ON_CHECK_MIN_MV 2300u
#define LOGIC_VR_POWER_ON_CHECK_MAX_MV 2900u

// VR 零點和死區電壓 (mV)
#define LOGIC_VR_DEADZONE_LOW_MV 2200u
#define LOGIC_VR_DEADZONE_HIGH_MV 2800u

// 反轉工作區電壓和帶正負號的輸出 (mV / Signed RPM)
#define LOGIC_VR_REVERSE_V_MIN_MV 200u
#define LOGIC_VR_REVERSE_V_MAX_MV LOGIC_VR_DEADZONE_LOW_MV
#define LOGIC_VR_REVERSE_OUTPUT_MIN -1500
#define LOGIC_VR_REVERSE_OUTPUT_MAX -7500

// 正轉工作區電壓和帶正負號的輸出 (mV / Signed RPM)
#define LOGIC_VR_FORWARD_V_MIN_MV LOGIC_VR_DEADZONE_HIGH_MV
#define LOGIC_VR_FORWARD_V_MAX_MV 4800u
#define LOGIC_VR_FORWARD_OUTPUT_MIN 2500
#define LOGIC_VR_FORWARD_OUTPUT_MAX 15500

// 死區輸出值
#define LOGIC_VR_OUTPUT_ZERO 0

// --- 加減速曲線表定義 ---
// Note: 減速時的 Step 值應為負數

// 正轉加速 (電壓閾值)
#define VR_FWD_ACCEL_STEP_TIME_ENTRY_0 {2, 4}
#define VR_FWD_ACCEL_STEP_TIME_ENTRY_1 {3, 3}
#define VR_FWD_ACCEL_STEP_TIME_ENTRY_2 {4, 2}
#define VR_FWD_ACCEL_STEP_TIME_ENTRY_3 {5, 1}
#define VR_FWD_ACCEL_VOLTAGE_THRESHOLDS {3300u, 3800u, 4300u, 5000u}

// 正轉減速 (輸出RPM絕對值閾值)
#define VR_FWD_DECEL_STEP_TIME_ENTRY_0 {-4, 1}
#define VR_FWD_DECEL_STEP_TIME_ENTRY_1 {-3, 2}
#define VR_FWD_DECEL_STEP_TIME_ENTRY_2 {-3, 2}
#define VR_FWD_DECEL_STEP_TIME_ENTRY_3 {-4, 1}
#define VR_FWD_DECEL_OUTPUT_THRESHOLDS {5750u, 9000u, 12250u, 15500u}

// 反轉加速 (電壓閾值)
#define VR_REV_ACCEL_STEP_TIME_ENTRY_0 {1, 4}
#define VR_REV_ACCEL_STEP_TIME_ENTRY_1 {1, 4}
#define VR_REV_ACCEL_STEP_TIME_ENTRY_2 {2, 3}
#define VR_REV_ACCEL_STEP_TIME_ENTRY_3 {2, 3}
#define VR_REV_ACCEL_VOLTAGE_THRESHOLDS {1700u, 1200u, 700u, 0u}

// 反轉減速 (輸出RPM絕對值閾值)
#define VR_REV_DECEL_STEP_TIME_ENTRY_0 {-1, 3}
#define VR_REV_DECEL_STEP_TIME_ENTRY_1 {-2, 2}
#define VR_REV_DECEL_STEP_TIME_ENTRY_2 {-2, 2}
#define VR_REV_DECEL_STEP_TIME_ENTRY_3 {-1, 3}
#define VR_REV_DECEL_OUTPUT_THRESHOLDS {3000u, 4500u, 6000u, 7500u}


// --- 函數原型 ---

/**
 * @brief 初始化 VR 邏輯模組並執行開機電壓檢測。
 * @param[in] u16InitialAdc 開機時檢測到的 VR 初始 ADC 值。
 * @return int8_t 0: 成功, -1: 開機抑制
 */
int8_t logic_vr_initAndCheck(uint16_t u16InitialAdc);

/**
 * @brief 根據輸入ADC值計算帶正負號的目標VR輸出值。
 * @param u16InputAdc 輸入ADC值。
 * @return int16_t 帶正負號的目標輸出值。
 */
int16_t logic_vr_getSignedTargetOutputFromAdc(uint16_t u16InputAdc);

/**
 * @brief 更新 VR 參數並計算目標轉速 (帶正負號RPM模型)。
 *
 * @param[out] psStepTime 計算出的步進時間參數結構指標。
 * @param[in,out] pi16TargetRpm 指向目標轉速的指標。函數會讀取它並可能因安全邏輯而修改它。
 * @param[in] i16CurrentRpm 當前的帶正負號轉速。
 * @param[in] u16AdcRawValue ADC原始讀值。
 * @param[in] fCurrentVehicleSpeedKmh 當前車速(km/h)，用於換向安全檢查。
 * @return int8_t 0: 成功, -1: 指標無效, -2: 開機抑制
 */
int8_t logic_vr_getUpdateParams(S_MOTOR_STEP_TIME_T *psStepTime,
                                int16_t *pi16TargetRpm,
                                int16_t i16CurrentRpm,
                                uint16_t u16AdcRawValue,
                                float fCurrentVehicleSpeedKmh);

#endif  // S_LOGIC_VR_H
