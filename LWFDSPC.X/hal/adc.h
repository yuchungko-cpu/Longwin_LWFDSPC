/*******************************************************************************
  ADC Configuration Routine Header File

  File Name:
    adc.h

  Summary:
    This header file lists ADC Configuration related functions and definitions

  Description:
    Definitions in the file are for dsPIC33CK256MP508 MC PIM plugged onto
    Motor Control Development board from Microchip

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
#ifndef _ADC_H
#define _ADC_H

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
// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************
// ADC MODULE Related Definitions
// 根據 PIN_TABLE_FOR_EDIT.md 的類比輸入通道定義

// 電流感測通道
#define ADCBUF_IA_CURRENT_OUT    ADCBUF0    // Pin 14: RA0 (OA1_OUT) - IA相電流輸出
#define ADCBUF_IA_CURRENT_NEG    ADCBUF1    // Pin 15: RA1 (OA1_IN-) - IA相電流負端
#define ADCBUF_IA_CURRENT_POS    ADCBUF2    // Pin 16: RA2 (OA1_IN+) - IA相電流正端
#define ADCBUF_IB_CURRENT_OUT    ADCBUF5    // Pin 33: RB2 (OA2_OUT) - IB相電流輸出
#define ADCBUF_IB_CURRENT_NEG    ADCBUF8    // Pin 34: RB3 (OA2_IN-) - IB相電流負端
#define ADCBUF_IB_CURRENT_POS    ADCBUF9    // Pin 35: RB4 (OA2_IN+) - IB相電流正端

// 反電動勢感測通道
#define ADCBUF_BEMF_A            ADCBUF5    // Pin 28: RB0 (BEMF_A) - A相反電動勢
#define ADCBUF_BEMF_B            ADCBUF6    // Pin 29: RB1 (BEMF_B) - B相反電動勢
#define ADCBUF_BEMF_C            ADCBUF18   // Pin 31: RD10 (BEMF_C) - C相反電動勢

// 運算放大器3相關通道
#define ADCBUF_OA3_OUT           ADCBUF4    // Pin 18: RA4 (OA3_OUT) - 運算放大器3輸出
#define ADCBUF_OA3_NEG           ADCBUF13   // Pin 22: RC1 (OA3_IN-) - 運算放大器3負端
#define ADCBUF_OA3_POS           ADCBUF14   // Pin 23: RC2 (OA3_IN+) - 運算放大器3正端

// 感測器通道
#define ADCBUF_THROTTLE_VR       ADCBUF17   // Pin 24: RC6 (IHS) - 油門/VR輸入
#define ADCBUF_BATTERY_VOLTAGE   ADCBUF15   // Pin 27: RC3 (VBATT) - 電池電壓偵測
#define ADCBUF_CONTROLLER_TEMP   ADCBUF12   // Pin 13: RC0 (TPV) - 控制器溫度偵測
#define ADCBUF_MOTOR_TEMP        ADCBUF16   // Pin 32: RC7 (NTC) - 馬達溫度偵測
#define ADCBUF_TORQUE_SENSOR     ADCBUF19   // Pin 30: RD11 (ITQS) - 扭力感測
#define ADCBUF_SPEED_PROTECT     ADCBUF3    // Pin 17: RA3 (ISPD) - 馬達速度偵測/保護
                                          
// 增加手煞車訊號 IEMB
#define ADCBUF_EMBRAKER          ADCBUF8    // Pin 39: RD8 (IEMB) - 電磁煞車開關

// 相容性定義（保持向後相容）
#define ADCBUF_INV_A_IPHASE1     ADCBUF_IA_CURRENT_OUT
#define ADCBUF_INV_A_IPHASE2     ADCBUF_IA_CURRENT_NEG
#define ADCBUF_SPEED_REF_A       ADCBUF_THROTTLE_VR
#define ADCBUF_VOLTAGE           ADCBUF_BATTERY_VOLTAGE
#define ADCBUF_TEMPERATURE_MOSFET ADCBUF_CONTROLLER_TEMP
//------ e-scooter H/W ------------------
/* 
#define ADCBUF_INV_A_IPHASE1    ADCBUF0
#define ADCBUF_INV_A_IPHASE2    ADCBUF1                   
#define ADCBUF_SPEED_REF_A      ADCBUF17    
#define ADCBUF_VOLTAGE          ADCBUF15
//#define ADCBUF_TEMPERATURE_MCU     ADCBUF19
#define ADCBUF_TEMPERATURE_MOSFET   ADCBUF12
 */
/* This defines number of current offset samples for averaging 
 * If the 2^n samples are considered specify n(in this case 2^7(= 128)=> 7*/
#define  CURRENT_OFFSET_SAMPLE_SCALER         7
        
#define EnableADCInterrupt()   _ADCAN17IE = 1
#define DisableADCInterrupt()  _ADCAN17IE = 0
#define ClearADCIF()           _ADCAN17IF = 0
#define ClearADCIF_ReadADCBUF() ADCBUF17
        
// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
void InitializeADCs(void);

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif
#endif      // end of ADC_H

