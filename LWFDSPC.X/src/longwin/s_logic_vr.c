#include "s_logic_vr.h"
#include "s_logic_error_handler.h"  // For error code handling
#include <stddef.h>                 // For NULL
#include <stdlib.h>                 // For abs()
                                    //
#include "codeSw.h"

// --- VR 抑制狀態 ---
static bool sb_vr_isPowerOnInhibited = false;    // 開機抑制狀態
static bool sb_vr_isRuntimeFaultActive = false;  // 運行時故障狀態

// --- VR Step/Time 表 ---
// 正轉加速
static const S_MOTOR_STEP_TIME_T s_vrFwdAccelStepTimeTable[] = {
    VR_FWD_ACCEL_STEP_TIME_ENTRY_0,
    VR_FWD_ACCEL_STEP_TIME_ENTRY_1,
    VR_FWD_ACCEL_STEP_TIME_ENTRY_2,
    VR_FWD_ACCEL_STEP_TIME_ENTRY_3};
static const uint16_t s_vrFwdAccelVoltageThresholds[] = VR_FWD_ACCEL_VOLTAGE_THRESHOLDS;
static const int s_iVrFwdAccelTableSize = sizeof(s_vrFwdAccelStepTimeTable) / sizeof(s_vrFwdAccelStepTimeTable[0]);

// 正轉減速
static const S_MOTOR_STEP_TIME_T s_vrFwdDecelStepTimeTable[] = {
    VR_FWD_DECEL_STEP_TIME_ENTRY_0,
    VR_FWD_DECEL_STEP_TIME_ENTRY_1,
    VR_FWD_DECEL_STEP_TIME_ENTRY_2,
    VR_FWD_DECEL_STEP_TIME_ENTRY_3};
static const uint16_t s_vrFwdDecelOutputThresholds[] = VR_FWD_DECEL_OUTPUT_THRESHOLDS;
static const int s_iVrFwdDecelTableSize = sizeof(s_vrFwdDecelStepTimeTable) / sizeof(s_vrFwdDecelStepTimeTable[0]);

// 反轉加速
static const S_MOTOR_STEP_TIME_T s_vrRevAccelStepTimeTable[] = {
    VR_REV_ACCEL_STEP_TIME_ENTRY_0,
    VR_REV_ACCEL_STEP_TIME_ENTRY_1,
    VR_REV_ACCEL_STEP_TIME_ENTRY_2,
    VR_REV_ACCEL_STEP_TIME_ENTRY_3};
static const uint16_t s_vrRevAccelVoltageThresholds[] = VR_REV_ACCEL_VOLTAGE_THRESHOLDS;
static const int s_iVrRevAccelTableSize = sizeof(s_vrRevAccelStepTimeTable) / sizeof(s_vrRevAccelStepTimeTable[0]);

// 反轉減速
static const S_MOTOR_STEP_TIME_T s_vrRevDecelStepTimeTable[] = {
    VR_REV_DECEL_STEP_TIME_ENTRY_0,
    VR_REV_DECEL_STEP_TIME_ENTRY_1,
    VR_REV_DECEL_STEP_TIME_ENTRY_2,
    VR_REV_DECEL_STEP_TIME_ENTRY_3};
static const uint16_t s_vrRevDecelOutputThresholds[] = VR_REV_DECEL_OUTPUT_THRESHOLDS;
static const int s_iVrRevDecelTableSize = sizeof(s_vrRevDecelStepTimeTable) / sizeof(s_vrRevDecelStepTimeTable[0]);

// --- 輔助函數 ---
static int _findIndexForUpperThreshold(uint16_t u16Value,
                                       const uint16_t pau16Thresholds[],
                                       int iTableSize) {
    for (int i = 0; i < iTableSize; ++i) {
        if (u16Value <= pau16Thresholds[i]) return i;
    }
    return iTableSize - 1;
}

static int _findIndexForLowerThreshold(uint16_t u16Value,
                                       const uint16_t pau16Thresholds[],
                                       int iTableSize) {
    for (int i = 0; i < iTableSize; ++i) {
        if (u16Value >= pau16Thresholds[i]) return i;
    }
    return iTableSize - 1;
}

static uint16_t _vrConvertAdcToVoltageMv(uint16_t u16AdcRawValue) {
    const uint32_t u32AdcMax = (1UL << LOGIC_VR_ADC_RESOLUTION_BITS) - 1;
    if (u32AdcMax == 0) return 0;
    uint32_t u32VAdcMv = ((uint32_t)u16AdcRawValue * (uint32_t)LOGIC_VR_VREF_MV) / u32AdcMax;
    const uint32_t u32R1 = LOGIC_VR_R1_OHMS;
    const uint32_t u32R2 = LOGIC_VR_R2_OHMS;
    if (u32R2 == 0) return 0;
    uint32_t u32R1PlusR2 = u32R1 + u32R2;
    uint32_t u32VInMv = (u32VAdcMv * u32R1PlusR2) / u32R2;
    return (u32VInMv > 65535u) ? 65535u : (uint16_t)u32VInMv;
}

// --- 函數實作 ---

int8_t logic_vr_initAndCheck(uint16_t u16InitialAdc) {
    uint16_t u16InitialVoltageMv = _vrConvertAdcToVoltageMv(u16InitialAdc);
    sb_vr_isPowerOnInhibited = (u16InitialVoltageMv < LOGIC_VR_POWER_ON_CHECK_MIN_MV) ||
                               (u16InitialVoltageMv > LOGIC_VR_POWER_ON_CHECK_MAX_MV);

    if (sb_vr_isPowerOnInhibited) {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, true);
        return -1;
    } else {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, false);
        sb_vr_isRuntimeFaultActive = false;
        return 0;
    }
}

int16_t logic_vr_getSignedTargetOutputFromAdc(uint16_t u16InputAdc) {
    uint16_t u16InputVoltageMv = _vrConvertAdcToVoltageMv(u16InputAdc);

    if (u16InputVoltageMv >= LOGIC_VR_DEADZONE_LOW_MV && u16InputVoltageMv <= LOGIC_VR_DEADZONE_HIGH_MV) {
        return LOGIC_VR_OUTPUT_ZERO;
    } else if (u16InputVoltageMv > LOGIC_VR_DEADZONE_HIGH_MV) {
        if (u16InputVoltageMv >= LOGIC_VR_FORWARD_V_MAX_MV) return LOGIC_VR_FORWARD_OUTPUT_MAX;
        uint32_t u32VoltageDiff = u16InputVoltageMv - LOGIC_VR_DEADZONE_HIGH_MV;
        uint32_t u32OutputRange = LOGIC_VR_FORWARD_OUTPUT_MAX - LOGIC_VR_FORWARD_OUTPUT_MIN;
        uint32_t u32VoltageRange = LOGIC_VR_FORWARD_V_MAX_MV - LOGIC_VR_DEADZONE_HIGH_MV;
        if (u32VoltageRange == 0) return LOGIC_VR_FORWARD_OUTPUT_MIN;
        uint32_t u32CalculatedOutput = ((u32VoltageDiff * u32OutputRange) / u32VoltageRange) + LOGIC_VR_FORWARD_OUTPUT_MIN;
        return (int16_t)u32CalculatedOutput;
    } else {  // u16InputVoltageMv < LOGIC_VR_DEADZONE_LOW_MV
        if (u16InputVoltageMv <= LOGIC_VR_REVERSE_V_MIN_MV) return LOGIC_VR_REVERSE_OUTPUT_MAX;
        uint32_t u32VoltageDiff = LOGIC_VR_REVERSE_V_MAX_MV - u16InputVoltageMv;
        uint32_t u32OutputRange = LOGIC_VR_REVERSE_OUTPUT_MIN - LOGIC_VR_REVERSE_OUTPUT_MAX;  // 1500 - 7500 = -6000
        uint32_t u32VoltageRange = LOGIC_VR_REVERSE_V_MAX_MV - LOGIC_VR_REVERSE_V_MIN_MV;
        if (u32VoltageRange == 0) return LOGIC_VR_REVERSE_OUTPUT_MIN;
        int32_t i32CalculatedOutput = (((int32_t)u32VoltageDiff * (int32_t)u32OutputRange) / (int32_t)u32VoltageRange) + LOGIC_VR_REVERSE_OUTPUT_MAX;
        return (int16_t)i32CalculatedOutput;
    }
}

int8_t logic_vr_getUpdateParams(S_MOTOR_STEP_TIME_T *psStepTime,
                                int16_t *pi16TargetRpm,
                                int16_t i16CurrentRpm,
                                uint16_t u16AdcRawValue,
                                float fCurrentVehicleSpeedKmh) {
    if (psStepTime == NULL || pi16TargetRpm == NULL)
        return -1;

    uint16_t u16CurrentVoltageMv = _vrConvertAdcToVoltageMv(u16AdcRawValue);

    // 運行時故障檢查
    if (u16CurrentVoltageMv > LOGIC_VR_FORWARD_V_MAX_MV) {  // 假設電壓不應超過正轉最大電壓
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, true);
        sb_vr_isRuntimeFaultActive = true;
        *pi16TargetRpm = 0;
        *psStepTime = (S_MOTOR_STEP_TIME_T){0, 0};
        return -1;
    } else {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, false);
        sb_vr_isRuntimeFaultActive = false;
    }

    // 開機抑制檢查
    if (sb_vr_isPowerOnInhibited == true) {
        if (u16CurrentVoltageMv >= LOGIC_VR_POWER_ON_CHECK_MIN_MV &&
            u16CurrentVoltageMv <= LOGIC_VR_POWER_ON_CHECK_MAX_MV) {
            sb_vr_isPowerOnInhibited = false;
        } else {
            *pi16TargetRpm = 0;
            *psStepTime = (S_MOTOR_STEP_TIME_T){0, 0};
            return -2;
        }
    }

    // 根據ADC計算原始目標轉速
    int16_t i16RawTargetRpm = logic_vr_getSignedTargetOutputFromAdc(u16AdcRawValue);

    // 換向安全邏輯
    bool isChangingToFwd = i16RawTargetRpm > 0 && i16CurrentRpm < 0;
    bool isChangingToRev = i16RawTargetRpm < 0 && i16CurrentRpm > 0;

    if ((isChangingToFwd || isChangingToRev) &&
        fCurrentVehicleSpeedKmh > CODESW_DIRECTION_CHANGE_SPEED_VR) {
        *pi16TargetRpm = 0;  // 強制目標為0，讓車輛減速
    } else {
        *pi16TargetRpm = i16RawTargetRpm;
    }

    // 根據目標和當前轉速，計算Step/Time
    S_MOTOR_STEP_TIME_T sStepTime = {0, 0};
    if (*pi16TargetRpm > i16CurrentRpm) {  // 加速
        if (*pi16TargetRpm > 0) {          // 正轉加速
            int idx = _findIndexForUpperThreshold(u16CurrentVoltageMv, s_vrFwdAccelVoltageThresholds, s_iVrFwdAccelTableSize);
            sStepTime = s_vrFwdAccelStepTimeTable[idx];
        } else {  // 反轉加速
            int idx = _findIndexForLowerThreshold(u16CurrentVoltageMv, s_vrRevAccelVoltageThresholds, s_iVrRevAccelTableSize);
            sStepTime = s_vrRevAccelStepTimeTable[idx];
        }
    } else if (*pi16TargetRpm < i16CurrentRpm) {  // 減速
        if (i16CurrentRpm > 0) {                  // 正轉減速
            int idx = _findIndexForUpperThreshold(abs(i16CurrentRpm), s_vrFwdDecelOutputThresholds, s_iVrFwdDecelTableSize);
            sStepTime = s_vrFwdDecelStepTimeTable[idx];
        } else {  // 反轉減速
            int idx = _findIndexForUpperThreshold(abs(i16CurrentRpm), s_vrRevDecelOutputThresholds, s_iVrRevDecelTableSize);
            sStepTime = s_vrRevDecelStepTimeTable[idx];
        }
    }

    *psStepTime = sStepTime;
    return 0;
}