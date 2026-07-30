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
