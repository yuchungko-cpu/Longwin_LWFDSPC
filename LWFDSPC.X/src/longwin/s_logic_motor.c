#include "s_logic_motor.h"
#include "s_logic_error_handler.h"  // For error code handling
#include <stddef.h>                 // For NULL
#include "../userparms.h"           // For Q15(), PI 增益等；刻度參數見 ../motor_scale.h

// --- Scaling Factors for Integer Math ---
#define RPM_SCALE_FACTOR 10  // RPM 放大倍數 (0.1 RPM 精度)
#define PPR_SCALE_FACTOR 10  // PPR 放大倍數 (0.1 PPR 精度)

// --- Static Module Variables ---
static uint32_t su32_motor_lastSystemTimeMs = 0;
static S_LOGIC_MOTOR_CONFIG_T s_currentMotorConfig;

#if CODESW_X2C_SCOPE_ENABLE == 1
// X2CScope 觀測用 (宣告與說明見 s_logic_motor.h)。純觀測，不參與任何控制決策。
int16_t g_i16ScopeCmdTarget = 0;
int16_t g_i16ScopeCmdRateLim = 0;
int16_t g_i16ScopeCmdOut = 0;
#endif

// --- Merged Configuration Functions ---

/**
 * @brief 初始化馬達參數配置模組
 * @note 將所有參數設定為預設值，並清除相關的錯誤狀態。應在系統啟動時調用。
 */
void logic_motor_configInit(void) {
    s_currentMotorConfig.u16WheelDimensionInches = LOGIC_MOTOR_DEFAULT_WHEEL_DIMENSION_INCH;
    s_currentMotorConfig.u8PolePairs = LOGIC_MOTOR_DEFAULT_POLE_PAIRS;
    s_currentMotorConfig.u16HallPPR = LOGIC_MOTOR_DEFAULT_HALL_PPR;
    s_currentMotorConfig.u8ExternalSensorPPR = LOGIC_MOTOR_DEFAULT_EXTERNAL_SENSOR_PPR;
    s_currentMotorConfig.eSpeedSource = LOGIC_MOTOR_DEFAULT_SPEED_SOURCE;

    // Clear LSN configuration-related alarm on init, as default is valid.
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, false);
}

/**
 * @brief 設定輪徑尺寸
 * @param u16Inches 輪徑，單位：吋 * 10
 */
void logic_motor_setWheelDimension(uint16_t u16Inches) {
    s_currentMotorConfig.u16WheelDimensionInches = u16Inches;
}

/**
 * @brief 取得目前設定的輪徑尺寸
 * @return uint16_t 輪徑，單位：吋 * 10
 */
uint16_t logic_motor_getWheelDimension(void) {
    return s_currentMotorConfig.u16WheelDimensionInches;
}

/**
 * @brief 設定馬達極對數
 * @param u8Pairs 馬達的極對數
 */
void logic_motor_setPolePairs(uint8_t u8Pairs) {
    s_currentMotorConfig.u8PolePairs = u8Pairs;
}

/**
 * @brief 取得目前設定的馬達極對數
 * @return uint8_t 馬達的極對數
 */
uint8_t logic_motor_getPolePairs(void) {
    return s_currentMotorConfig.u8PolePairs;
}

/**
 * @brief 設定馬達霍爾感測器每轉脈衝數 (PPR)
 * @param u16ppr Hall PPR值 * 10
 */
void logic_motor_setHallPPR(uint16_t u16ppr) {
    s_currentMotorConfig.u16HallPPR = u16ppr;
}

/**
 * @brief 取得馬達霍爾感測器每轉脈衝數 (PPR)
 * @return uint16_t Hall PPR值 * 10
 */
uint16_t logic_motor_getHallPPR(void) {
    return s_currentMotorConfig.u16HallPPR;
}

/**
 * @brief 設定外部輪速感測器每轉脈衝數 (PPR)
 * @param u8ppr 外部感測器PPR值。若在外部感測器模式下設為0，會觸發A14錯誤。
 */
void logic_motor_setExternalSensorPPR(uint8_t u8ppr) {
    s_currentMotorConfig.u8ExternalSensorPPR = u8ppr;
    if (s_currentMotorConfig.eSpeedSource == LOGIC_MOTOR_SPEED_SOURCE_EXTERNAL) {
        if (u8ppr == 0) {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, true);
        } else {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, false);
        }
    }
}

/**
 * @brief 取得外部輪速感測器每轉脈衝數 (PPR)
 * @return uint8_t 外部感測器PPR值
 */
uint8_t logic_motor_getExternalSensorPPR(void) {
    return s_currentMotorConfig.u8ExternalSensorPPR;
}

/**
 * @brief 設定速度感測器來源
 * @param eSource 速度感測器來源 (霍爾或外部)
 */
void logic_motor_setSpeedSource(E_LOGIC_MOTOR_SPEED_SOURCE_T eSource) {
    s_currentMotorConfig.eSpeedSource = eSource;

    if (eSource == LOGIC_MOTOR_SPEED_SOURCE_EXTERNAL) {
        if (s_currentMotorConfig.u8ExternalSensorPPR == 0) {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, true);
        } else {
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, false);
        }
    } else {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A14_LSN_FAULT, false);
    }
}

/**
 * @brief 取得目前設定的速度感測器來源
 * @return E_LOGIC_MOTOR_SPEED_SOURCE_T 速度感測器來源
 */
E_LOGIC_MOTOR_SPEED_SOURCE_T logic_motor_getSpeedSource(void) {
    return s_currentMotorConfig.eSpeedSource;
}

/**
 * @brief 取得目前的完整馬達參數配置
 * @return S_LOGIC_MOTOR_CONFIG_T 包含所有當前馬達參數的結構副本
 */
S_LOGIC_MOTOR_CONFIG_T logic_motor_getCurrentConfig(void) {
    return s_currentMotorConfig;  // Returns a copy
}

// --- Merged Calculation Functions ---

/**
 * @brief (LWFOC) 將FOC內部ProgramSpeed值轉換為內部RPM
 * @param s16ProgramSpeed FOC提供的ProgramSpeed值 (通常為int16_t)
 * @return uint16_t 內部RPM值 * 10
 */
uint16_t logic_motor_LwfocGetInternalRpm(int16_t s16ProgramSpeed) {
    if (LOGIC_MOTOR_PROGRAM_SPEED_MAX_VALUE == 0) return 0;
    // Formula: RPM = ProgramSpeed * 3000 / 32768
    uint32_t u32Result = (uint32_t)s16ProgramSpeed * LOGIC_MOTOR_PROGRAM_SPEED_RPM_FACTOR;
    return (uint16_t)((u32Result / LOGIC_MOTOR_PROGRAM_SPEED_MAX_VALUE) * RPM_SCALE_FACTOR);
}

/**
 * @brief (LWFOC) 將內部RPM轉換為外部RPM
 * @param u16LwfocInternalRpm 內部RPM * 10
 * @param u8PolePairs 馬達極對數
 * @param u16SensorPpr 感測器每轉脈衝數 * 10 (根據eSpeedSource選擇HallPPR或ExternalSensorPpr)
 * @return uint16_t 外部RPM值 * 10。如果u16SensorPpr為0，則回傳0。
 */
uint16_t logic_motor_LwfocGetExternalRpm(uint16_t u16LwfocInternalRpm, uint8_t u8PolePairs, uint16_t u16SensorPpr) {
    if (u16SensorPpr == 0) return 0;
    // Formula: 外部RPM = 內部RPM * 極對數 / 每轉Pulse
    // Input u16LwfocInternalRpm is RPM*10, u16SensorPpr is PPR*10
    uint32_t u32Result = (uint32_t)u16LwfocInternalRpm * u8PolePairs * PPR_SCALE_FACTOR;
    return (uint16_t)(u32Result / u16SensorPpr);
}

/**
 * @brief 將RPM值和輪徑轉換為速度 (KM/H)
 * @param u16GenericRpm RPM值 * 10 (可以是馬達RPM或輪子RPM)
 * @param u16WheelDimensionInches 輪徑 * 10 (吋)
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H
 */
uint16_t logic_motor_getSpeedKmhFromRpm(uint16_t u16GenericRpm, uint16_t u16WheelDimensionInches) {
    if (u16WheelDimensionInches == 0) return 0;
    // Formula: KM = RPM * 吋 * 0.0047878
    // We calculate KM*100. Inputs are RPM*10, 吋*10.
    // KM*100 = (RPM*10 * 吋*10 * 479) / 100000
    uint32_t u32Result = (uint32_t)u16GenericRpm * u16WheelDimensionInches * LOGIC_MOTOR_KMH_CONVERSION_FACTOR;
    return (uint16_t)(u32Result / 100000);  // Corrected based on formula analysis
}

/**
 * @brief (LWFOC) 複合計算：從FOC ProgramSpeed 計算速度 (KM/H)
 * @param s16ProgramSpeed FOC提供的ProgramSpeed值
 * @param psConfig 指向當前馬達配置的指標
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H。若psConfig為NULL，回傳0。
 */
uint16_t logic_motor_getSpeedKmhViaLwfoc(int16_t s16ProgramSpeed, const S_LOGIC_MOTOR_CONFIG_T* psConfig) {
    if (psConfig == NULL) return 0;

    uint16_t u16InternalRpm = logic_motor_LwfocGetInternalRpm(s16ProgramSpeed);
    uint16_t u16ActiveSensorPpr;

    if (psConfig->eSpeedSource == LOGIC_MOTOR_SPEED_SOURCE_HALL) {
        u16ActiveSensorPpr = psConfig->u16HallPPR;
    } else {
        u16ActiveSensorPpr = psConfig->u8ExternalSensorPPR * PPR_SCALE_FACTOR;
    }

    uint16_t u16ExternalRpm = logic_motor_LwfocGetExternalRpm(u16InternalRpm, psConfig->u8PolePairs, u16ActiveSensorPpr);
    return logic_motor_getSpeedKmhFromRpm(u16ExternalRpm, psConfig->u16WheelDimensionInches);
}

/**
 * @brief 從霍爾感測器頻率計算馬達機械RPM
 * @param u16HallFrequencyHz 霍爾感測器頻率 * 10 (Hz)
 * @param u8PolePairs 馬達極對數
 * @return uint16_t 馬達機械RPM * 10。如果u8PolePairs為0，則回傳0。
 */
uint16_t logic_motor_getMotorMechanicalRpmFromHallHz(uint16_t u16HallFrequencyHz, uint8_t u8PolePairs) {
    if (u8PolePairs == 0) return 0;
    // Formula: RPM = Hu * 60 / 極對數
    // Input is Hz*10. Output is RPM*10
    uint32_t u32Result = (uint32_t)u16HallFrequencyHz * LOGIC_MOTOR_RPM_FROM_HALL_HZ_FACTOR;
    return (uint16_t)(u32Result / u8PolePairs);
}

/**
 * @brief 從馬達機械RPM計算車輪RPM
 * @param u16MotorMechanicalRpm 馬達機械RPM * 10
 * @param u8PolePairs 馬達極對數
 * @param u16ExternalSensorPprAtWheel 外部輪速感測器每轉脈衝數 * 10 (PPR)
 * @return uint16_t 車輪RPM * 10。如果u16ExternalSensorPprAtWheel為0，則回傳0。
 */
uint16_t logic_motor_getWheelRpmFromMotorMechanicalRpm(uint16_t u16MotorMechanicalRpm, uint8_t u8PolePairs, uint16_t u16ExternalSensorPprAtWheel) {
    if (u16ExternalSensorPprAtWheel == 0) return 0;
    // Formula: 外部RPM = 內部RPM * 極對數 / Pulse
    // Inputs are RPM*10, PPR*10. Output is RPM*10
    uint32_t u32Result = (uint32_t)u16MotorMechanicalRpm * u8PolePairs * PPR_SCALE_FACTOR;
    return (uint16_t)(u32Result / u16ExternalSensorPprAtWheel);
}

/**
 * @brief (Hall Hz) 複合計算：從霍爾感測器頻率計算速度 (KM/H)
 * @param u16HallFrequencyHz 霍爾感測器頻率 * 10 (Hz)
 * @param psConfig 指向當前馬達配置的指標
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H。若psConfig為NULL，回傳0。
 */
uint16_t logic_motor_getSpeedKmhViaHallHz(uint16_t u16HallFrequencyHz, const S_LOGIC_MOTOR_CONFIG_T* psConfig) {
    if (psConfig == NULL) return 0;

    uint16_t u16MotorMechanicalRpm = logic_motor_getMotorMechanicalRpmFromHallHz(u16HallFrequencyHz, psConfig->u8PolePairs);
    uint16_t u16WheelRpm = logic_motor_getWheelRpmFromMotorMechanicalRpm(u16MotorMechanicalRpm, psConfig->u8PolePairs, psConfig->u8ExternalSensorPPR * PPR_SCALE_FACTOR);
    return logic_motor_getSpeedKmhFromRpm(u16WheelRpm, psConfig->u16WheelDimensionInches);
}

// --- Original Functions from s_logic_motor.c ---

/**
 * @brief 根據步進參數更新馬達轉速，實現平滑加減速
 * @param pu16ActiveRpm 指向實際轉速的指標 (輸出)
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param u16CurrentRpm 當前轉速
 * @param u16TargetRpm 目標轉速
 * @param u16MaxRpm 最大轉速限制
 * @param u16MinRpm 最小轉速限制
 * @param psStepTime 步進時間參數結構指標
 * @return 0: 成功, -1: 時間未到或指標無效, -2: 時間參數無效, -3: 無需調整
 */
int8_t logic_motor_getUpdateParams(uint16_t* pu16ActiveRpm,
                                   uint32_t u32SystemTimeMs,
                                   uint16_t u16CurrentRpm,
                                   uint16_t u16TargetRpm,
                                   uint16_t u16MaxRpm,
                                   uint16_t u16MinRpm,
                                   S_MOTOR_STEP_TIME_T* psStepTime) {
    // static uint32_t su32_motor_lastSystemTimeMs = 0; // Already defined above
    // 如果發生錯誤，可以讓 activeRpm 跟 currentRpm 一樣，也就是調整不變
    uint16_t u16ActiveRpm = u16CurrentRpm;

    // 檢查指標是否有效
    if (pu16ActiveRpm == NULL || psStepTime == NULL) {
        return -1;  // 指標無效
    }

    // 檢查時間參數是否有效
    if (psStepTime->i8Time <= 0) {
        return -2;  // 時間參數無效
    }

    uint32_t u32TimeDiff = u32SystemTimeMs - su32_motor_lastSystemTimeMs;
    // 檢查時間是否到達
    if (u32TimeDiff >= (uint32_t)psStepTime->i8Time) {
        su32_motor_lastSystemTimeMs = u32SystemTimeMs;

        // 如果是加速
        if (psStepTime->i8Step > 0) {
            // 如果要加速，將目前的RPM 加上要調整的 STEP
            u16ActiveRpm = u16CurrentRpm + (uint16_t)psStepTime->i8Step;

            // 如果目前的RPM 大於目標RPM，則將目標設為目標
            if (u16ActiveRpm > u16TargetRpm) {
                u16ActiveRpm = u16TargetRpm;
            }
            // 如果目前的RPM 大於最大RPM，則將目標設為最大RPM
            if (u16ActiveRpm > u16MaxRpm) {
                u16ActiveRpm = u16MaxRpm;
            }
            // 如果目前的RPM 小於最小RPM，則將目標設為最小RPM
            if (u16ActiveRpm < u16MinRpm) {
                u16ActiveRpm = u16MinRpm;
            }
        }
        // 如果減速
        else if (psStepTime->i8Step < 0) {
            // 將負數轉換為正數進行比較
            uint16_t u16StepAbs = (uint16_t)(-psStepTime->i8Step);

            // 判斷目前的RPM 是否大於要調整的 STEP
            if (u16CurrentRpm > u16StepAbs) {
                u16ActiveRpm = u16CurrentRpm - u16StepAbs;  // 減去要減的數值
                // 如果目前的目標小於要調整的RPM，則將目標設為目標
                if (u16ActiveRpm < u16TargetRpm) {
                    u16ActiveRpm = u16TargetRpm;
                }
            } else {
                // 如果目前的RPM 小於要調整的 STEP，則將目標設為0
                u16ActiveRpm = 0;
            }
        } else {
            // 代表不用增加或減少，直接返回
            return -3;
        }
    } else {
        // 如果時間未到，則返回錯誤
        return -1;
    }

    // 更新確定要執行的轉動值
    *pu16ActiveRpm = u16ActiveRpm;
    return 0;
}

#if CODESW_THROTTLE_ACCEL_FILTER_ENABLE == 1
// ---------------------------------------------------------------------------
//  限斜率 + 一階濾波 的加速曲線 (設計說明見 s_logic_motor.h)
// ---------------------------------------------------------------------------
static const S_ACCEL_FILTER_CURVE_T scsAccelFilterCurves[ACCEL_FILTER_CURVE_COUNT] =
    ACCEL_FILTER_CURVE_TABLE_INIT;
static const S_DECEL_FILTER_CURVE_T scsDecelFilterCurves[DECEL_FILTER_CURVE_COUNT] =
    DECEL_FILTER_CURVE_TABLE_INIT;
// 反轉 5 段曲線。段位在程式內固定 (REV_FILTER_CURVE_SELECT)，加減速共用同一個索引 ——
//   理由見 s_logic_motor.h 的說明。索引由編譯期常數決定，不需邊界夾制 (有 #error 護欄)。
static const S_ACCEL_FILTER_CURVE_T scsRevAccelFilterCurves[REV_FILTER_CURVE_COUNT] =
    REV_ACCEL_FILTER_CURVE_TABLE_INIT;
static const S_DECEL_FILTER_CURVE_T scsRevDecelFilterCurves[REV_FILTER_CURVE_COUNT] =
    REV_DECEL_FILTER_CURVE_TABLE_INIT;

static uint16_t su16AccelFilterRateLimited = 0;  // 限斜率後的目標 (count)
static uint32_t su32AccelFilterState = 0;        // 濾波狀態 (count << FRAC_BITS)
static uint32_t su32AccelFilterLastMs = 0;
static bool sbAccelFilterPrimed = false;         // 是否已預載起始速度

// 濾波狀態 → 命令 count。必須四捨五入而非截尾：一階濾波是漸近的，尾段會停在
//   「目標 − 不到 1 count」的位置，截尾會讓命令永遠差 1 count 到不了目標(實測 11999/12000)。
#define ACCEL_FILTER_STATE_TO_CMD(st) \
    ((uint16_t)(((st) + (1UL << (ACCEL_FILTER_FRAC_BITS - 1u))) >> ACCEL_FILTER_FRAC_BITS))

int8_t logic_motor_getUpdateParamsFiltered(uint16_t *pu16ActiveRpm,
                                           uint32_t u32SystemTimeMs,
                                           uint16_t u16CurrentRpm,
                                           uint16_t u16TargetRpm,
                                           uint16_t u16MaxRpm,
                                           uint16_t u16MinRpm,
                                           S_MOTOR_STEP_TIME_T *psStepTime,
                                           uint8_t u8CurveIndex,
                                           bool bReverse,
                                           uint8_t u8DecelCurveIndex) {
    (void)psStepTime;  // 加減速皆走濾波後已不需要 step/time；保留參數維持呼叫端相容
    if (pu16ActiveRpm == NULL) {
        return -1;
    }
    if (u8CurveIndex >= ACCEL_FILTER_CURVE_COUNT) {
        u8CurveIndex = ACCEL_FILTER_CURVE_COUNT - 1u;
    }
    if (u8DecelCurveIndex >= DECEL_FILTER_CURVE_COUNT) {
        u8DecelCurveIndex = DECEL_FILTER_CURVE_COUNT - 1u;
    }

    // 目標低於現值(含放油門的目標 0) → 減速；否則加速。兩者只換參數，濾波狀態共用。
    bool bDecel = (u16TargetRpm < u16CurrentRpm);

    // 四象限選參數：正轉/反轉 × 加速/減速
    //   正轉的段位由呼叫端傳入 (加速段位可能來自 Modbus，故已在上方夾制邊界);
    //   反轉的段位是編譯期常數 REV_FILTER_CURVE_INDEX，加減速共用。
    const S_ACCEL_FILTER_CURVE_T *pcsAccel =
            bReverse ? &scsRevAccelFilterCurves[REV_FILTER_CURVE_INDEX]
                     : &scsAccelFilterCurves[u8CurveIndex];
    const S_DECEL_FILTER_CURVE_T *pcsDecel =
            bReverse ? &scsRevDecelFilterCurves[REV_FILTER_CURVE_INDEX]
                     : &scsDecelFilterCurves[u8DecelCurveIndex];
    uint8_t u8Rate = bDecel ? pcsDecel->u8RateCountsPerTick : pcsAccel->u8RateCountsPerTick;
    uint8_t u8Shift = bDecel ? pcsDecel->u8FilterShift : pcsAccel->u8FilterShift;

    // 已經停在 0 且目標也是 0：不必跑濾波，順便解除預載讓下次起步重新預載起始速度。
    if ((u16TargetRpm == 0u) && (u16CurrentRpm == 0u)) {
        sbAccelFilterPrimed = false;
        su16AccelFilterRateLimited = 0u;
        su32AccelFilterState = 0u;
#if CODESW_X2C_SCOPE_ENABLE == 1
        g_i16ScopeCmdRateLim = 0;
#endif
        *pu16ActiveRpm = 0u;
        return -3;  // 無需調整
    }

    // 命令若被外部強制改寫(例如失速保護歸零、其他模式接手)，重新同步內部狀態，
    //   否則濾波會從舊值繼續，命令會跳動。
    uint16_t u16StateCmd = ACCEL_FILTER_STATE_TO_CMD(su32AccelFilterState);
    uint16_t u16Diff = (u16StateCmd > u16CurrentRpm) ? (uint16_t)(u16StateCmd - u16CurrentRpm)
                                                     : (uint16_t)(u16CurrentRpm - u16StateCmd);
    if (!sbAccelFilterPrimed || (u16Diff > ACCEL_FILTER_RESYNC_TOL)) {
        uint16_t u16Seed = u16CurrentRpm;
        if (!bDecel) {
            // 起步預載：直接跳到起始速度，省掉從 0 慢慢爬的那段 (原本要 750ms)。
            //   ⚠ 只有加速才預載。減速時種子必須就是當時速度 —— 若在此套 u16StartCmd
            //   或 u16MinRpm 地板，從高速放油門會先往下跳到 1500 count 造成階躍。
            if (u16Seed < pcsAccel->u16StartCmd) {
                u16Seed = pcsAccel->u16StartCmd;
            }
            if (u16Seed < u16MinRpm) {
                u16Seed = u16MinRpm;
            }
        }
        if (u16Seed > u16MaxRpm) {
            u16Seed = u16MaxRpm;
        }
        su16AccelFilterRateLimited = u16Seed;
        su32AccelFilterState = (uint32_t)u16Seed << ACCEL_FILTER_FRAC_BITS;
        su32AccelFilterLastMs = u32SystemTimeMs;
        sbAccelFilterPrimed = true;
#if CODESW_X2C_SCOPE_ENABLE == 1
        g_i16ScopeCmdRateLim = (int16_t)u16Seed;  // 起步預載，避免留下上一次的舊值
#endif
        *pu16ActiveRpm = u16Seed;
        return 0;
    }

    // 依經過時間補算 tick；主迴圈延遲時最多補 ACCEL_FILTER_CATCHUP_MAX 個，避免無界迴圈
    uint32_t u32Elapsed = u32SystemTimeMs - su32AccelFilterLastMs;
    if (u32Elapsed < ACCEL_FILTER_TICK_MS) {
        return -1;  // 時間未到
    }
    uint32_t u32Ticks = u32Elapsed / ACCEL_FILTER_TICK_MS;
    if (u32Ticks > ACCEL_FILTER_CATCHUP_MAX) {
        u32Ticks = ACCEL_FILTER_CATCHUP_MAX;
    }
    su32AccelFilterLastMs += u32Ticks * ACCEL_FILTER_TICK_MS;

    uint16_t u16Target = (u16TargetRpm > u16MaxRpm) ? u16MaxRpm : u16TargetRpm;

    for (uint32_t u32i = 0; u32i < u32Ticks; ++u32i) {
        // 第一級：限斜率 (決定到達時間)。上升與下降**對稱**,各用自己的 R。
        // ⚠ 必須先比大小再相減：兩者都是 unsigned，差值只要方向不對就會下溢成 65535，
        //   於是限斜率每 tick 都以為「還差很遠」而繼續走，一路衝過目標且不會自己回來
        //   (實測 300 tick 後從 12000 爬到 14408，靠重同步才被拉回，波形上是往上的三角尖峰)。
        // ⚠ 下降分支是本次新增的。改動前這裡是 `= u16Target` 直接跳到位(註解寫「目標下修時
        //   直接跟上，不往下慢慢走」),因此減速的斜率參數形同不存在,只有濾波在整形。
        if (su16AccelFilterRateLimited > u16Target) {
            uint16_t u16Room = (uint16_t)(su16AccelFilterRateLimited - u16Target);
            su16AccelFilterRateLimited -= (u16Room > u8Rate) ? u8Rate : u16Room;
        } else {
            uint16_t u16Room = (uint16_t)(u16Target - su16AccelFilterRateLimited);
            su16AccelFilterRateLimited += (u16Room > u8Rate) ? u8Rate : u16Room;
        }
        // 第二級：一階低通 (磨圓折角)。帶小數位元，故不會在小誤差時卡住。
        uint32_t u32TargetShifted = (uint32_t)su16AccelFilterRateLimited << ACCEL_FILTER_FRAC_BITS;
        if (u32TargetShifted >= su32AccelFilterState) {
            su32AccelFilterState += (u32TargetShifted - su32AccelFilterState) >> u8Shift;
        } else {
            su32AccelFilterState -= (su32AccelFilterState - u32TargetShifted) >> u8Shift;
        }
    }

    uint16_t u16Out = ACCEL_FILTER_STATE_TO_CMD(su32AccelFilterState);
    // ⚠ u16MinRpm 地板**只在加速時**套用。減速若套上，命令會被釘在 OUTPUT_MIN
    //   (1500 count = 1.04 km/h) 永遠到不了 0，車停不下來、UVW 短路與 EMB 也永遠鎖不上
    //   (bUvwStopCommanded 要求 ReferenceRAW 恰好為 0)。與段位 0 潛行是同一類錯誤。
    if (!bDecel && (u16Out < u16MinRpm)) {
        u16Out = u16MinRpm;
    }
    if (u16Out > u16MaxRpm) {
        u16Out = u16MaxRpm;
    }

    // 歸零吸附：殺掉一階濾波的漸近尾巴。理由見 s_logic_motor.h 的減速曲線段落 ——
    //   不吸附要多等約 7.6τ(K=4 時約 240ms) 命令才會四捨五入成 0，UVW/EMB 白等這段;
    //   且 K>=10 時整數衰減會卡在 1 count 永遠到不了 0。
    if ((u16TargetRpm == 0u) && (u16Out < pcsDecel->u16SnapToZeroCmd)) {
        u16Out = 0u;
        su16AccelFilterRateLimited = 0u;
        su32AccelFilterState = 0u;
        sbAccelFilterPrimed = false;  // 下次起步重新預載起始速度
    }
#if CODESW_X2C_SCOPE_ENABLE == 1
    g_i16ScopeCmdRateLim = (int16_t)su16AccelFilterRateLimited;  // 限斜率後、濾波前
#endif
    *pu16ActiveRpm = u16Out;
    return 0;
}
#endif  // CODESW_THROTTLE_ACCEL_FILTER_ENABLE

/**
 * @brief (Signed RPM Model) 根據步進參數更新馬達轉速，實現平滑加減速
 * @param pi16ActiveRpm 指向實際轉速的指標 (輸出, 帶正負號)
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param i16CurrentRpm 當前轉速 (帶正負號)
 * @param i16TargetRpm 目標轉速 (帶正負號)
 * @param psStepTime 步進時間參數結構指標
 * @return 0: 成功, -1: 時間未到或指標無效, -2: 時間參數無效, -3: 無需調整
 */
int8_t logic_motor_getUpdateParamsSigned(int16_t *pi16ActiveRpm,
                                         uint32_t u32SystemTimeMs,
                                         int16_t i16CurrentRpm,
                                         int16_t i16TargetRpm,
                                         S_MOTOR_STEP_TIME_T *psStepTime)
{
    static uint32_t u32LastUpdateTime = 0;

    if (psStepTime == NULL || pi16ActiveRpm == NULL)
    {
        return -1; // 指標無效
    }

    if (psStepTime->i8Time <= 0)
    {
        *pi16ActiveRpm = i16CurrentRpm;
        return -2; // 時間參數無效
    }

    // 檢查是否到達更新時間
    if ((u32SystemTimeMs - u32LastUpdateTime) < (uint32_t)psStepTime->i8Time)
    {
        *pi16ActiveRpm = i16CurrentRpm;
        return -1; // 時間未到
    }

    u32LastUpdateTime = u32SystemTimeMs;

    int16_t i16NewRpm = i16CurrentRpm;

    // 判斷加速或減速
    if (i16TargetRpm > i16CurrentRpm) // 加速
    {
        i16NewRpm += psStepTime->i8Step;
        if (i16NewRpm > i16TargetRpm)
        {
            i16NewRpm = i16TargetRpm;
        }
    }
    else if (i16TargetRpm < i16CurrentRpm) // 減速
    {
        i16NewRpm += psStepTime->i8Step; // i8Step 此時應為負值
        if (i16NewRpm < i16TargetRpm)
        {
            i16NewRpm = i16TargetRpm;
        }
    }
    else
    {
        *pi16ActiveRpm = i16CurrentRpm;
        return -3; // 無需調整
    }

    *pi16ActiveRpm = i16NewRpm;
    return 0; // 成功
}