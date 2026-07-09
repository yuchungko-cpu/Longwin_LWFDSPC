/**
 * @file system_time_example.c
 * @brief 系統時間函式使用範例
 * @note 這個檔案展示如何使用類似 Arduino 的 millis()/micros() 函式
 */

#include <stdint.h>
#include <stdbool.h>

// 外部函式宣告 (在 HallFOC_Regen.c 中實作)
extern uint32_t millis(void);
extern uint32_t micros(void);
extern void delay(uint32_t ms);
extern void delayMicroseconds(uint32_t us);

// =============================================================================
// 使用範例
// =============================================================================

/**
 * @brief 範例1: 基本時間測量
 */
void example_basic_timing(void)
{
    uint32_t start_time = millis();
    
    // 執行一些操作...
    // (模擬工作)
    
    uint32_t elapsed_time = millis() - start_time;
    
    // elapsed_time 現在包含經過的毫秒數
    // 可以用於效能測量或超時檢查
}

/**
 * @brief 範例2: 非阻塞式定時器
 */
void example_non_blocking_timer(void)
{
    static uint32_t last_blink_time = 0;
    static bool led_state = false;
    const uint32_t BLINK_INTERVAL = 500;  // 500ms
    
    uint32_t current_time = millis();
    
    if (current_time - last_blink_time >= BLINK_INTERVAL) {
        last_blink_time = current_time;
        led_state = !led_state;
        
        // 切換 LED 狀態
        // IORunLED = led_state;
    }
}

/**
 * @brief 範例3: 多個定時器管理
 */
void example_multiple_timers(void)
{
    static uint32_t timer1_last = 0;  // 快速任務
    static uint32_t timer2_last = 0;  // 中速任務  
    static uint32_t timer3_last = 0;  // 慢速任務
    
    uint32_t now = millis();
    
    // 每 10ms 執行的快速任務
    if (now - timer1_last >= 10) {
        timer1_last = now;
        // 執行快速任務
        // 例如：讀取感測器
    }
    
    // 每 100ms 執行的中速任務
    if (now - timer2_last >= 100) {
        timer2_last = now;
        // 執行中速任務
        // 例如：更新顯示
    }
    
    // 每 1000ms 執行的慢速任務
    if (now - timer3_last >= 1000) {
        timer3_last = now;
        // 執行慢速任務
        // 例如：保存資料
    }
}

/**
 * @brief 範例4: 超時檢查
 */
bool example_timeout_check(void)
{
    static uint32_t operation_start_time = 0;
    static bool operation_started = false;
    const uint32_t TIMEOUT_MS = 5000;  // 5秒超時
    
    if (!operation_started) {
        // 開始操作
        operation_start_time = millis();
        operation_started = true;
        return false;  // 操作進行中
    }
    
    // 檢查是否超時
    if (millis() - operation_start_time > TIMEOUT_MS) {
        operation_started = false;  // 重設狀態
        return true;  // 超時
    }
    
    // 檢查操作是否完成
    // if (operation_completed()) {
    //     operation_started = false;
    //     return false;  // 操作成功完成
    // }
    
    return false;  // 操作繼續進行
}

/**
 * @brief 範例5: 精確延遲（阻塞式）
 */
void example_precise_delay(void)
{
    // 延遲 1 秒
    delay(1000);
    
    // 延遲 500 微秒
    delayMicroseconds(500);
    
    // 注意：這些是阻塞式延遲，會停止程式執行
    // 在主迴圈中使用時要小心
}

/**
 * @brief 範例6: 效能測量
 */
void example_performance_measurement(void)
{
    uint32_t start_us = micros();
    
    // 執行需要測量的程式碼
    // 例如：複雜的計算
    volatile int result = 0;
    for (int i = 0; i < 1000; i++) {
        result += i * i;
    }
    
    uint32_t elapsed_us = micros() - start_us;
    
    // elapsed_us 包含執行時間（微秒）
    // 可以用於最佳化或除錯
}

/**
 * @brief 範例7: Modbus 排程器整合範例
 */
void example_modbus_integration(void)
{
    static uint32_t last_modbus_poll = 0;
    const uint32_t MODBUS_POLL_INTERVAL = 200;  // 200ms = 5Hz
    
    uint32_t now = millis();
    
    if (now - last_modbus_poll >= MODBUS_POLL_INTERVAL) {
        last_modbus_poll = now;
        
        // 執行 Modbus 輪詢
        // modbusScheduler_process();
        
        // 檢查資料更新
        // if (modbusScheduler_isDataReady(...)) {
        //     // 處理新資料
        // }
    }
}

/**
 * @brief 範例8: 溢位安全的時間比較
 */
bool time_elapsed(uint32_t start_time, uint32_t interval)
{
    // 這個函式可以安全處理 millis() 的溢位情況
    return (millis() - start_time) >= interval;
}

void example_overflow_safe_timing(void)
{
    static uint32_t last_action = 0;
    
    if (time_elapsed(last_action, 1000)) {  // 每秒執行
        last_action = millis();
        
        // 執行定期任務
        // 即使 millis() 溢位也能正常工作
    }
}

// =============================================================================
// 在主程式中的整合範例
// =============================================================================

/**
 * @brief 在主迴圈中使用時間函式的範例
 * @note 這個函式展示如何在 while(1) 迴圈中整合時間管理
 */
void example_main_loop_integration(void)
{
    // 這個函式應該在主 while(1) 迴圈中被呼叫
    
    // 非阻塞式定時器
    example_non_blocking_timer();
    
    // 多重定時器管理
    example_multiple_timers();
    
    // 超時檢查
    if (example_timeout_check()) {
        // 處理超時情況
        // 例如：重設通訊或顯示錯誤
    }
    
    // Modbus 整合
    example_modbus_integration();
    
    // 溢位安全的定時
    example_overflow_safe_timing();
}

// =============================================================================
// 時間函式的最佳實務
// =============================================================================

/**
 * 使用建議：
 * 
 * 1. 優先使用非阻塞式定時器而非 delay()
 * 2. 在主迴圈中避免使用 delay() 和 delayMicroseconds()
 * 3. 使用 millis() 進行長時間測量，micros() 進行短時間精確測量
 * 4. 注意 millis() 約49.7天後會溢位，設計時要考慮這點
 * 5. Timer2 中斷優先級設為1，低於ADC中斷，確保不影響馬達控制
 * 
 * 效能影響：
 * - Timer2 中斷每1ms執行一次，執行時間約1-2μs
 * - millis() 函式執行時間約0.5μs
 * - micros() 函式執行時間約1μs
 * - 對主要控制迴圈影響極小
 */ 