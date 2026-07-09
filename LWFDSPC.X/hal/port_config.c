/*******************************************************************************
  Input / Output Port Configuration Routine source File

  File Name:
    port_config.c

  Summary:
    This file includes subroutine for initializing GPIO pins as analog/digital,
    input or output etc. Also to PPS functionality to Remap-able input or output
    pins

  Description:
    Definitions in the file are for dsPIC33CK64MP105 plugged onto
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
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <xc.h>
// #include <p33CK32MP105.h>
#include <p33CK256MP506.h>
#include "port_config.h"
#include "../src/userparms.h"
// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
void MapGPIOHWFunction(void);
// *****************************************************************************
/* Function:
    SetupGPIOPorts()

  Summary:
    Routine to set-up GPIO ports

  Description:
    Function initializes GPIO pins for input or output ports,analog/digital pins,
    remap the peripheral functions to desires RPx pins.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void SetupGPIOPorts(void) {
// Reset all PORTx register (all inputs)
#ifdef TRISA
    TRISA = 0xFFFF;
    LATA = 0x0000;
#endif
#ifdef ANSELA
    ANSELA = 0x0000;
#endif

#ifdef TRISB
    TRISB = 0xFFFF;
    LATB = 0x0000;
#endif
#ifdef ANSELB
    ANSELB = 0x0000;
#endif

#ifdef TRISC
    TRISC = 0xFFFF;
    LATC = 0x0000;
#endif
#ifdef ANSELC
    ANSELC = 0x0000;
#endif

#ifdef TRISD
    TRISD = 0xFFFF;
    LATD = 0x0000;
#endif
#ifdef ANSELD
    ANSELD = 0x0000;
#endif

#ifdef TRISE
    TRISE = 0xFFFF;
    LATE = 0x0000;
#endif
#ifdef ANSELE
    ANSELE = 0x0000;
#endif

    MapGPIOHWFunction();

    return;
}
// *****************************************************************************
/* Function:
    Map_GPIO_HW_Function()

  Summary:
    Routine to setup GPIO pin used as input/output analog/digital etc

  Description:
    Function initializes GPIO pins as input or output port pins,analog/digital
    pins,remap the peripheral functions to desires RPx pins.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void MapGPIOHWFunction(void) {
    /* ANALOG SIGNALS */

    // Configure Port pins for Motor Current Sensing
    // IB相電流感測 - Pin 33, 34, 35
    ANSELBbits.ANSELB2 = 1;  // IB Out (Pin 33: RB2)
    TRISBbits.TRISB2 = 1;

    ANSELBbits.ANSELB3 = 1;  // IB- (Pin 34: RB3)
    TRISBbits.TRISB3 = 1;

    ANSELBbits.ANSELB4 = 1;  // IB+ (Pin 35: RB4)
    TRISBbits.TRISB4 = 1;

    // IA相電流感測 - Pin 14, 15, 16
    ANSELAbits.ANSELA0 = 1;  // IA Out (Pin 14: RA0)
    TRISAbits.TRISA0 = 1;

    ANSELAbits.ANSELA1 = 1;  // IA- (Pin 15: RA1)
    TRISAbits.TRISA1 = 1;

    ANSELAbits.ANSELA2 = 1;  // IA+ (Pin 16: RA2)
    TRISAbits.TRISA2 = 1;

    // 運算放大器配置
    AMPCON1Hbits.NCHDIS3 = 0;  // 運算放大器3寬輸入範圍
    AMPCON1Lbits.AMPEN3 = 1;   // 啟用運算放大器3

    AMPCON1Hbits.NCHDIS2 = 0;  // 運算放大器2寬輸入範圍
    AMPCON1Lbits.AMPEN2 = 1;   // 啟用運算放大器2

    AMPCON1Hbits.NCHDIS1 = 0;  // 運算放大器1寬輸入範圍
    AMPCON1Lbits.AMPEN1 = 1;   // 啟用運算放大器1

    AMPCON1Lbits.AMPON = 1;  // 啟用運算放大器模組

    // 油門/VR輸入 - Pin 24: RC6 (IHS)
    ANSELCbits.ANSELC6 = 1;
    TRISCbits.TRISC6 = 1;

    // 電池電壓偵測 - Pin 27: RC3 (VBATT)
    ANSELCbits.ANSELC3 = 1;
    TRISCbits.TRISC3 = 1;

    // 控制器溫度偵測 - Pin 13: RC0 (TPV)
    TRISCbits.TRISC0 = 1;
    ANSELCbits.ANSELC0 = 1;

    // 馬達溫度偵測 - Pin 32: RC7 (NTC)
    TRISCbits.TRISC7 = 1;
    ANSELCbits.ANSELC7 = 1;

    // 扭力感測 - Pin 30: RD11 (ITQS)
    TRISDbits.TRISD11 = 1;
    ANSELDbits.ANSELD11 = 1;

    // 馬達速度偵測/保護 - Pin 17: RA3 (ISPD)
    TRISAbits.TRISA3 = 1;
    ANSELAbits.ANSELA3 = 1;

    // 反電動勢感測
    // A相反電動勢 - Pin 28: RB0 (BEMF_A)
    TRISBbits.TRISB0 = 1;
    ANSELBbits.ANSELB0 = 1;

    // B相反電動勢 - Pin 29: RB1 (BEMF_B) (修正: 原定義錯誤為RD1)
    TRISBbits.TRISB1 = 1;
    ANSELBbits.ANSELB1 = 1;

    // C相反電動勢 - Pin 31: RD10 (BEMF_C)
    TRISDbits.TRISD10 = 1;
    ANSELDbits.ANSELD10 = 1;

    // 運算放大器3相關腳位
    // 運算放大器3輸出 - Pin 18: RA4 (OA3_OUT)
    TRISAbits.TRISA4 = 1;
    ANSELAbits.ANSELA4 = 1;

    // 運算放大器3負端 - Pin 22: RC1 (OA3_IN-)
    TRISCbits.TRISC1 = 1;
    ANSELCbits.ANSELC1 = 1;

    // 運算放大器3正端 - Pin 23: RC2 (OA3_IN+)
    TRISCbits.TRISC2 = 1;
    ANSELCbits.ANSELC2 = 1;

    /* Digital SIGNALS */
    // DIGITAL INPUT/OUTPUT PINS

    // 馬達驅動PWM輸出
    // PWM1H - Pin 1: RB14
    // PWM1L - Pin 2: RB15
    // PWM2H - Pin 63: RB12
    // PWM2L - Pin 64: RB13
    // PWM3H - Pin 61: RB10
    // PWM3L - Pin 62: RB11
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB15 = 0;
    TRISBbits.TRISB12 = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB10 = 0;
    TRISBbits.TRISB11 = 0;

    // 霍爾感測器輸入信號 - Pin 50, 51, 52
    // HALLU : RC4 (Pin 50: IHU)
    // HALLV : RC5 (Pin 51: IHV)
    // HALLW : RC10 (Pin 52: IHW)
    I_HALL_U_TRIS = 1;
    I_HALL_V_TRIS = 1;
    I_HALL_W_TRIS = 1;

    // 煞車訊號 - Pin 3: RC12 (IBKS)
    TRISCbits.TRISC12 = 1;
    CNPDCbits.CNPDC12 = 1;  // 啟用上拉電阻

    // 前進/後退開關 - Pin 11: RD14 (IFR)
    TRISDbits.TRISD14 = 1;
    CNPDDbits.CNPDD14 = 1;  // 啟下拉電阻

    // 啟動/停止開關 - Pin 8: RD15 (CRUISE)
    TRISDbits.TRISD15 = 1;
    CNPDDbits.CNPDD15 = 1;  // 啟用上拉電阻

    // 方向開關A - Pin 5: RC14 (ISNA)
    TRISCbits.TRISC14 = 1;
    CNPDCbits.CNPDC14 = 1;  // 啟用上拉電阻

    // 方向開關B - Pin 6: RC15 (ISNB)
    TRISCbits.TRISC15 = 1;
    CNPDCbits.CNPDC15 = 1;  // 啟用上拉電阻

    // 外部速度感測 - Pin 54: RD4 (ILSN)
    TRISDbits.TRISD4 = 1;
    CNPDDbits.CNPDD4 = 1;  // 啟用下拉電阻

    // 電磁煞車開關 - Pin 39: RD8 (IEMB)
    // ANSELDbits.ANSD8 = 0;
    TRISDbits.TRISD8 = 1;  // 設為輸入
    CNPUDbits.CNPUD8 = 0;  // 啟用RD8的內部上拉電阻
    CNPDDbits.CNPDD8 = 0;  // 禁用 RD8 的下拉電阻

    // LED輸出控制
    // 綠色LED - Pin 42: RD7 (OLPG)
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 0;  // 初始狀態為低電位

    // 黃色LED - Pin 43: RD6 (OLPY)
    TRISDbits.TRISD6 = 0;
    LATDbits.LATD6 = 0;  // 初始狀態為低電位

    // 紅色LED - Pin 44: RD5 (OLPR)
    TRISDbits.TRISD5 = 0;
    LATDbits.LATD5 = 0;  // 初始狀態為低電位

    // 大燈控制 - Pin 55: RD3 (OLAMP)
    TRISDbits.TRISD3 = 0;
    LATDbits.LATD3 = 0;  // 初始狀態為低電位

    // 煞車燈控制 - Pin 58: RD2 (OBKL)
    TRISDbits.TRISD2 = 0;
    LATDbits.LATD2 = 0;  // 初始狀態為低電位

    // 電磁煞車控制 - Pin 59: RD1 (OEMB)
    TRISDbits.TRISD1 = 0;
    LATDbits.LATD1 = 0;  // 初始狀態為低電位

    // CAN待機控制 - Pin 47: RB7 (STB)
    TRISBbits.TRISB7 = 0;
    LATBbits.LATB7 = 0;  // 初始狀態為低電位

    // RS485方向控制 - Pin 38: RD9 (485RE)
    TRISDbits.TRISD9 = 0;
    LATDbits.LATD9 = 0;  // 初始狀態為低電位

    // 未使用的腳位設為輸入
    // Pin 4: RC13 - 預設輸入；當 CODESW_DEBUG_ISR_PROFILE_ENABLE=1 時，main.c 會改設為
    //               ADC ISR 執行時間量測輸出 (O_DBG_ADC_ISR)。
    TRISCbits.TRISC13 = 1;

    // Pin 12: RD13 - 預設輸入；當 CODESW_DEBUG_ISR_PROFILE_ENABLE=1 時，main.c 會改設為
    //                速度命令處理時間量測輸出 (O_DBG_SPEED_PROFILE)。
    TRISDbits.TRISD13 = 1;

    // Pin 21: RD12 - 未標示
    TRISDbits.TRISD12 = 1;

    // Pin 53: RC11 - 未標示
    TRISCbits.TRISC11 = 1;

    // Pin 60: RD0 - 未標示
    TRISDbits.TRISD0 = 1;

    // 程式/除錯腳位
    // Pin 45: RB5 (PGED) - 程式/除錯資料
    TRISBbits.TRISB5 = 1;

    // Pin 46: RB6 (PGEC) - 程式/除錯時脈
    TRISBbits.TRISB6 = 1;

    // UART通訊腳位配置 - 修正腳位映射
    // Pin 36: RC8 (TX) - UART傳送
    // Pin 37: RC9 (RX) - UART接收
    _U1RXR = 57;           // RC9 -> UART1 RX
    _RP56R = 0b000001;     // RC8 -> UART1 TX
    CNPUCbits.CNPUC9 = 1;  // RC9上拉電阻

    // CAN通訊腳位配置
    // Pin 48: RB8 (CAN_TX) - CAN傳送
    // Pin 49: RB9 (CAN_RX) - CAN接收
    _TRISB8 = 1;                   // RB8設為輸入
    _TRISB9 = 1;                   // RB9設為輸入
    RPOR4bits.RP40R = 0x0015;      // RB8 -> CAN FD1 MODULE:CAN1TX
    RPINR26bits.CAN1RXR = 0x0029;  // RB9 -> CAN FD1 MODULE:CAN1RX
}
