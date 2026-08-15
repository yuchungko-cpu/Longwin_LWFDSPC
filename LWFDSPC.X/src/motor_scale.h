#ifndef MOTOR_SCALE_H
#define MOTOR_SCALE_H

// *****************************************************************************
//  馬達刻度定義 (Single Source of Truth)
// *****************************************************************************
//  本檔是「Hall 週期 <-> Q15 速度 <-> 機械 RPM <-> 車速」四者換算的唯一來源。
//  只需維護下方 4 個實體參數，其餘（HALL_MIN_PERIOD、Q15 刻度、RPM 換算）
//  全部由編譯期推導。
//
//  【背景】舊版把 NOPOLESPAIRS 填 12 (真值 3) 且 MAXIMUM_SPEED_RPM 填 3000
//  (Q15 實際滿刻度是 12000)，兩個錯誤剛好互相補償，使 HALL_MIN_PERIOD 得到
//  正確的 434。本檔把宣告改回實體真值，並保持 HALL_MIN_PERIOD == 434 不變，
//  因此 Speed 刻度、速度命令域、加減速表、PI 增益全部無須改動。
// *****************************************************************************

#include <stdint.h>
#include "../hal/clock.h"  // FCY

// -----------------------------------------------------------------------------
//  實體參數 (需人工維護)
// -----------------------------------------------------------------------------
// 馬達極對數。6 極馬達 = 3 極對。
// 驗證法：手轉車輪整整一圈，HallPulses 應累計 (6 * MOTOR_POLE_PAIRS *
//        GEAR_RATIO) = 18 * 20.3 = 365 個邊緣。
#define MOTOR_POLE_PAIRS 3

// 馬達機械轉速上限 (RPM)。僅供 sanity check 與文件用，不參與運算。
// 實測：輪端 200 RPM * 齒比 20.3 = 4060 RPM (約 7.7 km/h @ 8 吋輪)
#define MOTOR_MAX_RPM 4060

// 減速齒比 * 100 (馬達轉速 / 車輪轉速)。實測 20.30:1
#define GEAR_RATIO_X100 2030

// 車輪直徑 (吋 * 10)。8.0 吋 → 周長 638 mm。
// 這是「編譯期 km/h 換算」的唯一輪徑來源，供 KMHX100_TO_CMD() 與 userparms.h 的
//   EMB_* 位移／週期門檻使用。s_logic_motor.h 的 LOGIC_MOTOR_DEFAULT_WHEEL_DIMENSION_INCH
//   直接引用本值，避免「顯示用輪徑」與「限速用輪徑」各寫一份而分歧。
// ⚠ 換輪徑時，下列以 mm / km/h 表示的門檻**全部**會跟著改變意義，必須重新確認：
//     userparms.h 的 EMB_ROLLBACK_REV_EDGES(位移)、EMB_DOWNHILL_SLIDE_EDGES(位移)、
//     EMB_DOWNHILL_MAX_SPEED_KMHX100(車速)、s_logic_throttle.h 的段位限速表。
// ⚠ 顯示路徑另有執行期可改的輪徑 (logic_motor_setWheelDimension，LCD 可下)；
//   本值只影響編譯期常數，兩者不同步時「顯示車速」與「實際限速」會對不上。
#define WHEEL_DIAMETER_INCH_X10 80

// Q15 速度滿刻度 (Speed == 32768 所代表的馬達機械 RPM)。
// 這是「刻度選擇」而非馬達極限，必須 >= MOTOR_MAX_RPM 並保留超速裕度。
// 12000 = 2.96 * MOTOR_MAX_RPM，沿用既有刻度以維持原騎乘感 (見檔頭說明)。
// 【警告】改動此值會等比改變速度命令域、加減速率與 PI 迴路增益，
//        必須連帶重算 s_logic_throttle.h 的加減速表與 SPEEDCNTR_PTERM/ITERM。
#define SPEED_FS_RPM 12000

// Timer1 前除器 (Hall 週期量測用)
#define TIMER_PRESCALER 64

// -----------------------------------------------------------------------------
//  推導值 (編譯期計算，勿手改)
// -----------------------------------------------------------------------------
// Hall 狀態變化次數 / 機械轉 = 6 * 極對數 = 3 * 極數
#define HALL_EDGES_PER_REV (6 * MOTOR_POLE_PAIRS)

// Timer1 計數頻率 (Hz)
#define HALL_TIMER_HZ (FCY / TIMER_PRESCALER)

// Q15 滿刻度對應的 Hall 週期 (Timer1 ticks)。Speed = HALL_MIN_PERIOD / 實際週期
// 不先做 (RPM/60) 的整數除法，避免舊式寫法的截斷誤差。
#define HALL_MIN_PERIOD ((HALL_TIMER_HZ * 60UL) / (SPEED_FS_RPM * 1UL * HALL_EDGES_PER_REV))

// 由取整後的 HALL_MIN_PERIOD 回算真正的滿刻度，用於驗證截斷未造成刻度偏移
#define SPEED_FS_RPM_ACTUAL ((HALL_TIMER_HZ * 60UL) / (HALL_MIN_PERIOD * 1UL * HALL_EDGES_PER_REV))

// 車輪周長 (mm)。直徑(吋x10) x 25.4/10 x pi = 直徑(吋x10) x 7.9796
//   80 → 638 mm (截尾)。取 79796/10000 使 uint32 中間值最大僅 6.4e6，安全。
#define WHEEL_CIRCUM_MM ((WHEEL_DIAMETER_INCH_X10 * 79796UL) / 10000UL)

// -----------------------------------------------------------------------------
//  編譯期護欄
// -----------------------------------------------------------------------------
// 這道護欄是「控制迴路未被動到」的保證。任何造成 HALL_MIN_PERIOD 改變的參數
// 變動都會在此擋下，提醒必須同步重算命令域、加減速表與 PI 增益。
#if (HALL_MIN_PERIOD != 434)
#error "HALL_MIN_PERIOD 偏離 434: Speed 刻度已改變, 速度命令域/加減速表/PI 增益必須同步重算"
#endif

#if (SPEED_FS_RPM_ACTUAL != SPEED_FS_RPM)
#error "HALL_MIN_PERIOD 取整造成刻度偏移: 請改用能整除的 SPEED_FS_RPM"
#endif

#if (SPEED_FS_RPM < MOTOR_MAX_RPM)
#error "Q15 滿刻度低於馬達轉速上限: 高速時 Speed 會飽和於 32767"
#endif

// -----------------------------------------------------------------------------
//  換算巨集 / 函式
// -----------------------------------------------------------------------------
// 馬達機械 RPM * 10 -> Q15 速度命令 count (編譯期常數，四捨五入)
// 用 RPM*10 為單位是為了能位元級還原舊版硬編碼的 count 值。
// 溢位界限：r10 max 120000 時 120000*32768 = 3.93e9 < UINT32_MAX，安全。
#define RPMX10_TO_CMD(r10) \
    ((uint16_t)(((uint32_t)(r10) * 32768UL + (SPEED_FS_RPM * 10UL) / 2) / (SPEED_FS_RPM * 10UL)))

// -----------------------------------------------------------------------------
//  車速 (km/h * 100) -> Q15 速度命令 count   ★ 所有「速度設定」統一用這一個
// -----------------------------------------------------------------------------
//  入口單位 km/h x 100，即解析度 0.01 km/h：
//      KMHX100_TO_CMD(300) = 3.00 km/h      KMHX100_TO_CMD(274) = 2.74 km/h
//
//  [為何取 x100 而非 x10] 本車量程只有 1~8 km/h，0.1 km/h 在起步速度 (約 1 km/h) 上
//    就是 10% 的跳動;實車調校出來的值 (0.97 / 1.04 / 2.76 km/h) 在 x10 解析度下**無法
//    表達**，換算會被迫改掉已驗證的手感。x100 下全部現有值都保得住 (誤差 <= 0.3%)。
//
//  [為何不用浮點 (KMH_TO_CMD(3.00))] 浮點常數雖然也在編譯期 fold、不會產生執行期成本，
//    但 ISO C 不允許浮點出現在「整數常數運算式」—— #if/#elif、陣列大小、case 標籤、
//    _Static_assert 都不能用。那會讓速度設定失去 #error 範圍護欄 (例如 userparms.h 的
//    EMB_DOWNHILL_MAX_SPEED 上下界檢查)，代價比多打兩個零高。
//
//  ⚠ 換算內含輪徑 (WHEEL_DIAMETER_INCH_X10) 與齒比 (GEAR_RATIO_X100)。
//    用本巨集寫死的速度值只對該輪徑/齒比成立；換規格必須重新確認全部設定值。
//
//  換算鏈 (1 km/h = 1e6 mm/h)：
//      車輪 RPM x10 = kx * 1e6 / (600 * 周長mm)        8 吋輪(周長 638) → x2.612
//      馬達 RPM x10 = 車輪 RPM x10 * 齒比               x20.30
//      count        = 馬達 RPM x10 * 32768 / (SPEED_FS_RPM*10)
//    合計 1 km/h = 1447.9 count。溢位界限 kx <= 4000 (40 km/h)：中間值全在 uint32 內，
//    且馬達 RPM x10 <= 120000 仍在 RPMX10_TO_CMD 的安全範圍。本車頂速約 7.2 km/h。
#define KMHX100_TO_MOTOR_RPMX10(kx)                                             \
    (((((uint32_t)(kx) * 1000000UL) / (600UL * WHEEL_CIRCUM_MM)) *              \
      GEAR_RATIO_X100) / 100UL)

#define KMHX100_TO_CMD(kx) RPMX10_TO_CMD(KMHX100_TO_MOTOR_RPMX10(kx))

// Q15 速度 -> 馬達機械 RPM
static inline int16_t scale_speedToMotorRpm(int16_t i16SpeedQ15) {
    // 32767 * 12000 = 3.93e8，int32 內安全
    return (int16_t)(((int32_t)i16SpeedQ15 * (int32_t)SPEED_FS_RPM) >> 15);
}

// Q15 速度 -> 車輪 RPM * 10 (取絕對值)
// 分兩階運算避免 uint32 溢位：先算馬達 RPM*10 (max 120000)，再除齒比。
// 取絕對值先升位 int32 再取負，避免 int 為 16-bit 時 -(-32768) 溢位。
static inline uint16_t scale_speedToWheelRpmX10(int16_t i16SpeedQ15) {
    int32_t i32 = (int32_t)i16SpeedQ15;
    uint32_t u32Abs = (uint32_t)((i32 < 0) ? -i32 : i32);
    // 32768 * 120000 = 3.93e9 < UINT32_MAX，安全
    uint32_t u32MotorRpmX10 = (u32Abs * (SPEED_FS_RPM * 10UL)) >> 15;
    return (uint16_t)((u32MotorRpmX10 * 100UL) / GEAR_RATIO_X100);
}

// Hall 脈衝計數 (每 HALL_PULSE_WINDOW_MS 毫秒的邊緣數) -> 馬達機械 RPM
// 換算：1 pulse/100ms = 600/18 = 33.3 馬達 RPM
#define HALL_PULSE_WINDOW_MS 100
static inline uint16_t scale_pulsesToMotorRpm(int16_t i16Pulses) {
    int32_t i32 = (int32_t)i16Pulses;
    uint32_t u32Abs = (uint32_t)((i32 < 0) ? -i32 : i32);
    return (uint16_t)((u32Abs * (60000UL / HALL_PULSE_WINDOW_MS)) / HALL_EDGES_PER_REV);
}

#endif  // MOTOR_SCALE_H
