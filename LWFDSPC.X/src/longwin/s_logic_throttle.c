#include "s_logic_throttle.h"
#include "s_logic_error_handler.h"  // For error code handling
#include <stddef.h>                 // For NULL

// --- 油門抑制狀態 ---
static bool sb_throttle_isPowerOnInhibited = false;  // 開機抑制狀態

// --- 正轉加速 Step/Time 表 (基於輸入電壓 mV) ---
static const S_MOTOR_STEP_TIME_T su16ThrottleFwdAccelStepTimeTable[] = {
    THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_0,
    THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_1,
    THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_2,
    THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_3,
    THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_4};
static const uint16_t su16ThrottleFwdAccelVoltageThresholds[] = {
    LOGIC_THROTTLE_FWD_ACCEL_V1_MV,
    LOGIC_THROTTLE_FWD_ACCEL_V2_MV,
    LOGIC_THROTTLE_FWD_ACCEL_V3_MV,
    LOGIC_THROTTLE_FWD_ACCEL_V4_MV};
static const size_t S_THROTTLE_FWD_ACCEL_TABLE_SIZE = sizeof(su16ThrottleFwdAccelStepTimeTable) / sizeof(su16ThrottleFwdAccelStepTimeTable[0]);

// --- 正轉減速 Step/Time 表 (基於目前輸出) ---
static const S_MOTOR_STEP_TIME_T su16ThrottleFwdDecelStepTimeTable[] = {
    THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_0,
    THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_1,
    THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_2,
    THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_3,
    THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_4};
static const uint16_t su16ThrottleFwdDecelOutputThresholds[] = {
    LOGIC_THROTTLE_FWD_DECEL_O1,
    LOGIC_THROTTLE_FWD_DECEL_O2,
    LOGIC_THROTTLE_FWD_DECEL_O3,
    LOGIC_THROTTLE_FWD_DECEL_O4};
static const size_t S_THROTTLE_FWD_DECEL_TABLE_SIZE = sizeof(su16ThrottleFwdDecelStepTimeTable) / sizeof(su16ThrottleFwdDecelStepTimeTable[0]);

// --- 反轉加速 Step/Time 表 (基於輸入電壓 mV) ---
static const S_MOTOR_STEP_TIME_T su16ThrottleRevAccelStepTimeTable[] = {
    THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_0,
    THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_1,
    THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_2,
    THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_3,
    THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_4};
static const uint16_t su16ThrottleRevAccelVoltageThresholds[] = {
    LOGIC_THROTTLE_REV_ACCEL_V1_MV,
    LOGIC_THROTTLE_REV_ACCEL_V2_MV,
    LOGIC_THROTTLE_REV_ACCEL_V3_MV,
    LOGIC_THROTTLE_REV_ACCEL_V4_MV};
static const size_t S_THROTTLE_REV_ACCEL_TABLE_SIZE = sizeof(su16ThrottleRevAccelStepTimeTable) / sizeof(su16ThrottleRevAccelStepTimeTable[0]);

// --- 反轉減速 Step/Time 表 (基於目前輸出) ---
static const S_MOTOR_STEP_TIME_T su16ThrottleRevDecelStepTimeTable[] = {
    THROTTLE_REV_DECEL_STEP_TIME_ENTRY_0,
    THROTTLE_REV_DECEL_STEP_TIME_ENTRY_1,
    THROTTLE_REV_DECEL_STEP_TIME_ENTRY_2,
    THROTTLE_REV_DECEL_STEP_TIME_ENTRY_3,
    THROTTLE_REV_DECEL_STEP_TIME_ENTRY_4};
static const uint16_t su16ThrottleRevDecelOutputThresholds[] = {
    LOGIC_THROTTLE_REV_DECEL_O1,
    LOGIC_THROTTLE_REV_DECEL_O2,
    LOGIC_THROTTLE_REV_DECEL_O3,
    LOGIC_THROTTLE_REV_DECEL_O4};
static const size_t S_THROTTLE_REV_DECEL_TABLE_SIZE = sizeof(su16ThrottleRevDecelStepTimeTable) / sizeof(su16ThrottleRevDecelStepTimeTable[0]);

static const uint16_t su16ThrottleAssistLevelOutputMax[] = THROTTLE_ASSIST_LEVEL_MAX_OUTPUT_VALUES;
/**
 * @brief 根據當前方向和目標值選擇適當的步進值和時間。
 */
static S_MOTOR_STEP_TIME_T _throttleGetStepTimeParams(uint16_t u16TargetRpm,
                                                      uint16_t u16CurrentRpm,
                                                      uint16_t u16CurrentVoltageMv,
                                                      E_LOGIC_THROTTLE_DIRECTION_T eDirection) {
    S_MOTOR_STEP_TIME_T sStepTime = {0, 0};

    if (u16TargetRpm > u16CurrentRpm) {
        // 加速模式
        if (eDirection == LOGIC_THROTTLE_DIRECTION_FORWARD) {
            int iFoundIndex = S_THROTTLE_FWD_ACCEL_TABLE_SIZE - 1;
            for (int i = 0; i < (int)S_THROTTLE_FWD_ACCEL_TABLE_SIZE - 1; ++i) {
                if (u16CurrentVoltageMv <= su16ThrottleFwdAccelVoltageThresholds[i]) {
                    iFoundIndex = i;
                    break;
                }
            }
            sStepTime = su16ThrottleFwdAccelStepTimeTable[iFoundIndex];
        } else {
            int iFoundIndex = S_THROTTLE_REV_ACCEL_TABLE_SIZE - 1;
            for (int i = 0; i < (int)S_THROTTLE_REV_ACCEL_TABLE_SIZE - 1; ++i) {
                if (u16CurrentVoltageMv <= su16ThrottleRevAccelVoltageThresholds[i]) {
                    iFoundIndex = i;
                    break;
                }
            }
            sStepTime = su16ThrottleRevAccelStepTimeTable[iFoundIndex];
        }
    } else if (u16TargetRpm < u16CurrentRpm) {
        // 減速模式
        if (eDirection == LOGIC_THROTTLE_DIRECTION_FORWARD) {
            int iFoundIndex = S_THROTTLE_FWD_DECEL_TABLE_SIZE - 1;
            for (int i = 0; i < (int)S_THROTTLE_FWD_DECEL_TABLE_SIZE - 1; ++i) {
                if (u16CurrentRpm <= su16ThrottleFwdDecelOutputThresholds[i]) {
                    iFoundIndex = i;
                    break;
                }
            }
            sStepTime = su16ThrottleFwdDecelStepTimeTable[iFoundIndex];
        } else {
            int iFoundIndex = S_THROTTLE_REV_DECEL_TABLE_SIZE - 1;
            for (int i = 0; i < (int)S_THROTTLE_REV_DECEL_TABLE_SIZE - 1; ++i) {
                if (u16CurrentRpm <= su16ThrottleRevDecelOutputThresholds[i]) {
                    iFoundIndex = i;
                    break;
                }
            }
            sStepTime = su16ThrottleRevDecelStepTimeTable[iFoundIndex];
        }
    }

    return sStepTime;
}

int8_t logic_throttle_initAndCheck(uint16_t u16InitialVoltageMv) {
    // >= 而非 > ：電壓剛好等於起步門檻時已會輸出 OUTPUT_MIN，必須算成「未釋放」
    sb_throttle_isPowerOnInhibited = (u16InitialVoltageMv >= LOGIC_THROTTLE_POWER_ON_CHECK_MV);

    if (sb_throttle_isPowerOnInhibited) {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, true);
        return -1;
    } else {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, false);
        return 0;
    }
}

int8_t logic_throttle_getUpdateParams(S_MOTOR_STEP_TIME_T *psStepTime,
                                      uint16_t *pu16TargetRpm,
                                      uint16_t u16CurrentRpm,
                                      uint16_t u16CurrentVoltageMv,
                                      E_LOGIC_THROTTLE_DIRECTION_T eDirection,
                                      uint8_t u8AssistLevel) {
    // 檢查指標是否有效
    if (psStepTime == NULL || pu16TargetRpm == NULL) {
        return -1;
    }
    S_MOTOR_STEP_TIME_T sStepTime = {0, 0};

    // 檢查運行時電壓是否超出故障檢測範圍
    bool bCurrentVoltageIsFaulty = (u16CurrentVoltageMv < LOGIC_THROTTLE_FAULT_VOLTAGE_LOW_MV) ||
                                   (u16CurrentVoltageMv > LOGIC_THROTTLE_FAULT_VOLTAGE_HIGH_MV);

    if (bCurrentVoltageIsFaulty) {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, true);
        *pu16TargetRpm = 0;
        *psStepTime = (S_MOTOR_STEP_TIME_T)LOGIC_THROTTLE_FAULT_DECEL_STEP_TIME;
        return -1;
    } else {
        if (!sb_throttle_isPowerOnInhibited) {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, false);
        }
    }

    // 檢查開機抑制狀態
    if (sb_throttle_isPowerOnInhibited) {
        if (u16CurrentVoltageMv < LOGIC_THROTTLE_POWER_ON_CHECK_MV) {  // 與上面的 >= 對稱
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, false);
            sb_throttle_isPowerOnInhibited = false;
        } else {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A03_THROTTLE_FAULT, true);
            *pu16TargetRpm = 0;
            *psStepTime = (S_MOTOR_STEP_TIME_T){0, 0};
            return -2;
        }
    }

    // 段位索引邊界夾制。LCD 經 Modbus 送來的助力段位是 4-bit (u8WheelCfgLo & 0x0F) = 0~15,
    //   但 su16ThrottleAssistLevelOutputMax[] 只有 THROTTLE_ASSIST_LEVEL_COUNT(6) 筆 ——
    //   段位 6~15 會讀到陣列外的記憶體。夾到最高段位，行為等同段位 5。
    if (u8AssistLevel >= THROTTLE_ASSIST_LEVEL_COUNT) {
        u8AssistLevel = THROTTLE_ASSIST_LEVEL_COUNT - 1u;
    }

    // [e-lock] 助力段位 0 = 電子鎖車：前進與倒退皆完全禁止，並讓 EMB 鎖住馬達。
    //   必須在此早退，不能只靠段位上限表的第 0 筆(值為 0) —— 下游有三道 OUTPUT_MIN 地板會把
    //   它釘回起步速度 1500 count (1.04 km/h)，這正是原本「段位 0 仍以 1 km/h 潛行」的 bug:
    //     (1) 「確保動態上限不低於下限」的保護把上限 0 提升成 OUTPUT_MIN;
    //     (2) 線性內插後的 `< u16OutputMin` 地板;
    //     (3) 加速時的 `u16CurrentRpm < u16OutputMin` 地板。
    //   不分方向判斷，故一次同時擋掉前進與倒退(反轉是「無號大小 + uGF.DirSW」模型，沒有負命令)。
    //
    //   目標歸零後，「減速到停 → UVW 短路 → 夾 EMB」由既有停車鏈自動完成，不需要新的立即鎖定
    //   分支；EMB 也會因 _isMotorCommandActive(0)==false 而不再放開煞車。因此騎行中切到 0 段是
    //   平順減速到停後才鎖，不會帶速硬鎖。
    //
    //   減速曲線取自正常減速表(而非 LOGIC_THROTTLE_FAULT_DECEL_STEP_TIME)，讓停車手感與鬆油門
    //   一致；回傳 0(成功)讓呼叫端走正常路徑，回傳負值會被當成故障處理。
    if (u8AssistLevel == 0u) {
        *pu16TargetRpm = 0;
        *psStepTime = _throttleGetStepTimeParams(0u, u16CurrentRpm, u16CurrentVoltageMv, eDirection);
        return 0;
    }

    // 依據電壓計算目標RPM值
    uint16_t u16ThrottleTargetRpm = 0;
    uint16_t u16VoltageMinMv, u16VoltageMaxMv, u16OutputMin, u16OutputMax, u16OutputZero;

    if (eDirection == LOGIC_THROTTLE_DIRECTION_FORWARD) {
        u16VoltageMinMv = LOGIC_THROTTLE_FWD_VOLTAGE_MIN_MV;
        u16VoltageMaxMv = LOGIC_THROTTLE_FWD_VOLTAGE_MAX_MV;
        u16OutputMin = LOGIC_THROTTLE_FWD_OUTPUT_MIN;
        // u16OutputMax = LOGIC_THROTTLE_FWD_OUTPUT_MAX;
        u16OutputMax = su16ThrottleAssistLevelOutputMax[u8AssistLevel];  // 正轉使用傳入的動態上限
        u16OutputZero = LOGIC_THROTTLE_FWD_OUTPUT_ZERO;
    } else {
        u16VoltageMinMv = LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV;
        u16VoltageMaxMv = LOGIC_THROTTLE_REV_VOLTAGE_MAX_MV;
        u16OutputMin = LOGIC_THROTTLE_REV_OUTPUT_MIN;
        u16OutputMax = LOGIC_THROTTLE_REV_OUTPUT_MAX;  // 反轉使用固定的上限
        u16OutputZero = LOGIC_THROTTLE_REV_OUTPUT_ZERO;
    }

    // 確保動態上限不會低於下限，避免計算錯誤
    if (u16OutputMax < u16OutputMin) {
        u16OutputMax = u16OutputMin;
    }

    if (u16CurrentVoltageMv >= u16VoltageMaxMv) {
        u16ThrottleTargetRpm = u16OutputMax;
    } else if (u16CurrentVoltageMv >= u16VoltageMinMv) {
        uint32_t u32VoltageDiff = u16CurrentVoltageMv - u16VoltageMinMv;
        uint32_t u32OutputRange = u16OutputMax - u16OutputMin;
        uint32_t u32VoltageRange = u16VoltageMaxMv - u16VoltageMinMv;
        if (u32VoltageRange == 0) {
            u16ThrottleTargetRpm = u16OutputMin;
        } else {
            uint32_t u32Numerator = u32VoltageDiff * u32OutputRange;
            uint32_t u32CalculatedOutput = (u32Numerator / u32VoltageRange) + u16OutputMin;
            u16ThrottleTargetRpm = (u32CalculatedOutput > u16OutputMax) ? u16OutputMax : (uint16_t)u32CalculatedOutput;
        }
        if (u16ThrottleTargetRpm < u16OutputMin) {
            u16ThrottleTargetRpm = u16OutputMin;
        }
    } else {
        u16ThrottleTargetRpm = u16OutputZero;
    }

    // 根據目標選擇加減速曲線
    sStepTime = _throttleGetStepTimeParams(u16ThrottleTargetRpm, u16CurrentRpm, u16CurrentVoltageMv, eDirection);

    if (sStepTime.i8Step > 0 && u16CurrentRpm < u16OutputMin) {
        u16ThrottleTargetRpm = u16OutputMin;
    }

    *psStepTime = sStepTime;
    *pu16TargetRpm = u16ThrottleTargetRpm;

    return 0;
}
