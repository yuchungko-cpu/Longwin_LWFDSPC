#ifndef S_LOGIC_TEMP_MOTOR_H
#define S_LOGIC_TEMP_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

// --- 枚舉定義：馬達溫度保護狀態 ---
typedef enum
{
    E_LOGIC_MOTOR_TEMP_STATUS_NORMAL = 0,       // 正常狀態
    E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_PENDING, // 高溫待確認 (連續計數中)
    E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE,  // 高溫保護已啟動 (馬達應禁止輸出)
    E_LOGIC_MOTOR_TEMP_STATUS_RECOVERY_PENDING  // 溫度恢復待確認 (連續計數中)
} E_LOGIC_MOTOR_TEMP_STATUS_T;

// --- 常數定義 (來自 longwinConrtrolFunction_chunchi.md) ---

// 馬達NTC感測器相關
// 硬體PIN定義通常在HAL層或設定檔。例如：
// #define S_LOGIC_MOTOR_TEMP_NTC_ADC_CHANNEL (ADC_CHANNEL_X)

// NTC電路相關電阻值 (用於可能的電壓-電阻轉換，但目前函式直接接收溫度)
#define LOGIC_MOTOR_TEMP_NTC_PULLUP_RESISTOR_OHM (560U)   // 假設為NTC的上拉電阻或其他固定電阻
#define LOGIC_MOTOR_TEMP_NTC_SERIES_RESISTOR_OHM (10000U) // 假設為NTC串聯的固定電阻 (或NTC在某溫度下的參考電阻)
                                                        // 這些值的具體用途取決於電路設計和溫度轉換方式

// NTC原始信號有效性 (如果直接處理電壓，這類檢查應在轉換為溫度前提早進行)
// #define S_LOGIC_MOTOR_TEMP_NTC_MIN_VALID_VOLTAGE_MV (1600U) // 示例：低於此電壓可能表示感測器故障或斷線

// 馬達溫度閾值與計數
#define LOGIC_MOTOR_TEMP_OVERHEAT_REQUIRED_READINGS (6U)   // 觸發過熱保護所需的連續超溫讀取次數
#define LOGIC_MOTOR_TEMP_OVERHEAT_THRESHOLD_C (12000)      // 過熱保護溫度閾值 * 100 (120°C)
// #define S_LOGIC_MOTOR_TEMP_OVERHEAT_THRESHOLD_V    (1700U) // 對應電壓 (mV) - 備註：若函式直接用溫度，此電壓值僅供參考或外部轉換使用

#define LOGIC_MOTOR_TEMP_RECOVERY_REQUIRED_READINGS (6U) // 解除過熱保護所需的連續正常溫度讀取次數
#define LOGIC_MOTOR_TEMP_RECOVERY_THRESHOLD_C (9500)       // 恢復溫度閾值 * 100 (95°C)
// #define S_LOGIC_MOTOR_TEMP_RECOVERY_THRESHOLD_V    (1800U) // 對應電壓 (mV) - 備註：若函式直接用溫度，此電壓值僅供參考或外部轉換使用

#define LOGIC_MOTOR_TEMP_HYSTERESIS_C (2500) // 溫度遲滯值 * 100 (25°C)，用於恢復點計算 (120-95=25)

// 通信代碼 (顯示用)
#define LOGIC_MOTOR_TEMP_COMM_CODE_OVERHEAT_ACTIVE "A20" // 過熱保護啟動時的通信碼

// --- 函數原型 ---

/**
 * @brief 初始化馬達溫度保護邏輯模組。
 *        應在系統啟動時調用一次，以設定初始狀態為 E_LOGIC_MOTOR_TEMP_STATUS_NORMAL。
 * @example
 * \code
 * logic_motorTemp_init();
 * \endcode
 */
void logic_motorTemp_init(void);

/**
 * @brief 更新馬達溫度保護狀態。
 *
 * 此函式應定期調用，例如每 100ms 一次。
 * 它會根據輸入的馬達溫度更新內部的狀態機，並判斷是否需要啟動或解除馬達過熱保護，
 * 同時會通過 s_logic_error_handler 設定或清除 LOGIC_ALARM_A20_MOTOR_OVER_TEMP 警報。
 *
 * @param u32MotorTempC 馬達的目前溫度 * 100 (單位：攝氏度)。
 * @return E_LOGIC_MOTOR_TEMP_STATUS_T 目前的馬達溫度保護狀態。
 *         若返回 E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE，則表示馬達過熱，應禁止輸出。
 * @note  實際的 ADC 讀取及轉換為溫度的部分需在此函式外部處理。
 * @example
 * \code
 * uint32_t current_motor_temp = get_motor_temperature_c() * 100;
 * E_LOGIC_MOTOR_TEMP_STATUS_T motor_status = logic_motorTemp_update(current_motor_temp);
 *
 * if (motor_status == E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE) {
 *     // Motor is overheated, disable motor output (A20 alarm is active).
 * } else {
 *     // Motor temperature is normal or recovering, allow normal operation (A20 alarm is inactive).
 * }
 * \endcode
 */
E_LOGIC_MOTOR_TEMP_STATUS_T logic_motorTemp_update(uint32_t u32MotorTempC);

/**
 * @brief 獲取當前馬達溫度保護的狀態。
 *
 * @return E_LOGIC_MOTOR_TEMP_STATUS_T 當前的馬達溫度保護狀態枚舉值。
 * @example
 * \code
 * E_LOGIC_MOTOR_TEMP_STATUS_T current_status = logic_motorTemp_getStatus();
 * printf("Current motor temperature status: %d\n", current_status);
 * \endcode
 */
E_LOGIC_MOTOR_TEMP_STATUS_T logic_motorTemp_getStatus(void);

#endif // S_LOGIC_MOTOR_TEMP_H