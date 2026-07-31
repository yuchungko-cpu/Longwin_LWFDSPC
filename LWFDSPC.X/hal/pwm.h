/*******************************************************************************
   Header File for High-Resolution PWM with Fine Edge Placement Configuration

  File Name:
    pwm.h

  Summary:
    This header file lists routines to configure High-Resolution PWM with Fine 
    Edge Placement. 

  Description:
    Definitions in the file are for dsPIC33CK256MP508 MC PIM plugged onto
    Motor Control Development board from Microchip.

*******************************************************************************/
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
#ifndef _PWM_H
#define _PWM_H

#ifdef __cplusplus  // Provide C++ Compatability
    extern "C" {
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <xc.h>
#include <stdint.h>
#include "clock.h"
// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

// MC PWM MODULE Related Definitions
#define INVERTERA_PWM_PDC1      PG1DC
#define INVERTERA_PWM_PDC2      PG2DC
#define INVERTERA_PWM_PDC3      PG3DC

/* Specify PWM Frequency in Hertz */
#define PWMFREQUENCY_HZ         20000
/* Specify dead time in micro seconds */
#define DEADTIME_MICROSEC       1.0
/* Specify PWM Period in seconds, (1/ PWMFREQUENCY_HZ) */
#define LOOPTIME_SEC            0.00005
/* Specify PWM Period in micro seconds */
#define LOOPTIME_MICROSEC       50

// Specify bootstrap charging time in Seconds (mention at least 10mSecs)
#define BOOTSTRAP_CHARGING_TIME_SECS 0.01

// Calculate Bootstrap charging time in number of PWM Half Cycles
#define BOOTSTRAP_CHARGING_COUNTS (uint16_t)((BOOTSTRAP_CHARGING_TIME_SECS/LOOPTIME_SEC )* 2)

// Definition to enable or disable PWM Fault
#undef ENABLE_PWM_FAULT

#define DDEADTIME               (uint16_t)(DEADTIME_MICROSEC*FOSC_MHZ)
// loop time in terms of PWM clock period
#define LOOPTIME_TCY            (uint16_t)(((LOOPTIME_MICROSEC*FOSC_MHZ)/2)-1)
// [FIX] 原定義為 `(signed int)(LOOPTIME_TCY + 1)/2;` —— 帶拖尾分號且缺外層括號。
//   只有在「敘述結尾」位置才能用(展開成 ;; 無害)，一放進表達式就會壞：
//   `if (x > HALF_PWMDUTY)` → 語法錯誤；`a / HALF_PWMDUTY` → 優先序變成 (a/(TCY+1))/2。
//   展開值不變(仍為 2500)，純粹是巨集衛生修正。
#define HALF_PWMDUTY            ((signed int)((LOOPTIME_TCY + 1) / 2))

// DC bus 電流正規化增益 (Q11)。IbusCalc() 用 __builtin_mpy + __builtin_sacr(-4)，
//   等效增益 = K × 16 / 32768 = 32768 / LOOPTIME_TCY。
//   [FIX] 原本硬寫 26843 = 32768/2500，把 duty 偏移量除以「半週期」而非「週期」，
//   使 Ibus 為真實母線電流的 2 倍。正確分母是 MPER (= LOOPTIME_TCY = 4999) → 13424。
#define IBUS_NORM_Q11           ((signed int)((32768UL * 2048UL) / LOOPTIME_TCY))

// 死區時間造成的母線電流偏差補償 (Q15)。互補 PWM 在死區期間節點被回流二極體夾到某一軌，
//   有效工作比變成 dᵢ − δ·sign(iᵢ)(δ = 死區/PWM 週期) → Ibus_真實 = Σdᵢiᵢ − δ·Σ|iᵢ|。
//   這是單向偏差(與運轉象限無關，死區期間電流一律被導向回充方向)，用暫存器 duty 算出來的
//   Ibus 恆偏高，靠 Σiᵢ=0 消不掉。
//   δ 的值取決於 dead-time 暫存器的時鐘：PG1DTL/PG1DTH = DDEADTIME = 200 counts，
//     若與 duty 共用 PWM 時鐘 (10ns/count) → 死區 2.0us → δ = 200/4999 = 4.0% → Q15 1311
//     若 dead-time 走 FOSC (5ns/count)     → 死區 1.0us → δ = 2.0%          → Q15  655
//   FRM 尚未核對，故 ⚠ 預設 0 = 不補償(行為與加入本功能前完全相同)。
//   上機用鉤錶量母線電流、與上報值比對後再填入實測值。
#define IBUS_DEADTIME_COMP_Q15  0
/* Specify ADC Triggering Point w.r.t PWM Output for sensing Motor Currents */
#define ADC_SAMPLING_POINT      0x0000

#define MIN_DUTY            0x0000
// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
void InitPWMGenerators(void);        
// *****************************************************************************
#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif      // end of PWM_H


