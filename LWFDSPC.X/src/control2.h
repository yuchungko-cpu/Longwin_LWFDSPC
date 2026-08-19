/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  

// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation
typedef struct
{
    /* Run motor indication */
    unsigned char RunMotor;
    /* Open loop/closed loop indication */
    unsigned char OpenLoop;
    /* Mode changed indication - from open to closed loop */
    unsigned char ChangeMode;
    /* Speed doubled indication */
    unsigned char ChangeSpeed;
    unsigned char Direction;
    unsigned char DirectionDefault;
    unsigned char DirSW;
    unsigned char DirSWOld;
    unsigned char RunSW;
    unsigned char RunSWOld;
    unsigned char RunSW_Throttle;         // enabled motor drive by throttle
    unsigned char RunSWOld_Throttle;    
    unsigned char CtrlMode;
    unsigned char ReGenEnable;
    unsigned char ReGenSet;         // used to control ReGenEnable
    unsigned char ReGenBlock;       // used to control ReGenEnable
    unsigned char ReGenFlag;        // 1: ReGen is working
    unsigned char UVWLock;        // UVW shorted for speed mode.
                                  // [V0.24] 1 = 三相下橋全開。放油門且近停時進鎖，EMB 夾住的
                                  // 全程保持進鎖(見 main.c bUvwEmbClamped)；出鎖只由「駕駛給出
                                  // 明顯油門且 EMB 已放開」觸發。
    unsigned char Fault;
    unsigned char BrakeSWOn;        // 1: brake SW enabled
    unsigned char CAN_Runmotor;     // 1: enable the motor from CAN
    unsigned char ReGenMode;
    unsigned char DriveMode;
    unsigned char Coast;          // [V0.24 已退役] 原本 1 = 關閉全部 PWM 輸出(六個開關全關,
                                  // 馬達浮接)。speed mode 已不使用 coast:「馬達不要出力」改由
                                  // UVW 三相短路達成(結構上不可能驅動馬達,又有制動力,且橋的
                                  // 狀態明確)。PWM 全關只剩 RunMotor==0 / Fault / 電池禁制
                                  // 三個系統理由,見 main.c PWM 輸出分支的說明。
                                  // 仍有寫入者(bMotorStop、堵轉歸零、VR 失效路徑)但**無讀取者**,
                                  // 保留欄位是為了不動那些路徑的既有語意與 #if 0 舊碼。
}UGF_T;

typedef struct
{
    unsigned char MotorStall;
    unsigned char Undervoltage;
    unsigned char Overvoltage;
    unsigned char MOSOverHeat;
    unsigned char MCUOverHeat;
}FaultFlags_T;

typedef struct
{
    signed int RatedIq;
    signed int OverCurrent;
    signed long Sum;
    signed long Limit;
}IqSqure_T;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

