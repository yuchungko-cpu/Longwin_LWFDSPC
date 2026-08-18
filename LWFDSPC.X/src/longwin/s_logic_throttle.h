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
// [實車調校 2026-08-14] 1.04 → 0.97 km/h，起步較平順。
//   本值是「最低可命令車速」的**真正**決定者：它同時是油門內插的地板、加速中的地板，
//   以及加速濾波預載與每 tick 輸出的地板 (u16MinRpm)。只改
//   ACCEL_FILTER_START_CMD_DEFAULT 會被本值墊回去，等於沒改 (見該巨集的 ⚠)。
#define LOGIC_THROTTLE_FWD_OUTPUT_MIN KMHX100_TO_CMD(82)     // 0.82 km/h — 正轉起步速度
#define LOGIC_THROTTLE_FWD_OUTPUT_MAX KMHX100_TO_CMD(829)    // 8.29 km/h — 油門滿刻度 (實機約 7.2)

// --- 反轉油門電壓與輸出對應關係 ---
#define LOGIC_THROTTLE_REV_VOLTAGE_MIN_MV (1200u)  // 反轉油門有效起始電壓 (對應輸出 MIN)，提高 deadzone 抑制低端誤觸/抖動
#define LOGIC_THROTTLE_REV_VOLTAGE_MAX_MV (4800u)  // 反轉油門最大有效電壓 ( >= 此值則目標為 MAX)

// 倒退**只有一段速度**，不受助力段位影響 (見 s_logic_throttle.c 的 u16OutputMax 指派:
//   正轉吃段位表，反轉固定用 REV_OUTPUT_MAX)。段位 0(電子鎖車) 會一併擋掉倒退。
// [2026-08-14] 改以 km/h 表達，取代原本的 RPM 寫法。0.01 km/h 解析度剛好保住實車測 OK
//   的原值 (1.04 / 2.76 km/h)，count 僅因四捨五入差 +0.1%，手感不受影響。
#define LOGIC_THROTTLE_REV_OUTPUT_ZERO (0u)                 // 反轉電壓低於 MIN 時的目標輸出
#define LOGIC_THROTTLE_REV_OUTPUT_MIN KMHX100_TO_CMD(82)    // 0.82 km/h — 倒退起步速度 (與正轉一致)
#define LOGIC_THROTTLE_REV_OUTPUT_MAX KMHX100_TO_CMD(250)   // 2.50 km/h — 倒退唯一的速度上限

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

// 減速段 (輸出值下降)。這四個是「用哪一段減速率」的切換門檻，不是速度限制。
#define LOGIC_THROTTLE_FWD_DECEL_O1 KMHX100_TO_CMD(249)  // 2.49 km/h
#define LOGIC_THROTTLE_FWD_DECEL_O2 KMHX100_TO_CMD(394)  // 3.94 km/h
#define LOGIC_THROTTLE_FWD_DECEL_O3 KMHX100_TO_CMD(539)  // 5.39 km/h
#define LOGIC_THROTTLE_FWD_DECEL_O4 KMHX100_TO_CMD(684)  // 6.84 km/h

// --- 反轉加速/減速 Step/Time 表格 ---
// 加速段 (電壓上升)
#define LOGIC_THROTTLE_REV_ACCEL_V1_MV (1200u)
#define LOGIC_THROTTLE_REV_ACCEL_V2_MV (2100u)
#define LOGIC_THROTTLE_REV_ACCEL_V3_MV (3000u)
#define LOGIC_THROTTLE_REV_ACCEL_V4_MV (3900u)

// 減速段 (輸出值下降)。同上，是減速率的切換門檻，不是速度限制。
#define LOGIC_THROTTLE_REV_DECEL_O1 KMHX100_TO_CMD(138)  // 1.38 km/h
#define LOGIC_THROTTLE_REV_DECEL_O2 KMHX100_TO_CMD(173)  // 1.73 km/h
#define LOGIC_THROTTLE_REV_DECEL_O3 KMHX100_TO_CMD(207)  // 2.07 km/h
#define LOGIC_THROTTLE_REV_DECEL_O4 KMHX100_TO_CMD(242)  // 2.42 km/h

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

// =============================================================================
//  段位限速表 —— **直接以 km/h 定義**
// =============================================================================
//  主機 (LCD) 送來的是**段位編號**而非速度值 (Modbus: u8WheelCfgLo & 0x0F → u8AssistLevel)，
//  所以每一段對應多少 km/h 完全由本表決定，改本表需重新編譯燒錄。
//    索引 0     = 電子鎖車 (e-lock)，不是速度 —— 命令歸零且 EMB 不放開，見 s_logic_embraker.h
//    索引 1~5   = 五個助力段位
//    索引 6~15  = 主機若送出 (欄位是 4-bit)，s_logic_throttle.c 會夾到 5，見該處邊界檢查
//
//  KMHX100_TO_CMD(kx) 的入口單位是 km/h x100 (解析度 0.01 km/h)，換算內含
//  motor_scale.h 的輪徑 WHEEL_DIAMETER_INCH_X10(8.0 吋) 與齒比 GEAR_RATIO_X100(20.30)。
//  ⚠ 換輪徑或換齒比 → 本表的 km/h 需重新確認 (數字不必改，但實際車速會跟著變)。
//
//  ⚠ 實機頂速約 7.2 km/h (≈10400 count)。段位 5 的 8.3 km/h **到不了**，多出的約 13%
//    是刻意留給負載/爬坡的扭力頭 —— 也就是任何高於約 7.2 的設定值都等同「不限速」。
//
//  [2026-08-14] 兩件事同時改：
//    (1) 單位由 RPMX10_TO_CMD(機械RPM x10) 改為 KMHX100_TO_CMD(km/h x100)。舊寫法的
//        RPM 數字 (14648/21973/…) 是為了位元級還原更舊版本硬編碼的 count
//        (4000/6000/8000/10000/12000) 而反推出來的，可讀性等於零。
//    (2) **車速依規格重新指定為 3 / 4 / 5 / 6 / 8 km/h**(原 2.76/4.15/5.53/6.91/8.29)。
//        對應 count：4340 / 5787 / 7239 / 8686 / 11580。
//        中段變慢較多 (段位 3 由 5.53 → 5.00 km/h，count -9.5%;段位 4 由 6.91 → 6.00，
//        -13%)，這是刻意的規格變更，不是換算誤差 —— 上車前需確認各段手感符合預期。
#define THROTTLE_ASSIST_LEVEL_MAX_OUTPUT_VALUES { \
    0,                       /* 段位 0：電子鎖車 (不是 0 km/h，是不放開煞車) */ \
    KMHX100_TO_CMD(300),     /* 段位 1：3.00 km/h */ \
    KMHX100_TO_CMD(400),     /* 段位 2：4.00 km/h */ \
    KMHX100_TO_CMD(500),     /* 段位 3：5.00 km/h */ \
    KMHX100_TO_CMD(600),     /* 段位 4：6.00 km/h */ \
    KMHX100_TO_CMD(800)      /* 段位 5：8.00 km/h (受硬體限制，實際約 7.2) */ \
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
