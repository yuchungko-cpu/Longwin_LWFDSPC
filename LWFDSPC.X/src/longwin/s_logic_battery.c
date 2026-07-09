#include "s_logic_battery.h"
#include "s_logic_error_handler.h" // Added for error code integration
#include <stdlib.h>                // For abs()
#include <stddef.h>                // For NULL if ever needed, not strictly for this C file now
#include <stdint.h>

// --- 定點數定義 (使用 16 位元整數表示電壓，單位為 0.01V) ---
#define VOLTAGE_SCALE_FACTOR    100     // 電壓放大倍數 (0.01V 精度)
#define VOLTAGE_MAX_VALUE       10000   // 最大電壓值 (100.00V)
#define VOLTAGE_MIN_VALUE       0       // 最小電壓值 (0.00V)

// --- 結構體定義 (使用整數替代浮點數) ---
typedef struct
{
    uint16_t u16LowVoltageCutoff;   // 低電壓切斷值 (A01) * 100
    uint16_t u16LowVoltageRecovery; // 低電壓恢復值 * 100
    uint16_t u16OverVoltageCutoff;  // 過電壓切斷值 (A15) * 100
    // SOC 10-segment thresholds (upper bounds for each 10% step)
    uint16_t u16Soc10SegmentThresholds[10]; // For 10%, 20%, ..., 100% SOC levels * 100
    // 3-LED display thresholds
    uint16_t u16LedRedMin;    // 紅燈亮起的最低電壓 * 100
    uint16_t u16LedYellowMin; // 黃燈亮起的最低電壓 * 100
    uint16_t u16LedGreenMin;  // 綠燈亮起的最低電壓 * 100
} S_BATTERY_VOLTAGE_THRESHOLDS_INT_T;

// Define thresholds for 24V, 36V, 48V systems using #define values
static const S_BATTERY_VOLTAGE_THRESHOLDS_INT_T scarst_voltageThresholdsInt[LOGIC_BATTERY_NOMINAL_VOLTAGE_COUNT] = {
    // 24V 電池閾值 (使用 #define 值)
    {
        .u16LowVoltageCutoff = VOLTAGE_THRESHOLDS_24V_LOW_CUTOFF,
        .u16LowVoltageRecovery = VOLTAGE_THRESHOLDS_24V_LOW_RECOVERY,
        .u16OverVoltageCutoff = VOLTAGE_THRESHOLDS_24V_OVER_CUTOFF,
        .u16Soc10SegmentThresholds = VOLTAGE_THRESHOLDS_24V_SOC_SEGMENTS,
        .u16LedRedMin = VOLTAGE_THRESHOLDS_24V_LED_RED_MIN,
        .u16LedYellowMin = VOLTAGE_THRESHOLDS_24V_LED_YELLOW_MIN,
        .u16LedGreenMin = VOLTAGE_THRESHOLDS_24V_LED_GREEN_MIN,
    },
    // 36V 電池閾值 (使用 #define 值)
    {
        .u16LowVoltageCutoff = VOLTAGE_THRESHOLDS_36V_LOW_CUTOFF,
        .u16LowVoltageRecovery = VOLTAGE_THRESHOLDS_36V_LOW_RECOVERY,
        .u16OverVoltageCutoff = VOLTAGE_THRESHOLDS_36V_OVER_CUTOFF,
        .u16Soc10SegmentThresholds = VOLTAGE_THRESHOLDS_36V_SOC_SEGMENTS,
        .u16LedRedMin = VOLTAGE_THRESHOLDS_36V_LED_RED_MIN,
        .u16LedYellowMin = VOLTAGE_THRESHOLDS_36V_LED_YELLOW_MIN,
        .u16LedGreenMin = VOLTAGE_THRESHOLDS_36V_LED_GREEN_MIN,
    },
    // 48V 電池閾值 (使用 #define 值)
    {
        .u16LowVoltageCutoff = VOLTAGE_THRESHOLDS_48V_LOW_CUTOFF,
        .u16LowVoltageRecovery = VOLTAGE_THRESHOLDS_48V_LOW_RECOVERY,
        .u16OverVoltageCutoff = VOLTAGE_THRESHOLDS_48V_OVER_CUTOFF,
        .u16Soc10SegmentThresholds = VOLTAGE_THRESHOLDS_48V_SOC_SEGMENTS,
        .u16LedRedMin = VOLTAGE_THRESHOLDS_48V_LED_RED_MIN,
        .u16LedYellowMin = VOLTAGE_THRESHOLDS_48V_LED_YELLOW_MIN,
        .u16LedGreenMin = VOLTAGE_THRESHOLDS_48V_LED_GREEN_MIN,
    }
};

// --- Static global variable to hold the current battery information ---
static S_LOGIC_BATTERY_INFO_T sst_currentBatteryInfo;

// --- IIR 低通濾波器相關變數 ---
static uint32_t s_u32FilteredVoltage_scaled = 0; // Scaled by 256 (<< 8) for fixed-point math
static bool s_bFilterInitialized = false;

// --- [DEBUG] 變數，用於外部獲取內部狀態 ---
static uint16_t s_u16DebugInstantVoltage = 0;
static uint16_t s_u16DebugVoltageForCheck = 0;

// --- 整數版本的電壓閾值表 ---
static uint16_t _logicBattery_calculateActualBatteryVoltageInt(uint16_t u16AdcCount)
{
    if (LOGIC_BATTERY_VOLTAGE_DIVIDER_R2_KOHM == 0 || LOGIC_BATTERY_ADC_MAX_COUNT == 0)
    {
        return 0; // 避免除零
    }

    // 步驟 1: 將 ADC 原始計數值轉換為分壓點的電壓 (單位: mV)
    // 公式: Divider Voltage (mV) = (ADC Count / ADC Max Count) * Vref (mV)
    uint64_t u64DividerVoltageMv = ((uint64_t)u16AdcCount * LOGIC_BATTERY_ADC_VREF_MV) / LOGIC_BATTERY_ADC_MAX_COUNT;

    // 步驟 2: 將分壓點電壓反推回實際電池電壓
    // 公式: Actual Voltage = Divider Voltage * (R1 + R2) / R2
    uint64_t u64TotalResistance = (uint64_t)LOGIC_BATTERY_VOLTAGE_DIVIDER_R1_KOHM + LOGIC_BATTERY_VOLTAGE_DIVIDER_R2_KOHM;
    uint64_t u64ActualVoltageMv = (u64DividerVoltageMv * u64TotalResistance) / LOGIC_BATTERY_VOLTAGE_DIVIDER_R2_KOHM;

    // 返回以 0.01V 為單位的電壓值 (即 mV / 10)
    return (uint16_t)(u64ActualVoltageMv / 10);
}

// --- 輔助函數：更新電池計算 (整數版本) ---
static void _logicBattery_updateBatteryCalculationsInt(void)
{
    const S_BATTERY_VOLTAGE_THRESHOLDS_INT_T *pcstThresholds = &scarst_voltageThresholdsInt[sst_currentBatteryInfo.eNominalVoltage];
    uint16_t u16CurrentVoltageInt = sst_currentBatteryInfo.u16CurrentVoltage;

    // SOC 計算 (基於 10 段閾值)
    // --- [NEW] Tick-Down Animation Logic ---

    // 1. Calculate raw SOC from current voltage
    uint8_t u8RawSocPercent = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (u16CurrentVoltageInt >= pcstThresholds->u16Soc10SegmentThresholds[i])
        {
            u8RawSocPercent = (i + 1) * 10;
        }
    }
    if (u16CurrentVoltageInt >= pcstThresholds->u16Soc10SegmentThresholds[9])
    {
        u8RawSocPercent = 100;
    }
    if (u16CurrentVoltageInt < pcstThresholds->u16Soc10SegmentThresholds[0] && 
        u16CurrentVoltageInt > pcstThresholds->u16LowVoltageCutoff)
    {
        u8RawSocPercent = 5; // Between 0-10%
    }

    // 2. Apply tick-down animation logic
    static bool s_bIsTickingDown = false;
    static uint8_t s_u8TargetSoc = 0;
    static uint32_t s_u32LastTickTime = 0;

    extern uint32_t getSystemTimeMs(void); // Get time from main.c
    uint32_t u32CurrentTimeMs = getSystemTimeMs();
    int16_t i16SocDifference = u8RawSocPercent - sst_currentBatteryInfo.u8StateOfChargePercent;

    if (i16SocDifference > LOGIC_BATTERY_SOC_HYSTERESIS_PERCENT) {
        // SOC is rising, update immediately and cancel any tick-down.
        sst_currentBatteryInfo.u8StateOfChargePercent = u8RawSocPercent;
        s_bIsTickingDown = false;
    } else if (i16SocDifference < -LOGIC_BATTERY_SOC_HYSTERESIS_PERCENT) {
        // SOC is dropping, start or update the tick-down process.
        if (!s_bIsTickingDown) {
            // A new drop is detected, start the process.
            s_bIsTickingDown = true;
            s_u8TargetSoc = u8RawSocPercent;
            s_u32LastTickTime = u32CurrentTimeMs;
        } else {
            // A drop is already in progress, update the target if it drops further.
            if (u8RawSocPercent < s_u8TargetSoc) {
                s_u8TargetSoc = u8RawSocPercent;
            }
        }
    }

    // 3. Execute the tick-down if it's active
    if (s_bIsTickingDown) {
        if (sst_currentBatteryInfo.u8StateOfChargePercent > s_u8TargetSoc) {
            if ((u32CurrentTimeMs - s_u32LastTickTime) >= LOGIC_BATTERY_TICK_DOWN_INTERVAL_MS) {
                sst_currentBatteryInfo.u8StateOfChargePercent--;
                s_u32LastTickTime = u32CurrentTimeMs;
            }
        } else {
            // Target reached, stop ticking down.
            s_bIsTickingDown = false;
        }
    }

    // LED 狀態計算 (基於 3-LED 閾值)
    sst_currentBatteryInfo.sLedStatus.eRedLed = LOGIC_LED_STATE_OFF;
    sst_currentBatteryInfo.sLedStatus.eYellowLed = LOGIC_LED_STATE_OFF;
    sst_currentBatteryInfo.sLedStatus.eGreenLed = LOGIC_LED_STATE_OFF;

    if (sst_currentBatteryInfo.eSystemStatus == LOGIC_BATTERY_STATUS_LOW_CUTOFF ||
        sst_currentBatteryInfo.eSystemStatus == LOGIC_BATTERY_STATUS_OVER_VOLTAGE)
    {
        sst_currentBatteryInfo.sLedStatus.eRedLed = LOGIC_LED_STATE_FLASHING;
    }
    else
    {
        // --- [NEW] Bar-graph style display logic ---
        if (u16CurrentVoltageInt >= pcstThresholds->u16LedGreenMin) {
            sst_currentBatteryInfo.sLedStatus.eGreenLed = LOGIC_LED_STATE_ON;
        }
        if (u16CurrentVoltageInt >= pcstThresholds->u16LedYellowMin) {
            sst_currentBatteryInfo.sLedStatus.eYellowLed = LOGIC_LED_STATE_ON;
        }
        if (u16CurrentVoltageInt >= pcstThresholds->u16LedRedMin) {
            sst_currentBatteryInfo.sLedStatus.eRedLed = LOGIC_LED_STATE_ON;
        }
        // If voltage is below even the red threshold but not yet in cutoff, keep red on.
        else if (u16CurrentVoltageInt > pcstThresholds->u16LowVoltageCutoff)
        {
             sst_currentBatteryInfo.sLedStatus.eRedLed = LOGIC_LED_STATE_ON;
        }
    }
}

// --- Function Implementations ---
void logic_battery_init(E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T eInitialNominalVoltage)
{
    sst_currentBatteryInfo.eNominalVoltage = eInitialNominalVoltage;
    sst_currentBatteryInfo.u16CurrentVoltage = LOGIC_BATTERY_DEFAULT_CURRENT_VOLTAGE;
    sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_DEFAULT_SYSTEM_STATUS;
    sst_currentBatteryInfo.u8StateOfChargePercent = LOGIC_BATTERY_DEFAULT_SOC_PERCENT;
    sst_currentBatteryInfo.bProhibitOutput = false;
    
    // 初始化 LED 狀態
    sst_currentBatteryInfo.sLedStatus.eRedLed = LOGIC_LED_STATE_OFF;
    sst_currentBatteryInfo.sLedStatus.eYellowLed = LOGIC_LED_STATE_OFF;
    sst_currentBatteryInfo.sLedStatus.eGreenLed = LOGIC_LED_STATE_OFF;

    // 初始化IIR濾波器
    s_u32FilteredVoltage_scaled = 0;
    s_bFilterInitialized = false;
    
    _logicBattery_updateBatteryCalculationsInt();
    
    // 清除任何預存的電池相關警報
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, false);
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE, false);
}

// --- Function to set the nominal voltage of the battery ---
void logic_battery_setNominalVoltage(E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T eNominalVoltage)
{
    if (eNominalVoltage < LOGIC_BATTERY_NOMINAL_VOLTAGE_COUNT)
    {
        sst_currentBatteryInfo.eNominalVoltage = eNominalVoltage;
        
        // 重置濾波器以反映新的電壓標準
        s_bFilterInitialized = false;
        
        // 重新計算當前電壓
        uint16_t u16CurrentVoltageInt = sst_currentBatteryInfo.u16CurrentVoltage;
        uint32_t u32VoltageRatio = (uint32_t)LOGIC_BATTERY_VOLTAGE_DIVIDER_R1_KOHM * 100 / LOGIC_BATTERY_VOLTAGE_DIVIDER_R2_KOHM;
        uint16_t u16AdcVoltageEquivalent = (uint16_t)(u16CurrentVoltageInt * 100 / (100 + u32VoltageRatio));
        
        logic_battery_updateVoltage(u16AdcVoltageEquivalent);
    }
}

// --- Function to update the battery voltage and status based on new ADC reading ---
void logic_battery_updateVoltage(uint16_t u16MeasuredAdcCount)
{
    // 步驟 1: 計算瞬時電壓
    uint16_t u16InstantVoltage = _logicBattery_calculateActualBatteryVoltageInt(u16MeasuredAdcCount);
    uint16_t u16FilteredVoltage;

    s_u16DebugInstantVoltage = u16InstantVoltage; // [DEBUG] 儲存瞬時電壓

    // 步驟 2: 更新 IIR 低通濾波器，產生平滑的電壓值
    if (!s_bFilterInitialized) {
        // 首次初始化，直接使用瞬時電壓
        s_u32FilteredVoltage_scaled = (uint32_t)u16InstantVoltage << LOGIC_BATTERY_IIR_FILTER_SHIFT;
        s_bFilterInitialized = true;
    } else {
        // 執行 IIR 濾波
        // new_filt = old_filt + alpha * (new_inst - old_filt)
        // 使用定點數運算: new_filt_scaled = old_filt_scaled + alpha * (new_inst_scaled - old_filt_scaled) / 256
        int32_t s32Error_scaled = ((int32_t)u16InstantVoltage << LOGIC_BATTERY_IIR_FILTER_SHIFT) - s_u32FilteredVoltage_scaled;
        s_u32FilteredVoltage_scaled += (s32Error_scaled * LOGIC_BATTERY_IIR_FILTER_ALPHA) >> LOGIC_BATTERY_IIR_FILTER_SHIFT;
    }
    u16FilteredVoltage = (uint16_t)(s_u32FilteredVoltage_scaled >> LOGIC_BATTERY_IIR_FILTER_SHIFT);
    
    // 步驟 3: 根據使用者需求，分離「顯示」與「保護」的電壓邏輯

    // 3.1: 顯示邏輯 (SOC/LED) - 採用「上升快，下降慢」的非對稱濾波
    uint16_t u16DisplayVoltage;
    if (u16InstantVoltage > u16FilteredVoltage) {
        // 電壓上升，顯示應立即反應
        u16DisplayVoltage = u16InstantVoltage;
    } else {
        // 電壓下降，顯示應緩慢變化 (使用IIR濾波後的值)
        u16DisplayVoltage = u16FilteredVoltage;
    }
    // 將最終用於顯示的電壓存入全局結構
    sst_currentBatteryInfo.u16CurrentVoltage = u16DisplayVoltage;

    // 3.2: 保護邏輯 (過壓/欠壓) - 採用最即時的電壓
    uint16_t u16VoltageForCheck = u16InstantVoltage;
    s_u16DebugVoltageForCheck = u16VoltageForCheck; // [DEBUG] 更新用於檢查的電壓

    // 步驟 4: 使用選擇出的電壓值 (u16VoltageForCheck) 進行保護邏輯判斷
    const S_BATTERY_VOLTAGE_THRESHOLDS_INT_T *pcstThresholds = &scarst_voltageThresholdsInt[sst_currentBatteryInfo.eNominalVoltage];
    E_LOGIC_BATTERY_STATUS_T previousStatus = sst_currentBatteryInfo.eSystemStatus;
    extern uint32_t getSystemTimeMs(void);
    uint32_t u32CurrentTimeMs = getSystemTimeMs();

    // --- Over-voltage Check (Startup Only) ---
    // To prevent false positives from back-EMF, over-voltage is only checked for a few seconds at startup.
    // If an over-voltage event occurs, the error is latched and requires a power cycle to clear.
    if (u32CurrentTimeMs < LOGIC_BATTERY_STARTUP_OV_CHECK_MS)
    {
        if (u16VoltageForCheck >= pcstThresholds->u16OverVoltageCutoff)
        {
            sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_OVER_VOLTAGE;
            sst_currentBatteryInfo.bProhibitOutput = true;
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE, true);
        }
    }

    // --- State Machine for Battery Status ---
    if (sst_currentBatteryInfo.eSystemStatus == LOGIC_BATTERY_STATUS_OVER_VOLTAGE)
    {
        // Over-voltage is a latched state. Do nothing, requires power cycle to clear.
        // Ensure under-voltage alarm is not active.
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, false);
    }
    else if (u16VoltageForCheck <= pcstThresholds->u16LowVoltageCutoff)
    {
        // Under-voltage condition met.
        sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_LOW_CUTOFF;
        sst_currentBatteryInfo.bProhibitOutput = true;
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, true);
    }
    else if (previousStatus == LOGIC_BATTERY_STATUS_LOW_CUTOFF || previousStatus == LOGIC_BATTERY_STATUS_AWAITING_RECOVERY)
    {
        // In an under-voltage state, check for recovery.
        if (u16VoltageForCheck >= pcstThresholds->u16LowVoltageRecovery)
        {
            sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_OK;
            sst_currentBatteryInfo.bProhibitOutput = false;
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, false);
        }
        else
        {
            // Voltage is still low, remain in recovery state.
            sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_AWAITING_RECOVERY;
            sst_currentBatteryInfo.bProhibitOutput = true;
        }
    }
    else
    {
        // No fault conditions are active.
        sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_OK;
        sst_currentBatteryInfo.bProhibitOutput = false;
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, false);
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE, false); // Clear any stale OV alarm

        // Check if we should enter the low voltage warning state.
        if (u16VoltageForCheck < pcstThresholds->u16Soc10SegmentThresholds[0])
        {
            sst_currentBatteryInfo.eSystemStatus = LOGIC_BATTERY_STATUS_LOW_WARNING;
        }
    }

    _logicBattery_updateBatteryCalculationsInt();
}

/**
 * @brief 取得當前計算出的實際電池電壓 (V+)
 * @return uint16_t 實際電池電壓 * 100 (單位為 0.01V)
 * @note 此函式回傳的電壓值是從分壓器電壓計算出來的電池實際電壓
 *       計算公式：電池電壓 = 分壓器電壓 × (R1 + R2) / R2
 * 
 * @example
 * // 取得當前電池電壓
 * uint16_t u16BatteryVoltage = logic_battery_getActualVoltage();
 * // 轉換為實際電壓值 (V)
 * float fActualVoltage = (float)u16BatteryVoltage / 100.0f;
 * // 例如：u16BatteryVoltage = 4800 表示 48.00V
 */
uint16_t logic_battery_getActualVoltage(void)
{
    return sst_currentBatteryInfo.u16CurrentVoltage;
}

/**
 * @brief 直接從分壓器電壓計算電池實際電壓
 * @param u16DividerVoltage 分壓器電壓值 (ADC 讀數)
 * @return uint16_t 計算出的電池實際電壓 * 100 (單位為 0.01V)
 * @note 此函式直接計算，不更新內部狀態，適用於即時計算需求
 *       計算公式：電池電壓 = 分壓器電壓 × (R1 + R2) / R2
 * 
 * @example
 * // 直接從分壓器電壓計算電池電壓
 * uint16_t u16DividerVoltage = 1200; // 分壓器電壓 12.00V
 * uint16_t u16BatteryVoltage = logic_battery_calculateVoltageFromDivider(u16DividerVoltage);
 * // u16BatteryVoltage 現在包含計算出的電池實際電壓
 */
uint16_t logic_battery_calculateVoltageFromDivider(uint16_t u16DividerVoltage)
{
    return _logicBattery_calculateActualBatteryVoltageInt(u16DividerVoltage);
}

/**
 * @brief 取得當前完整的電池資訊
 * @return S_LOGIC_BATTERY_INFO_T 包含所有電池相關狀態的結構
 * @note 此函式回傳完整的電池資訊，包含電壓、SOC、LED狀態、系統狀態等
 * 
 * @example
 * // 取得完整電池資訊
 * S_LOGIC_BATTERY_INFO_T stBatteryInfo = logic_battery_getInfo();
 * // 存取各個成員
 * uint16_t u16Voltage = stBatteryInfo.u16CurrentVoltage;
 * uint8_t u8SOC = stBatteryInfo.u8StateOfChargePercent;
 */
S_LOGIC_BATTERY_INFO_T logic_battery_getInfo(void)
{
    return sst_currentBatteryInfo;
}

/**
 * @brief 取得當前電池的系統狀態
 * @return E_LOGIC_BATTERY_STATUS_T 當前系統狀態碼
 * @note 狀態包括：正常、低電壓警告、低電壓切斷、過電壓、等待恢復等
 * 
 * @example
 * // 檢查電池系統狀態
 * E_LOGIC_BATTERY_STATUS_T eStatus = logic_battery_getSystemStatus();
 * if (eStatus == LOGIC_BATTERY_STATUS_OK) {
 *     // 電池狀態正常
 * }
 */
E_LOGIC_BATTERY_STATUS_T logic_battery_getSystemStatus(void)
{
    return sst_currentBatteryInfo.eSystemStatus;
}

/**
 * @brief 取得當前電池的剩餘電量百分比 (SOC)
 * @return uint8_t SOC百分比 (0-100)
 * @note 此值基於電壓閾值計算，提供 10 段電量顯示
 * 
 * @example
 * // 取得電池電量百分比
 * uint8_t u8SOC = logic_battery_getSOCPercent();
 * if (u8SOC < 20) {
 *     // 電量低於 20%，需要充電
 * }
 */
uint8_t logic_battery_getSOCPercent(void)
{
    return sst_currentBatteryInfo.u8StateOfChargePercent;
}

/**
 * @brief 取得當前三色LED的應有狀態
 * @return S_LOGIC_BATTERY_LED_STATUS_T 包含紅黃綠LED狀態的結構
 * @note LED狀態基於電壓閾值自動計算，包括關閉、點亮、閃爍等狀態
 * 
 * @example
 * // 取得LED狀態
 * S_LOGIC_BATTERY_LED_STATUS_T stLedStatus = logic_battery_getLedStatus();
 * if (stLedStatus.eGreenLed == LOGIC_LED_STATE_ON) {
 *     // 綠燈點亮，電量充足
 * }
 */
S_LOGIC_BATTERY_LED_STATUS_T logic_battery_getLedStatus(void)
{
    return sst_currentBatteryInfo.sLedStatus;
}

/**
 * @brief 檢查是否因為電池狀態而應禁止馬達輸出
 * @return true 如果應禁止輸出, false 則否
 * @note 當電池電壓過低或過高時，系統會自動禁止馬達輸出以保護電池
 * 
 * @example
 * // 檢查是否應禁止馬達輸出
 * if (logic_battery_shouldProhibitOutput()) {
 *     // 禁止馬達輸出，電池狀態異常
 *     motor_disable();
 * }
 */
bool logic_battery_shouldProhibitOutput(void)
{
    return sst_currentBatteryInfo.bProhibitOutput;
}


uint16_t logic_battery_getInstantVoltage(void)
{
    return s_u16DebugInstantVoltage;
}

uint16_t logic_battery_getVoltageForCheck(void)
{
    return s_u16DebugVoltageForCheck;
}