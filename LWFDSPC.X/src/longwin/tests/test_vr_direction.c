/**
 * @file test_vr_direction.c
 * @brief VR方向偵測功能測試
 * @author CHUN CHI
 * @date 2025
 * 
 * @details 此測試檔案驗證VR方向偵測功能的正確性：
 *          - 方向判斷邏輯測試
 *          - 方向變化偵測測試
 *          - 邊界條件測試
 *          - 錯誤處理測試
 */

#include "s_logic_vr.h"
#include <stdio.h>
#include <assert.h>

// --- 測試輔助函數 ---

/**
 * @brief 測試方向判斷邏輯
 */
void testDirectionLogic(void)
{
    printf("測試方向判斷邏輯...\n");
    
    // 測試停止狀態 (2.2V ~ 2.8V)
    uint16_t u16StopAdc1 = (2200 * 4095) / 3300; // 2.2V
    uint16_t u16StopAdc2 = (2500 * 4095) / 3300; // 2.5V
    uint16_t u16StopAdc3 = (2800 * 4095) / 3300; // 2.8V
    
    assert(logic_vr_getCurrentDirection(u16StopAdc1) == LOGIC_VR_DIRECTION_STOPPED);
    assert(logic_vr_getCurrentDirection(u16StopAdc2) == LOGIC_VR_DIRECTION_STOPPED);
    assert(logic_vr_getCurrentDirection(u16StopAdc3) == LOGIC_VR_DIRECTION_STOPPED);
    
    // 測試正轉狀態 (> 2.8V)
    uint16_t u16ForwardAdc1 = (2900 * 4095) / 3300; // 2.9V
    uint16_t u16ForwardAdc2 = (3500 * 4095) / 3300; // 3.5V
    uint16_t u16ForwardAdc3 = (5000 * 4095) / 3300; // 5.0V
    
    assert(logic_vr_getCurrentDirection(u16ForwardAdc1) == LOGIC_VR_DIRECTION_FORWARD);
    assert(logic_vr_getCurrentDirection(u16ForwardAdc2) == LOGIC_VR_DIRECTION_FORWARD);
    assert(logic_vr_getCurrentDirection(u16ForwardAdc3) == LOGIC_VR_DIRECTION_FORWARD);
    
    // 測試反轉狀態 (< 2.2V)
    uint16_t u16ReverseAdc1 = (2100 * 4095) / 3300; // 2.1V
    uint16_t u16ReverseAdc2 = (1500 * 4095) / 3300; // 1.5V
    uint16_t u16ReverseAdc3 = (0 * 4095) / 3300;    // 0V
    
    assert(logic_vr_getCurrentDirection(u16ReverseAdc1) == LOGIC_VR_DIRECTION_REVERSE);
    assert(logic_vr_getCurrentDirection(u16ReverseAdc2) == LOGIC_VR_DIRECTION_REVERSE);
    assert(logic_vr_getCurrentDirection(u16ReverseAdc3) == LOGIC_VR_DIRECTION_REVERSE);
    
    printf("✓ 方向判斷邏輯測試通過\n");
}

/**
 * @brief 測試方向變化偵測
 */
void testDirectionChangeDetection(void)
{
    printf("測試方向變化偵測...\n");
    
    // 初始化VR模組
    uint16_t u16InitAdc = (2500 * 4095) / 3300; // 2.5V (停止狀態)
    assert(logic_vr_initAndCheck(u16InitAdc) == 0);
    
    // 測試從停止到正轉
    uint16_t u16ForwardAdc = (3000 * 4095) / 3300; // 3.0V
    assert(logic_vr_isDirectionChanged(u16ForwardAdc) == true);
    assert(logic_vr_getCurrentDirection(u16ForwardAdc) == LOGIC_VR_DIRECTION_FORWARD);
    
    // 測試從正轉到反轉
    uint16_t u16ReverseAdc = (1500 * 4095) / 3300; // 1.5V
    assert(logic_vr_isDirectionChanged(u16ReverseAdc) == true);
    assert(logic_vr_getCurrentDirection(u16ReverseAdc) == LOGIC_VR_DIRECTION_REVERSE);
    
    // 測試從反轉到停止
    uint16_t u16StopAdc = (2500 * 4095) / 3300; // 2.5V
    assert(logic_vr_isDirectionChanged(u16StopAdc) == true);
    assert(logic_vr_getCurrentDirection(u16StopAdc) == LOGIC_VR_DIRECTION_STOPPED);
    
    // 測試無變化情況
    uint16_t u16SameAdc = (2500 * 4095) / 3300; // 2.5V (與上次相同)
    assert(logic_vr_isDirectionChanged(u16SameAdc) == false);
    
    printf("✓ 方向變化偵測測試通過\n");
}

/**
 * @brief 測試完整方向狀態獲取
 */
void testCompleteDirectionStatus(void)
{
    printf("測試完整方向狀態獲取...\n");
    
    // 測試正轉狀態
    uint16_t u16ForwardAdc = (3500 * 4095) / 3300; // 3.5V
    S_LOGIC_VR_DIRECTION_STATUS_T stDirectionStatus;
    
    assert(logic_vr_getDirectionStatus(u16ForwardAdc, &stDirectionStatus) == 0);
    assert(stDirectionStatus.eCurrentDirection == LOGIC_VR_DIRECTION_FORWARD);
    assert(stDirectionStatus.u16DirectionVoltageMv >= 3500);
    assert(stDirectionStatus.bDirectionChanged == true); // 應該檢測到變化
    
    // 測試反轉狀態
    uint16_t u16ReverseAdc = (1500 * 4095) / 3300; // 1.5V
    assert(logic_vr_getDirectionStatus(u16ReverseAdc, &stDirectionStatus) == 0);
    assert(stDirectionStatus.eCurrentDirection == LOGIC_VR_DIRECTION_REVERSE);
    assert(stDirectionStatus.u16DirectionVoltageMv <= 1500);
    assert(stDirectionStatus.bDirectionChanged == true);
    
    // 測試停止狀態
    uint16_t u16StopAdc = (2500 * 4095) / 3300; // 2.5V
    assert(logic_vr_getDirectionStatus(u16StopAdc, &stDirectionStatus) == 0);
    assert(stDirectionStatus.eCurrentDirection == LOGIC_VR_DIRECTION_STOPPED);
    assert(stDirectionStatus.u16DirectionVoltageMv >= 2500);
    assert(stDirectionStatus.bDirectionChanged == true);
    
    printf("✓ 完整方向狀態獲取測試通過\n");
}

/**
 * @brief 測試錯誤處理
 */
void testErrorHandling(void)
{
    printf("測試錯誤處理...\n");
    
    // 測試NULL指標
    S_LOGIC_VR_DIRECTION_STATUS_T *psNullPointer = NULL;
    uint16_t u16TestAdc = (2500 * 4095) / 3300;
    
    assert(logic_vr_getDirectionStatus(u16TestAdc, psNullPointer) == -1);
    
    // 測試開機抑制
    uint16_t u16InvalidAdc = (1000 * 4095) / 3300; // 1.0V (低於開機檢測下限)
    assert(logic_vr_initAndCheck(u16InvalidAdc) == -1);
    
    printf("✓ 錯誤處理測試通過\n");
}

/**
 * @brief 測試邊界條件
 */
void testBoundaryConditions(void)
{
    printf("測試邊界條件...\n");
    
    // 測試死區邊界
    uint16_t u16DeadzoneLowAdc = (2200 * 4095) / 3300;  // 2.2V (死區下限)
    uint16_t u16DeadzoneHighAdc = (2800 * 4095) / 3300; // 2.8V (死區上限)
    
    assert(logic_vr_getCurrentDirection(u16DeadzoneLowAdc) == LOGIC_VR_DIRECTION_STOPPED);
    assert(logic_vr_getCurrentDirection(u16DeadzoneHighAdc) == LOGIC_VR_DIRECTION_STOPPED);
    
    // 測試死區外邊界
    uint16_t u16JustBelowDeadzoneAdc = (2199 * 4095) / 3300; // 2.199V
    uint16_t u16JustAboveDeadzoneAdc = (2801 * 4095) / 3300; // 2.801V
    
    assert(logic_vr_getCurrentDirection(u16JustBelowDeadzoneAdc) == LOGIC_VR_DIRECTION_REVERSE);
    assert(logic_vr_getCurrentDirection(u16JustAboveDeadzoneAdc) == LOGIC_VR_DIRECTION_FORWARD);
    
    // 測試極限值
    uint16_t u16MaxAdc = 4095; // 最大ADC值
    uint16_t u16MinAdc = 0;    // 最小ADC值
    
    assert(logic_vr_getCurrentDirection(u16MaxAdc) == LOGIC_VR_DIRECTION_FORWARD);
    assert(logic_vr_getCurrentDirection(u16MinAdc) == LOGIC_VR_DIRECTION_REVERSE);
    
    printf("✓ 邊界條件測試通過\n");
}

/**
 * @brief 執行所有測試
 */
void runAllVrDirectionTests(void)
{
    printf("開始VR方向偵測功能測試\n");
    printf("======================\n\n");
    
    testDirectionLogic();
    testDirectionChangeDetection();
    testCompleteDirectionStatus();
    testErrorHandling();
    testBoundaryConditions();
    
    printf("\n所有測試完成！✓\n");
    printf("VR方向偵測功能測試通過\n");
}

/**
 * @brief 性能測試
 */
void testPerformance(void)
{
    printf("執行性能測試...\n");
    
    const int iTestIterations = 1000;
    uint16_t u16TestAdc = (2500 * 4095) / 3300;
    
    // 測試方向判斷性能
    for (int i = 0; i < iTestIterations; i++) {
        logic_vr_getCurrentDirection(u16TestAdc);
    }
    
    // 測試方向狀態獲取性能
    S_LOGIC_VR_DIRECTION_STATUS_T stDirectionStatus;
    for (int i = 0; i < iTestIterations; i++) {
        logic_vr_getDirectionStatus(u16TestAdc, &stDirectionStatus);
    }
    
    // 測試方向變化偵測性能
    for (int i = 0; i < iTestIterations; i++) {
        logic_vr_isDirectionChanged(u16TestAdc);
    }
    
    printf("✓ 性能測試完成 (%d 次迭代)\n", iTestIterations);
} 