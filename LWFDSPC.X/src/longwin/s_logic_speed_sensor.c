#include "s_logic_speed_sensor.h" // Updated include

#include <stdlib.h>  // For abs, NULL (though stddef.h is better for NULL)
#include <stddef.h>  // For NULL
#include <stdbool.h> // For bool type, as functions now return bool

// --- 模組內部常數與資料 ---
// Note: Constants moved to .h file

// 速度對應 step/time 的查表結構
typedef struct
{
    uint16_t u16SpeedKmh;  // 速度閾值 (km/h)
    uint16_t u16TimeMs;    // 時間間隔 (ms)
    uint16_t u16StepValue; // 步階變化值
} S_INTERNAL_SPEED_TIME_STEP_T;

// 查表資料
static const S_INTERNAL_SPEED_TIME_STEP_T saInternalStepTimeTable[] = {
    INTERNAL_STEP_TIME_TABLE_ENTRY_0, // <= 5.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_1, // <= 7.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_2, // <= 9.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_3, // <= 11.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_4, // <= 13.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_5, // <= 16.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_6, // <= 19.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_7, // <= 22.0 km/h
    INTERNAL_STEP_TIME_TABLE_ENTRY_8  // > 22.0 km/h
};

// 計算表格的大小
static const uint8_t su8InternalSpeedTableSize = sizeof(saInternalStepTimeTable) / sizeof(saInternalStepTimeTable[0]);

// --- 模組內部狀態變數 ---

#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL
// 上一次的正交編碼器狀態 ( ISNB << 1 | ISNA )
static volatile uint8_t su8LastQuadState = 0;
// 正交解碼 -> 方向 的查找表 (LUT)
// 索引: (上一次狀態 << 2) | 目前狀態
// 值: +1=前進, -1=後退, 0=沒變化或錯誤
static const int8_t sa8DirectionLut[16] = {
    // 目前: 00  01  10  11   <- (ISNB,ISNA)
    0, 1, -1, 0, // 上次: 00
    -1, 0, 0, 1, // 上次: 01
    1, 0, 0, -1, // 上次: 10
    0, -1, 1, 0  // 上次: 11
};
#elif LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_SINGLE
// 上一次的單信號狀態
static volatile uint8_t su8LastSingleState = 0;
#endif

// 連續前進的步數計數
static volatile uint16_t su16ForwardCount = 0;
// 連續後退的步數計數 (僅 DUAL 模式有效)
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL
static volatile uint16_t su16BackwardCount = 0;
#endif
// 驅動啟用旗標 (根據 DRIVE_TYPE 模式決定其意義)
static volatile bool sbDriveEnabledFlag = false;
// 反踩偵測旗標 (僅 DUAL + FORWARD 模式有效)
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
static volatile bool sbBackpedalDetectedFlag = false;
#endif

// 停踩檢測旗標
static volatile bool sbPedalStopDetectedFlag = false;

// --- ISR 與主循環通信變數 ---
static volatile uint32_t su32LastDeltaTime = 0;    // ISR 記錄的最新有效時間差 (ms)
static volatile bool sbSpeedDataReadyFlag = false; // ISR 設置，表示有新的 DeltaTime 可供計算速度

// --- 與 ISR/高速計數相關 ---
static volatile uint32_t su32LastEdgeTimestamp = 0;    // 上一次有效邊緣的時間戳記 (ms)
static volatile uint16_t su16CurrentSpeedScaled10 = 0; // 目前計算出的速度 (0.1 km/h)

// --- 組態設定相關 ---
static uint16_t su16CountsPerRevolution = LOGIC_SPEED_SENSOR_DEFAULT_COUNTS_PER_REV; // 每轉計數
static uint32_t su32WheelCircumferenceMm = 0;                                        // 輪子周長 (毫米)

// 內部常數 (π * 1000 for integer calculation)
#define LOGIC_PI_TIMES_1000 3142
// 吋轉毫米 (25.4 * 10)
#define LOGIC_INCH_TO_MM_TIMES_10 254

// --- 速度感測器內部狀態管理 ---
static uint16_t s_speed_currentTargetPwmRpm = 0;
static bool s_speed_isMoving = false;
static uint32_t s_speed_lastUpdateTimeMs = 0;
static int16_t s_speed_currentStepValue = 0;
static uint8_t s_speed_currentStepTime = 1;

// --- 函數實作 ---

/**
 * @brief 初始化速度感測器邏輯子程序模組。
 */
void logic_speedSensor_init(uint16_t u16WheelDiameterInch,
                            uint16_t u16CountsPerRevolution)
{
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL
    su8LastQuadState = 0;
    su16BackwardCount = 0;
#if LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
    sbBackpedalDetectedFlag = false;
#endif
#elif LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_SINGLE
    su8LastSingleState = 0;
#endif
    su16ForwardCount = 0;
    sbDriveEnabledFlag = false;
    sbPedalStopDetectedFlag = false;
    su32LastEdgeTimestamp = 0;
    su16CurrentSpeedScaled10 = 0;
    su32LastDeltaTime = 0;
    sbSpeedDataReadyFlag = false;
    su16CountsPerRevolution = (u16CountsPerRevolution > 0) ? u16CountsPerRevolution : LOGIC_SPEED_SENSOR_DEFAULT_COUNTS_PER_REV;
    logic_speedSensor_setWheelDiameterInch(u16WheelDiameterInch);
}

/**
 * @brief 設定輪徑 (吋) 並更新輪子周長。
 */
void logic_speedSensor_setWheelDiameterInch(uint16_t u16WheelDiameterInch)
{
    uint16_t u16DiameterToUse = u16WheelDiameterInch;
    // ? 輪徑為 0 時，使用預設值
    if (u16DiameterToUse == 0)
    {
        u16DiameterToUse = LOGIC_SPEED_SENSOR_DEFAULT_WHEEL_DIAMETER_INCH;
    }
    // ? 計算輪子周長
    uint32_t u32DiameterMmTimes10 = (uint32_t)u16DiameterToUse * LOGIC_INCH_TO_MM_TIMES_10;
    su32WheelCircumferenceMm = (u32DiameterMmTimes10 * LOGIC_PI_TIMES_1000) / 10000UL;

    // ? 輪子周長為 0 時，使用預設值
    if (su32WheelCircumferenceMm == 0)
    {
        uint16_t u16DefaultDiameter = LOGIC_SPEED_SENSOR_DEFAULT_WHEEL_DIAMETER_INCH;

        // 轉成mm
        u32DiameterMmTimes10 = (uint32_t)u16DefaultDiameter * LOGIC_INCH_TO_MM_TIMES_10;

        su32WheelCircumferenceMm = (u32DiameterMmTimes10 * LOGIC_PI_TIMES_1000) / 10000UL;
        if (su32WheelCircumferenceMm == 0)
            su32WheelCircumferenceMm = 1;
    }
}

/**
 * @brief 根據目前速度(已縮放)計算目標 RPM/PWM 輸出值。
 */
static uint16_t _speedSensorGetTargetOutput(uint16_t u16CurrentSpeedScaled10)
{
    if (u16CurrentSpeedScaled10 <= LOGIC_SPEED_SENSOR_SPEED_ZERO_SCALED10)
    {
        // 狀況：速度為 0 或以下
        return LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO;
    }
    else if (u16CurrentSpeedScaled10 <= LOGIC_SPEED_SENSOR_SPEED_PHASE1_END_SCALED10)
    {
        // 狀況：第一段 (0 < 速度 <= 13.0 km/h)
        // 線性內插：從 LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO 到 LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END
        int32_t i32SpeedLong = (int32_t)u16CurrentSpeedScaled10;
        int32_t i32OutputRange1 = (int32_t)LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END - (int32_t)LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO; // = 14000
        int32_t i32SpeedRange1 = (int32_t)LOGIC_SPEED_SENSOR_SPEED_PHASE1_END_SCALED10;                                          // = 130

        // 公式: 目標 = (速度 * 輸出範圍) / 速度範圍 + 起始輸出
        // 使用 32 位元整數避免中間計算溢位
        // 檢查除以零 (雖然此處 speedRange1 是常數 130)
        if (i32SpeedRange1 == 0)
            return LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO;
        int32_t i32TargetOutputLong = (i32SpeedLong * i32OutputRange1) / i32SpeedRange1 + (int32_t)LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO;

        // 邊界檢查
        if (i32TargetOutputLong > LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END)
        {
            return LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END;
        }
        if (i32TargetOutputLong < LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO)
        {
            return LOGIC_SPEED_SENSOR_OUTPUT_AT_ZERO;
        }
        return (uint16_t)i32TargetOutputLong;
    }
    else if (u16CurrentSpeedScaled10 <= LOGIC_SPEED_SENSOR_SPEED_PHASE2_END_SCALED10)
    {
        // 狀況：第二段 (13.0 < 速度 <= 25.0 km/h)
        // 使用推導出的公式: 目標 = 22500 - 50 * 縮放後速度
        // 其中 22500 = 16000 + (16000 - 10000) * 130 / (250 - 130)
        // 且 50 = (16000 - 10000) * 10 / (250 - 130)
        int32_t i32SpeedLong = (int32_t)u16CurrentSpeedScaled10;
        int32_t i32TargetOutputLong = 22500L - 50L * i32SpeedLong;

        // 邊界檢查
        if (i32TargetOutputLong < LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE2_END)
        {
            return LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE2_END;
        }
        if (i32TargetOutputLong > LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END)
        {
            return LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE1_END;
        }
        return (uint16_t)i32TargetOutputLong;
    }
    else
    {
        // 狀況：第三段 (速度 > 25.0 km/h)
        return LOGIC_SPEED_SENSOR_OUTPUT_AT_PHASE2_END; // 固定為最小值
    }
}

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
bool logic_speedSensor_getUpdateParams(uint32_t u32MilliSecond,
                                       uint16_t u16CurrentPwmRpm,
                                       uint16_t *pu16TargetPwmRpm,
                                       uint16_t u16CurrentSpeedScaled10)
{
    if (pu16TargetPwmRpm == NULL)
    {
        return false;
    }

    // 初始化目標值為當前輸出值
    *pu16TargetPwmRpm = u16CurrentPwmRpm;

    // 計算新的目標輸出值
    uint16_t u16NewTargetOutput = _speedSensorGetTargetOutput(u16CurrentSpeedScaled10);

    // 檢查是否需要更新目標值
    bool bTargetChanged = (u16NewTargetOutput != s_speed_currentTargetPwmRpm);

    if (bTargetChanged)
    {
        s_speed_currentTargetPwmRpm = u16NewTargetOutput;
        s_speed_isMoving = true;
        s_speed_lastUpdateTimeMs = u32MilliSecond;

        // 根據目前速度查表選擇適當的步進值和時間
        uint16_t u16CurrentSpeedKmh = u16CurrentSpeedScaled10 / 10;
        int16_t s16FoundIndex = -1;
        for (uint8_t u8Idx = 0; u8Idx < su8InternalSpeedTableSize; ++u8Idx)
        {
            if (u16CurrentSpeedKmh <= saInternalStepTimeTable[u8Idx].u16SpeedKmh)
            {
                s16FoundIndex = u8Idx;
                break;
            }
        }

        if (s16FoundIndex == -1)
        {
            s16FoundIndex = su8InternalSpeedTableSize - 1;
        }

        s_speed_currentStepValue = saInternalStepTimeTable[s16FoundIndex].u16StepValue;
        s_speed_currentStepTime = (uint8_t)saInternalStepTimeTable[s16FoundIndex].u16TimeMs;
    }

    // 檢查時間間隔是否到達
    bool bTimeIntervalReached = false;
    if (s_speed_isMoving)
    {
        uint32_t u32TimeDiff = u32MilliSecond - s_speed_lastUpdateTimeMs;
        if (u32TimeDiff >= s_speed_currentStepTime)
        {
            bTimeIntervalReached = true;
            s_speed_lastUpdateTimeMs = u32MilliSecond;
        }
    }

    // 如果時間間隔到達，計算新的輸出值
    if (bTimeIntervalReached && s_speed_isMoving)
    {
        uint16_t u16NewOutput = u16CurrentPwmRpm;

        if (s_speed_currentTargetPwmRpm > u16CurrentPwmRpm)
        {
            // 增加：增加步進值
            if (s_speed_currentStepValue > 0)
            {
                uint32_t u32NewOutput = (uint32_t)u16CurrentPwmRpm + s_speed_currentStepValue;
                if (u32NewOutput > s_speed_currentTargetPwmRpm)
                {
                    u16NewOutput = s_speed_currentTargetPwmRpm;
                }
                else
                {
                    u16NewOutput = (uint16_t)u32NewOutput;
                }
            }
        }
        else if (s_speed_currentTargetPwmRpm < u16CurrentPwmRpm)
        {
            // 減少：減少步進值
            if (s_speed_currentStepValue > 0)
            {
                uint32_t u32NewOutput = (uint32_t)u16CurrentPwmRpm - s_speed_currentStepValue;
                if (u32NewOutput < s_speed_currentTargetPwmRpm)
                {
                    u16NewOutput = s_speed_currentTargetPwmRpm;
                }
                else
                {
                    u16NewOutput = (uint16_t)u32NewOutput;
                }
            }
        }

        // 檢查是否已達到目標值
        if (u16NewOutput == s_speed_currentTargetPwmRpm)
        {
            s_speed_isMoving = false;
        }

        *pu16TargetPwmRpm = u16NewOutput;
        return (u16NewOutput != u16CurrentPwmRpm);
    }
    else
    {
        *pu16TargetPwmRpm = u16CurrentPwmRpm;
        return false;
    }
}

// --- 處理輸入與速度計算 (根據模式條件編譯) ---

#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL
/**
 * @brief (雙信號模式) 處理速度感測器狀態變化，更新計數/旗標/時間戳。
 */
void logic_speedSensor_processInputCount(uint8_t u8IsnaState,
                                         uint8_t u8IsnbState,
                                         uint32_t u32MilliSecond)
{
    uint8_t u8CurrentQuadState = ((u8IsnbState & 0x01) << 1) | (u8IsnaState & 0x01);
    uint32_t u32TimeSinceLastEdge = u32MilliSecond - su32LastEdgeTimestamp;
    // Handle timer wrap-around for timeout check
    if (u32MilliSecond < su32LastEdgeTimestamp)
    {
        u32TimeSinceLastEdge = (0xFFFFFFFFUL - su32LastEdgeTimestamp) + u32MilliSecond + 1;
    }

    if (u8CurrentQuadState != su8LastQuadState)
    {
        uint8_t u8TransitionIndex = (su8LastQuadState << 2) | u8CurrentQuadState;
        int8_t s8Direction = sa8DirectionLut[u8TransitionIndex & 0x0F]; // +1 Fwd, -1 Bwd, 0 Invalid

        // --- 更新 DeltaTime 和 Flag (僅在有效邊緣) ---
        if (s8Direction != 0)
        {
            su32LastDeltaTime = u32TimeSinceLastEdge; // 記錄有效邊緣的時間差
            sbSpeedDataReadyFlag = true;              // 設置標誌，通知主循環計算速度
            su32LastEdgeTimestamp = u32MilliSecond;   // 更新時間戳
            sbPedalStopDetectedFlag = 0;              // 有效邊緣表示正在踩踏，清除停踩旗標
        }

        // --- 更新計數器和旗標 (根據方向和驅動模式) ---
        if (s8Direction > 0)
        { // 前進
#if LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
            su16BackwardCount = 0; // 重置反向計數
            su16ForwardCount++;
            if (su16ForwardCount >= LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD)
            {
                sbDriveEnabledFlag = true;                                          // 前驅模式下，前進啟用驅動
                su16ForwardCount = LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD; // 防止溢位
            }
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
            sbBackpedalDetectedFlag = false; // 清除反踩旗標
#endif
#elif LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_BACKWARD
            su16BackwardCount = 0;      // 重置反向計數 (後驅模式下，前進會清除後退計數)
            su16ForwardCount++;         // 仍然計數前進步數，但不用於驅動啟用
            sbDriveEnabledFlag = false; // 前進時，禁用後驅模式的驅動
#endif
        }
        else if (s8Direction < 0)
        { // 後退
#if LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
            su16ForwardCount = 0;       // 重置正向計數
            sbDriveEnabledFlag = false; // 清除前驅啟用旗標
            su16BackwardCount++;
#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
            if (su16BackwardCount >= LOGIC_SPEED_SENSOR_BACKPEDAL_DETECT_COUNT_THRESHOLD)
            {
                sbBackpedalDetectedFlag = true;                                          // 偵測到反踩
                su16BackwardCount = LOGIC_SPEED_SENSOR_BACKPEDAL_DETECT_COUNT_THRESHOLD; // 防止溢位
            }
#endif
            // 後退時，速度重置為 0 (或可以根據需求保留負速度?)
            su16CurrentSpeedScaled10 = 0;
#elif LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_BACKWARD
            su16ForwardCount = 0; // 重置正向計數
            su16BackwardCount++;
            if (su16BackwardCount >= LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD)
            {                                                                        // 使用驅動閾值
                sbDriveEnabledFlag = true;                                           // 後驅模式下，後退啟用驅動
                su16BackwardCount = LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD; // 防止溢位
            }
            // 後退時，速度重置為 0
            su16CurrentSpeedScaled10 = 0;
#endif
        }
        // 無論方向如何，都更新最後狀態
        su8LastQuadState = u8CurrentQuadState;
    }
    else
    { // 狀態未變
        // 僅檢查速度超時
        if (su16CurrentSpeedScaled10 > 0 && u32TimeSinceLastEdge >= LOGIC_SPEED_SENSOR_SPEED_TIMEOUT_MS)
        {
            // su16CurrentSpeedScaled10 = 0; // 速度歸零移到 CalculateAndUpdateSpeed 函數中處理
            su32LastDeltaTime = u32TimeSinceLastEdge; // 記錄超時的時間差
            sbSpeedDataReadyFlag = true;              // 設置標誌，通知主循環處理超時（歸零速度）
            // 注意：這裡不更新 su32LastEdgeTimestamp，以便超時狀態持續
        }
    }
}

#elif LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_SINGLE
/**
 * @brief (單信號模式) 處理速度感測器狀態變化，更新計數/旗標/時間戳。
 */
void logic_speedSensor_processInputCount(uint8_t sensorState, uint32_t currentTimeMillis)
{
    uint8_t currentState = (sensorState & 0x01); // 只關心 0 或 1
    uint32_t timeSinceLastEdge = currentTimeMillis - su32LastEdgeTimestamp;
    // Handle timer wrap-around for timeout check
    if (currentTimeMillis < su32LastEdgeTimestamp)
    {
        timeSinceLastEdge = (0xFFFFFFFFUL - su32LastEdgeTimestamp) + currentTimeMillis + 1;
    }

    if (currentState != su8LastSingleState)
    { // 狀態改變 (邊緣觸發)
        // 在單信號模式下，我們假設任何邊緣都代表前進一步
        // int8_t direction = 1; // Assume forward (不再需要 direction 變數)

        // --- 更新 DeltaTime 和 Flag ---
        su32LastDeltaTime = timeSinceLastEdge;     // 記錄邊緣的時間差
        sbSpeedDataReadyFlag = true;               // 設置標誌，通知主循環計算速度
        su32LastEdgeTimestamp = currentTimeMillis; // Update timestamp on edge
        sbPedalStopDetectedFlag = 0;               // 有效邊緣表示正在踩踏，清除停踩旗標

        // --- 更新計數器和旗標 ---
        // 單信號模式只支援前進驅動 ( LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD )
#if LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
        su16ForwardCount++;
        if (su16ForwardCount >= LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD)
        {
            sbDriveEnabledFlag = true;
            su16ForwardCount = LOGIC_SPEED_SENSOR_DRIVE_ENABLE_COUNT_THRESHOLD; // Prevent overflow
        }
#endif
        // 不處理 su16BackwardCount 或 sbBackpedalDetectedFlag

        su8LastSingleState = currentState; // 更新最後狀態
    }
    else
    { // 狀態未變
        // 僅檢查速度超時
        if (su16CurrentSpeedScaled10 > 0 && timeSinceLastEdge >= LOGIC_SPEED_SENSOR_SPEED_TIMEOUT_MS)
        {
            // su16CurrentSpeedScaled10 = 0; // 速度歸零移到 CalculateAndUpdateSpeed 函數中處理
            su32LastDeltaTime = timeSinceLastEdge; // 記錄超時的時間差
            sbSpeedDataReadyFlag = true;           // 設置標誌，通知主循環處理超時（歸零速度）
            // 注意：這裡不更新 su32LastEdgeTimestamp，以便超時狀態持續
        }
    }
}
#endif // LOGIC_SPEED_SENSOR_SIGNAL

/**
 * @brief 根據 ISR 提供的最新感測器數據，計算並更新內部速度。
 *
 *        檢查是否有來自 ISR 的新數據標誌 (sbSpeedDataReadyFlag)。
 *        若有，則讀取時間差 (deltaTime)，計算速度 (單位 0.1 km/h)，
 *        並處理超時情況 (將速度歸零、判斷是否停踩)。
 *        建議在主迴圈中定期調用此函數。
 *
 * @return bool 如果執行了速度計算則返回 true，否則返回 false。
 */
bool logic_speedSensor_calculateAndUpdateSpeed(void)
{
    if (!sbSpeedDataReadyFlag)
    {
        return false; // 沒有新數據可計算
    }

    // 讀取 ISR 設置的 deltaTime
    // 注意：如果 su32LastDeltaTime 在 8/16 位 MCU 上不是原子讀取，
    // 且 ISR 可能在讀取過程中修改它，這裡需要保護。
    // 簡單的保護是暫時禁用中斷：
    // uint32_t interruptState = disable_interrupts(); // 特定平台的函數
    uint32_t deltaTime = su32LastDeltaTime;
    sbSpeedDataReadyFlag = false; // 清除標誌 (盡快清除，減少競爭)
    // restore_interrupts(interruptState); // 特定平台的函數

    // 讀取當前速度，用於判斷停踩超時閾值
    // 在速度歸零前讀取
    uint16_t speedBeforeUpdate = su16CurrentSpeedScaled10;

    // 檢查是否超時或 deltaTime 無效
    if (deltaTime == 0 || deltaTime >= LOGIC_SPEED_SENSOR_SPEED_TIMEOUT_MS)
    {
        su16CurrentSpeedScaled10 = 0; // 先將速度歸零

        // 根據歸零前的速度決定停踩超時時間
        uint32_t timeoutStopPedal;
        if (speedBeforeUpdate > LOGIC_SPEED_SENSOR_STOP_PEDAL_SPEED_BOUNDARY_SCALED10)
        {
            timeoutStopPedal = LOGIC_SPEED_SENSOR_STOP_PEDAL_TIMEOUT_HIGH_SPEED_MS;
        }
        else
        {
            timeoutStopPedal = LOGIC_SPEED_SENSOR_STOP_PEDAL_TIMEOUT_LOW_SPEED_MS;
        }

        // 檢查是否達到停踩超時條件
        if (deltaTime >= timeoutStopPedal)
        {
            sbPedalStopDetectedFlag = true; // 設置停踩旗標
        }
    }
    else
    {
        // 正常速度計算 (未超時)
        if (su16CountsPerRevolution > 0 && su32WheelCircumferenceMm > 0)
        {
            // 執行速度計算 (0.1 km/h)
            uint32_t numerator = (uint32_t)su32WheelCircumferenceMm * LOGIC_SPEED_FACTOR_NUM;
            uint32_t denominator = (uint32_t)su16CountsPerRevolution * deltaTime * LOGIC_SPEED_FACTOR_DEN_MS_MULT;
            if (denominator > 0)
            {
                su16CurrentSpeedScaled10 = (uint16_t)(numerator / denominator);
            }
            else
            {
                su16CurrentSpeedScaled10 = 0; // 避免除以零
            }
        }
        else
        {
            su16CurrentSpeedScaled10 = 0; // 配置錯誤
        }
        // 正常計算速度時，不應該設置停踩旗標 (已在 ISR 中清除)
        // sbPedalStopDetectedFlag = 0; // Redundant due to ISR clearing
    }
    return true; // 已執行計算
}

/**
 * @brief 檢查是否已達到驅動啟用閾值。
 *        (意義取決於 LOGIC_SPEED_SENSOR_DRIVE)
 */
bool logic_speedSensor_isDriveEnabled(void)
{
    // 實際檢查哪個計數器取決於DRIVE_TYPE，但旗標 sbDriveEnabledFlag 已被正確設置
    return sbDriveEnabledFlag;
}

#if LOGIC_SPEED_SENSOR_SIGNAL == LOGIC_SPEED_SENSOR_DETECTION_DUAL && LOGIC_SPEED_SENSOR_DRIVE == LOGIC_SPEED_SENSOR_DRIVE_FORWARD
/**
 * @brief (僅雙信號+前驅模式) 檢查是否已達到反踩偵測閾值。
 */
bool logic_speedSensor_isBackpedalDetected(void)
{
    return sbBackpedalDetectedFlag;
}

/**
 * @brief (僅雙信號+前驅模式) 清除內部的反踩已偵測旗標。
 */
void logic_speedSensor_clearBackpedalFlag(void)
{
    sbBackpedalDetectedFlag = false;
    su16BackwardCount = 0; // 清除旗標時也重置計數器
}
#endif // Conditional compilation for backpedal functions

/**
 * @brief 檢查是否已偵測到停止踩踏。
 */
bool logic_speedSensor_isPedalStopDetected(void)
{
    // 考慮到 sbPedalStopDetectedFlag 是 volatile 且通常是原子讀取的 (uint8_t),
    // 直接返回即可。
    return sbPedalStopDetectedFlag;
}

/**
 * @brief 清除內部的停止踩踏已偵測旗標。
 */
void logic_speedSensor_clearPedalStopFlag(void)
{
    // 考慮到寫入 uint8_t volatile 通常是原子的。
    sbPedalStopDetectedFlag = false;
}

/**
 * @brief 獲取內部計算出的目前速度 (已縮放)。
 */
uint16_t logic_speedSensor_getCurrentSpeedKmhScaled10(void)
{
    // 考慮到 su16CurrentSpeedScaled10 是由 CalculateAndUpdateSpeed 更新的，
    // 並且讀取 uint16_t 可能不是原子的，最安全的做法是在讀取時禁用中斷，
    // 或者接受可能讀到中間值的風險（對於速度顯示通常可接受）。
    // uint32_t interruptState = disable_interrupts();
    uint16_t speed = su16CurrentSpeedScaled10;
    // restore_interrupts(interruptState);
    return speed;
}