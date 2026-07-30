// <editor-fold defaultstate="collapsed" desc="Description/Instruction ">
/**
 * uart2.h
 *
 * This header file lists interface functions to configure and enable the UART2
 * module. It mirrors hal/uart1.h (RS485/Modbus on UART1) but targets UART2,
 * which is dedicated to the X2CScope debug link on RB8 (U2TX) / RB9 (U2RX).
 *
 * Definitions in this file are for dsPIC33CK256MP506.
 *
 * Unlike uart1.h/uart1.c, UART2 is implemented header-only: every function,
 * including UART2_Initialize(), is `inline static`. This keeps the X2CScope
 * UART fully self-contained (only diagnostics_x2cscope.c includes it) and
 * avoids registering a new .c file in the MPLAB X project / Makefiles.
 *
 * Component: HAL - UART2
 */
// </editor-fold>

#ifndef __UART2_H
#define __UART2_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enables and initializes UART2 with the same default configuration used for
 * UART1: asynchronous 8-bit, no parity, one Stop bit, 16x baud clock (Standard
 * mode). TX and RX are enabled; the module itself is left disabled (UARTEN = 0)
 * so the caller can set the baud rate before calling UART2_ModuleEnable().
 */
inline static void UART2_Initialize(void)
{
    /* UARTx Configuration Register */
    U2MODE = 0;
    U2MODEbits.UTXEN = 1;   /* Transmit enabled  */
    U2MODEbits.URXEN = 1;   /* Receive enabled   */
    U2MODEbits.MOD   = 0;   /* Asynchronous 8-bit UART, no parity */
    U2MODEbits.BRGH  = 0;   /* Standard speed: 16 clocks per bit  */

    /* UARTx Configuration Register High */
    U2MODEH = 0;

    /* UARTx Status Register (clear all interrupt enables / flags) */
    U2STA = 0;

    /* UARTx Status High Register */
    U2STAH = 0;
    U2STAHbits.UTXISEL = 7;  /* TX interrupt when 1 empty slot left */
    U2STAHbits.URXISEL = 0;  /* RX interrupt when 1 word or more    */
    U2STAHbits.UTXBE   = 1;
    U2STAHbits.URXBE   = 1;

    /* Baud rate registers (set separately via UART2_BaudRateDividerSet) */
    U2BRG  = 0;
    U2BRGH = 0;

    /* Data / timing / checksum / smart-card / interrupt registers */
    U2RXREG = 0;
    U2TXREG = 0;
    U2P1    = 0;
    U2P2    = 0;
    U2P3    = 0;
    U2P3H   = 0;
    U2TXCHK = 0;
    U2RXCHK = 0;
    U2SCCON = 0;
    U2SCINT = 0;
    U2INT   = 0;

    U2MODEHbits.ACTIVE = 0;
    U2MODEbits.UARTEN  = 0;  /* keep disabled until baud rate is configured */
}

inline static void UART2_InterruptTransmitFlagClear(void) { _U2TXIF = 0; }
inline static void UART2_InterruptTransmitFlagSet(void)   { _U2TXIF = 1; }
inline static void UART2_InterruptReceiveFlagClear(void)  { _U2RXIF = 0; }
inline static void UART2_InterruptTransmitEnable(void)    { _U2TXIE = 1; }
inline static void UART2_InterruptTransmitDisable(void)   { _U2TXIE = 0; }
inline static void UART2_InterruptReceiveEnable(void)     { _U2RXIE = 1; }
inline static void UART2_InterruptReceiveDisable(void)    { _U2RXIE = 0; }

/**
 * Sets the UART2 receive interrupt priority level (0..7).
 *
 * Priority convention on this board: ADC (_ADCAN17IP) and PWM are IPL 7, Timer1 is
 * IPL 2, and UART1 RX (Modbus/RS485) is left at the reset default of IPL 4. The
 * X2CScope link is debug traffic, so it must sit below both motor control and
 * Modbus - use IPL 3.
 */
inline static void UART2_InterruptReceivePrioritySet(uint16_t priority)
{
    _U2RXIP = priority;
}

inline static void UART2_InterruptTransmitPrioritySet(uint16_t priority)
{
    _U2TXIP = priority;
}

/** Standard baud rate mode: Baud = FREQ_UART_CLK / (16 * (BRG + 1)) */
inline static void UART2_SpeedModeStandard(void)  { U2MODEbits.BRGH = 0; }
/** High-speed baud rate mode: Baud = FREQ_UART_CLK / (4 * (BRG + 1)) */
inline static void UART2_SpeedModeHighSpeed(void) { U2MODEbits.BRGH = 1; }

inline static void UART2_BaudRateDividerSet(uint16_t baudRateDivider)
{
    U2BRG = baudRateDivider;
}

inline static void UART2_ModuleDisable(void)     { U2MODEbits.UARTEN = 0; }
inline static void UART2_ModuleEnable(void)      { U2MODEbits.UARTEN = 1; }
inline static void UART2_TransmitModeEnable(void){ U2MODEbits.UTXEN  = 1; }
inline static void UART2_TransmitModeDisable(void){ U2MODEbits.UTXEN = 0; }

inline static bool UART2_IsReceiveBufferDataReady(void)
{
    return (!U2STAHbits.URXBE);
}

inline static bool UART2_IsReceiveBufferOverFlowDetected(void)
{
    return (U2STAbits.OERR);
}

inline static bool UART2_IsFrameErrorDetected(void)
{
    return (U2STAbits.FERR);
}

inline static bool UART2_IsParityErrorDetected(void)
{
    return (U2STAbits.PERR);
}

inline static bool UART2_IsReceiverIdle(void)
{
    return (U2STAHbits.RIDLE);
}

inline static bool UART2_IsTransmissionComplete(void)
{
    return (U2STAbits.TRMT);
}

inline static bool UART2_StatusBufferFullTransmitGet(void)
{
    return U2STAHbits.UTXBF;
}

inline static uint16_t UART2_StatusGet(void)
{
    return U2STA;
}

inline static void UART2_ReceiveBufferOverrunErrorFlagClear(void)
{
    U2STAbits.OERR = 0;
}

inline static void UART2_DataWrite(uint16_t data)
{
    U2TXREGbits.TXREG = (uint8_t)data;
}

inline static uint16_t UART2_DataRead(void)
{
    return U2RXREG;
}

#ifdef __cplusplus
}
#endif

#endif  // end of __UART2_H
