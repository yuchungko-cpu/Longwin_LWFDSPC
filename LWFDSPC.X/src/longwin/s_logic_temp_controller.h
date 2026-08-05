/**
 * @file s_logic_temp_controller.h
 * @brief 控制器溫度保護模組
 *
 * @details
 * 此模組負責：
 * 1. 根據 NTC 溫度感測器的 ADC 值，透過查表法換算出控制器溫度。
 * 2. 實作一個包含遲滯效應 (Hysteresis) 的多段式狀態機，來決定當前的溫度保護區間。
 * 3. 根據所在的溫度區間，輸出一个對應的 Q15 格式電流限制比例。
 *    - 1.0 (Q15(1.0)) 表示 100% 無限制。
 *    - 0.0 (Q15(0.0)) 表示 0%，完全停止輸出。
 */

#ifndef S_LOGIC_TEMP_CONTROLLER_H_
#define S_LOGIC_TEMP_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


// --- NTC 溫度轉換查表定義 ---

/**
 * @brief ADC-溫度對照表中的一個數據點
 */
typedef struct {
    uint16_t adc_value;  // ADC 原始讀值
    int16_t temp_c_100;  // 對應的溫度 * 100 (例如 2500 代表 25.00°C)
} temp_lut_entry_t;

/**
 * @brief NTC 溫度對照表 (ADC -> 溫度)
 * @note
 * - 根據 NTC 電阻值，以及 "VCC -> 3.3kΩ -> ADC -> NTC -> GND" 的上拉電阻電路計算得出。
 * - 表格必須按 ADC 值升序排列 (即溫度降序)。
 * - 定義為 static const 以避免多重定義的連結錯誤，並放入唯讀記憶體。
 */
static const temp_lut_entry_t temp_lookup_table[] = {
    {931, 10000},
    {1128, 9000},
    {1374, 8000},
    {1651, 7000},
    {1956, 6000},
    {2316, 5000},
    {2481, 4500},
    {2639, 4000},
    {2791, 3500},
    {2937, 3000},
    {3078, 2500},
    {3216, 2000},
    {3438, 1000},
    {3653, 0},
    {3803, -1000},
    {3904, -2000}};
static const size_t temp_lookup_table_size = sizeof(temp_lookup_table) / sizeof(temp_lut_entry_t);

// --- 枚舉定義：溫度控制區間狀態 ---
/**
 * @brief 定義了不同的溫度保護等級區間
 */
typedef enum {
    LOGIC_TEMP_CONTROLLER_ZONE_NORMAL = 0,  // 正常區間，不進行任何限制
    LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_1,     // 第一級限流區間
    LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_2,     // 第二級限流區間
    LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_3,     // 第三級限流區間
    LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_4,     // 第四級限流區間
    LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_5,     // 第五級限流區間
    LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP     // 過溫區間，完全停止輸出
} E_LOGIC_TEMP_CONTROLLER_ZONE_T;

// --- 常數定義 ---

/**
 * @brief 遲滯溫度值 (Hysteresis)
 * @details 用於防止溫度在閾值邊緣跳動時，狀態機頻繁切換。
 *          例如，60°C 觸發限流，則需要降溫到 58°C (60-2) 才會解除。
 */
#define LOGIC_TEMP_CONTROLLER_HYSTERESIS_C (200)  // 單位: °C * 100

// --- 激活限流的溫度閾值 (溫度上升時觸發) * 100 ---
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_1_C (5300)   // 53.00°C
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_2_C (5800)   // 58.00°C
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_3_C (6300)   // 63.00°C
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_4_C (6800)   // 68.00°C
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_5_C (7300)   // 73.00°C
#define LOGIC_TEMP_CONTROLLER_ACTIVATE_OVERTEMP_C (7800)  // 78.00°C

// --- 解除限流的溫度閾值 (溫度下降時觸發) * 100 ---
#define LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_1_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_1_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)
#define LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_2_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_2_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)
#define LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_3_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_3_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)
#define LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_4_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_4_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)
#define LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_5_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_5_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)
#define LOGIC_TEMP_CONTROLLER_RELEASE_OVERTEMP_C (LOGIC_TEMP_CONTROLLER_ACTIVATE_OVERTEMP_C - LOGIC_TEMP_CONTROLLER_HYSTERESIS_C)

// --- 對應各溫度區間的電流限制比例 (浮點數) ---
// 註: 比例基於額定電流 50A (RATED_CURRENT from codeSw.h) 計算得出
#define LOGIC_TEMP_CONTROLLER_RATIO_NORMAL (0.35f)     //  16A 
#define LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_1 (0.33f)       //  16A
#define LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_2 (0.31f)       //  15A
#define LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_3 (0.291f)     //  14A
#define LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_4 (0.273f)     //  13A
#define LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_5 (0.254f)     //  12A
#define LOGIC_TEMP_CONTROLLER_RATIO_OVERTEMP (0.0f)    // 0%

#define LOGIC_TEMP_CONTROLLER_DEFAULT_CURRENT_ZONE (LOGIC_TEMP_CONTROLLER_ZONE_NORMAL)

// --- 函數原型 ---

/**
 * @brief 初始化溫度控制模組
 */
void logic_temp_controller_init(void);

/**
 * @brief 獲取當前最新的控制器溫度值
 * @return int32_t 溫度值 (單位: °C * 100)
 */
int32_t logic_temp_controller_getTemp(void);

/**
 * @brief 更新溫度狀態並獲取當前電流限制比例 (Q15)
 * @details 此為本模組最主要的核心函式，應在主迴圈中定期呼叫。
 * @param u16AdcValue 從 ADC 讀取的 NTC 腳位的原始值
 * @param[out] pbIsOverTemp 指標，用於回傳是否處於控制器過溫狀態 (完全停止輸出)
 * @param[out] pbIsOverLoad 指標，用於回傳是否處於任何有效的過載限流狀態
 * @return int16_t 計算出的允許最大電流比例 (Q15 格式)
 */
int16_t logic_temp_controller_updateTempAndGetCurrentLimit(uint16_t u16AdcValue,
                                                          bool *pbIsOverTemp,
                                                          bool *pbIsOverLoad);

#endif  // S_LOGIC_TEMP_CONTROLLER_H_EMP_CONTROLLER_H_