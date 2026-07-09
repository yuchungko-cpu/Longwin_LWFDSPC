/**
 * @file system_timer_example.c
 * @brief 系統時間管理模組使用範例
 * @note 展示如何使用高精度時間函式
 * @version 1.0
 * @date 2025-01
 * @author Longwin Integration Team
 */

#include "../codeSw.h"

#if FEATURE_SYSTEM_TIMER_ENABLE
#include "../system_timer.h"

// =============================================================================
// 使用範例 1: 基本時間函式
// =============================================================================

/**
 * @brief 基本時間函式使用範例
 * @note 展示 millis(), micros(), delay() 的基本用法
 */
void systemTimerExample_basic(void)
{
    // 初始化系統時間管理
    if (!systemTimer_init()) {
        // 初始化失敗處理
        return;
    }
    
    // 取得當前時間
    uint32_t start_time = millis();
    uint32_t start_time_us = micros();
    
    // 延遲 100 毫秒
    delay(100);
    
    // 計算實際經過的時間
    uint32_t elapsed_ms = millis() - start_time;
    uint32_t elapsed_us = micros() - start_time_us;
    
    // 短延遲 (微秒級)
    delayMicroseconds(500);  // 延遲 500 微秒
    
    // 檢查時間精度
    uint32_t final_time = millis();
    // elapsed_ms 應該約為 100ms
    // final_time - start_time 應該約為 100ms
}

// =============================================================================
// 使用範例 2: 非阻塞式定時器
// =============================================================================

/**
 * @brief 非阻塞式定時器使用範例
 * @note 展示如何使用非阻塞式定時器實現週期性任務
 */
void systemTimerExample_nonBlocking(void)
{
    // 定義不同的定時器
    static SystemTimer_NonBlocking_T timer_1s;    // 1秒定時器
    static SystemTimer_NonBlocking_T timer_500ms; // 500ms定時器
    static SystemTimer_NonBlocking_T timer_100ms; // 100ms定時器
    static bool timers_initialized = false;
    
    // 初始化定時器 (只執行一次)
    if (!timers_initialized) {
        systemTimer_initNonBlocking(&timer_1s, 1000);    // 1秒
        systemTimer_initNonBlocking(&timer_500ms, 500);  // 500ms
        systemTimer_initNonBlocking(&timer_100ms, 100);  // 100ms
        timers_initialized = true;
    }
    
    // 在主迴圈中檢查定時器
    // 這個函式需要在主迴圈中頻繁呼叫
    
    // 每秒執行的任務
    if (systemTimer_checkNonBlocking(&timer_1s)) {
        // 執行每秒的任務
        // 例如：更新狀態、發送心跳包等
        systemTimerExample_heartbeat();
    }
    
    // 每500ms執行的任務
    if (systemTimer_checkNonBlocking(&timer_500ms)) {
        // 執行每500ms的任務
        // 例如：更新顯示、檢查感測器等
        systemTimerExample_updateDisplay();
    }
    
    // 每100ms執行的任務
    if (systemTimer_checkNonBlocking(&timer_100ms)) {
        // 執行每100ms的任務
        // 例如：快速響應、控制迴圈等
        systemTimerExample_fastControl();
    }
}

/**
 * @brief 心跳任務範例 (每秒執行)
 */
void systemTimerExample_heartbeat(void)
{
    static uint32_t heartbeat_counter = 0;
    heartbeat_counter++;
    
    // 輸出心跳資訊 (如果有串列埠或除錯介面)
    // printf("Heartbeat: %lu, Uptime: %lu seconds\n", heartbeat_counter, millis() / 1000);
    
    // 切換LED或其他指示器
    // LED_HEARTBEAT_TOGGLE();
}

/**
 * @brief 更新顯示任務範例 (每500ms執行)
 */
void systemTimerExample_updateDisplay(void)
{
    // 更新顯示資訊
    uint32_t uptime_ms = millis();
    uint32_t uptime_seconds = uptime_ms / 1000;
    uint32_t uptime_minutes = uptime_seconds / 60;
    
    // 顯示運行時間
    // display_printf("Uptime: %02lu:%02lu", uptime_minutes, uptime_seconds % 60);
    
    // 更新其他狀態顯示
    // display_update_status();
}

/**
 * @brief 快速控制任務範例 (每100ms執行)
 */
void systemTimerExample_fastControl(void)
{
    // 快速響應任務
    // 例如：讀取感測器、更新控制參數等
    
    // 模擬感測器讀取
    static uint16_t sensor_value = 0;
    sensor_value = (sensor_value + 1) % 1000;
    
    // 根據感測器值調整控制參數
    if (sensor_value > 800) {
        // 高值處理
    } else if (sensor_value < 200) {
        // 低值處理
    }
}

// =============================================================================
// 使用範例 3: 效能測量
// =============================================================================

/**
 * @brief 效能測量使用範例
 * @note 展示如何測量函式或程式碼段的執行時間
 */
void systemTimerExample_profiling(void)
{
    SystemTimer_Profiler_T profiler;
    
    // 測量快速函式的執行時間
    systemTimer_profileStart(&profiler);
    systemTimerExample_fastFunction();
    uint32_t fast_duration = systemTimer_profileEnd(&profiler);
    
    // 測量慢速函式的執行時間
    systemTimer_profileStart(&profiler);
    systemTimerExample_slowFunction();
    uint32_t slow_duration = systemTimer_profileEnd(&profiler);
    
    // 輸出測量結果
    // printf("Fast function: %lu us\n", fast_duration);
    // printf("Slow function: %lu us\n", slow_duration);
    
    // 測量程式碼段的執行時間
    systemTimer_profileStart(&profiler);
    
    // 要測量的程式碼段
    for (int i = 0; i < 1000; i++) {
        volatile int dummy = i * i;  // 模擬計算
    }
    
    uint32_t loop_duration = systemTimer_profileEnd(&profiler);
    // printf("Loop duration: %lu us\n", loop_duration);
}

/**
 * @brief 快速函式範例
 */
void systemTimerExample_fastFunction(void)
{
    // 模擬快速執行的函式
    volatile int result = 0;
    for (int i = 0; i < 10; i++) {
        result += i;
    }
}

/**
 * @brief 慢速函式範例
 */
void systemTimerExample_slowFunction(void)
{
    // 模擬較慢執行的函式
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += i * i;
    }
}

// =============================================================================
// 使用範例 4: 安全的時間比較
// =============================================================================

/**
 * @brief 安全時間比較使用範例
 * @note 展示如何正確處理 millis() 溢位情況
 */
void systemTimerExample_safeTimeComparison(void)
{
    static uint32_t last_action_time = 0;
    const uint32_t ACTION_INTERVAL = 5000;  // 5秒間隔
    
    // 錯誤的時間比較方式 (不安全，溢位時會出錯)
    // if ((millis() - last_action_time) >= ACTION_INTERVAL) {
    //     // 執行動作
    // }
    
    // 正確的時間比較方式 (安全，自動處理溢位)
    if (systemTimer_isTimeElapsed(last_action_time, ACTION_INTERVAL)) {
        // 執行動作
        systemTimerExample_periodicAction();
        last_action_time = millis();  // 更新時間
    }
}

/**
 * @brief 週期性動作範例
 */
void systemTimerExample_periodicAction(void)
{
    // 執行週期性動作
    // 例如：資料記錄、狀態檢查等
    
    static uint32_t action_count = 0;
    action_count++;
    
    // printf("Periodic action #%lu at %lu ms\n", action_count, millis());
}

// =============================================================================
// 使用範例 5: 整合到主程式
// =============================================================================

/**
 * @brief 主程式整合範例
 * @note 展示如何在主程式中整合系統時間管理
 */
void systemTimerExample_mainIntegration(void)
{
    // 在 main() 函式的初始化階段呼叫
    if (!systemTimer_init()) {
        // 處理初始化失敗
        // error_handler("System timer init failed");
        return;
    }
    
    // 初始化非阻塞式定時器
    static SystemTimer_NonBlocking_T main_timer;
    systemTimer_initNonBlocking(&main_timer, 1000);  // 1秒定時器
    
    // 在主迴圈中的使用
    while (1) {
        // 原有的主迴圈程式碼
        // ...
        
        // 添加定時任務
        if (systemTimer_checkNonBlocking(&main_timer)) {
            // 每秒執行的維護任務
            systemTimerExample_maintenanceTask();
        }
        
        // 其他時間相關的任務
        systemTimerExample_timeBasedControl();
        
        // 繼續原有的主迴圈程式碼
        // ...
    }
}

/**
 * @brief 維護任務範例
 */
void systemTimerExample_maintenanceTask(void)
{
    // 系統維護任務
    // 例如：記憶體檢查、統計更新等
    
    // 取得系統統計資訊
    uint32_t overflow_count;
    uint32_t current_time = systemTimer_getStats(&overflow_count);
    
    // 檢查是否接近溢位
    if (current_time > 0xF0000000UL) {  // 接近溢位時
        // 可以考慮重設或記錄
        // printf("Warning: System time approaching overflow\n");
    }
    
    // 記錄系統運行時間
    // log_system_uptime(current_time, overflow_count);
}

/**
 * @brief 時間基礎控制範例
 */
void systemTimerExample_timeBasedControl(void)
{
    // 基於時間的控制邏輯
    static uint32_t control_start_time = 0;
    static bool control_active = false;
    
    // 檢查是否需要開始控制
    if (!control_active && /* 某些條件 */ true) {
        control_start_time = millis();
        control_active = true;
    }
    
    // 在控制期間執行特定邏輯
    if (control_active) {
        uint32_t control_duration = millis() - control_start_time;
        
        if (control_duration < 1000) {
            // 前1秒的控制邏輯
        } else if (control_duration < 5000) {
            // 1-5秒的控制邏輯
        } else {
            // 超過5秒，停止控制
            control_active = false;
        }
    }
}

// =============================================================================
// 模組資訊和診斷
// =============================================================================

/**
 * @brief 系統時間模組診斷
 * @note 展示如何診斷系統時間模組的狀態
 */
void systemTimerExample_diagnostics(void)
{
    // 檢查模組是否已初始化
    if (!systemTimer_isInitialized()) {
        // printf("ERROR: System timer not initialized\n");
        return;
    }
    
    // 取得模組版本資訊
    const char* version = systemTimer_getVersion();
    // printf("System Timer Version: %s\n", version);
    
    // 取得記憶體使用量
    uint16_t memory_usage = systemTimer_getMemoryUsage();
    // printf("Memory Usage: %u bytes\n", memory_usage);
    
    // 取得統計資訊
    uint32_t overflow_count;
    uint32_t current_time = systemTimer_getStats(&overflow_count);
    // printf("Current Time: %lu ms, Overflows: %lu\n", current_time, overflow_count);
    
    // 計算運行時間
    uint32_t uptime_seconds = current_time / 1000;
    uint32_t uptime_minutes = uptime_seconds / 60;
    uint32_t uptime_hours = uptime_minutes / 60;
    uint32_t uptime_days = uptime_hours / 24;
    
    // printf("Uptime: %lu days, %02lu:%02lu:%02lu\n", 
    //        uptime_days, 
    //        uptime_hours % 24, 
    //        uptime_minutes % 60, 
    //        uptime_seconds % 60);
}

#endif /* FEATURE_SYSTEM_TIMER_ENABLE */

/**
 * 使用指南：
 * 
 * 1. 在 main() 函式開始時呼叫 systemTimer_init()
 * 2. 使用 millis() 和 micros() 取得當前時間
 * 3. 使用 delay() 和 delayMicroseconds() 進行延遲
 * 4. 使用非阻塞式定時器實現週期性任務
 * 5. 使用效能測量工具最佳化程式碼
 * 6. 使用安全的時間比較函式避免溢位問題
 * 
 * 注意事項：
 * - 系統時間約在49.7天後溢位
 * - delay() 是阻塞式的，會停止程式執行
 * - 非阻塞式定時器需要在主迴圈中頻繁呼叫
 * - Timer2 中斷優先級設為1，低於ADC中斷
 */ 