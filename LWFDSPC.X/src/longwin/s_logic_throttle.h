#ifndef S_LOGIC_THROTTLE_H
#define S_LOGIC_THROTTLE_H

#include <stdint.h>   // For uint16_t, uint32_t, UINT16_MAX etc.
#include <stdbool.h>  // For bool type
#include "codeSw.h"
#include "s_logic_motor.h"  // For motor control
#include "../motor_scale.h"  // RPMX10_TO_CMD / SPEED_FS_RPM

// *****************************************************************************
//  【重要】本模組所有 OUTPUT_xxx 與 DECEL_Oxx 的單位是「Q15 速度命令 count」，
//  不是 RPM (雖然變數名為 u16TargetRpm)。它們最終流向：
//     i16ActiveRpm -> ReferenceRAW -> ctrlParm.qVelRef -> piInputOmega.inReference
//  而速度環的另一端 inMeasure = Speed 是 Q15 (滿刻度 32768 = SPEED_FS_RPM 機械 RPM)。
//
//  為了讓這些常數與 Q15 刻度解耦，改用 RPMX10_TO_CMD(機械RPM * 10) 表示。
//  換算 (SPEED_FS_RPM=12000, 齒比 20.3, 8 吋輪)：
//     1 count = 0.366 機械 RPM = 0.018 輪 RPM = 0.00069 km/h
//  註解中的 km/h 為理論值；實機頂速約 7.2 km/h (≈10400 counts)。
// *****************************************************************************

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
#define LOGIC_THROTTLE_FWD_VOLTAGE_MIN_MV (1200u)  // 正轉油門有效起始電壓 (對應輸出 MIN)，提高 deadzone 抑制低端誤觸/抖動
#define LOGIC_THROTTLE_FWD_VOLTAGE_MAX_MV (4800u)  // 正轉油門最大有效電壓 ( >= 此值則目標為 MAX)

#define LOGIC_THROTTLE_FWD_OUTPUT_ZERO (0u)                  // 正轉電壓低於 MIN 時的目標輸出
// [實車調校 2026-08-14] 5493 (1.04 km/h) → 5124 (512.4 RPM = 0.97 km/h = 1399 count)。
//   與 ACCEL_FILTER_START_CMD_DEFAULT 同步下降 —— 本值是「最低可命令車速」的**真正**決定者:
//   它同時是油門內插的地板、加速中的地板，以及加速濾波預載與每 tick 輸出的地板 (u16MinRpm)。
//   只改 ACCEL_FILTER_START_CMD_DEFAULT 會被本值墊回去，等於沒改 (見該巨集的 ⚠)。
#define LOGIC_THROTTLE_FWD_OUTPUT_MIN RPMX10_TO_CMD(5124)    // 512.4 RPM =  0.97 km/h (= 1399 count)
#define LOGIC_THROTTLE_FWD_OUTPUT_MAX RPMX10_TO_CMD(43945)   // 4394.5 RPM = 8.29 km/h (= 舊值 12000)

// --- 反轉油門電壓與輸出對應關係 ---
#define LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV (1200u)  // 反轉油門有效起始電壓 (對應輸出 MIN)，提高 deadzone 抑制低端誤觸/抖動
#define LOGIC_THROTTLE_REV_VOLTAGE_MAX_MV (4800u)  // 反轉油門最大有效電壓 ( >= 此值則目標為 MAX)

#define LOGIC_THROTTLE_REV_OUTPUT_ZERO (0u)                 // 反轉電壓低於 MIN 時的目標輸出
#define LOGIC_THROTTLE_REV_OUTPUT_MIN RPMX10_TO_CMD(5493)   // 549.3 RPM = 1.04 km/h (= 舊值 1500)
#define LOGIC_THROTTLE_REV_OUTPUT_MAX RPMX10_TO_CMD(14648)  // 1464.8 RPM = 2.76 km/h (= 舊值 4000)

// 開機安全檢查
// 「已釋放」的定義必須與「會出力」的門檻一致，否則會出現「不出力但永久抑制」的死角：
//   例如門檻 1200mV 而此值仍為舊的 1000mV 時，歸位電壓 1100mV 的油門輸出為 0(在死區內)，
//   卻永遠滿足抑制條件(>1000)而解不開。因此此值由起步門檻自動推導，取正反轉較嚴格(較低)者。
// 判定用 >= (見 s_logic_throttle.c)：電壓「剛好等於」起步門檻時已會輸出 OUTPUT_MIN，
//   必須算成未釋放，否則開機瞬間就會以 1.04 km/h 潛行。
#define LOGIC_THROTTLE_POWER_ON_CHECK_MV                                        \
    ((LOGIC_THROTTLE_FWD_VOLTAGE_MIN_MV < LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV)    \
             ? LOGIC_THROTTLE_FWD_VOLTAGE_MIN_MV                                \
             : LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV)  // 目前 = 1200mV

// 運行時故障檢測電壓閾值
#define LOGIC_THROTTLE_FAULT_VOLTAGE_LOW_MV (0u)     //300 油門信號電壓過低故障閾值 (mV), e.g., open circuit
#define LOGIC_THROTTLE_FAULT_VOLTAGE_HIGH_MV (5000u)   //4500 油門信號電壓過高故障閾值 (mV), e.g., short to VCC
#define LOGIC_THROTTLE_FAULT_DECEL_STEP_TIME {-10, 1}  // 運行時故障的快速減速曲線 {Step, Time}

// --- 正轉加速/減速 Step/Time 表格 ---
// 加速段 (電壓上升)
// [待確認，暫不動] 起步門檻改為 1200mV 後，V1 恰好等於起步門檻，第 0 段(最緩的起步
//   加速 {2,1})只有在電壓恰為 1200mV 時才會命中，實務上輕踩起步會直接落到第 1 段
//   {3,2}。若要恢復最緩起步段需把 V1~V4 往上搬(等比例為 1560/2370/3180/3990，
//   即維持輸出佔比 10%/32.5%/55%/77.5%)，但那會改變騎乘感，故保留原值待實車確認。
#define LOGIC_THROTTLE_FWD_ACCEL_V1_MV (1200u)
#define LOGIC_THROTTLE_FWD_ACCEL_V2_MV (2100u)
#define LOGIC_THROTTLE_FWD_ACCEL_V3_MV (3000u)
#define LOGIC_THROTTLE_FWD_ACCEL_V4_MV (3900u)

// 減速段 (輸出值下降)
#define LOGIC_THROTTLE_FWD_DECEL_O1 RPMX10_TO_CMD(13184)  // 1318.4 RPM = 2.49 km/h (= 舊值 3600)
#define LOGIC_THROTTLE_FWD_DECEL_O2 RPMX10_TO_CMD(20874)  // 2087.4 RPM = 3.94 km/h (= 舊值 5700)
#define LOGIC_THROTTLE_FWD_DECEL_O3 RPMX10_TO_CMD(28564)  // 2856.4 RPM = 5.39 km/h (= 舊值 7800)
#define LOGIC_THROTTLE_FWD_DECEL_O4 RPMX10_TO_CMD(36255)  // 3625.5 RPM = 6.84 km/h (= 舊值 9900)

// --- 反轉加速/減速 Step/Time 表格 ---
// 加速段 (電壓上升)
#define LOGIC_THROTTLE_REV_ACCEL_V1_MV (1200u)
#define LOGIC_THROTTLE_REV_ACCEL_V2_MV (2100u)
#define LOGIC_THROTTLE_REV_ACCEL_V3_MV (3000u)
#define LOGIC_THROTTLE_REV_ACCEL_V4_MV (3900u)

// 減速段 (輸出值下降)
#define LOGIC_THROTTLE_REV_DECEL_O1 RPMX10_TO_CMD(7324)   //  732.4 RPM = 1.38 km/h (= 舊值 2000)
#define LOGIC_THROTTLE_REV_DECEL_O2 RPMX10_TO_CMD(9155)   //  915.5 RPM = 1.73 km/h (= 舊值 2500)
#define LOGIC_THROTTLE_REV_DECEL_O3 RPMX10_TO_CMD(10986)  // 1098.6 RPM = 2.07 km/h (= 舊值 3000)
#define LOGIC_THROTTLE_REV_DECEL_O4 RPMX10_TO_CMD(12817)  // 1281.7 RPM = 2.42 km/h (= 舊值 3500)

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
// 段位對應的正轉命令上限。索引 0~5，共 THROTTLE_ASSIST_LEVEL_COUNT 筆。
// 括號內為理論車速 (8 吋輪, 齒比 20.3)；實機頂速約 7.2 km/h，故段位 5 的上限
// 用不完，多出的約 13% 餘裕是留給負載/爬坡的扭力頭。
#define THROTTLE_ASSIST_LEVEL_MAX_OUTPUT_VALUES { \
    0,                       /*         0 RPM = 0.00 km/h (= 舊值 0)     */ \
    RPMX10_TO_CMD(14648),    /*  1464.8 RPM = 2.76 km/h (= 舊值  4000)  */ \
    RPMX10_TO_CMD(21973),    /*  2197.3 RPM = 4.15 km/h (= 舊值  6000)  */ \
    RPMX10_TO_CMD(29297),    /*  2929.7 RPM = 5.53 km/h (= 舊值  8000)  */ \
    RPMX10_TO_CMD(36621),    /*  3662.1 RPM = 6.91 km/h (= 舊值 10000)  */ \
    RPMX10_TO_CMD(43945)     /*  4394.5 RPM = 8.29 km/h (= 舊值 12000)  */ \
}
#define THROTTLE_ASSIST_LEVEL_COUNT (6)  // 上表筆數，供 s_logic_throttle.c 做索引邊界檢查

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
