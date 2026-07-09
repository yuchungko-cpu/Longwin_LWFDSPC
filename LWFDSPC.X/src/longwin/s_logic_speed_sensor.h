#ifndef S_LOGIC_SPEED_SENSOR_H
#define S_LOGIC_SPEED_SENSOR_H

#include <stdint.h>  // 使用 uint16_t, uint8_t, uint32_t 等標準整數型別
#include <stdbool.h> // 使用 bool 型別
#include "common_step_time.h"

// --- 公開常數定義 ---

// 偵測模式選項 (Detection Mode Options)
#define LOGIC_SPEED_SENSOR_DETECTION_SINGLE (0u) // 單信號模式 (僅 ISNA 或 ISNB)
#define LOGIC_SPEED_SENSOR_DETECTION_DUAL (1u)   // 雙信號模式 (ISNA 和 ISNB 正交)

// 驅動類型選項 (Drive Type Options)
#define LOGIC_SPEED_SENSOR_DRIVE_FORWARD (0u)  // 基於前進觸發驅動
#define LOGIC_SPEED_SENSOR_DRIVE_BACKWARD (1u) // 基於後退觸發驅動 (例如反踩啟動)

// --- 編譯時配置 ---
// 根據實際硬體和需求選擇模式

/**
 * @brief 設定編譯時使用的偵測模式。
 */
#define LOGIC_SPEED_SENSOR_SIGNAL LOGIC_SPEED_SENSOR_DETECTION_DUAL

/**
 * @brief 設定編譯時使用的驅動觸發類型。
 */
#define LOGIC_SPEED_SENSOR_DRIVE LOGIC_SPEED_SENSOR_DRIVE_FORWARD

// 驗證配置組合
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_SINGLE && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_BACKWARD
#error "Backward drive detection is not supported in single signal mode."
#endif

// --- 預設值與閾值 ---
#define LOGIC_SPEED_SENSOR_DEFAULT_WHEEL_DIAMETER_INCH (26u)
#define LOGIC_SPEED_SENSOR_DEFAULT_COUNTS_PER_REV (36u)
#define LOGIC_SPEED_SENSOR_SPEED_TIMEOUT_MS (300u)

// 速度閾值 (單位: 0.1 km/h)
#define LOGIC_SPEED_SENSOR_SPEED_ZERO_SCALED10 (0u)
#define LOGIC_SPEED_SENSOR_SPEED_PHASE1_END_SCALED10 (130u)
#define LOGIC_SPEED_SENSOR_SPEED_PHASE2_END_SCALED10 (250u)

// 關鍵速度點的輸出值 (RPM/PWM)
#define LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO (2000u)
#define LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END (16000u)
#define LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE2_END (10000u)

// 計數閾值
#define LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD (4u)
#define LOGIC_SPEED_SENSOR_BACKPEDAL_DETECT_COUNT_THRESHOLD (3u)

// --- 速度計算常數 ---
#define LOGIC_SPEED_FACTOR_NUM (36UL)
#define LOGIC_SPEED_FACTOR_DEN_MS_MULT (100UL)

// --- 停踩判斷相關閾值 ---
#define LOGIC_SPEED_SENSOR_STOP_PEDAL_SPEED_BOUNDARY_SCALED10 (60u)
#define LOGIC_SPEED_SENSOR_STOP_PEDAL_TIMEOUT_HIGH_SPEED_MS (150u)
#define LOGIC_SPEED_SENSOR_STOP_PEDAL_TIMEOUT_LOW_SPEED_MS (300u)

// --- Internal Speed Time Step Table Definitions ---
// Format: {SpeedKmh_Threshold, MotorStep, TimeMs}
#define INTERNAL_STEP_TIME_TABLE_ENTRY_0 {5.0f, 1, 3}  // <= 5.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_1 {7.0f, 2, 2}  // <= 7.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_2 {9.0f, 3, 2}  // <= 9.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_3 {11.0f, 2, 2} // <= 11.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_4 {13.0f, 1, 3} // <= 13.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_5 {16.0f, 2, 1} // <= 16.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_6 {19.0f, 3, 1} // <= 19.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_7 {22.0f, 3, 1} // <= 22.0 km/h
#define INTERNAL_STEP_TIME_TABLE_ENTRY_8 {25.0f, 4, 1} // > 22.0 km/h (represents up to 25km/h for lookup)

// --- 函數宣告 ---

void logic_speedSensor_init(
    uint16_t u16WheelDiameterInch,
    uint16_t u16CountsPerRevolution);

void logic_speedSensor_setWheelDiameterInch(uint16_t u16WheelDiameterInch);


/**
 * @brief 根據目前速度計算目標輸出值，內部管理時間間隔和漸進式變化
 * 
 * @details 此函數執行以下功能:
 *          1. 根據目前速度計算目標輸出值
 *          2. 內部管理時間間隔，決定是否更新目標值
 *          3. 根據速度查表選擇適當的步進值和時間
 *          4. 漸進式變化到目標值，而不是一次到位
 *
 * @param u16CurrentSpeedScaled10 目前速度 (km/h * 10)
 * @param u16CurrentOutput 當前輸出值
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param pu16TargetOutput 指向目標輸出值的指標
 *
 * @return true 如果輸出值有變化，false 如果輸出值無變化
 */
bool logic_speedSensor_getUpdateParams(uint16_t u16CurrentSpeedScaled10,
    uint16_t u16CurrentOutput,
                                       uint32_t u32SystemTimeMs,
                                       uint16_t *pu16TargetOutput);

#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL
void logic_speedSensor_processInputCount(
    uint8_t u8IsnaState,
    uint8_t u8IsnbState,
    uint32_t u32CurrentTimeMillis);

#elif LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_SINGLE
void logic_speedSensor_processInputCount(uint8_t u8SensorState, uint32_t u32CurrentTimeMillis);
#endif

bool logic_speedSensor_calculateAndUpdateSpeed(void);
bool logic_speedSensor_isDriveEnabled(void);

#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
bool logic_speedSensor_isBackpedalDetected(void);
void logic_speedSensor_clearBackpedalFlag(void);
#endif

bool logic_speedSensor_isPedalStopDetected(void);
void logic_speedSensor_clearPedalStopFlag(void);
uint16_t logic_speedSensor_getCurrentSpeedKmhScaled10(void);

#endif // S_LOGIC_SPEED_SENSOR_H