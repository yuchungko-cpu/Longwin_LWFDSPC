#ifndef S_LOGIC_THROTTLE_H
#define S_LOGIC_THROTTLE_H

#include <stdint.h>   // For uint16_t, uint32_t, UINT16_MAX etc.
#include <stdbool.h>  // For bool type
#include "codeSw.h"
#include "s_logic_motor.h"  // For motor control

// --- 硬體相關參數定義 ---

// Throttle Input (IHS) on Pin RC6
#define THROTTLE_DIVIDER_R1 51000UL
#define THROTTLE_DIVIDER_R2 100000UL

// --- 正反轉方向定義 ---
typedef enum {
    LOGIC_THROTTLE_DIRECTION_FORWARD = 0, /**< 油門正轉 */
    LOGIC_THROTTLE_DIRECTION_REVERSE = 1  /**< 油門反轉 */
} E_LOGIC_THROTTLE_DIRECTION_T;

// --- 正轉油門電壓與輸出對應關係 ---
#define LOGIC_THROTTLE_FWD_VOLTAGE_MIN_MV (800u)  // 正轉油門有效起始電壓 (對應輸出 MIN)，提高 deadzone 抑制低端誤觸/抖動
#define LOGIC_THROTTLE_FWD_VOLTAGE_MAX_MV (4800u)  // 正轉油門最大有效電壓 ( >= 此值則目標為 MAX)

#define LOGIC_THROTTLE_FWD_OUTPUT_ZERO (0u)     // 正轉電壓低於 MIN 時的目標輸出
#define LOGIC_THROTTLE_FWD_OUTPUT_MIN (1500u)   // 正轉對應 VOLTAGE_MIN_MV 的輸出
#define LOGIC_THROTTLE_FWD_OUTPUT_MAX (12000u)  // 正轉對應 VOLTAGE_MAX_MV 的輸出

// --- 反轉油門電壓與輸出對應關係 ---
#define LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV (800u)  // 反轉油門有效起始電壓 (對應輸出 MIN)，提高 deadzone 抑制低端誤觸/抖動
#define LOGIC_THROTTLE_REV_VOLTAGE_MAX_MV (4800u)  // 反轉油門最大有效電壓 ( >= 此值則目標為 MAX)

#define LOGIC_THROTTLE_REV_OUTPUT_ZERO (0u)     // 反轉電壓低於 MIN 時的目標輸出
#define LOGIC_THROTTLE_REV_OUTPUT_MIN (1500u)   // 反轉對應 VOLTAGE_MIN_MV 的輸出
#define LOGIC_THROTTLE_REV_OUTPUT_MAX (4000u)  // 反轉對應 VOLTAGE_MAX_MV 的輸出

// 開機安全檢查
#define LOGIC_THROTTLE_POWER_ON_CHECK_MV (1000u)  // 開機檢測電壓閾值，油門電壓需低於此值才視為安全/已釋放

// 運行時故障檢測電壓閾值
#define LOGIC_THROTTLE_FAULT_VOLTAGE_LOW_MV (0u)     //300 油門信號電壓過低故障閾值 (mV), e.g., open circuit
#define LOGIC_THROTTLE_FAULT_VOLTAGE_HIGH_MV (5000u)   //4500 油門信號電壓過高故障閾值 (mV), e.g., short to VCC
#define LOGIC_THROTTLE_FAULT_DECEL_STEP_TIME {-10, 1}  // 運行時故障的快速減速曲線 {Step, Time}

// --- 正轉加速/減速 Step/Time 表格 ---
// 加速段 (電壓上升)
#define LOGIC_THROTTLE_FWD_ACCEL_V1_MV (1200u)
#define LOGIC_THROTTLE_FWD_ACCEL_V2_MV (2100u)
#define LOGIC_THROTTLE_FWD_ACCEL_V3_MV (3000u)
#define LOGIC_THROTTLE_FWD_ACCEL_V4_MV (3900u)

// 減速段 (輸出值下降)
#define LOGIC_THROTTLE_FWD_DECEL_O1 (3600u)
#define LOGIC_THROTTLE_FWD_DECEL_O2 (5700u)
#define LOGIC_THROTTLE_FWD_DECEL_O3 (7800u)
#define LOGIC_THROTTLE_FWD_DECEL_O4 (9900u)

// --- 反轉加速/減速 Step/Time 表格 ---
// 加速段 (電壓上升)
#define LOGIC_THROTTLE_REV_ACCEL_V1_MV (1200u)
#define LOGIC_THROTTLE_REV_ACCEL_V2_MV (2100u)
#define LOGIC_THROTTLE_REV_ACCEL_V3_MV (3000u)
#define LOGIC_THROTTLE_REV_ACCEL_V4_MV (3900u)

// 減速段 (輸出值下降)
#define LOGIC_THROTTLE_REV_DECEL_O1 (2000u)
#define LOGIC_THROTTLE_REV_DECEL_O2 (2500u)
#define LOGIC_THROTTLE_REV_DECEL_O3 (3000u)
#define LOGIC_THROTTLE_REV_DECEL_O4 (3500u)

// --- Throttle Forward Acceleration Step Time Table Definitions ---
// Format: {Step, TimeMs}
#define THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_0 {2, 1}  // <= LOGIC_THROTTLE_FWD_ACCEL_V1_MV (1700mV)
#define THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_1 {3, 2}  // <= LOGIC_THROTTLE_FWD_ACCEL_V2_MV (2200mV)
#define THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_2 {4, 3}  // <= LOGIC_THROTTLE_FWD_ACCEL_V3_MV (2700mV)
#define THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_3 {5, 4}  // <= LOGIC_THROTTLE_FWD_ACCEL_V4_MV (3200mV)
#define THROTTLE_FWD_ACCEL_STEP_TIME_ENTRY_4 {6, 4}  // > LOGIC_THROTTLE_FWD_ACCEL_V4_MV (3200mV)

// --- Throttle Forward Deceleration Step Time Table Definitions ---
// Format: {Step, TimeMs}
#define THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_0 {-25, 1}  // <= LOGIC_THROTTLE_FWD_DECEL_O1 (4800)
#define THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_1 {-30, 1}  // <= LOGIC_THROTTLE_FWD_DECEL_O2 (7600)
#define THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_2 {-35, 1}  // <= LOGIC_THROTTLE_FWD_DECEL_O3 (10400)
#define THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_3 {-40, 1}  // <= LOGIC_THROTTLE_FWD_DECEL_O4 (13200)
#define THROTTLE_FWD_DECEL_STEP_TIME_ENTRY_4 {-40, 1}  // > LOGIC_THROTTLE_FWD_DECEL_O4 (13200)

// --- Throttle Reverse Acceleration Step Time Table Definitions ---
// Format: {Step, TimeMs}
#define THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_0 {1, 2}  // <= LOGIC_THROTTLE_REV_ACCEL_V1_MV (1700mV)
#define THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_1 {1, 3}  // <= LOGIC_THROTTLE_REV_ACCEL_V2_MV (2200mV)
#define THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_2 {1, 3}  // <= LOGIC_THROTTLE_REV_ACCEL_V3_MV (2700mV)
#define THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_3 {2, 4}  // <= LOGIC_THROTTLE_REV_ACCEL_V4_MV (3200mV)
#define THROTTLE_REV_ACCEL_STEP_TIME_ENTRY_4 {3, 4}  // > LOGIC_THROTTLE_REV_ACCEL_V4_MV (3200mV)

// --- Throttle Reverse Deceleration Step Time Table Definitions ---
// Format: {Step, TimeMs}
#define THROTTLE_REV_DECEL_STEP_TIME_ENTRY_0 {-10, 1}  // <= LOGIC_THROTTLE_REV_DECEL_O1 (4800)
#define THROTTLE_REV_DECEL_STEP_TIME_ENTRY_1 {-13, 1}  // <= LOGIC_THROTTLE_REV_DECEL_O2 (7600)
#define THROTTLE_REV_DECEL_STEP_TIME_ENTRY_2 {-16, 1}  // <= LOGIC_THROTTLE_REV_DECEL_O3 (10400)
#define THROTTLE_REV_DECEL_STEP_TIME_ENTRY_3 {-19, 1}  // <= LOGIC_THROTTLE_REV_DECEL_O4 (13200)
#define THROTTLE_REV_DECEL_STEP_TIME_ENTRY_4 {-22, 1}  // > LOGIC_THROTTLE_REV_DECEL_O4 (13200)
                                                      //
// 初始化定義最高轉速
#define THROTTLE_ASSIST_LEVEL_DEFAULT (5)
#define THROTTLE_ASSIST_LEVEL_MAX_OUTPUT_VALUES {0, 4000, 6000, 8000, 10000, 12000} //  9500=4km  10500=4.5km  11800=5km  12800=5.5km  14000=6km 

// --- 函數宣告 ---

/**
 * @brief 初始化油門邏輯模組並執行開機安全檢查。
 * @note  此函式應在系統啟動時呼叫一次。
 *        它會檢查初始油門電壓是否處於安全區間(低於 LOGIC_THROTTLE_POWER_ON_CHECK_MV)。
 *        如果電壓過高，會設定內部抑制旗標，並回傳錯誤碼。
 *
 * @param u16InitialVoltageMv 開機時檢測到的初始油門電壓 (單位: mV)。
 * @return int8_t 0: 初始化成功, -1: 開機時油門電壓過高，進入抑制狀態。
 */
int8_t logic_throttle_initAndCheck(uint16_t u16InitialVoltageMv);

/**
 * @brief 根據即時的油門電壓，更新並計算出對應的目標轉速(Target RPM)及加減速曲線。
 * @note  此函式應在主迴圈中定期呼叫。
 *        它包含了完整的油門邏輯，包括：
 *        - 運行時故障檢測 (電壓過高/過低)
 *        - 開機抑制狀態的恢復邏輯
 *        - 根據電壓計算目標轉速
 *        - 根據當前狀態(加速/減速)查表取得對應的加減速 Step/Time 參數
 *
 * @param psStepTime [輸出] 計算後得到的加減速步進/時間參數，將回填此指標指向的結構。
 * @param pu16TargetRpm [輸出] 計算後得到的目標轉速，將回填此指標指向的變數。
 * @param u16CurrentRpm [輸入] 馬達目前的轉速，用於判斷加速或減速。
 * @param u16CurrentVoltageMv [輸入] 即時的油門電壓值 (單位: mV)。
 * @param eDirection [輸入] 當前的馬達運轉方向 (正轉/反轉)。
 * @param u8AssistLevel [輸入] 當前段位對應的動態最高轉速上限 (僅對正轉有效)。
 * @return int8_t 0: 成功, -1: 運行時故障, -2: 仍處於開機抑制狀態。
 */
int8_t logic_throttle_getUpdateParams(S_MOTOR_STEP_TIME_T *psStepTime,
                                      uint16_t *pu16TargetRpm,
                                      uint16_t u16CurrentRpm,
                                      uint16_t u16CurrentVoltageMv,
                                      E_LOGIC_THROTTLE_DIRECTION_T eDirection,
                                      uint8_t u8AssistLevel);

#endif  // S_LOGIC_THROTTLE_H
