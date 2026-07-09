#ifndef S_LOGIC_BATTERY_H_
#define S_LOGIC_BATTERY_H_

#include <stdbool.h>
#include <stdint.h>

// --- ADC Configuration ---
#define LOGIC_BATTERY_ADC_RESOLUTION_BITS   (12)    // ADC 解析度 (例如: 12-bit)
#define LOGIC_BATTERY_ADC_VREF_MV           (3300)  // ADC 參考電壓 (mV)
#define LOGIC_BATTERY_ADC_MAX_COUNT         ((1 << LOGIC_BATTERY_ADC_RESOLUTION_BITS) - 1) // 例如: 4095 for 12-bit

// --- IIR Low-pass Filter for Display (Slow Fall) ---
#define LOGIC_BATTERY_IIR_FILTER_SHIFT (8) // Scaling factor for fixed-point math (division by 256)
#define LOGIC_BATTERY_IIR_FILTER_ALPHA (3) // Alpha value for the filter. Gives a ~2-3s time constant with 20ms sample time.

// --- [NEW] Over-voltage Protection Timing ---
// Time in milliseconds from startup during which over-voltage is checked.
// After this period, the check is disabled to prevent false positives from back-EMF.
#define LOGIC_BATTERY_STARTUP_OV_CHECK_MS 2000

#define LOGIC_BATTERY_ASYMMETRIC_FILTER_ENABLE (1)    // 1: 啟用非對稱濾波 (電壓上升反應快, 下降反應慢), 0: 傳統對稱濾波

// --- Battery Voltage Divider Resistors (based on schematic) ---
#define LOGIC_BATTERY_VOLTAGE_DIVIDER_R1_KOHM (3631) // R1 in kOhms * 100
#define LOGIC_BATTERY_VOLTAGE_DIVIDER_R2_KOHM (200)  // R2 in kOhms * 100

// --- Battery Thresholds for 24V System (values * 100) ---
#define VOLTAGE_THRESHOLDS_24V_LOW_CUTOFF         (2090)  // 20.90V * 100
#define VOLTAGE_THRESHOLDS_24V_LOW_RECOVERY       (2200)  // 22.00V * 100
#define VOLTAGE_THRESHOLDS_24V_OVER_CUTOFF        (3300)  // 33.00V * 100
#define VOLTAGE_THRESHOLDS_24V_SOC_SEGMENTS       {2100, 2430, 2470, 2500, 2540, 2600, 2660, 2710, 2770, 2840}  // 24V SOC segments * 100
#define VOLTAGE_THRESHOLDS_24V_LED_RED_MIN        (2100)  // 21.00V * 100
#define VOLTAGE_THRESHOLDS_24V_LED_YELLOW_MIN     (2470)  // 24.70V * 100
#define VOLTAGE_THRESHOLDS_24V_LED_GREEN_MIN      (2660)  // 26.60V * 100

// --- Battery Thresholds for 36V System (values * 100) ---
#define VOLTAGE_THRESHOLDS_36V_LOW_CUTOFF         (3190)  // 31.90V * 100
#define VOLTAGE_THRESHOLDS_36V_LOW_RECOVERY       (3300)  // 33.00V * 100
#define VOLTAGE_THRESHOLDS_36V_OVER_CUTOFF        (4600)  // 46.00V * 100
#define VOLTAGE_THRESHOLDS_36V_SOC_SEGMENTS       {3200, 3470, 3530, 3580, 3630, 3720, 3800, 3880, 3950, 4050}  // 36V SOC segments * 100
#define VOLTAGE_THRESHOLDS_36V_LED_RED_MIN        (3200)  // 32.00V * 100
#define VOLTAGE_THRESHOLDS_36V_LED_YELLOW_MIN     (3530)  // 35.30V * 100
#define VOLTAGE_THRESHOLDS_36V_LED_GREEN_MIN      (3800)  // 38.00V * 100

// --- Battery Thresholds for 48V System (values * 100) ---
#define VOLTAGE_THRESHOLDS_48V_LOW_CUTOFF         (4190)  // 41.90V * 100
#define VOLTAGE_THRESHOLDS_48V_LOW_RECOVERY       (4300)  // 43.00V * 100
#define VOLTAGE_THRESHOLDS_48V_OVER_CUTOFF        (6400)  // 64.00V * 100
#define VOLTAGE_THRESHOLDS_48V_SOC_SEGMENTS       {4200, 4510, 4600, 4650, 4710, 4830, 4940, 5040, 5140, 5270}  // 48V SOC segments * 100
#define VOLTAGE_THRESHOLDS_48V_LED_RED_MIN        (4200)  // 42.00V * 100
#define VOLTAGE_THRESHOLDS_48V_LED_YELLOW_MIN     (4600)  // 46.00V * 100
#define VOLTAGE_THRESHOLDS_48V_LED_GREEN_MIN      (4940)  // 49.40V * 100

// --- Battery System Nominal Voltages ---
typedef enum {
    LOGIC_BATTERY_NOMINAL_VOLTAGE_24V = 0,
    LOGIC_BATTERY_NOMINAL_VOLTAGE_36V = 1,
    LOGIC_BATTERY_NOMINAL_VOLTAGE_48V = 2,
    LOGIC_BATTERY_NOMINAL_VOLTAGE_COUNT
} E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T;

// --- Default Battery Configuration Values ---
#define LOGIC_BATTERY_DEFAULT_NOMINAL_VOLTAGE    (LOGIC_BATTERY_NOMINAL_VOLTAGE_24V)  // 預設標稱電壓
#define LOGIC_BATTERY_DEFAULT_CURRENT_VOLTAGE    (0)                                  // 預設當前電壓 * 100
#define LOGIC_BATTERY_DEFAULT_SYSTEM_STATUS      (LOGIC_BATTERY_STATUS_OK)            // 預設系統狀態
#define LOGIC_BATTERY_DEFAULT_SOC_PERCENT        (0)                                  // 預設電量百分比
// --- [NEW] Tick-Down SOC Stability Control ---
// Hysteresis to prevent small flickers around a value.
#define LOGIC_BATTERY_SOC_HYSTERESIS_PERCENT 1 
// The time interval for each 1% drop in the tick-down animation.
// 300ms results in a 10% drop taking 3 seconds.
#define LOGIC_BATTERY_TICK_DOWN_INTERVAL_MS 3000

// --- Battery Status Codes ---
typedef enum {
    LOGIC_BATTERY_STATUS_OK = 0,            // 電池狀態正常
    LOGIC_BATTERY_STATUS_LOW_WARNING,     // 電池電量低警告 (例如接近10%)
    LOGIC_BATTERY_STATUS_LOW_CUTOFF,      // 電池低電壓切斷 (A01)
    LOGIC_BATTERY_STATUS_OVER_VOLTAGE,    // 電池過電壓 (A15)
    LOGIC_BATTERY_STATUS_AWAITING_RECOVERY // 低電壓切斷後，等待恢復電壓
} E_LOGIC_BATTERY_STATUS_T;

// --- LED State ---
typedef enum {
    LOGIC_LED_STATE_OFF = 0,
    LOGIC_LED_STATE_ON,
    LOGIC_LED_STATE_FLASHING
} E_LOGIC_LED_STATE_T;

// --- Structure for 3-LED display status ---
typedef struct {
    E_LOGIC_LED_STATE_T eRedLed;
    E_LOGIC_LED_STATE_T eYellowLed;
    E_LOGIC_LED_STATE_T eGreenLed;
} S_LOGIC_BATTERY_LED_STATUS_T;

// --- Structure to hold all battery related information ---
typedef struct {
    E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T eNominalVoltage; // 當前設定的系統標稱電壓
    uint16_t u16CurrentVoltage;                     // 當前偵測到的電池電壓 * 100
    E_LOGIC_BATTERY_STATUS_T eSystemStatus;         // 當前電池系統狀態
    uint8_t u8StateOfChargePercent;                 // 剩餘電量百分比 (0-100%)
    S_LOGIC_BATTERY_LED_STATUS_T sLedStatus;        // 三色LED狀態
    bool bProhibitOutput;                           // 是否應禁止馬達輸出
} S_LOGIC_BATTERY_INFO_T;

// --- Function Prototypes ---

/**
 * @brief 初始化電池管理模組
 * @param eInitialNominalVoltage 系統初始的標稱電壓 (24V, 36V, or 48V)
 */
void logic_battery_init(E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T eInitialNominalVoltage);

/**
 * @brief 設定系統的標稱電池電壓
 * @param eNominalVoltage 要設定的標稱電壓
 */
void logic_battery_setNominalVoltage(E_LOGIC_BATTERY_NOMINAL_VOLTAGE_T eNominalVoltage);

/**
 * @brief 根據當前 ADC 偵測到的電池分壓後的原始計數值更新電池狀態
 * @param u16MeasuredAdcCount 從 ADC 讀取的 VBATT 接腳的原始計數值
 */
void logic_battery_updateVoltage(uint16_t u16MeasuredAdcCount);

/**
 * @brief 取得當前計算出的實際電池電壓 (V+)
 * @return uint16_t 實際電池電壓 * 100
 */
uint16_t logic_battery_getActualVoltage(void);

/**
 * @brief 直接從分壓器電壓計算電池實際電壓
 * @param u16DividerVoltage 分壓器電壓值 (ADC 讀數)
 * @return uint16_t 計算出的電池實際電壓 * 100
 */
uint16_t logic_battery_calculateVoltageFromDivider(uint16_t u16DividerVoltage);

/**
 * @brief 取得當前完整的電池資訊
 * @return S_LOGIC_BATTERY_INFO_T 包含所有電池相關狀態的結構
 */
S_LOGIC_BATTERY_INFO_T logic_battery_getInfo(void);

/**
 * @brief 取得當前電池的系統狀態 (OK, LOW_CUTOFF, OVER_VOLTAGE, etc.)
 * @return E_LOGIC_BATTERY_STATUS_T 當前系統狀態碼
 */
E_LOGIC_BATTERY_STATUS_T logic_battery_getSystemStatus(void);

/**
 * @brief 取得當前電池的剩餘電量百分比 (SOC)
 * @return uint8_t SOC百分比 (0-100)
 */
uint8_t logic_battery_getSOCPercent(void);

/**
 * @brief 取得當前三色LED的應有狀態
 * @return S_LOGIC_BATTERY_LED_STATUS_T 包含紅黃綠LED狀態的結構
 */
S_LOGIC_BATTERY_LED_STATUS_T logic_battery_getLedStatus(void);

/**
 * @brief 檢查是否因為電池狀態 (低電壓/過電壓) 而應禁止馬達輸出
 * @return true 如果應禁止輸出, false 則否
 */
bool logic_battery_shouldProhibitOutput(void);

/**
 * @brief [DEBUG] 取得最近一次的瞬時電壓值
 * @return uint16_t 瞬時電壓 * 100
 */
uint16_t logic_battery_getInstantVoltage(void);

/**
 * @brief [DEBUG] 取得用於保護判斷的電壓值
 * @return uint16_t 用於判斷的電壓 * 100
 */
uint16_t logic_battery_getVoltageForCheck(void);

#endif // S_LOGIC_BATTERY_H_