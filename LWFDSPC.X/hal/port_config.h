/*******************************************************************************
  Hardware specific routine definition and interfaces Header File.

  File Name:
    port_config.h

  Summary:
    This file includes subroutine for initializing GPIO pins as analog/digital,
    input or output etc. Also to PPS functionality to Remap-able input or output 
    pins

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
#ifndef _PORTCONFIG_H
#define _PORTCONFIG_H

#include <xc.h>

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

// 根據 PIN_TABLE_FOR_EDIT.md 的腳位定義
// 主要控制腳位定義
#define BUTTON_START_STOP        PORTCbits.RC12  // Pin 3: RC12 (IBKS) - 煞車訊號
#define BUTTON_FORWARD_REVERSE   PORTDbits.RD14  // Pin 11: RD14 (IFR) - 前進/後退開關
#define BUTTON_CRUISE_WALK       PORTDbits.RD15  // Pin 8: RD15 (CRUISE) - 啟動/停止開關

// 霍爾感測器腳位定義
#define HALL_SENSOR_U            PORTCbits.RC4   // Pin 50: RC4 (IHU) - 馬達霍爾感測U
#define HALL_SENSOR_V            PORTCbits.RC5   // Pin 51: RC5 (IHV) - 馬達霍爾感測V
#define HALL_SENSOR_W            PORTCbits.RC10  // Pin 52: RC10 (IHW) - 馬達霍爾感測W

// 速度感測器腳位定義
#define SPEED_SENSOR_A           PORTCbits.RC14  // Pin 5: RC14 (ISNA) - 方向開關A
#define SPEED_SENSOR_B           PORTCbits.RC15  // Pin 6: RC15 (ISNB) - 方向開關B
#define EXT_SPEED_SENSOR         PORTDbits.RD4   // Pin 54: RD4 (ILSN) - 外部速度感測

// 類比感測器腳位定義
#define THROTTLE_VR_INPUT        PORTCbits.RC6   // Pin 24: RC6 (IHS) - 油門/VR輸入
#define BATTERY_VOLTAGE          PORTCbits.RC3   // Pin 27: RC3 (VBATT) - 電池電壓偵測
#define CONTROLLER_TEMP          PORTCbits.RC0   // Pin 13: RC0 (TPV) - 控制器溫度偵測
#define MOTOR_TEMP               PORTCbits.RC7   // Pin 32: RC7 (NTC) - 馬達溫度偵測
#define TORQUE_SENSOR           PORTDbits.RD11  // Pin 30: RD11 (ITQS) - 扭力感測
#define SPEED_PROTECT           PORTAbits.RA3   // Pin 17: RA3 (ISPD) - 馬達速度偵測/保護
#define EM_BRAKE_SWITCH         PORTDbits.RD8  // Pin 39: RD8 (IEMB) - 電磁煞車開關

// LED輸出腳位定義
#define LED_GREEN               LATDbits.LATD7  // Pin 42: RD7 (OLPG) - 綠色LED
#define LED_YELLOW              LATDbits.LATD6  // Pin 43: RD6 (OLPY) - 黃色LED
#define LED_RED                 LATDbits.LATD5  // Pin 44: RD5 (OLPR) - 紅色LED

// 控制輸出腳位定義
#define HEAD_LIGHT              LATDbits.LATD3  // Pin 55: RD3 (OLAMP) - 大燈控制
#define BRAKE_LIGHT             LATDbits.LATD2  // Pin 58: RD2 (OBKL) - 煞車燈控制
#define EM_BRAKE_CTRL           LATDbits.LATD1  // Pin 59: RD1 (OEMB) - 電磁煞車控制
#define CAN_STANDBY             LATBbits.LATB7  // Pin 47: RB7 (STB) - CAN待機控制
#define RS485_DIRECTION         LATDbits.LATD9  // Pin 38: RD9 (485RE) - RS485方向控制

// PWM輸出腳位定義
#define PWM1H                   LATBbits.LATB14 // Pin 1: RB14 (PWM1H) - 馬達PWM1上橋臂
#define PWM1L                   LATBbits.LATB15 // Pin 2: RB15 (PWM1L) - 馬達PWM1下橋臂
#define PWM2H                   LATBbits.LATB12 // Pin 63: RB12 (PWM2H) - 馬達PWM2上橋臂
#define PWM2L                   LATBbits.LATB13 // Pin 64: RB13 (PWM2L) - 馬達PWM2下橋臂
#define PWM3H                   LATBbits.LATB10 // Pin 61: RB10 (PWM3H) - 馬達PWM3上橋臂
#define PWM3L                   LATBbits.LATB11 // Pin 62: RB11 (PWM3L) - 馬達PWM3下橋臂

// 通訊腳位定義
#define UART_TX                 PORTCbits.RC8   // Pin 36: RC8 (TX) - UART傳送
#define UART_RX                 PORTCbits.RC9   // Pin 37: RC9 (RX) - UART接收
#define CAN_TX                  PORTBbits.RB8   // Pin 48: RB8 (CAN_TX) - CAN傳送
#define CAN_RX                  PORTBbits.RB9   // Pin 49: RB9 (CAN_RX) - CAN接收

// 程式/除錯腳位定義
#define PGED                    PORTBbits.RB5   // Pin 45: RB5 (PGED) - 程式/除錯資料
#define PGEC                    PORTBbits.RB6   // Pin 46: RB6 (PGEC) - 程式/除錯時脈

// 未使用的腳位定義（需要確認用途）
#define UNUSED_RC13            PORTCbits.RC13  // Pin 4: RC13 - 未標示
#define UNUSED_RD13            PORTDbits.RD13  // Pin 12: RD13 - 未標示
#define UNUSED_RD12            PORTDbits.RD12  // Pin 21: RD12 - 未標示
#define UNUSED_RC11            PORTCbits.RC11  // Pin 53: RC11 - 未標示
#define UNUSED_RD0             PORTDbits.RD0   // Pin 60: RD0 - 未標示

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
void SetupGPIOPorts(void);

// *****************************************************************************
// *****************************************************************************
#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif
#endif      // end of PORTCONFIG_H


