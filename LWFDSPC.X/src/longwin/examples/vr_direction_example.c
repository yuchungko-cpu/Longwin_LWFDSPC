/**
 * @file vr_direction_example.c
 * @brief VR方向偵測功能使用範例
 * @author CHUN CHI
 * @date 2025
 * 
 * @details 此範例展示如何使用VR方向偵測功能：
 *          - 基本方向狀態獲取
 *          - 方向變化偵測
 *          - 方向指示燈控制
 *          - 安全保護機制
 */

#include "s_logic_vr.h"
#include <stdio.h>

// --- 模擬ADC讀取函數 ---
static uint16_t s_u16SimulatedVrAdc = 2048; // 模擬ADC值 (約2.5V)

/**
 * @brief 模擬ADC讀取VR值
 * @return uint16_t ADC值
 */
static uint16_t simulateVrAdcRead(void)
{
    // 模擬VR電壓變化
    static uint16_t s_u16VoltageMv = 2500; // 起始電壓 2.5V
    static int16_t s_i16VoltageStep = 100;  // 電壓變化步長
    
    // 模擬電壓變化 (在0V到5V之間循環)
    s_u16VoltageMv += s_i16VoltageStep;
    if (s_u16VoltageMv >= 5000) {
        s_u16VoltageMv = 5000;
        s_i16VoltageStep = -100; // 開始下降
    } else if (s_u16VoltageMv <= 0) {
        s_u16VoltageMv = 0;
        s_i16VoltageStep = 100;  // 開始上升
    }
    
    // 將電壓轉換為ADC值 (簡化計算)
    s_u16SimulatedVrAdc = (s_u16VoltageMv * 4095) / 5000;
    
    return s_u16SimulatedVrAdc;
}

/**
 * @brief 範例1：基本方向狀態獲取
 */
void example1_basicDirectionStatus(void)
{
    printf("=== 範例1：基本方向狀態獲取 ===\n");
    
    uint16_t u16VrAdc = simulateVrAdcRead();
    S_LOGIC_VR_DIRECTION_STATUS_T stDirectionStatus;
    
    if (logic_vr_getDirectionStatus(u16VrAdc, &stDirectionStatus) == 0) {
        printf("VR ADC值: %d\n", u16VrAdc);
        printf("VR電壓: %d mV\n", stDirectionStatus.u16DirectionVoltageMv);
        
        switch (stDirectionStatus.eCurrentDirection) {
            case LOGIC_VR_DIRECTION_FORWARD:
                printf("方向狀態: 正轉\n");
                break;
            case LOGIC_VR_DIRECTION_REVERSE:
                printf("方向狀態: 反轉\n");
                break;
            case LOGIC_VR_DIRECTION_STOPPED:
                printf("方向狀態: 停止\n");
                break;
        }
        
        if (stDirectionStatus.bDirectionChanged) {
            printf("*** 方向發生變化！ ***\n");
        }
    }
}

/**
 * @brief 範例2：方向指示燈控制
 */
void example2_directionIndicatorControl(void)
{
    printf("=== 範例2：方向指示燈控制 ===\n");
    
    uint16_t u16VrAdc = simulateVrAdcRead();
    E_LOGIC_VR_DIRECTION_T eDirection = logic_vr_getCurrentDirection(u16VrAdc);
    
    // 模擬LED控制
    switch (eDirection) {
        case LOGIC_VR_DIRECTION_FORWARD:
            printf("LED控制: 綠燈亮起 (正轉)\n");
            // 實際應用中：GPIO_SetPin(LED_GREEN, HIGH);
            break;
        case LOGIC_VR_DIRECTION_REVERSE:
            printf("LED控制: 紅燈亮起 (反轉)\n");
            // 實際應用中：GPIO_SetPin(LED_RED, HIGH);
            break;
        case LOGIC_VR_DIRECTION_STOPPED:
            printf("LED控制: 黃燈亮起 (停止)\n");
            // 實際應用中：GPIO_SetPin(LED_YELLOW, HIGH);
            break;
    }
}

/**
 * @brief 範例3：安全保護機制
 */
void example3_safetyProtection(void)
{
    printf("=== 範例3：安全保護機制 ===\n");
    
    uint16_t u16VrAdc = simulateVrAdcRead();
    
    // 檢查方向變化
    if (logic_vr_isDirectionChanged(u16VrAdc)) {
        printf("*** 安全警告：VR方向發生變化！ ***\n");
        printf("建議操作：\n");
        printf("1. 立即停止馬達輸出\n");
        printf("2. 等待速度降至安全範圍\n");
        printf("3. 確認新方向後重新啟動\n");
        
        // 實際應用中：
        // - 立即停止馬達
        // - 觸發安全警報
        // - 記錄事件日誌
    }
}

/**
 * @brief 範例4：方向變化統計
 */
void example4_directionChangeStatistics(void)
{
    printf("=== 範例4：方向變化統計 ===\n");
    
    static uint32_t su32ForwardCount = 0;
    static uint32_t su32ReverseCount = 0;
    static uint32_t su32StoppedCount = 0;
    static uint32_t su32ChangeCount = 0;
    
    uint16_t u16VrAdc = simulateVrAdcRead();
    S_LOGIC_VR_DIRECTION_STATUS_T stDirectionStatus;
    
    if (logic_vr_getDirectionStatus(u16VrAdc, &stDirectionStatus) == 0) {
        // 統計各方向使用次數
        switch (stDirectionStatus.eCurrentDirection) {
            case LOGIC_VR_DIRECTION_FORWARD:
                su32ForwardCount++;
                break;
            case LOGIC_VR_DIRECTION_REVERSE:
                su32ReverseCount++;
                break;
            case LOGIC_VR_DIRECTION_STOPPED:
                su32StoppedCount++;
                break;
        }
        
        // 統計方向變化次數
        if (stDirectionStatus.bDirectionChanged) {
            su32ChangeCount++;
        }
        
        printf("方向統計：\n");
        printf("- 正轉次數: %lu\n", su32ForwardCount);
        printf("- 反轉次數: %lu\n", su32ReverseCount);
        printf("- 停止次數: %lu\n", su32StoppedCount);
        printf("- 變化次數: %lu\n", su32ChangeCount);
    }
}

/**
 * @brief 主範例函數
 */
void vrDirectionExampleMain(void)
{
    printf("VR方向偵測功能範例開始\n");
    printf("========================\n\n");
    
    // 初始化VR模組
    uint16_t u16InitialAdc = simulateVrAdcRead();
    if (logic_vr_initAndCheck(u16InitialAdc) != 0) {
        printf("錯誤：VR初始化失敗\n");
        return;
    }
    
    // 執行各個範例
    for (int i = 0; i < 10; i++) {
        printf("\n--- 第 %d 次測試 ---\n", i + 1);
        
        example1_basicDirectionStatus();
        example2_directionIndicatorControl();
        example3_safetyProtection();
        example4_directionChangeStatistics();
        
        // 模擬時間延遲
        // 實際應用中：delay_ms(1000);
    }
    
    printf("\nVR方向偵測功能範例結束\n");
}

/**
 * @brief 使用說明
 */
void printUsageInstructions(void)
{
    printf("VR方向偵測功能使用說明：\n");
    printf("========================\n");
    printf("1. 基本方向狀態獲取：使用 logic_vr_getDirectionStatus()\n");
    printf("2. 簡化方向檢查：使用 logic_vr_getCurrentDirection()\n");
    printf("3. 方向變化偵測：使用 logic_vr_isDirectionChanged()\n");
    printf("4. 應用場景：\n");
    printf("   - 方向指示燈控制\n");
    printf("   - 安全保護機制\n");
    printf("   - 使用者介面顯示\n");
    printf("   - 使用模式分析\n");
    printf("\n注意事項：\n");
    printf("- 方向變化時應立即停止馬達輸出\n");
    printf("- 等待速度降至安全範圍後再重新啟動\n");
    printf("- 建議在方向變化時觸發安全警報\n");
} 