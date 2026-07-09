/**
 * @file s_logic_temp_controller.c
 * @brief 控制器溫度保護模組的實作
 */

#include "s_logic_temp_controller.h"
#include "s_logic_error_handler.h"
// #include <libq.h> // For Q15 macro
#include "../general.h"

// --- 靜態模組變數 ---

//! 當前溫度保護所在的區間狀態
static E_LOGIC_TEMP_CONTROLLER_ZONE_T s_temp_currentZone = LOGIC_TEMP_CONTROLLER_ZONE_NORMAL;
//! 當前最新的控制器溫度 (單位: °C * 100)
static int32_t s_temp_controller_currentTempC = 2500;

// --- 內部輔助函數 ---

/**
 * @brief 透過查表與線性內插法，將 ADC 讀值轉換為溫度
 * @param u16AdcValue ADC 原始讀值
 * @return int32_t 溫度值 (單位: °C * 100)
 */
static int32_t _convertAdcToTemp(uint16_t u16AdcValue) {
    // 處理邊界情況：如果 ADC 值超出表格範圍，直接回傳端點溫度
    if (u16AdcValue <= temp_lookup_table[0].adc_value) {
        return temp_lookup_table[0].temp_c_100;
    }
    if (u16AdcValue >= temp_lookup_table[temp_lookup_table_size - 1].adc_value) {
        return temp_lookup_table[temp_lookup_table_size - 1].temp_c_100;
    }

    // 遍歷表格，尋找 ADC 值所在的區間
    for (size_t i = 0; i < temp_lookup_table_size - 1; ++i) {
        if (u16AdcValue >= temp_lookup_table[i].adc_value && u16AdcValue < temp_lookup_table[i + 1].adc_value) {
            // 找到區間後，進行線性內插計算，以獲得更精確的溫度值
            uint16_t x0 = temp_lookup_table[i].adc_value;
            int32_t y0 = temp_lookup_table[i].temp_c_100;
            uint16_t x1 = temp_lookup_table[i + 1].adc_value;
            int32_t y1 = temp_lookup_table[i + 1].temp_c_100;
            return y0 + ((int32_t)(u16AdcValue - x0) * (y1 - y0)) / (x1 - x0);
        }
    }
    // 理論上不應執行到此處，但作為保護返回最後一個值
    return temp_lookup_table[temp_lookup_table_size - 1].temp_c_100;
}

/**
 * @brief 根據溫度直接判斷其所屬的目標區間 (不考慮遲滯效應)
 * @param s32ControllerTempC 溫度值
 * @return E_LOGIC_TEMP_CONTROLLER_ZONE_T 目標區間
 */
static E_LOGIC_TEMP_CONTROLLER_ZONE_T _determineZone(int32_t s32ControllerTempC) {
    // 為了效率，判斷順序從最可能發生的最高溫開始
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_OVERTEMP_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP;
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_5_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_5;
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_4_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_4;
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_3_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_3;
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_2_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_2;
    if (s32ControllerTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_1_C)
        return LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_1;
    return LOGIC_TEMP_CONTROLLER_ZONE_NORMAL;
}

// --- 公開函式實作 ---

void logic_temp_controller_init(void) {
    s_temp_currentZone = LOGIC_TEMP_CONTROLLER_DEFAULT_CURRENT_ZONE;
    // 模組初始化時，清除相關的警報旗標
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A05_MOTOR_OVERCURRENT, false);
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP, false);
}

int32_t logic_temp_controller_getTemp(void) {
    return s_temp_controller_currentTempC;
}

int16_t logic_temp_controller_updateTempAndGetCurrentLimit(uint16_t u16AdcValue,
                                                           bool *pbIsOverTemp,
                                                           bool *pbIsOverLoad) {
    // 步驟 1: 更新內部溫度值
    s_temp_controller_currentTempC = _convertAdcToTemp(u16AdcValue);

    // 初始化輸出旗標
    if (pbIsOverTemp != NULL)
        *pbIsOverTemp = false;

    if (pbIsOverLoad != NULL)
        *pbIsOverLoad = false;

    E_LOGIC_TEMP_CONTROLLER_ZONE_T ePreviousZone = s_temp_currentZone;
    E_LOGIC_TEMP_CONTROLLER_ZONE_T eNextZone = s_temp_currentZone;

    // 步驟 2: 根據當前溫度和歷史狀態，更新溫度區間 (狀態機)
    // 這個狀態機實作了「遲滯效應」，避免在閾值邊緣頻繁切換狀態
    switch (s_temp_currentZone) {
        case LOGIC_TEMP_CONTROLLER_ZONE_NORMAL:
            // 正常區間：只有在溫度上升超過第一級限流點時，才需要重新判斷區間
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_1_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_1:
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_2_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);  // 溫度繼續上升
            else if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_1_C)
                eNextZone = LOGIC_TEMP_CONTROLLER_ZONE_NORMAL;  // 溫度下降，恢復正常
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_2:
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_3_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            else if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_2_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_3:
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_4_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            else if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_3_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_4:
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_LEVEL_5_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            else if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_4_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_5:
            if (s_temp_controller_currentTempC >= LOGIC_TEMP_CONTROLLER_ACTIVATE_OVERTEMP_C)
                eNextZone = LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP;
            else if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_LEVEL_5_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        case LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP:
            if (s_temp_controller_currentTempC < LOGIC_TEMP_CONTROLLER_RELEASE_OVERTEMP_C)
                eNextZone = _determineZone(s_temp_controller_currentTempC);
            break;
        default:  // 未知狀態，重置為安全狀態
            logic_temp_controller_init();
            eNextZone = s_temp_currentZone;
            break;
    }
    s_temp_currentZone = eNextZone;

    // 步驟 3: 根據狀態變化，更新全局的錯誤/警報旗標
    bool bIsInLimitingZone = (s_temp_currentZone > LOGIC_TEMP_CONTROLLER_ZONE_NORMAL &&
                              s_temp_currentZone < LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP);

    if (ePreviousZone != s_temp_currentZone) {
        // 更新過溫警報 (A09)
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP,
                                          s_temp_currentZone == LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP);
        // 更新過載警報 (A05)
        //logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A05_MOTOR_OVERCURRENT,
        //                                  bIsInLimitingZone);
    }

    // 設定過載旗標，供外部參考
    if (bIsInLimitingZone && pbIsOverLoad != NULL)
        *pbIsOverLoad = true;

    // 步驟 4: 根據最終的溫度區間，回傳對應的 Q15 電流比例
    switch (s_temp_currentZone) {
        case LOGIC_TEMP_CONTROLLER_ZONE_OVERTEMP:
            if (pbIsOverTemp != NULL)
                *pbIsOverTemp = true;
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_OVERTEMP);
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_5:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_5);
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_4:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_4);
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_3:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_3);
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_2:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_2);
        case LOGIC_TEMP_CONTROLLER_ZONE_LEVEL_1:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_LEVEL_1);
        default:
            return Q15(LOGIC_TEMP_CONTROLLER_RATIO_NORMAL);
    }
}
