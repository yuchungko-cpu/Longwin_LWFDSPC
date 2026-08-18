/*******************************************************************************
 * Copyright (c) 2020 released Microchip Technology Inc.  All rights reserved.
 *
 * SOFTWARE LICENSE AGREEMENT:
 *
 * Microchip Technology Incorporated ("Microchip") retains all ownership and
 * intellectual property rights in the code accompanying this message and in all
 * derivatives hereto.  You may use this code, and any derivatives created by
 * any person or entity by or on your behalf, exclusively with Microchip's
 * proprietary products.  Your acceptance and/or use of this code constitutes
 * agreement to the terms and conditions of this notice.
 *
 * CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS".  NO
 * WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP'S
 * PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE LIABLE,
 * WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF
 * STATUTORY DUTY),STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE,
 * FOR ANY INDIRECT, SPECIAL,PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE,
 * HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR
 * THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW,
 * MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS CODE,
 * SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP SPECIFICALLY TO
 * HAVE THIS CODE DEVELOPED.
 *
 * You agree that you are solely responsible for testing the code and
 * determining its suitability.  Microchip has no obligation to modify, test,
 * certify, or support the code.
 *
 *******************************************************************************/
#ifndef USERPARMS_H
#define USERPARMS_H

#ifdef __cplusplus
extern "C" {
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include "../hal/clock.h"

#include "motor_scale.h"  // 極對數/齒比/Q15 速度刻度的唯一來源
#include "longwin/s_logic_motor.h"

#ifdef __XC16__  // See comments at the top of this header file
#include <xc.h>
#endif  // __XC16__

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************
#define TURN_ON 1
#define TURN_OFF 0
#define HALF_MIN 600000UL
#define TEN_SEC 200000UL
#define FIVE_SEC 100000UL
#define TWO_SEC 40000
#define BRAKE_CONSTANT 50000
#define SW_12V LATCbits.LATC11
#define PERIOD_OPENLOOP_SPEED (uint16_t)6677  //@6kph (Timer period value)
#define PERIOD_AT_VERY_LOW_SPEED 20000        //@2kph (Timer period value)
#define PERIOD_AT_VERY_HIGH_SPEED 1425        //@28kph (Timer period value)
//*****************************************************************************
#define CTRLMODE 0  // 0: speed 1:Torque, 2: Speed limit, torque control

/*************** PWM and Control Timing Parameters ****************************/
/* Specify PWM Frequency in Hertz */
#define PWMFREQUENCY_HZ 20000
/* Specify dead time in micro seconds */
/* Specify PWM Period in seconds, (1/ PWMFREQUENCY_HZ) */
#define LOOPTIME_SEC 0.00005

/*********************************** ADC Scaling ******************************/
/* Scaling constants: Determined by calibration or hardware design. */

/* scaling factor for pot */
#define KPOT Q15(0.5)
/* scaling factor for current phase A */
#define KCURRA Q15(0.5)
/* scaling factor for current phase B */
#define KCURRB Q15(0.5)
/* scaling factor for current phase C */
#define KCURRC Q15(0.5)
#define SCALER 10
/****************************** Motor Parameters ******************************/
/********************  support xls file definitions begin *********************/
/* The following values are given in the xls attached file */

/* 極對數、齒比、Q15 速度滿刻度、Timer1 前除器、Hall 週期基準 (HALL_MIN_PERIOD)
 * 全部移至 src/motor_scale.h 統一定義。
 * 已移除的舊巨集 (皆為無引用或已被取代)：
 *   NOPOLESPAIRS / MAXIMUM_SPEED_RPM  -> MOTOR_POLE_PAIRS / SPEED_FS_RPM
 *   MINPERIOD                         -> HALL_MIN_PERIOD (值仍為 434)
 *   MINIMUM_SPEED_RPM / *_SPEED_ELECTR / SPEED_MULTIPLIER_1,2 / SPEED_MULTI_ELEC
 *   MAXPERIOD / PERIOD_CONSTANT       (無引用，且 MAXPERIOD 算式漏除 6*60，錯 100 倍)
 */
#define MAXSPEED_REF_LIMIT 2000   // 速度模式轉速上限 (機械 RPM)
#define MAXSPEED_CtrlMode_2 1160  // CtrlMode = 2 的轉速上限 (機械 RPM)
// 註：下列兩個 Q15 上限的消費者 (SpeedModeCtrlLimit / SpeedCtrlLimit) 目前全部位於
//     main.c 的 #if 0 死碼區 (2019-2208)，僅保留以維持 main.c:810/812 可編譯。
#define Q15_MAXSPEED_REF_LIMIT RPMX10_TO_CMD(MAXSPEED_REF_LIMIT * 10)
#define Q15_MAXSPEED_CtrlMode_2 RPMX10_TO_CMD(MAXSPEED_CtrlMode_2 * 10)

/* The following values are given in the xls attached file */
#define NORM_CURRENT_CONST 0.000214
/**********************  support xls file definitions end *********************/

//(FCY/(TIMER_PRESCALER*FPWM)*(65536/6))
// 註：換相角度步進與極對數無關 (每 60 度電氣角一個 Hall 邊緣)，故不受刻度變更影響。
#define PHASE_INC_CALC (unsigned long)((float)FCY / ((float)(TIMER_PRESCALER) * (float)(PWMFREQUENCY_HZ)) * (float)(65536 / 6))
#define ANGLESTEP (unsigned long)((float)FCY / ((float)(TIMER_PRESCALER) * (float)(PWMFREQUENCY_HZ)) * (float)(65536 / 6))

/* current transformation macro, used below */
#define NORM_CURRENT(current_real) (Q15(current_real / NORM_CURRENT_CONST / 32768))

#define R_SHUNT_Ohm (float)0.002
#define CURRENT_GAIN_OPAMP (float)7.89  // 實測放大倍率 (原記載 12.2 有誤)
// #define RATED_AMPS (float)10.0
// #define RATED_CURRENT_Q15   RATED_AMPS*R_SHUNT_Ohm*CURRENT_GAIN_OPAMP*32768*2/3.3
#define RATED_CURRENT_Q15 Q15(0.5)  // Half of the maximal peak current

// *****************************************************************************
//  相電流 Q15 <-> 安培 換算 (單一來源，改硬體參數即自動跟著變)
// *****************************************************************************
//  量測鏈：分流電阻 → 運放 → ADC (12-bit, FORM=1 左對齊) → MeasCompCurr()
//    零電流時運放輸出 = VREF/2 = 1.65V (雙極性)。
//
//  [FIX 2026-07-31] MeasCompCurr() 的淨增益是 1.0，不是 KCURRA (=0.5)。
//    meascurr.s 的組語是 `mpy w4*w5,A` 之後 `sac A,#-1,w4` —— sac 的負位移量是「左移」，
//    #-1 就是再乘 2，該檔自己的註解也寫著 `qIa = 2 * qKa * CorrADC1`。
//    Microchip 原本就是設計成用 KCURRA = Q15(0.5) 去抵掉組語裡的 x2，讓淨增益為 1.0，
//    使「感測器軌到軌」剛好對應「Q15 滿刻度」。
//    本檔原註解把 0.5 又算了一次 → counts/A 少一半 → 回推安培多一倍，
//    實機上 IbusAmpX10 / Modbus 上報電流是實際值的 2 倍。
//
//  counts/A = (R_SHUNT x GAIN / VREF) x ADC滿刻度counts x (2 x KCURRA)
//           = (0.002 x 7.89 / 3.3) x 65520 x 1.0 = 313.3      (1 count = 3.2 mA)
//  → Q15 滿刻度 32768 ↔ 104.6 A，而這正好等於感測器物理極限 ±1.65V/0.01578 = ±104.6 A
//    (兩者一致是這組數字正確的自我驗證：軌到軌 = 滿刻度)
#define ADC_FULL_SCALE_COUNTS 65520UL  // 12-bit 左對齊 (0xFFF0)
#define ADC_VREF_V (float)3.3

// MeasCompCurr() 的淨增益 = 2 x KCURRA (組語 sac #-1 的 x2)。目前 = 1.0
#define MEASCURR_NET_GAIN ((2.0f * (float)KCURRA) / 32768.0f)

// counts/A x 100 (整數化，供下方換算用)。目前 = 31330
#define IABC_COUNTS_PER_AMP_X100                                                   \
    ((uint32_t)((R_SHUNT_Ohm * CURRENT_GAIN_OPAMP * MEASCURR_NET_GAIN * 100.0f *     \
                 (float)ADC_FULL_SCALE_COUNTS) /                                    \
                ADC_VREF_V))

// Q15 電流 counts -> 0.1 A (四捨五入)。輸入須為非負，最大 32767 時輸出 1046 (104.6A)。
// 溢位界限：32767 x 1000 = 3.3e7 < UINT32_MAX，安全。
#define IABC_Q15_TO_A_X10(q15)                                                     \
    ((uint16_t)(((uint32_t)(q15) * 1000UL + IABC_COUNTS_PER_AMP_X100 / 2) /         \
                IABC_COUNTS_PER_AMP_X100))
/* Open loop q current setup - */
#define Q_CURRENT_REF_OPENLOOP NORM_CURRENT(0.1008)  //@6kph

/* In case of the potentiometer speed reference, a reference ramp
is needed for assuring the motor can follow the reference imposed /
minimum value accepted */
// #define SPEEDREFRAMP   Q15(0.00006)
#define SPEED_SLOP_CNTR_SET 20  // 20: 10ms @ 2ms speed control loop (counter value)
#define ACC_SET 50              // 1: 0.3% per second of Max speed (acceleration rate)
#define DE_ACC_SET 50           // 1: 0.3% per second of Max speed (deceleration rate)
#define TORQ_ACCDELAY 10        // 扭力加速延遲計數器設定 (delay count)
/* The Speed Control Loop Executes every  SPEEDREFRAMP_COUNT */
// #define SPEEDREFRAMP_COUNT   2

/* PI controllers tuning values - */

/* D Control Loop Coefficients */
#define D_CURRCNTR_PTERM Q15(0.02)
#define D_CURRCNTR_ITERM Q15(0.001)
#define D_CURRCNTR_CTERM Q15(0.999)
#define D_CURRCNTR_OUTMAX 0x7FFF

/* Q Control Loop Coefficients */
#define Q_CURRCNTR_PTERM Q15(0.02)
#define Q_CURRCNTR_ITERM Q15(0.001)
#define Q_CURRCNTR_CTERM Q15(0.999)
#define Q_CURRCNTR_OUTMAX 0x7FFF

/* Velocity Control Loop Coefficients
 * 【重要】速度環的誤差 (inReference - inMeasure) 是 Q15 命令 count，其物理意義由
 *   motor_scale.h 的 SPEED_FS_RPM 決定。因此下列增益與 SPEED_FS_RPM 成反比綁定：
 *     SPEEDCNTR_PTERM 為 Q1.11 格式 (見 motor_control_declarations.h)，
 *     有效 Kp = 5000/2048 = 2.441 (輸出 count / 誤差 count)
 *             = 每 1 馬達RPM 誤差產生 6.7 counts 的 Iq 命令
 *     有效 Ki = 20/32768，速度環 1kHz → 每 1 count 誤差每 ms 累加 6.10e-4
 *   若改動 SPEED_FS_RPM，兩者必須同乘 (舊FS/新FS) 才能維持相同騎乘感。
 *   motor_scale.h 的 HALL_MIN_PERIOD != 434 護欄即為此而設。
 * 飽和門檻：outMax 由溫控電流上限覆寫 (常態 Q15(0.35)=11469)，
 *   → 誤差 >= 11469/2.441 = 4699 counts (1720 馬達RPM) 即滿輸出。
 *   ⚠ 增益提高後飽和門檻由 2703 降到 1720 馬達RPM，更容易貼上限 —— X2CScope 實測
 *     全速時 inMeasure 停在 11400 count 而命令 12000，那 600 count 的穩態落差就是
 *     輸出飽和的指紋 (帶積分項的 PI 若未飽和，穩態誤差應趨近 0)。
 *     待確認飽和來源是母線電壓不足或溫控夾制：scope 看 piOutputOmega.out 是否貼在
 *     piInputOmega.piState.outMax 上。
 *
 * ⚠ 下方是 #if 0 / #else：live 值是 #else 那組 (PTERM 5000 / ITERM 20)。
 *   V0.16 為 3000 / 10，於 bcc1671 為改善上坡起步而提高 (Kp +67%、Ki +100%)。
 *   副作用：減速命令的階躍被忠實放大成頓挫 → 因此 V0.19 補上減速命令濾波。
 */
#if 0
#define SPEEDCNTR_PTERM Q15(0.2)
#define SPEEDCNTR_ITERM Q15(0.006)
#define SPEEDCNTR_CTERM Q15(0.999)
#define SPEEDCNTR_OUTMAX Q15(0.175)  //~Hardware: 13.2A peak, 10mR
#else
#define SPEEDCNTR_PTERM 5000
#define SPEEDCNTR_ITERM 20
#define SPEEDCNTR_CTERM Q15(0.999)
#define SPEEDCNTR_OUTMAX Q15(0.5)  //~Hardware: 13.2A peak, 10mR  0.175
#endif

// 油門控制迴路參數設定
#define THROTTLECNTR_PTERM Q15(0.2)     // 油門控制比例增益項 (P項)
#define THROTTLECNTR_ITERM Q15(0.006)   // 油門控制積分增益項 (I項)
#define THROTTLECNTR_CTERM Q15(0.999)   // 油門控制反饋增益項 (C項)
#define THROTTLECNTR_OUTMAX Q15(0.175)  // 油門控制輸出限制值，對應硬體限制約13.2A峰值電流，10mΩ分流電阻

// 過電流保護相關參數
#define OVERCURRENT_COUNTER 10000  // 過電流計數器閾值，用於防止瞬間過電流誤判 (counter value)

// 電壓保護相關參數 (ADC values)
#define VOLTAGE_LIMITER 562         // 電壓限制器閾值，約對應 28.8V (ADC raw value)
#define UNDERVOLTAGE_COUNTER 20000  // 低電壓計數器閾值，用於防止瞬間低壓誤判 (counter value)
#define UNDERVOLTAGE_INDICATOR 200  // 低電壓指示閾值，約對應 10.2V (ADC raw value)
#define OVERVOLTAGE_LIMITER 822     // 過電壓限制器閾值，約對應 42V (ADC raw value)
#define OVERVOLTAGE_COUNTER 10      // 過電壓計數器閾值，用於防止瞬間過壓誤判 (counter value)

// 溫度保護相關參數 (ADC values)
#define OVERTEMP_LIMITER_57 225  // 馬達溫度 57°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_LIMITER_35 235  // 馬達溫度 35°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_MOSFET_60 435   // MOSFET 溫度 60°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_MOSFET_73 329   // MOSFET 溫度 73°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_MOSFET_80 280   // MOSFET 溫度 80°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_MOSFET_90 220   // MOSFET 溫度 90°C 對應的 ADC 值 (ADC raw value)
#define OVERTEMP_COUNTER 20      // 過溫計數器閾值，20 x 50us = 1ms (counter value)

// 馬達堵轉保護 (MotorStallDetect is executed every 20ms)
// 註：下列門檻的物理值以 SPEED_FS_RPM=12000、齒比 20.3、8 吋輪換算
//     (1 count = 0.366 馬達 RPM = 0.00069 km/h)
#define MOTOR_STALL_COMMAND_THRESHOLD 1000     // 命令 count；366 馬達RPM = 0.69 km/h 以上視為要求驅動
#define MOTOR_STALL_SPEED_THRESHOLD Q15(0.03)  // 983 count = 360 馬達RPM = 0.68 km/h 以下視為堵轉(不只 Speed==0)
#define MOTOR_STALL_SPEED_RELEASE_THRESHOLD Q15(0.06) // 1966 count = 720 馬達RPM = 1.36 km/h，確實在動→清計數器
#define MOTOR_STALL_RELEASE_CNTR 25            // 25 x 20ms = 500ms clear movement to release stall limiting
#define MOTOR_STALL_CURRENT_LIMIT_CNTR 400     // 400 x 20ms = 8s, reduce current
#define MOTOR_STALL_LOCK_CNTR 3000             // 3000 x 20ms = 60s, stop output and latch
#define MOTOR_STALL_CURRENT_LIMIT_PERCENT 50   // Current limit after 8s stall
#define MOTOR_STALL_CNTR MOTOR_STALL_LOCK_CNTR // Legacy alias: final stall lock threshold

// UVW low-speed lock entry filter (speed profile task is executed every 1ms)
#define UVW_LOCK_SPEED_THRESHOLD Q15(0.01)      // 328 count = 120 馬達RPM = 0.23 km/h (舊註解「≈30RPM」是按錯誤的極對數 12 算的)
#define UVW_LOCK_CURRENT_THRESHOLD Q15(0.02)    // (保留) 舊版進鎖電流門檻；新版遲滯邏輯已不使用
#define UVW_LOCK_RELEASE_REF 500                // 油門目標高於此值則解鎖；183 馬達RPM = 0.35 km/h。鬆油門=0，踩下時 >= OUTPUT_MIN(=1399 count)
// 進鎖脈衝門檻：HallPulsesLatch(每100ms邊緣數) < 此值視為已接近靜止。
// 換算 (18 邊緣/機械轉, 齒比 20.3)：1 pulse/100ms = 33.3 馬達RPM = 0.063 km/h
//   → 4 pulses = 133 馬達RPM = 0.25 km/h  (舊註解「7≈馬達58RPM, 極對數12」已失效)
// 獨立於 ReGen 的 BrakeStopSpeedPulses，調此值不影響 ReGen 起煞點。
// [實車調校 2026-08-10] 3 → 4。V0.18 曾設 15(0.94 km/h) 導致車還在滾就夾煞 → 頓挫與煞車片
//   異音;調回 3 後頓挫消除但進鎖略慢，實測 4 (0.25 km/h) 是兩者的平衡點。
// [2026-08-11 評估後維持 4，未改為 1] 曾考慮改成 1 來解「下坡點放油門後溜車」:
//   判斷式是 `HallPulsesLatch < 此值`，故 1 = 要求 latch **恰為 0**(整整 100ms 內零個邊緣)。
//   動機:點油門 → EMB 釋放、車被重力拉動 → 立刻放掉 → 命令歸零。此時若 UVW latch 上,三相
//   短路的制動扭矩 ∝ 反電動勢 ∝ 轉速,剛起步 ω≈0 幾乎無扭矩撐不住坡度;而 UVW 是**單向閂鎖**
//   (唯一出鎖條件 |TargetRpm| >= UVW_LOCK_RELEASE_REF),制動力更強的速度環 PI 被鎖在門外。
//   門檻若為 1,坡上車一動就有邊緣 → UVW 進不去 → 制動權留在 PI 手上。
//
//   ⚠ 但**不採用**，因為本巨集被兩處共用，改 1 會打穿另一道保護:
//     (1) main.c 的 UVW 進鎖判斷 (bUvwStopCommanded) —— 原本想改的目標。
//     (2) main.c 的 bStallReleaseCoast「頂牆/堵轉放開後近停強制 coast」—— 防止零速↔反向交界
//         的換相把馬達往後主動驅動(曾實測 runaway 後退 1m+)。
//     而 runaway 的**必要條件就是停車顫動**(顫動才會讓 HallPeriod <= HallMinPeriod、Speed 被
//     斷言成 32767、進而繞過 CalculateParkAngleHall 的低速方向無關防護)，顫動必然產生霍爾
//     邊緣 ⇒ 該 100ms 窗口的 latch > 0 ⇒ 門檻 1 之下 coast 不成立 ⇒ PWM 保持開啟 ⇒ 錯向
//     滿電流脈衝放行。門檻 4 留的那 1~3 個邊緣餘裕，正是給這個情況的。
//   → 下坡點放改由專屬的下坡滑動偵測 (EMB_DOWNHILL_*，見下方) 處理，不動本值。
//   → 若未來仍要試 1，正確做法是先把 (2) 拆成獨立巨集並維持 4。
#define UVW_LOCK_STOP_PULSES 4

// =============================================================================
//  有動力倒溜/倒衝偵測 (命令一個方向、車實際往反方向動 → EMB 立即鎖定)
// =============================================================================
//  規格：倒溜量不得超過 1/4 車輪 = 91.4 個霍爾邊緣 = 159.6 mm。
//  偵測訊號改為「每個霍爾邊緣」比對滾動方向與命令方向 (見 main.c 的 CNRead_Inline)，
//  反向 +1 / 正向 -1 的淨計數達 EMB_ROLLBACK_REV_EDGES 即鎖 —— 與速度無關，再慢的潛行
//  倒溜也會在固定位移內被抓到，這是滿足上述規格的關鍵 (舊版靠速度門檻，慢速倒溜永遠不觸發)。
//  用「淨」計數而非「連續」計數，是因為規格管的是淨位移：上坡與重力拉鋸時連續計數會被
//  中間那幾個正向邊緣一再歸零，淨位移卻早已超標。詳見 main.c CNRead_Inline 的註解。
//
//  位移換算 (18 邊緣/機械轉 x 齒比 20.30 = 365.4 邊緣/輪轉, 8 吋輪周長 638 mm)：
//    1 邊緣 = 1.75 mm
//      N=3  →   5.2 mm (1/122 輪)      N=16 →  28.0 mm (1/23 輪)
//      N=4  →   7.0 mm (1/91  輪)      N=45 →  78.6 mm (1/8  輪)
//      N=8  →  14.0 mm (1/46  輪)      N=91 → 159.0 mm (1/4  輪 ← 規格上限)
// [2026-08-15 停用] 倒溜偵測改用「排檔方向 + EMB 曾 RELEASE」武裝，不再看命令方向。
//   保留定義以防日後回退,但無 live 消費者(main.c 已改讀 uGF.DirSW)。
#define EMB_ROLLBACK_CMD_THRESHOLD 100  // (停用) 舊版:|inReference| > 此值才啟用偵測
// [實車調校 2026-08-10] 90 → 70。90(157 mm) 貼著 1/4 車輪護欄 91(159 mm) 太近,
//   70 = 122 mm 留了餘裕且實測倒溜量可接受。
// [2026-08-15] 70 → 16 (122 mm → 28 mm)。因客戶回報「上坡下滑 EMB 上鎖時前輪翹起」，
//   夾煞時峰值減速度 ∝ 動能 ∝ 觸發位移，門檻降低直接減小翹前輪風險。
//   舊值 70 之所以較大，是因為當時武裝條件是「命令 > 100 count」，命令降到 100 以下
//   會突然解除武裝並清零計數，需要留位移餘裕避免競賽窗口失效 → 得取離規格護欄 (91)
//   較近的值。改用「排檔方向 + EMB 曾 RELEASE」武裝後 (見 main.c 的說明)，整段都在
//   武裝，不再有解武裝的競賽，可以取更小的門檻。
//   16 邊緣的量級與 V0.21 沒減速偵測的 26 mm 相當，兩條路徑覆蓋同一保護範圍。
//   相對規格護欄 91 (159 mm) 有 5.7 倍餘裕。若實測誤觸再退回 24 (42 mm)。
#define EMB_ROLLBACK_REV_EDGES 16        // 淨反向霍爾邊緣門檻 → 立即鎖定。16 = 倒退 28 mm
                                        //   太靈敏(誤鎖)就加大，太遲鈍就縮小；上限見下方護欄

// 每輪轉的霍爾邊緣數與 1/4 車輪上限 (由 motor_scale.h 的參數推導，改齒比/極對數會自動跟著變)
//   ⚠ 1UL 必須放在最前面:18 * 2030 = 36540 溢位 16-bit int,而 C 的左結合律讓
//     "18 * 2030 * 1UL" 仍先做 int 乘法。預處理器 #if 用 long 運算不溢位,但執行期會踩雷。
#define EMB_HALL_EDGES_PER_WHEEL_REV ((1UL * HALL_EDGES_PER_REV * GEAR_RATIO_X100) / 100UL)
#define EMB_ROLLBACK_MAX_EDGES (EMB_HALL_EDGES_PER_WHEEL_REV / 4UL)
#if (EMB_ROLLBACK_REV_EDGES) > (EMB_ROLLBACK_MAX_EDGES)
#error "EMB_ROLLBACK_REV_EDGES 超過 1/4 車輪 (91 邊緣) 的倒溜上限規格"
#endif

// 附加閘門：HallPulsesLatch(每 100ms 邊緣數) >= 此值才允許鎖定。**0 = 停用**(預設)。
//   ⚠ 設為非 0 會重新引入「最低倒溜速度」的限制 (1 pulse/100ms = 0.063 km/h)，
//   低於該速度的慢速潛行倒溜將永遠不觸發鎖定 → 會違反「不超過 1/4 車輪」的規格；
//   而且 HallPulsesLatch 每 100ms 才更新，會多出最多 100ms 的延遲。
//   保留可調是為了在實機出現誤鎖時，能用它排除極低速的抖動來源。
#define EMB_ROLLBACK_MIN_PULSES 0

// F/R 排檔切換後的誤觸抑制窗 (ms)。排檔硬體開關被切換的瞬間，車與新排檔方向不一致是必然
//   的過渡態(車還沒回應) → 若不抑制會立即誤觸鎖定。抑制窗內不觸發鎖定，讓車有時間停下或
//   在新方向上開始移動。
//   [2026-08-15] 300 → 500 ms。舊版由「命令」方向翻轉起算，命令歸零＝解除武裝作為
//   附加保險;新版由「排檔」方向翻轉起算，沒有命令歸零的保險，需要更長的抑制窗涵蓋
//   車實際減速停止的時間。
#define EMB_ROLLBACK_FLIP_HOLDOFF_MS 500

// =============================================================================
//  下坡滑動偵測 (命令已歸零、車卻沒有在減速 → EMB 立即鎖定)
// =============================================================================
//  ⚠ 與上面的「有動力倒溜」(EMB_ROLLBACK_*) 是**不同的物理現象**，參數刻意完全獨立：
//    倒溜     = 車往與命令**相反**的方向動 → 方向問題 → 規格管位移 (≤1/4 車輪)
//    下坡滑動 = 車往與命令**相同**的方向動但比命令快 → 速度問題 → 無位移規格
//  兩者共用霍爾邊緣中斷，但計數方式與判斷條件都不同，不可合併。
//
//  [為什麼倒溜偵測抓不到下坡滑動]
//    g_u8EmbRevEdgeCnt 是「反向 +1、正向 -1、**地板 0**」的淨計數。下坡往前滑時每個邊緣
//    都是 -1，被地板夾在 0 → 結構上永遠不可能達到門檻。這不是門檻調不調的問題。
//
//  [情境] 停在坡上「點下油門後立刻放掉」：
//    點油門 → EMB 釋放、車被重力拉動 → 立刻放掉 → 命令歸零。此時
//      (1) UVW 短路的制動扭矩 ∝ 反電動勢 ∝ 轉速，剛起步 ω≈0 幾乎無扭矩，撐不住坡度;
//      (2) 速度環 PI 也接不住 —— reference 為 0 時「零速度誤差 ⇒ 零扭矩命令」，
//          誤差小 → P 項小、I 項 (SPEEDCNTR_ITERM=20) 爬得慢，扭矩來不及建立;
//      (3) 倒溜偵測見上，結構上不涵蓋。
//    → 三條路都不通，只剩 EMBRAKER_LOCK_TIMEOUT_MS 的 failsafe，帶速硬夾且不舒適。
//    本偵測在「車剛開始滑動」時就上鎖，此時動能極小，不適感最低。
//
//  [偵測邏輯 — V0.21 改版] 下列閘門同時成立才鎖，(1)(2)(3) 全在 CNRead_Inline 內判斷：
//    (1) 命令已歸零   piInputOmega.inReference == 0
//                     —— 即 g_i16EmbZeroCmdEdgeCnt 的計數閘門，命令非零就歸零重算，
//                        故計數天然從「命令歸零」那一刻起算。
//    (2) 車沒有減速   本次霍爾週期 <= (LOOKBACK 個邊緣前的週期) x (1 + 1/2^TOL_SHIFT)，
//                     連續成立 EMB_DOWNHILL_NODECEL_CONFIRM 次 → g_u8EmbNoDecelCnt 達標
//    (3) 車速低於上限 HallPeriod > EMB_DOWNHILL_MIN_PERIOD (週期下限 = 速度上限)
//    (4) 位移門檻     |g_i16EmbZeroCmdEdgeCnt| >= EMB_DOWNHILL_SLIDE_EDGES (額外雜訊裕度)
//
//  [為什麼 (2) 是關鍵] 自由滑行的減速度 = 滾動阻力 + 傳動阻力，**恆為正**，所以平路與
//    上坡的霍爾週期必然逐步變長。「週期沒變長」因此等價於「重力已抵銷全部阻力 → 車不會
//    自己停下來」，正是本功能真正要偵測的事。這是唯一不依賴車速門檻、也不依賴任何跨迴圈
//    狀態的判別 —— 平路正常停車與室內點放油門微調完全不受影響 (週期一路變長，(2) 恆不
//    成立)，而且門檻只需要贏過量測雜訊，不必與坡度大小賽跑。
//
//  [為什麼 (3) 不可省] 它把不適感**在建構上**夾住：不論上游邏輯出什麼錯，都不可能在
//    上限速度之上夾煞。下坡帶速放油門 (例如 6 km/h) 因此不會被本偵測硬夾，而是交回
//    UVW 短路/回充處理，等車速降到上限以下且**仍在加速**時才由 EMB 接手。
//
//  [V0.20 的武裝旗標為何被移除 —— 不要加回來]
//    舊版靠 s_bEmbDownhillArmed (RELEASE 時武裝;「命令非零 且 HallPulsesLatch >
//    EMB_DOWNHILL_DISARM_PULSES(8 = 0.5 km/h)」時 sticky 解除) 來擋平路誤鎖。但油門的最低
//    輸出是 LOGIC_THROTTLE_FWD_OUTPUT_MIN = 0.97 km/h ≈ 15.4 pulses/100ms (V0.20 當時是
//    1.04 km/h ≈ 16.5)，且加速濾波一按油門就直接預載到該值 —— 解除武裝門檻 (0.5 km/h)
//    **低於車子能被命令的最低速度**，
//    於是「按油門」這個動作本身就會在駕駛放開之前把武裝解除 → 下坡點放永遠偵測不到。
//    V0.20 實車確認: SLIDE_EDGES 由 24 調到 16 完全無反應，因為判斷式的前半段早已是 false。
//    → 舊參數 EMB_DOWNHILL_DISARM_PULSES / EMB_DOWNHILL_ARM_WINDOW_MS 隨旗標一併刪除。
//    → 「防平路誤鎖」的職責由 (2) 的物理判別承接，「限制不適感」由 (3) 的速度上限承接。
//      兩者都不需要跨迴圈的狀態，因此也不再有「旗標卡在武裝狀態 → 每次平路停車都帶速
//      硬夾」這個單點失效 (那是舊設計自己記錄的最大風險)。
//
//  [失效時的保底] 任一閘門不成立就不鎖，此時仍有 EMBRAKER_LOCK_TIMEOUT_MS(1000ms) 的
//    failsafe 無條件夾 (它刻意沒有車速閘門)。故本偵測的失效模式是「夾得比較晚」而非
//    「不夾」；反過來說，任何比 failsafe 更早的觸發都嚴格更輕柔。

// --- 「沒有減速」判別 (在 CNRead_Inline 內以霍爾週期比較) ---
//  [判準] **只要週期沒變長，就是沒減速。**
//    自由滑行的減速度 = 滾動阻力 + 傳動阻力，恆為正 → 平路與上坡的霍爾週期**必然**逐步
//    變長。因此「週期沒變長」等價於「重力至少已抵銷全部阻力」= 車不會自己停下來。
//    這比「週期縮短超過某比例」正確得多：後者的門檻必須與坡度大小賽跑 —— 6 邊緣基線在
//    10% 坡上只給 -8.6% 的變化，撐不起 12.5% 的裕度，緩坡更是完全抓不到，而漏抓的正是
//    「車不會自己停」的情形。改成比對「有沒有變長」之後，門檻只需要贏過量測雜訊，
//    與坡度陡緩無關。

// 比較間隔 (邊緣數)。**必須是 6 的倍數** —— 相隔 6 個邊緣才是同一組霍爾狀態轉換，
//   感測器裝配不等距造成的週期差因此對消;非 6 的倍數會把裝配誤差誤判成減速/加速。
//   [為何取 12 而不是 6] 基線越長，平路的「變長」訊號越明顯，而裝配誤差與邊界抖動不會
//   隨基線累積 → 拉長基線同時提升靈敏度與雜訊免疫。以起點 0.97 km/h 計，平路自由滑行
//   使週期變長：6 邊緣基線 +3.1%、12 邊緣基線 +6.4% —— 後者對下方的容忍值有 4 倍餘裕。
//   代價：最小偵測位移由 (6+CONFIRM) 變成 (12+CONFIRM) 個邊緣。
#define EMB_DOWNHILL_NODECEL_LOOKBACK 12

// 「沒變長」的雜訊容忍 = 1/2^n。6 → 1.6%：週期變長不超過 1.6% 仍視為沒減速。
//   換算成減速度：1.6% / 12 邊緣基線 / 起點 0.97 km/h ⇒ 約 0.054 m/s²。也就是
//   「減速慢於 0.054 m/s²」被當成沒在減速 —— 那種車實際上也不會在合理距離內停下來。
//   對照 (12 邊緣基線、起點 0.97 km/h = 油門最低命令、阻力估 0.2 m/s²)：
//      平路      → 週期 +6.4%  → 判定減速 ✔ (與容忍值有 4 倍距離)
//      2% 坡     → 約 ±0%      → 判定沒減速 (阻力與重力打平，車等速滑行不會停)
//      5% 坡     → 週期 -7.5%      10% 坡 → -17%      20% 坡 → -30%
//   ⚠ 調整方向與舊的「縮短裕度」相反：
//      本值**加大**(7 = 0.78%) → 更嚴格，更不易誤鎖，但阻力極小的車可能漏抓極緩坡;
//      本值**縮小**(5 = 3.1%)  → 更寬鬆，若實車阻力低於估值可用，但平路誤鎖風險上升。
//   誤鎖的必要條件是「平路阻力低到週期變長不足 1.6%」≈ 阻力 < 0.06 m/s²，
//   對有齒輪組的輪轂馬達不現實 —— 這是本判準比舊版穩健的地方。
#define EMB_DOWNHILL_NODECEL_TOL_SHIFT 6

// 需連續成立幾次才確認。3 次 → 最少 (LOOKBACK + 3) = 15 個邊緣 = 26 mm 的持續「沒減速」，
//   單一顛簸或單次邊界抖動吃不到 (雜訊要連續 3 次同向才會騙過)。
//   調低反應更快但更容易誤觸;調高則觸發位移等量增加。
#define EMB_DOWNHILL_NODECEL_CONFIRM 3

// 淨位移邊緣門檻。1 邊緣 = 車輪 1.75 mm (同 EMB_ROLLBACK 的換算)。
//   [V0.20 → V0.21] 24 (42 mm) → 15 (26 mm)。舊值必須撐起「平路 vs 下坡」的區分，故得
//   大於平路滑行距離;現在該職責由上面的「沒有減速」判別承擔，本門檻只剩雜訊裕度的角色。
//   預設刻意等於該判別的隱含下限 (LOOKBACK + CONFIRM) —— 兩個閘門同時達標，觸發點就是
//   26 mm，沒有任何一個閘門在空轉。用推導式而非字面值，改 LOOKBACK 時不會悄悄失效。
//   ⚠ 設成小於 (LOOKBACK + CONFIRM) 不會讓觸發提前 (「沒有減速」判別才是瓶頸)，只會讓
//     本門檻失去作用;要延後觸發點請改成更大的字面值。
#define EMB_DOWNHILL_SLIDE_EDGES (EMB_DOWNHILL_NODECEL_LOOKBACK + EMB_DOWNHILL_NODECEL_CONFIRM)

// --- 車速上限 → 霍爾週期下限 (編譯期換算) ---
//   為何用 HallPeriod 而不用其他速度來源：
//     ✗ Speed —— 停車顫動時 HallPeriod 極短會使其飽和/失真，全檔案都在處理這個雷
//                (見 main.c CNRead_Inline 的 [FIX 2026-08-11] 說明)。
//     △ HallPulsesLatch —— 穩定但每 100ms 才更新，3 km/h 時有約 8 cm 的陳舊誤差。
//     ✓ HallPeriod —— ISR 當下就有的瞬時值。週期與車速成反比，「速度上限」等價於
//                     「週期下限」，一次無號比較即可，不需除法。
//   輪徑與齒比取自 motor_scale.h (WHEEL_DIAMETER_INCH_X10 = 8.0 吋 → 周長 638 mm、
//   GEAR_RATIO_X100 = 20.30)，與本節「1 邊緣 = 1.75 mm」同一組假設。
//   HALL_MIN_PERIOD 若因刻度變更而偏離 434，motor_scale.h 的編譯期護欄會先擋下。
//   對照表 (實際由下式算出的值，改參數後可用來核對)：
//     1.5 km/h → 6562      2.5 km/h → 3929
//     2.0 km/h → 4915      3.0 km/h → 3276
#define EMB_KMHX100_TO_HALL_PERIOD(kx)                                            \
    ((uint16_t)((HALL_MIN_PERIOD * 32768UL) /                                     \
                RPMX10_TO_CMD(KMHX100_TO_MOTOR_RPMX10(kx))))

// 允許本偵測夾煞的車速上限 (km/h x10)。**上限之上絕不可能由本偵測夾煞** —— 見上方 (3)。
//   下限被兩件事夾住：必須高於油門最低命令 0.97 km/h，且要留給陡坡的裕度 ——
//   偵測達標 (15 邊緣 = 26 mm) 時的車速隨坡度上升 (起點 0.97 km/h，阻力估 0.2 m/s²)：
//      坡度  5% → 1.06 km/h        坡度 20% → 1.46 km/h
//      坡度 10% → 1.21 km/h        坡度 30% → 1.67 km/h
//   設太低會讓**坡越陡越容易漏鎖** (車先衝過上限才走完位移)，那是保護方向顛倒，
//   寧可留裕度。3 km/h 對 30% 坡仍有近一倍餘裕。
//   對照：現況 failsafe 1000ms 在 10% 坡上要到約 3.9 km/h / 60 cm 才夾，本值是嚴格改善。
#define EMB_DOWNHILL_MAX_SPEED_KMHX100 300  // 3.00 km/h
#define EMB_DOWNHILL_MIN_PERIOD EMB_KMHX100_TO_HALL_PERIOD(EMB_DOWNHILL_MAX_SPEED_KMHX100)

#if (EMB_DOWNHILL_MAX_SPEED_KMHX100) < 150
#error "EMB_DOWNHILL_MAX_SPEED_KMHX100 低於 1.50 km/h: 未高於油門最低命令(0.97 km/h)的必要裕度, 陡坡必然漏鎖"
#endif
#if (EMB_DOWNHILL_MAX_SPEED_KMHX100) > 500
#error "EMB_DOWNHILL_MAX_SPEED_KMHX100 高於 5.00 km/h: 帶速硬夾的衝擊與煞車片磨耗不可接受"
#endif

// 電壓向量限制
#define MAX_VOLTAGE_VECTOR 0.95  // 最大電壓向量限制為 95%，用於 SVPWM 調變

//-----------------------------------------------------------------------------
// Hall sensor input pin definitions based on PIN_TABLE_FOR_EDIT.md
// Pin 50: RC4 (IHU), Pin 51: RC5 (IHV), Pin 52: RC10 (IHW)
//-----------------------------------------------------------------------------
#define I_HALL_U_TRIS _TRISC4
#define I_HALL_V_TRIS _TRISC5
#define I_HALL_W_TRIS _TRISC10
#define I_HALL_U_PIN _RC4
#define I_HALL_V_PIN _RC5
#define I_HALL_W_PIN _RC10
#define I_HALL_U_CNIE CNENCbits.CNIEC4
#define I_HALL_V_CNIE CNENCbits.CNIEC5
#define I_HALL_W_CNIE CNENCbits.CNIEC10

//-----------------------------------------------------------------------------
// Control I/O pin definitions based on PIN_TABLE_FOR_EDIT.md
//-----------------------------------------------------------------------------
// Pin 3: RC12 (IBKS) - 煞車訊號
#define I_BRAKE_PIN _RC12  // IBKS, Brake Input
#define I_BRAKE_TRIS _TRISC12
#define I_BRAKE_PULLUP CNPUCbits.CNPUC12  // 煞車訊號上拉電阻控制

// Pin 11: RD14 (IFR) - 前進/後退開關
#define I_FR_SWITCH_PIN _RD14  // IFR, Forward/Reverse Switch
#define I_FR_SWITCH_TRIS _TRISD14
#define I_FR_SWITCH_PULLUP CNPUDbits.CNPUD14  // 前進/後退開關上拉電阻控制

// Pin 8: RD15 (CRUISE) - 啟動/停止開關
#define I_CRUISE_PIN _RD15  // CRUISE, Cruise Control
#define I_CRUISE_TRIS _TRISD15
#define I_CRUISE_PULLUP CNPUDbits.CNPUD15  // 啟動/停止開關上拉電阻控制

// Pin 5: RC14 (ISNA) - 方向開關A
#define I_SPEED_SENSOR_A_PIN _RC14  // ISNA, Speed Sensor A
#define I_SPEED_SENSOR_A_TRIS _TRISC14
#define I_SPEED_SENSOR_A_PULLUP CNPUCbits.CNPUC14  // 方向開關A上拉電阻控制

// Pin 6: RC15 (ISNB) - 方向開關B
#define I_SPEED_SENSOR_B_PIN _RC15  // ISNB, Speed Sensor B
#define I_SPEED_SENSOR_B_TRIS _TRISC15
#define I_SPEED_SENSOR_B_PULLUP CNPUCbits.CNPUC15  // 方向開關B上拉電阻控制

// Pin 54: RD4 (ILSN) - 外部速度感測
#define I_EXT_SPEED_SENSOR_PIN _RD4  // ILSN, External Speed Sensor
#define I_EXT_SPEED_SENSOR_TRIS _TRISD4
#define I_EXT_SPEED_SENSOR_PULLUP CNPUDbits.CNPUD4  // 外部速度感測上拉電阻控制

//-----------------------------------------------------------------------------
// Analog Input pin definitions (類比輸入腳位定義)
//-----------------------------------------------------------------------------
// Pin 30: RD11 (ITQS) - 扭力感測 (AN19)
#define A_TORQUE_SENSOR_PIN _RD11  // ITQS, Torque Sensor Input (AN19)
#define A_TORQUE_SENSOR_TRIS _TRISD11

// Pin 32: RC7 (NTC) - 溫度偵測（NTC）(AN16)
#define A_MOTOR_TEMP_PIN _RC7  // NTC, Motor Temperature Sensor (AN16)
#define A_MOTOR_TEMP_TRIS _TRISC7

// Pin 13: RC0 (TPV) - 溫度感測 (AN12)
#define A_CONTROLLER_TEMP_PIN _RC0  // TPV, Controller Temperature Sensor (AN12)
#define A_CONTROLLER_TEMP_TRIS _TRISC0

// Pin 17: RA3 (ISPD) - 馬達速度偵測/保護 (AN3)
#define A_SPEED_PROTECT_PIN _RA3  // ISPD, Speed Protection Input (AN3)
#define A_SPEED_PROTECT_TRIS _TRISA3

// Pin 27: RC3 (VBATT) - 電池電壓偵測 (AN15)
#define A_BATTERY_VOLTAGE_PIN _RC3  // VBATT, Battery Voltage Sensor (AN15)
#define A_BATTERY_VOLTAGE_TRIS _TRISC3

// Pin 24: RC6 (IHS) - IHS / 油門VR (共用) (AN17)
#define A_THROTTLE_VR_PIN _RC6  // IHS, Throttle/VR Input (AN17)
#define A_THROTTLE_VR_TRIS _TRISC6

// Pin 28: RB0 (BEMF_A) - A相反電動勢 (AN5)
#define A_BEMF_A_PIN _RB0  // BEMF_A, A-phase Back EMF (AN5)
#define A_BEMF_A_TRIS _TRISB0

// Pin 29: RB1 (BEMF_B) - B相反電動勢 (AN6)
#define A_BEMF_B_PIN _RB1  // BEMF_B, B-phase Back EMF (AN6)
#define A_BEMF_B_TRIS _TRISB1

// Pin 31: RD10 (BEMF_C) - C相反電動勢 (AN18)
#define A_BEMF_C_PIN _RD10  // BEMF_C, C-phase Back EMF (AN18)
#define A_BEMF_C_TRIS _TRISD10

// Pin 33: RB2 (OA2_OUT) - IB相電流輸出 (AN1/AN7)
#define A_IB_CURRENT_OUT_PIN _RB2  // OA2_OUT, IB Current Output (AN1/AN7)
#define A_IB_CURRENT_OUT_TRIS _TRISB2

// Pin 34: RB3 (OA2_IN-) - IB相電流負端 (AN8)
#define A_IB_CURRENT_NEG_PIN _RB3  // OA2_IN-, IB Current Negative (AN8)
#define A_IB_CURRENT_NEG_TRIS _TRISB3

// Pin 35: RB4 (OA2_IN+) - IB相電流正端 (非直接ADC通道)
#define A_IB_CURRENT_POS_PIN _RB4  // OA2_IN+, IB Current Positive
#define A_IB_CURRENT_POS_TRIS _TRISB4

// Pin 14: RA0 (OA1_OUT) - IA相電流輸出 (AN0)
#define A_IA_CURRENT_OUT_PIN _RA0  // OA1_OUT, IA Current Output (AN0)
#define A_IA_CURRENT_OUT_TRIS _TRISA0

// Pin 15: RA1 (OA1_IN-) - IA相電流負端 (ANA1)
#define A_IA_CURRENT_NEG_PIN _RA1  // OA1_IN-, IA Current Negative (ANA1)
#define A_IA_CURRENT_NEG_TRIS _TRISA1

// Pin 16: RA2 (OA1_IN+) - IA相電流正端 (AN9)
#define A_IA_CURRENT_POS_PIN _RA2  // OA1_IN+, IA Current Positive (AN9)
#define A_IA_CURRENT_POS_TRIS _TRISA2

// Pin 18: RA4 (OA3_OUT) - 運算放大器3輸出 (AN4)
#define A_OA3_OUT_PIN _RA4  // OA3_OUT, Op-Amp 3 Output (AN4)
#define A_OA3_OUT_TRIS _TRISA4

// Pin 22: RC1 (OA3_IN-) - 運算放大器3負端 (AN13)
#define A_OA3_NEG_PIN _RC1  // OA3_IN-, Op-Amp 3 Negative (AN13)
#define A_OA3_NEG_TRIS _TRISC1

// Pin 23: RC2 (OA3_IN+) - 運算放大器3正端 (AN14)
#define A_OA3_POS_PIN _RC2  // OA3_IN+, Op-Amp 3 Positive (AN14)
#define A_OA3_POS_TRIS _TRISC2

// Pin 39: RD8 (IEMB) - 電磁煞車開關 (類比輸入)
// #define A_EM_BRAKE_SWITCH_PIN _RD8  // IEMB, EM-Brake Switch Input
// #define A_EM_BRAKE_SWITCH_TRIS _TRISD8
#define I_EM_BRAKE_SWITCH_PIN _RD8
#define I_EM_BRAKE_SWTICH_TRIS _TRISD8
#define I_EM_BRAKE_SWITCH_PULLUP CNPUDbits.CNPUPD8

//-----------------------------------------------------------------------------
// LED and Output Control pin definitions
//-----------------------------------------------------------------------------
// Pin 42: RD7 (OLPG) - 綠色LED
#define O_LED_GREEN_LAT _LATD7  // OLPG, Green LED
#define O_LED_GREEN_TRIS _TRISD7

// Pin 43: RD6 (OLPY) - 黃色LED
#define O_LED_YELLOW_LAT _LATD6  // OLPY, Yellow LED
#define O_LED_YELLOW_TRIS _TRISD6

// Pin 44: RD5 (OLPR) - 紅色LED
#define O_LED_RED_LAT _LATD5  // OLPR, Red LED
#define O_LED_RED_TRIS _TRISD5

// Pin 55: RD3 (OLAMP) - 驅動大燈On
#define O_HEAD_LIGHT_LAT _LATD3  // OLAMP, Headlight Output
#define O_HEAD_LIGHT_TRIS _TRISD3

// Pin 58: RD2 (OBKL) - 煞車燈控制
#define O_BRAKE_LIGHT_LAT _LATD2  // OBKL, Brake Light Output
#define O_BRAKE_LIGHT_TRIS _TRISD2

// Pin 59: RD1 (OEMB) - 驅動電磁煞車On/Off
#define O_EM_BRAKE_CTRL_LAT _LATD1  // OEMB, EM-Brake Control Output
#define O_EM_BRAKE_CTRL_TRIS _TRISD1

//-----------------------------------------------------------------------------
// Communication I/O pin definitions
//-----------------------------------------------------------------------------
// Pin 36: RC8 (TX) - UART傳送
#define O_UART_TX_PIN _RC8  // TX, UART Transmit
#define O_UART_TX_TRIS _TRISC8

// Pin 37: RC9 (RX) - UART接收
#define I_UART_RX_PIN _RC9  // RX, UART Receive
#define I_UART_RX_TRIS _TRISC9

// Pin 38: RD9 (485RE) - RS485方向控制
#define O_RS485_RE_LAT _LATD9  // 485RE, RS485 Receive Enable
#define O_RS485_RE_TRIS _TRISD9

// Pin 47: RB7 (STB) - CAN待機控制
#define O_CAN_STB_LAT _LATB7  // STB, CAN Standby
#define O_CAN_STB_TRIS _TRISB7

// Pin 48: RB8 - 原 CAN_TX，CAN 停用後改為 X2CScope 專屬 UART2 傳送 (U2TX)
#define O_CAN_TX_PIN _RB8  // CAN_TX, CAN Transmit (deprecated: now X2C U2TX)
#define O_CAN_TX_TRIS _TRISB8
#define O_X2C_TX_PIN _RB8   // X2CScope U2TX (RB8, RP40)
#define O_X2C_TX_TRIS _TRISB8

// Pin 49: RB9 - 原 CAN_RX，CAN 停用後改為 X2CScope 專屬 UART2 接收 (U2RX)
#define I_CAN_RX_PIN _RB9  // CAN_RX, CAN Receive (deprecated: now X2C U2RX)
#define I_CAN_RX_TRIS _TRISB9
#define I_X2C_RX_PIN _RB9   // X2CScope U2RX (RB9, RP41)
#define I_X2C_RX_TRIS _TRISB9

//-----------------------------------------------------------------------------
// PWM Output pin definitions
//-----------------------------------------------------------------------------
// Pin 1: RB14 (PWM1H) - 馬達PWM1上橋臂
#define O_PWM1H_PIN _RB14  // PWM1H, Motor PWM1 High
#define O_PWM1H_TRIS _TRISB14

// Pin 2: RB15 (PWM1L) - 馬達PWM1下橋臂
#define O_PWM1L_PIN _RB15  // PWM1L, Motor PWM1 Low
#define O_PWM1L_TRIS _TRISB15

// Pin 63: RB12 (PWM2H) - 馬達PWM2上橋臂
#define O_PWM2H_PIN _RB12  // PWM2H, Motor PWM2 High
#define O_PWM2H_TRIS _TRISB12

// Pin 64: RB13 (PWM2L) - 馬達PWM2下橋臂
#define O_PWM2L_PIN _RB13  // PWM2L, Motor PWM2 Low
#define O_PWM2L_TRIS _TRISB13

// Pin 61: RB10 (PWM3H) - 馬達PWM3上橋臂
#define O_PWM3H_PIN _RB10  // PWM3H, Motor PWM3 High
#define O_PWM3H_TRIS _TRISB10

// Pin 62: RB11 (PWM3L) - 馬達PWM3下橋臂
#define O_PWM3L_PIN _RB11  // PWM3L, Motor PWM3 Low
#define O_PWM3L_TRIS _TRISB11

//-----------------------------------------------------------------------------
// Debug and Programming pin definitions
//-----------------------------------------------------------------------------
// Pin 45: RB5 (PGED) - 程式/除錯資料
#define I_PGED_PIN _RB5  // PGED, Programming Data
#define I_PGED_TRIS _TRISB5

// Pin 46: RB6 (PGEC) - 程式/除錯時脈
#define I_PGEC_PIN _RB6  // PGEC, Programming Clock
#define I_PGEC_TRIS _TRISB6

//-----------------------------------------------------------------------------
// Unused pins (需要確認用途)
//-----------------------------------------------------------------------------
// Pin 4: RC13 - 除錯量測腳位：ADC ISR 執行期間拉高 (示波器量測)
#define O_DBG_ADC_ISR_LAT _LATC13  // Debug: ADC ISR profiling output
#define O_DBG_ADC_ISR_TRIS _TRISC13

// Pin 12: RD13 - 除錯量測腳位：速度命令處理期間拉高 (示波器量測)
#define O_DBG_SPEED_PROFILE_LAT _LATD13  // Debug: speed-command profiling output
#define O_DBG_SPEED_PROFILE_TRIS _TRISD13

// Pin 21: RD12 - 未標示，請確認用途
#define I_UNUSED_RD12_PIN _RD12  // Unused pin RD12
#define I_UNUSED_RD12_TRIS _TRISD12

// Pin 53: RC11 - 未標示，請確認用途
#define I_UNUSED_RC11_PIN _RC11  // Unused pin RC11
#define I_UNUSED_RC11_TRIS _TRISC11

// Pin 60: RD0 - 未標示，請確認用途
#define I_UNUSED_RD0_PIN _RD0  // Unused pin RD0
#define I_UNUSED_RD0_TRIS _TRISD0

void OverCurrentEnable(void);
void CMP1_ISR(void);
void CN_Configure(void);

/**
 * @brief 初始化數位輸入腳位的內部上拉電阻
 * @details 啟用重要輸入腳位的內部上拉電阻，以改善訊號穩定性
 */
void InitDigitalInputPullups(void);

#ifdef __cplusplus
}
#endif

#endif /* USERPARMS_H */
