/**
 * @file integration_example.c
 * @brief Longwin 韌體整合範例
 * @note 這個檔案展示如何在主程式中整合各種 Longwin 功能模組
 */

#include <stdint.h>
#include <stdbool.h>

// 引入功能控制開關
#include "../codeSw.h"

// 根據功能開關引入相應的標頭檔
#if FEATURE_MODBUS_SCHEDULER_ENABLE
#include "../s_modbus_scheduler.h"
#endif

// 外部變數和函式宣告（來自主程式）
extern volatile uint16_t Speed;
extern signed int TorqMode_IqMax;
extern volatile uint16_t faultOverTempMOSFET;
extern uint32_t millis(void);

// =============================================================================
// 整合初始化函式
// =============================================================================

/**
 * @brief Longwin 功能整合初始化
 * @note 在主程式的初始化階段呼叫
 * @return 0: 成功, -1: 失敗
 */
int longwinIntegration_init(void)
{
    #if FEATURE_MODBUS_SCHEDULER_ENABLE
    // 初始化 Modbus 排程器
    if (!modbusScheduler_init()) {
        #if FEATURE_MODBUS_DEBUG_OUTPUT
        // printf("ERROR: Modbus Scheduler initialization failed!\n");
        #endif
        return -1;
    }
    
    // 配置各個 Slave 設備
    modbusScheduler_configSlave(MODBUS_SCHEDULER_SLAVE_BATTERY, 0x01, 0x0000, 10);
    modbusScheduler_configSlave(MODBUS_SCHEDULER_SLAVE_LCD_APP, 0x02, 0x0000, 8);
    modbusScheduler_configSlave(MODBUS_SCHEDULER_SLAVE_PC_GUI, 0x03, 0x0000, 6);
    
    #if FEATURE_MODBUS_DEBUG_OUTPUT
    // printf("Modbus Scheduler initialized successfully\n");
    #endif
    #endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */
    
    return 0;  // 初始化成功
}

// =============================================================================
// 主迴圈處理函式
// =============================================================================

/**
 * @brief Longwin 功能整合主處理函式
 * @note 在主程式的 while(1) 迴圈中呼叫
 */
void longwinIntegration_process(void)
{
    #if FEATURE_MODBUS_SCHEDULER_ENABLE
    // 處理 Modbus 排程器 - 非阻塞式高頻輪詢
    modbusScheduler_process();
    
    // 處理電池資料
    _processBatteryData();
    
    // 處理 LCD/APP 資料
    _processLcdAppData();
    
    // 處理 PC GUI 資料
    _processPcGuiData();
    #endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */
    
    #if FEATURE_PERFORMANCE_MONITOR
    // 更新效能監控統計
    _updatePerformanceStats();
    #endif
}

// =============================================================================
// 內部處理函式
// =============================================================================

#if FEATURE_MODBUS_SCHEDULER_ENABLE

/**
 * @brief 處理電池資料並整合到馬達控制中
 */
static void _processBatteryData(void)
{
    if (modbusScheduler_isDataReady(MODBUS_SCHEDULER_SLAVE_BATTERY)) {
        const S_MODBUS_SCHEDULER_BATTERY_DATA_T* batteryData = modbusScheduler_getBatteryData();
        if (batteryData != NULL) {
            
            // 根據電池電壓調整最大輸出功率
            if (batteryData->u16Voltage < 4000) {  // 40.00V
                // 電壓過低，降低最大扭矩到40%
                TorqMode_IqMax = Q15(0.4);
            } else if (batteryData->u16Voltage < 4200) {  // 42.00V
                // 電壓偏低，降低最大扭矩到60%
                TorqMode_IqMax = Q15(0.6);
            } else {
                // 電壓正常，恢復正常扭矩
                TorqMode_IqMax = Q15(0.75);
            }
            
            // 根據電池溫度調整保護機制
            if (batteryData->u16Temperature > 600) {  // 60.0°C
                // 電池過熱，進一步降低功率
                if (TorqMode_IqMax > Q15(0.3)) {
                    TorqMode_IqMax = Q15(0.3);
                }
            }
            
            // 根據 SOC 調整功率限制
            if (batteryData->u8StateOfCharge < 20) {  // SOC < 20%
                // 電量不足，啟用節能模式
                if (TorqMode_IqMax > Q15(0.5)) {
                    TorqMode_IqMax = Q15(0.5);
                }
            }
            
            #if FEATURE_MODBUS_DEBUG_OUTPUT
            // printf("Battery: V=%d, T=%d, SOC=%d%%, Iq_max=%.2f\n", 
            //        batteryData->u16Voltage, batteryData->u16Temperature, 
            //        batteryData->u8StateOfCharge, (float)TorqMode_IqMax/32768.0);
            #endif
        }
    }
}

/**
 * @brief 處理 LCD/APP 資料並更新系統設定
 */
static void _processLcdAppData(void)
{
    if (modbusScheduler_isDataReady(MODBUS_SCHEDULER_SLAVE_LCD_APP)) {
        const S_MODBUS_SCHEDULER_LCD_DATA_T* lcdData = modbusScheduler_getLcdData();
        if (lcdData != NULL) {
            
            // 根據使用者設定的控制模式調整系統
            // 這裡可以根據實際需求調整控制邏輯
            
            // 根據輔助等級調整功率
            switch (lcdData->u8AssistLevel) {
                case 1:  // 低輔助
                    if (TorqMode_IqMax > Q15(0.3)) TorqMode_IqMax = Q15(0.3);
                    break;
                case 2:  // 中輔助
                    if (TorqMode_IqMax > Q15(0.5)) TorqMode_IqMax = Q15(0.5);
                    break;
                case 3:  // 高輔助
                    if (TorqMode_IqMax > Q15(0.75)) TorqMode_IqMax = Q15(0.75);
                    break;
                default:
                    // 保持當前設定
                    break;
            }
            
            #if FEATURE_MODBUS_DEBUG_OUTPUT
            // printf("LCD: Mode=%d, Speed=%d, Assist=%d\n", 
            //        lcdData->u8ControlMode, lcdData->u16TargetSpeed, lcdData->u8AssistLevel);
            #endif
        }
    }
}

/**
 * @brief 處理 PC GUI 資料並更新診斷資訊
 */
static void _processPcGuiData(void)
{
    if (modbusScheduler_isDataReady(MODBUS_SCHEDULER_SLAVE_PC_GUI)) {
        // PC GUI 主要用於監控和診斷，通常不會直接影響控制邏輯
        // 但可以用於調整診斷參數或記錄資料
        
        #if FEATURE_DATA_LOGGING_ENABLE
        // 記錄資料到日誌
        // logSystemData();
        #endif
        
        #if FEATURE_MODBUS_DEBUG_OUTPUT
        // printf("PC GUI data received\n");
        #endif
    }
}

#endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */

// =============================================================================
// 效能監控函式
// =============================================================================

#if FEATURE_PERFORMANCE_MONITOR

static uint32_t s_u32LastStatsUpdate = 0;
static uint32_t s_u32LoopCounter = 0;

/**
 * @brief 更新效能監控統計
 */
static void _updatePerformanceStats(void)
{
    s_u32LoopCounter++;
    
    // 每秒更新一次統計資訊
    uint32_t now = millis();
    if (now - s_u32LastStatsUpdate >= 1000) {
        s_u32LastStatsUpdate = now;
        
        // 計算主迴圈頻率
        uint32_t loop_freq = s_u32LoopCounter;
        s_u32LoopCounter = 0;
        
        #if FEATURE_MODBUS_SCHEDULER_ENABLE
        // 取得 Modbus 統計資訊
        const S_MODBUS_SCHEDULER_STATS_T* stats = modbusScheduler_getStats();
        if (stats != NULL) {
            // 檢查通訊品質
            if (stats->u32TotalErrors > 0) {
                uint32_t error_rate = (stats->u32TotalErrors * 100) / 
                                    (stats->u32TotalSuccess + stats->u32TotalErrors);
                
                if (error_rate > 10) {  // 錯誤率 > 10%
                    #if FEATURE_MODBUS_DEBUG_OUTPUT
                    // printf("WARNING: High Modbus error rate: %d%%\n", error_rate);
                    #endif
                }
            }
            
            #if FEATURE_MODBUS_DEBUG_OUTPUT
            // printf("Loop: %dHz, MB Success: %d, Errors: %d, Timeouts: %d\n",
            //        loop_freq, stats->u32TotalSuccess, stats->u32TotalErrors, stats->u32TotalTimeouts);
            #endif
        }
        #endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */
        
        #if FEATURE_MODBUS_DEBUG_OUTPUT
        // printf("Main loop frequency: %d Hz\n", loop_freq);
        #endif
    }
}

#endif /* FEATURE_PERFORMANCE_MONITOR */

// =============================================================================
// CAN 系統整合範例
// =============================================================================

/**
 * @brief 將 Modbus 資料整合到現有的 CAN 系統中
 * @param can_id CAN 訊息 ID
 * @param can_data CAN 資料緩衝區
 * @return 0: 成功, -1: 失敗
 */
int longwinIntegration_handleCanMessage(uint16_t can_id, uint8_t* can_data)
{
    #if FEATURE_MODBUS_SCHEDULER_ENABLE
    
    switch (can_id) {
        case 0x20:  // 請求電池資訊
            {
                const S_MODBUS_SCHEDULER_BATTERY_DATA_T* batteryData = modbusScheduler_getBatteryData();
                if (batteryData != NULL) {
                    can_data[0] = 0x20;
                    can_data[1] = batteryData->u16Voltage & 0xFF;
                    can_data[2] = (batteryData->u16Voltage >> 8) & 0xFF;
                    can_data[3] = batteryData->u16Temperature & 0xFF;
                    can_data[4] = batteryData->u8StateOfCharge;
                    return 0;
                }
            }
            break;
            
        case 0x21:  // 請求 LCD 資訊
            {
                const S_MODBUS_SCHEDULER_LCD_DATA_T* lcdData = modbusScheduler_getLcdData();
                if (lcdData != NULL) {
                    can_data[0] = 0x21;
                    can_data[1] = lcdData->u8ControlMode;
                    can_data[2] = lcdData->u16TargetSpeed & 0xFF;
                    can_data[3] = (lcdData->u16TargetSpeed >> 8) & 0xFF;
                    can_data[4] = lcdData->u8AssistLevel;
                    return 0;
                }
            }
            break;
            
        case 0x25:  // 強制新的 Modbus 輪詢週期
            modbusScheduler_forceNewCycle();
            can_data[0] = 0x25;
            can_data[1] = 0x01;  // 確認
            return 0;
            
        default:
            break;
    }
    
    #endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */
    
    return -1;  // 未處理的訊息
}

// =============================================================================
// 智慧故障診斷範例
// =============================================================================

#if FEATURE_FAULT_DIAGNOSIS_ENHANCED

/**
 * @brief 基於 Modbus 資料的智慧故障判斷
 * @return 故障代碼，0 表示無故障
 */
uint16_t longwinIntegration_smartFaultDiagnosis(void)
{
    #if FEATURE_MODBUS_SCHEDULER_ENABLE
    
    const S_MODBUS_SCHEDULER_BATTERY_DATA_T* batteryData = modbusScheduler_getBatteryData();
    if (batteryData != NULL) {
        
        // 電池相關故障檢查
        if (batteryData->u16Voltage < 3000) {  // 30V
            return 0x1001;  // 電池電壓過低故障
        }
        
        if (batteryData->u16Temperature > 700) {  // 70°C
            return 0x1002;  // 電池過熱故障
        }
        
        if (batteryData->u8StateOfCharge < 5) {  // SOC < 5%
            return 0x1003;  // 電池電量極低故障
        }
        
        // 檢查電池與馬達溫度的關聯性
        if (batteryData->u16Temperature > 500 && faultOverTempMOSFET.monitor < 500) {
            return 0x1004;  // 電池異常發熱（馬達溫度正常但電池過熱）
        }
    }
    
    // 檢查 Modbus 通訊狀態
    const S_MODBUS_SCHEDULER_STATS_T* stats = modbusScheduler_getStats();
    if (stats != NULL) {
        uint32_t total_attempts = stats->u32TotalSuccess + stats->u32TotalErrors + stats->u32TotalTimeouts;
        if (total_attempts > 100) {  // 有足夠的統計樣本
            uint32_t error_rate = ((stats->u32TotalErrors + stats->u32TotalTimeouts) * 100) / total_attempts;
            if (error_rate > 50) {  // 錯誤率 > 50%
                return 0x2001;  // Modbus 通訊故障
            }
        }
    }
    
    #endif /* FEATURE_MODBUS_SCHEDULER_ENABLE */
    
    return 0;  // 無故障
}

#endif /* FEATURE_FAULT_DIAGNOSIS_ENHANCED */

// =============================================================================
// 使用範例說明
// =============================================================================

/**
 * 整合使用範例：
 * 
 * 1. 在主程式的初始化階段：
 *    if (longwinIntegration_init() != 0) {
 *        // 處理初始化失敗
 *    }
 * 
 * 2. 在主迴圈中：
 *    while(1) {
 *        // 現有的主迴圈程式碼...
 *        
 *        longwinIntegration_process();
 *        
 *        // 其他主迴圈程式碼...
 *    }
 * 
 * 3. 在 CAN 訊息處理中：
 *    if (CAN1_Receive(&My_CAN_RXMSG) == true) {
 *        // 先嘗試 Longwin 整合處理
 *        if (longwinIntegration_handleCanMessage(msg_id, can_data) == 0) {
 *            CAN1_Transmit(1, &response_msg);
 *        } else {
 *            // 現有的 CAN 處理邏輯
 *        }
 *    }
 * 
 * 4. 故障診斷：
 *    uint16_t fault_code = longwinIntegration_smartFaultDiagnosis();
 *    if (fault_code != 0) {
 *        // 處理故障
 *    }
 */ 