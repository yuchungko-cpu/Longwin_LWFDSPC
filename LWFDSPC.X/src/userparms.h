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
 *     有效 Kp = 3000/2048 = 1.465 (輸出 count / 誤差 count)
 *             = 每 1 馬達RPM 誤差產生 4.0 counts 的 Iq 命令
 *     有效 Ki = 10/32768，速度環 1kHz → 每 1 count 誤差每 ms 累加 3.05e-4
 *   若改動 SPEED_FS_RPM，兩者必須同乘 (舊FS/新FS) 才能維持相同騎乘感。
 *   motor_scale.h 的 HALL_MIN_PERIOD != 434 護欄即為此而設。
 * 飽和門檻：outMax 由溫控電流上限覆寫 (常態 Q15(0.33)=10813)，
 *   → 誤差 >= 10813/1.465 = 7382 counts (2703 馬達RPM) 即滿輸出。
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
#define UVW_LOCK_RELEASE_REF 500                // 油門目標高於此值則解鎖；183 馬達RPM = 0.35 km/h。鬆油門=0，踩下時 >= OUTPUT_MIN(=1500 count)
// 進鎖脈衝門檻：HallPulsesLatch(每100ms邊緣數) < 此值視為已接近靜止。
// 換算 (18 邊緣/機械轉, 齒比 20.3)：1 pulse/100ms = 33.3 馬達RPM = 0.063 km/h
//   → 3 pulses = 100 馬達RPM = 0.19 km/h  (舊註解「7≈馬達58RPM, 極對數12」已失效)
// 獨立於 ReGen 的 BrakeStopSpeedPulses，調此值不影響 ReGen 起煞點。
#define UVW_LOCK_STOP_PULSES 15

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
#define EMB_ROLLBACK_CMD_THRESHOLD 100  // |inReference| 需 > 此值才啟用偵測；100 = 36.6 馬達RPM = 0.069 km/h
#define EMB_ROLLBACK_REV_EDGES 90        // 淨反向霍爾邊緣門檻 → 立即鎖定。3 = 倒退 5.2 mm、90 = 157 mm
                                        //   太靈敏(誤鎖)就加大，太遲鈍就縮小；上限見下方護欄

// 每輪轉的霍爾邊緣數與 1/4 車輪上限 (由 motor_scale.h 的參數推導，改齒比/極對數會自動跟著變)
#define EMB_HALL_EDGES_PER_WHEEL_REV ((HALL_EDGES_PER_REV * GEAR_RATIO_X100) / 100)
#define EMB_ROLLBACK_MAX_EDGES (EMB_HALL_EDGES_PER_WHEEL_REV / 4)
#if (EMB_ROLLBACK_REV_EDGES) > (EMB_ROLLBACK_MAX_EDGES)
#error "EMB_ROLLBACK_REV_EDGES 超過 1/4 車輪 (91 邊緣) 的倒溜上限規格"
#endif

// 附加閘門：HallPulsesLatch(每 100ms 邊緣數) >= 此值才允許鎖定。**0 = 停用**(預設)。
//   ⚠ 設為非 0 會重新引入「最低倒溜速度」的限制 (1 pulse/100ms = 0.063 km/h)，
//   低於該速度的慢速潛行倒溜將永遠不觸發鎖定 → 會違反「不超過 1/4 車輪」的規格；
//   而且 HallPulsesLatch 每 100ms 才更新，會多出最多 100ms 的延遲。
//   保留可調是為了在實機出現誤鎖時，能用它排除極低速的抖動來源。
#define EMB_ROLLBACK_MIN_PULSES 0

// F/R 切換後的誤觸抑制窗 (ms)。方向更新只允許在車速 <= 1.0 km/h 時發生，但切換後車仍可能
//   低速滑行於舊方向 → 命令與滾動反向 → 立即鎖定並閂鎖至鬆油門。只在「命令符號由非零翻到
//   相反非零」時起算；由 0 變非零(上坡起步)不抑制，故坡道偵測仍是立即的。0 = 停用。
#define EMB_ROLLBACK_FLIP_HOLDOFF_MS 300

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
