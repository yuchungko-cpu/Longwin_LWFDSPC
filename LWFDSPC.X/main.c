#ifdef __XC16__  // See comments at the top of this header file
#include <xc.h>
#endif  // __XC16__
#include <libq.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <p33CK32MP105.h>
#include <p33CK256MP506.h>

#include "src/longwin/codeHw.h"
#include "src/longwin/codeSw.h"

#include "src/motor_control_noinline.h"

#include "src/general.h"
#include "src/userparms.h"

#include "src/control.h"
#include "src/control2.h"

#include "src/meascurr.h"
#include "src/readadc.h"
#include "src/sccp3_tmr.h"

#include "hal/board_service.h"

#if CODESW_SCOPE_ENABLE == 1
#include "diagnostics/diagnostics.h"
#endif

#include "src/HallScan_EE.h"

#include "mcc_generated_files/system.h"
// #include "mcc_generated_files/X2Cscope/X2Cscope.h"
// #include "mcc_generated_files/sccp1_tmr.h"
// #include "mcc_generated_files/pin_manager.h"
#include "hal/uart1.h"
#include "mcc_generated_files/can1.h"
#include "mcc_generated_files/can_types.h"

#if CODESW_THROTTLE_ENABLE
#include "src/longwin/s_logic_throttle.h"
#endif
#if CODESW_VR_ENABLE
#include "src/longwin/s_logic_vr.h"
#endif
#if CODESW_BATTERY_ENABLE
#include "src/longwin/s_logic_battery.h"
#endif
#if CODESW_TEMPERATURE_CONTROLLER_ENABLE
#include "src/longwin/s_logic_temp_controller.h"
#endif
#include "src/longwin/s_logic_convert.h"
#include "src/longwin/s_logic_motor.h"
#if CODESW_EMBRAKER_ENABLE
#include "src/longwin/s_logic_embraker.h"
#endif
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
#include "src/longwin/s_hal_rs485.h"
#include "src/longwin/s_modbus_decode.h"
#include "src/longwin/s_modbus_master.h"

// S_MODBUS_APP_DATA_RAW g_stAppData;
// S_MODBUS_PC_GUI_DATA_RAW g_stPcGuiData;
// S_MODBUS_BATTERY_DATA_RAW g_stBatteryData;
// S_MODBUS_ALL_DATA g_stAllData;  // 改用 modbusService_getDataPtr() 存取
#endif
#include "src/longwin/s_logic_error_handler.h"

#if CODESW_EMBRAKER_TEST
bool bEM_BRAKE_SWITCH;
#endif
#if CODESW_TEMPERATURE_CONTROLLER_TEST
uint16_t u16ControllerTempRaw;
// For IDE testing: An array of ADC values to simulate temperature changes.
const uint16_t temp_test_adc_values[] = {3078, 2172, 1651, 1743, 1804, 1400, 1513};
const int num_temp_test_values = sizeof(temp_test_adc_values) / sizeof(temp_test_adc_values[0]);
#endif

// --- 程式開關定義 ---

// *****************************************************************************
// *****************************************************************************
// Section: System Data Structures
// *****************************************************************************
// *****************************************************************************

/**
 * @brief 系統資料結構
 * 存放系統相關的各種資料，包括感測器讀值、狀態資訊等
 */
#define TIMER_COUNT_1MS \
    20  // 這裡是20kHz 的計數器，計數到1ms的上限計數值 (20kHz/20=1kHz=1ms)

typedef struct {
    uint16_t u16TimeUs;
    uint32_t u32TimeMs;
    int16_t i16CurrentRpm;
    int16_t i16TargetRpm;
    int16_t i16ActiveRpm;

    // ADC 原始值
    uint16_t u16BatteryVoltageRaw;  // 電池電壓 ADC 原始值 (Pin 27: RC3)
    uint16_t u16ControllerTempRaw;  // 控制器溫度 ADC 原始值 (Pin 13: RC0)
    uint16_t u16MotorTempRaw;       // 馬達溫度 ADC 原始值 (Pin 32: RC7)
    uint16_t u16TorqueSensorRaw;    // 扭力感測器 ADC 原始值 (Pin 30: RD11)
    int16_t i16SpeedProtectRaw;     // 速度保護感測器 ADC 原始值 (Pin 17: RA3)
    int16_t i16SpeedFiltered;       // 速度保護感測器 Q15 格式的過濾速度值
    uint16_t u16ThrottleVRRaw;      // 油門/VR輸入 ADC 原始值 (Pin 24: RC6)
    uint16_t u16IEMBRaw;            // 電磁煞車 ADC 原始值 (Pin 39: RD8)

    // --- Converted mV Values ---
    uint16_t u16ThrottleVRMv;  // 油門/VR 電壓值 (mV)
    uint16_t u16IEMBMv;        // 電磁煞車 電壓值 (mV)

    // 轉換後的數值 (可根據需要擴展)
    uint16_t u16BatteryVoltage;     // 電池電壓 (V x 100) 例如:2400代表24.00V
    uint16_t u16BatteryPercent;     // 電池百分比
    uint16_t u16ControllerTemp;     // 控制器溫度 (°C x 10) 例如:234代表23.4°C
    uint16_t u16MotorTemp;          // 馬達溫度 (°C x 10)
    uint16_t u16TorqueSensor;       // 扭力感測器數值 (Q15格式)
    uint16_t u16SpeedProtect;       // 速度保護數值 (Q15格式)
    uint16_t u16ThrottleVR;         // 油門/VR數值 (Q15格式)
    uint16_t u16MotorRpm;           // 速度保護轉速 (RPM)
    uint16_t u16ThrottleLevelMax;   // 依據 Assist Level 得到最高的限速
    S_MOTOR_STEP_TIME_T sStepTime;  // 油門/VR步進時間
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
    S_BATTERY_DATA sBatteryData;
    S_SHARED_DEVICE_DATA sSharedData;
#endif

    // 狀態標誌 (可根據需要擴展)
    bool bBatteryVoltageValid;  // 電池電壓有效標誌
    bool bControllerTempValid;  // 控制器溫度有效標誌
    bool bMotorTempValid;       // 馬達溫度有效標誌
    bool bTorqueSensorValid;    // 扭力感測器有效標誌
    bool bSpeedProtectValid;    // 速度保護有效標誌
    bool bMotorDirection;
#if CODESW_THROTTLE_ENABLE == 1
    bool bThrottleVrRelease;
    bool bThrottleVRValid;  // 油門/VR有效標誌
    // bool bThrottleIsSpeedTooHighForChange;
    // bool bThrottleIsDirectionChageRequested;
#endif
    bool bMotorStop;
#if CODESW_TEMPERATURE_CONTROLLER_ENABLE
    bool bControllerIsOverTemp;
    bool bControllerIsOverLoad;
#endif
#if CODESW_BATTERY_ENABLE
    bool bBatteryShouldProhibit;
#endif
} S_SYSTEM_DATA_T;

// 全域系統資料結構
S_SYSTEM_DATA_T g_stSystemData = {0};

#define millis() g_stSystemData.u32TimeMs

#if CODESW_SCOPE_ENABLE == 1
uint16_t u16CurrentSpeedKmh_x10;
#endif
// 用於 RS485 的時間函式
uint32_t getSystemTimeMs(void) {
    return g_stSystemData.u32TimeMs;
}
// --------------------------------------------------------------
UGF_T uGF;
IqSqure_T IqSquare;
volatile CTRL_PARM_T ctrlParm;
MOTOR_STARTUP_DATA_T motorStartUpData;

MCAPP_DATA_T mcappData;

MEAS_CURR_PARM_T measCurrParm;
READ_ADC_PARM_T readADCParm;

volatile int16_t thetaElectrical = 0;
uint16_t pwmPeriod;

MC_ALPHABETA_T valphabeta;
MC_ALPHABETA_T ialphabeta;
MC_SINCOS_T sincosTheta;
MC_DQ_T vdq, idq;
MC_DUTYCYCLEOUT_T pwmDutycycle;
MC_ABC_T vabc, iabc;

MC_PIPARMIN_T piInputIq;
MC_PIPARMOUT_T piOutputIq;
MC_PIPARMIN_T piInputId;
MC_PIPARMOUT_T piOutputId;
MC_PIPARMIN_T piInputOmega;
MC_PIPARMOUT_T piOutputOmega;

volatile FAULT_DATA_T faultUndervoltage;
volatile FAULT_DATA_T faultOvervoltage;
volatile FAULT_DATA_T faultOverTempMCU;
volatile FAULT_DATA_T faultOverTempMOSFET;
volatile FAULT_DATA_T faultOverCurrent;
volatile FAULT_DATA_T faultMotorStall;
volatile uint16_t adcDataBuffer;
volatile uint16_t measCurrOffsetFlag = 0;

volatile FaultFlags_T FaultFlags;

volatile signed int HallAngle;

// bool FindHallStart;
// bool throttleDisengaged = 0;
// bool motorStarted = 0;
// bool fromRegenBrake = 0;
// bool throttleEnabled = 0;
// bool testBool = 0;
// uint32_t brakeCounter = 0;
int16_t HallOffset = 0;  // 3000:~16.5 degrees

// uint16_t regenCounter = 0;
// uint16_t openLoopCounter = 0;
uint16_t HallState = 0;
uint16_t OldHallState = 0;
uint16_t TMRLatch = 30000;
signed int HallPulses = 0;
signed int HallPulsesLatch = 0;
unsigned int HallPulsesCntr = 0;
const int16_t PhaseValues[8] = {0, 0, -21844, -10922, 21844, 10922, 32767, 0};
signed int TempVar;     // main loop
signed int TempVar16;   // interrupt
signed int Temp2Var16;  // interrupt
signed long TempVar32;  // interrupt
signed int Ibus = 0;
signed int IbusAVG = 0;
signed int V_PhaseA = 0;
signed int V_PhaseB = 0;
signed int V_PhaseC = 0;
int32_t FilteredSpeed;  // Used for display
// int32_t FilteredHallPeriod;

extern unsigned int CANCntr;
unsigned int CANRXCntr = 0;
CAN1_TX_FIFO_CHANNELS My_CAN_FIFO_CHANNELS;
CAN_MSG_OBJ My_CAN_TXMSG;
CAN_MSG_OBJ My_CAN_RXMSG;
CAN_MSG_FIELD My_CAN_MSG_FIELD;
uint8_t CANTXBuffer[5] = {0, 0, 0, 0, 0};
extern uint8_t *CANRXPtr;
extern CAN_TX_MSG_REQUEST_STATUS
CAN1_Transmit(const CAN1_TX_FIFO_CHANNELS fifoChannel, CAN_MSG_OBJ *txCanMsg);

volatile uint16_t HallPeriod;
volatile uint16_t HallPeriodFiltered;
// volatile uint32_t AvgPeriod;
volatile uint16_t hallValue;
volatile int16_t Speed = 0;
volatile uint16_t startUpCounter = 0;

extern unsigned int CCP3TMR_INTCntr;
unsigned int DeltaT_Index = 0;
unsigned long DeltaT_Array[] = {32767, 32767, 32767, 32767,
                                32767, 32767, 32767, 32767};
unsigned long PeriodAverage = 0;
unsigned long DeltaT_Sum = 0;
unsigned int HallMinPeriod;
extern tFindHallAngle FindHallAngle;
unsigned int T1INTCnt = 0;
// unsigned char DirSWCntr = 0;
// unsigned char RunSWCntr = 0;
unsigned int PollingCntr = 0;
signed int ReferenceRAW = 0;
signed int ReferenceRAWSet = 0;
signed int ReferenceRAWSetStep = 10;
signed int ReferenceRAWADC = 0;
unsigned char SpeedLoopCntr = 0;
unsigned int SpeedSlopCntr = 0;
signed int ReGenTorq = Q15(0.1);   // Initial ReGen brake duty cycle
signed int ReGenSpeed = Q15(0.0);  // Low limit of speed for UVW lock
#if CODESW_SCOPE_ENABLE == 1
unsigned int X2CPG1Duty = 0;
unsigned int X2CPG2Duty = 0;
unsigned int X2CPG3Duty = 0;
#endif
signed int UVWLockSpeed = Q15(0.06);  // main.c    3000*(0.05)=150RPM   以下轉速啟動Lock
signed int SpeedCtrlLimit;
signed int SpeedModeCtrlLimit;
unsigned int AccSet = ACC_SET;
unsigned int DeAccSet = DE_ACC_SET;
signed int TorqMode_IqMax = Q15(0.58);
signed int BrakeStopSpeed =
    600;  // 600 = Q15(0.0183)  No braking below the speed
signed int BrakeStartSpeed =
    1000;  // 1000 = Q15(0.0305) ;   // No braking below the speed
signed int MotorStartSpeed = Q15(0.01);
signed int BrakeStartSpeedPulses = 15;  // use pulses instead of speed
signed int BrakeStopSpeedPulses = 7;
signed int MotorStartSpeedPulses = 0;
unsigned int MOSFET_OverTemp = OVERTEMP_MOSFET_90;
unsigned long ThrottleHighCntr = 0;  // used to start/stop
unsigned int LEDFlashCntr = 0;
unsigned int FaultLEDFLashCntr = 0;
unsigned int SpeedSlopCntrSet = SPEED_SLOP_CNTR_SET;
register int a_Reg asm("A");  // DSP accumulator A for __builtin_mpy/__builtin_sacr.
unsigned int TorqAccCntr = 0;
unsigned int TorqAccCntrSet = TORQ_ACCDELAY;

void InitControlParameters(void);
void DoControl(void);
void ResetParmeters(void);
void MeasCurrOffset(int16_t *pOffseta, int16_t *pOffsetb);
extern void CalculateParkAngleHall(void);
void InitMovingAvgPeriod(void);
void CalcMovingAvgPeriod(uint16_t instPeriod);
void CalcMovingAvgSpeed(int32_t instSpeed);
void UndervoltageDetect(void);
void OvertemperatureDetectMCU(void);
void Braking(void);
void OvertemperatureDetectMOSFET(void);
extern void SpeedCalculation(void);
void CNRead_Inline(void);
extern void GetInitHallAngle(void);
void SetupTimer1(void);
extern void GetHallAngleAuto_Inline(void);
void VoltageDetect(void);
void MotorStallDetect(void);
bool MotorStallIsCurrentLimitActive(void);
bool MotorStallShouldForceOutputZeroOnThrottleRelease(void);
void MotorStallForceOutputZero(void);
void ClearAllFault(void);
void OvervoltageDetect(void);
extern signed int FracMpy(signed int mul_1, signed int mul_2);
void IqSquareIntegral(void);
void IbusCalc(void);
void IbusCalc2(void);

// Modbus 排程器相關函式宣告
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
// 電池裝置偵測與備用機制相關函式
bool checkExternalBatteryValidity(
    const S_MODBUS_BATTERY_DATA_RAW *pstBatteryData);
void updateLocalBatteryDataAsBackup(S_BATTERY_DATA *pstBatteryData);
#endif

// --- [NEW] LED Hardware Abstraction Macros ---
static inline void HW_RedLed_On() {
    O_LED_RED_LAT = 1;
}
static inline void HW_RedLed_Off() {
    O_LED_RED_LAT = 0;
}
static inline void HW_RedLed_Toggle() {
    O_LED_RED_LAT = !O_LED_RED_LAT;
}

static inline void HW_YellowLed_On() {
    O_LED_YELLOW_LAT = 1;
}
static inline void HW_YellowLed_Off() {
    O_LED_YELLOW_LAT = 0;
}
static inline void HW_YellowLed_Toggle() {
    O_LED_YELLOW_LAT = !O_LED_YELLOW_LAT;
}

static inline void HW_GreenLed_On() {
    O_LED_GREEN_LAT = 1;
}
static inline void HW_GreenLed_Off() {
    O_LED_GREEN_LAT = 0;
}
static inline void HW_GreenLed_Toggle() {
    O_LED_GREEN_LAT = !O_LED_GREEN_LAT;
}

// --- [NEW] Helper function to get flash count for a given alarm code ---
static uint8_t get_flash_count_for_alarm(E_LOGIC_ALARM_CODE_T alarm) {
    switch (alarm) {
        case LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE:
            return 8;  // A01
        case LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT:
            return 2;  // A02
        case LOGIC_ALARM_A03_THROTTLE_FAULT:
            return 13;  // A03
        case LOGIC_ALARM_A04_MOTOR_HALL_FAULT:
            return 6;  // Doc: A07
        case LOGIC_ALARM_A05_MOTOR_OVERCURRENT:
            return 7;  // Doc: A09
        case LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP:
            return 9;  // Doc: A06
        case LOGIC_ALARM_A14_LSN_FAULT:
            return 16;  // A14
        case LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE:
            return 15;  // A15
        case LOGIC_ALARM_A19_EMB_SENSOR_FAULT:
            return 17;  // A19
        case LOGIC_ALARM_A20_MOTOR_OVER_TEMP:
            return 18;  // A20
        default:
            return 1;  // Default to 1 flash for unlisted errors
    }
}

// --- [REFACTORED] Unified LED Display Logic with Pattern Flashing State Machine ---
static void update_led_display(void) {
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 1

    typedef enum {
        LED_STATE_NORMAL,
        LED_STATE_FLASHING_ON,
        LED_STATE_FLASHING_OFF,
        LED_STATE_PAUSED
    } LedFlashState_t;

    static LedFlashState_t s_eLedFlashState = LED_STATE_NORMAL;
    static uint8_t s_u8FlashCount = 0;
    static uint8_t s_u8TargetFlashes = 0;
    static uint32_t s_u32FlashTimer = 0;

    E_LOGIC_ALARM_CODE_T eCurrentAlarm = logic_errorHandler_getHighestPriorityActiveAlarm();

    if (eCurrentAlarm == LOGIC_ALARM_NONE) {
        // --- State: NORMAL (No Errors) ---
        s_eLedFlashState = LED_STATE_NORMAL;
        S_LOGIC_BATTERY_LED_STATUS_T sLedStatus = logic_battery_getLedStatus();

#ifdef CODESW_MEDICAL_SCOOTER
        // 醫療代步車：RD7(綠)=前進燈、RD6(黃)=後退燈 (取代電量長條顯示)
        // bMotorDirection == 0 為前進 (見 throttle 輸出上限選擇邏輯)
        if (g_stSystemData.bMotorDirection == 0) {
            HW_GreenLed_On();    // RD7: 前進 (FWD)
            HW_YellowLed_Off();  // RD6: 後退 (REV)
        } else {
            HW_GreenLed_Off();
            HW_YellowLed_On();
        }
#else
        // Bar-graph style display
        if (sLedStatus.eGreenLed == LOGIC_LED_STATE_ON) {
            HW_GreenLed_On();
        } else {
            HW_GreenLed_Off();
        }
        if (sLedStatus.eYellowLed == LOGIC_LED_STATE_ON) {
            HW_YellowLed_On();
        } else {
            HW_YellowLed_Off();
        }
#endif
        if (sLedStatus.eRedLed == LOGIC_LED_STATE_ON) {
            HW_RedLed_On();
        } else {
            HW_RedLed_Off();
        }
        return;
    }

    // --- Error States ---
    // Turn off Green and Yellow LEDs when an error is active
    HW_GreenLed_Off();
    HW_YellowLed_Off();

    uint32_t u32CurrentTimeMs = getSystemTimeMs();

    switch (s_eLedFlashState) {
        case LED_STATE_NORMAL:  // Transition from Normal to Flashing
            s_u8TargetFlashes = get_flash_count_for_alarm(eCurrentAlarm);
            s_u8FlashCount = 0;
            s_u32FlashTimer = u32CurrentTimeMs;
            s_eLedFlashState = LED_STATE_FLASHING_ON;
            HW_RedLed_On();  // Start the first flash immediately
            break;

        case LED_STATE_FLASHING_ON:
            if ((u32CurrentTimeMs - s_u32FlashTimer) > 250) {  // 250ms ON time
                s_u32FlashTimer = u32CurrentTimeMs;
                s_eLedFlashState = LED_STATE_FLASHING_OFF;
                HW_RedLed_Off();
                s_u8FlashCount++;
            }
            break;

        case LED_STATE_FLASHING_OFF:
            if (s_u8FlashCount >= s_u8TargetFlashes) {  // Check if sequence is complete
                s_u32FlashTimer = u32CurrentTimeMs;
                s_eLedFlashState = LED_STATE_PAUSED;
                // LED is already off
            } else if ((u32CurrentTimeMs - s_u32FlashTimer) > 250) {  // 250ms OFF time
                s_u32FlashTimer = u32CurrentTimeMs;
                s_eLedFlashState = LED_STATE_FLASHING_ON;
                HW_RedLed_On();
            }
            break;

        case LED_STATE_PAUSED:
            if ((u32CurrentTimeMs - s_u32FlashTimer) > 1500) {  // 1.5s PAUSE time
                // Restart the sequence
                s_eLedFlashState = LED_STATE_NORMAL;
            }
            break;
    }
#endif
}

// *****************************************************************************
/* Function:
   main()

  Summary:
    main() function

  Description:
    program entry point, calls the system initialization function group

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

int main(void) {
    HallMinPeriod = MINPERIOD;
    // HallMinPeriod = 70;
    HallPeriod = 30000;

    /* Initialize Peripherals */
    Init_Peripherals();
    SW_12V = TURN_ON;  // must before CAN_Initialize();
    CAN1_Initialize();
    // SCCP3_TMR_Initialize();
    // CN_Configure();
    OverCurrentEnable();
    /* Initializing Current offsets in structure variable */
    measCurrOffsetFlag = 1;
    MeasCurrOffset(&measCurrParm.Offseta, &measCurrParm.Offsetb);
    // measCurrParm.Offseta = 384;
    // measCurrParm.Offsetb = 192;
    // HAL_MC1PhaseStateChangeMaxPeriodSet(PERIOD_CONSTANT);

    CORCONbits.SATA = 1;
    CORCONbits.SATB = 1;
    CORCONbits.ACCSAT = 1;

    CORCONbits.SATA = 0;

    /* Initialize PI control parameters */
    InitControlParameters();

    /* Reset parameters used for running motor through Inverter */
    ResetParmeters();

    /* Uncomment to enable throttle control*/
    // CN_PortCEnable();
    // CN_PortCDisable();

    // 加入 userparms.h 的定義，看IO要那些初始化，在這邊進行
    // 還有 GPIO 的設定，輸入輸出
    // 初始化GPIO輸出入腳位及狀態

    // LED輸出初始化 - 預設關閉
    O_LED_GREEN_TRIS = 0;   // 綠色LED設為輸出
    O_LED_GREEN_LAT = 0;    // 初始關閉
    O_LED_YELLOW_TRIS = 0;  // 黃色LED設為輸出
    O_LED_YELLOW_LAT = 0;   // 初始關閉
    O_LED_RED_TRIS = 0;     // 紅色LED設為輸出
    O_LED_RED_LAT = 0;      // 初始關閉

#if CODESW_DEBUG_ISR_PROFILE_ENABLE == 1
    // 除錯量測腳位 - 示波器量測執行時間 (RC13: ADC ISR, RD13: 速度命令處理)
    O_DBG_ADC_ISR_TRIS = 0;        // RC13 設為輸出
    O_DBG_ADC_ISR_LAT = 0;         // 初始拉低
    O_DBG_SPEED_PROFILE_TRIS = 0;  // RD13 設為輸出
    O_DBG_SPEED_PROFILE_LAT = 0;   // 初始拉低
#endif

    // 控制輸出初始化
    O_HEAD_LIGHT_TRIS = 0;     // 大燈控制設為輸出
    O_HEAD_LIGHT_LAT = 0;      // 初始關閉
    O_BRAKE_LIGHT_TRIS = 0;    // 煞車燈控制設為輸出
    O_BRAKE_LIGHT_LAT = 0;     // 初始關閉
    O_EM_BRAKE_CTRL_TRIS = 0;  // 電磁煞車控制設為輸出
#if CODESW_EMBRAKER_ENABLE
    O_EM_BRAKE_CTRL_LAT = 0;   // EMB enabled: start locked
#else
    O_EM_BRAKE_CTRL_LAT = 1;   // EMB disabled for test: keep released
#endif

// 通訊相關初始化
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
    O_UART_TX_TRIS = 0;   // UART TX設為輸出
    I_UART_RX_TRIS = 1;   // UART RX設為輸入
    O_RS485_RE_TRIS = 0;  // RS485方向控制設為輸出
    O_RS485_RE_LAT = 0;   // 初始設為接收模式
#endif
    O_CAN_STB_TRIS = 0;  // CAN待機控制設為輸出
    O_CAN_STB_LAT = 0;   // 初始啟動(非待機)
    O_CAN_TX_TRIS = 0;   // CAN TX設為輸出
    I_CAN_RX_TRIS = 1;   // CAN RX設為輸入

    // 數位輸入初始化
    I_BRAKE_TRIS = 1;             // IBKS - 煞車訊號輸入
    I_SPEED_SENSOR_A_TRIS = 1;    // ISNA - 方向開關A輸入
    I_SPEED_SENSOR_B_TRIS = 1;    // ISNB - 方向開關B輸入
    I_CRUISE_TRIS = 1;            // CRUISE - 啟動/停止開關輸入
    I_FR_SWITCH_TRIS = 1;         // IFR - 前進/後退開關輸入
    I_EXT_SPEED_SENSOR_TRIS = 1;  // ILSN - 外部速度感測輸入
    // 初始化數位和類比輸入腳位的內部上拉電阻

    InitDigitalInputPullups();
    SetupTimer1();
#if CODESW_SCOPE_ENABLE == 1
    DiagnosticsInit();
#endif
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
    // ===== 最簡單UART測試初始化 =====

    // 1. 初始化UART模組 (使用MCC生成的版本)
    // UART1_Initialize();
    //
    UART1_InterruptReceiveDisable();
    UART1_InterruptReceiveFlagClear();
    UART1_InterruptTransmitDisable();
    UART1_InterruptTransmitFlagClear();

    UART1_Initialize();

    // 2. 設定波特率 (57600 baud)
    UART1_BaudRateDividerSet(107);  // 57600 baud with 100MHz FCY
    UART1_SpeedModeStandard();

    // 3. 確保UART模組和發送模式啟用
    UART1_ModuleEnable();
    // 4. 初始化RS485方向控制
    O_RS485_RE_TRIS = 0;  // 設為輸出
    O_RS485_RE_LAT = 0;   // 初始設為接收模式

#if CODESW_UART_TEST_ENABLE == 1
    // ===== 最簡單UART測試初始化 =====

    // 1. 初始化UART模組 (使用MCC生成的版本)
    UART1_Initialize();

    // 2. 設定波特率 (57600 baud)
    UART1_BaudRateDividerSet(107);  // 57600 baud with 100MHz FCY

    // 3. 確保UART模組和發送模式啟用
    UART1_ModuleEnable();
    UART1_TransmitModeEnable();

    // 4. 初始化RS485方向控制
    O_RS485_RE_TRIS = 0;  // 設為輸出
    O_RS485_RE_LAT = 0;   // 初始設為接收模式

#endif
#else
    O_UART_TX_TRIS = 0;
    I_UART_RX_TRIS = 1;
    O_RS485_RE_TRIS = 0;
    O_RS485_RE_LAT = 0;   // x2cscope uses MCU UART pins directly; keep RS485 transceiver in receive/high-Z
#endif
    OldHallState = HW_Hall_U_Read() + ((unsigned int)HW_Hall_V_Read() * 2) +
                   ((unsigned int)HW_Hall_W_Read() * 4);  // initial Hall value
    HallState = OldHallState;
    GetInitHallAngle();
    // uGF.ReGenEnable = 1;

#define MotorAngle_6
//=============================================================================
// MotorAngle_1~6, 546231 with "increasing" angles
//=============================================================================
#ifdef MotorAngle_1  // works for Hust BLDC Motor, increasing angle
    FindHallAngle.Pos[5] = 10922 + HallOffset;
    FindHallAngle.Pos[4] = 21844 + HallOffset;
    FindHallAngle.Pos[6] = 32767 + HallOffset;
    FindHallAngle.Pos[2] = -21844 + HallOffset;
    FindHallAngle.Pos[3] = -10922 + HallOffset;
    FindHallAngle.Pos[1] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
#ifdef MotorAngle_2
    FindHallAngle.Pos[1] = 10922 + HallOffset;
    FindHallAngle.Pos[5] = 21844 + HallOffset;
    FindHallAngle.Pos[4] = 32767 + HallOffset;
    FindHallAngle.Pos[6] = -21844 + HallOffset;
    FindHallAngle.Pos[2] = -10922 + HallOffset;
    FindHallAngle.Pos[3] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
#ifdef MotorAngle_3  // Xiaomi e-kick scooter
    FindHallAngle.Pos[3] = 10922 + HallOffset;
    FindHallAngle.Pos[1] = 21844 + HallOffset;
    FindHallAngle.Pos[5] = 32767 + HallOffset;
    FindHallAngle.Pos[4] = -21844 + HallOffset;
    FindHallAngle.Pos[6] = -10922 + HallOffset;
    FindHallAngle.Pos[2] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
#ifdef MotorAngle_4
    FindHallAngle.Pos[2] = 10922 + HallOffset;
    FindHallAngle.Pos[3] = 21844 + HallOffset;
    FindHallAngle.Pos[1] = 32767 + HallOffset;
    FindHallAngle.Pos[5] = -21844 + HallOffset;
    FindHallAngle.Pos[4] = -10922 + HallOffset;
    FindHallAngle.Pos[6] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
#ifdef MotorAngle_5
    FindHallAngle.Pos[6] = 10922 + HallOffset;
    FindHallAngle.Pos[2] = 21844 + HallOffset;
    FindHallAngle.Pos[3] = 32767 + HallOffset;
    FindHallAngle.Pos[1] = -21844 + HallOffset;
    FindHallAngle.Pos[5] = -10922 + HallOffset;
    FindHallAngle.Pos[4] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
#ifdef MotorAngle_6
    FindHallAngle.Pos[4] = 10922 + HallOffset;
    FindHallAngle.Pos[6] = 21844 + HallOffset;
    FindHallAngle.Pos[2] = 32767 + HallOffset;
    FindHallAngle.Pos[3] = -21844 + HallOffset;
    FindHallAngle.Pos[1] = -10922 + HallOffset;
    FindHallAngle.Pos[5] = 0 + HallOffset;
    uGF.DirectionDefault = 1;  // 1: For Hurst motor Hall signal sequence
#endif
//=============================================================================
// MotorAngle_1~6, 546231 with decreasing angles
//=============================================================================
#ifdef MotorAngle_7  // works for NIDEC R35 90W BLDC Motor
    FindHallAngle.Pos[5] = 10922 + HallOffset;
    FindHallAngle.Pos[1] = 21844 + HallOffset;
    FindHallAngle.Pos[3] = 32767 + HallOffset;
    FindHallAngle.Pos[2] = -21844 + HallOffset;
    FindHallAngle.Pos[6] = -10922 + HallOffset;
    FindHallAngle.Pos[4] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
#ifdef MotorAngle_8
    FindHallAngle.Pos[4] = 10922 + HallOffset;
    FindHallAngle.Pos[5] = 21844 + HallOffset;
    FindHallAngle.Pos[1] = 32767 + HallOffset;
    FindHallAngle.Pos[3] = -21844 + HallOffset;
    FindHallAngle.Pos[2] = -10922 + HallOffset;
    FindHallAngle.Pos[6] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
#ifdef MotorAngle_9
    FindHallAngle.Pos[6] = 10922 + HallOffset;
    FindHallAngle.Pos[4] = 21844 + HallOffset;
    FindHallAngle.Pos[5] = 32767 + HallOffset;
    FindHallAngle.Pos[1] = -21844 + HallOffset;
    FindHallAngle.Pos[3] = -10922 + HallOffset;
    FindHallAngle.Pos[2] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
#ifdef MotorAngle_10
    FindHallAngle.Pos[2] = 10922 + HallOffset;
    FindHallAngle.Pos[6] = 21844 + HallOffset;
    FindHallAngle.Pos[4] = 32767 + HallOffset;
    FindHallAngle.Pos[5] = -21844 + HallOffset;
    FindHallAngle.Pos[1] = -10922 + HallOffset;
    FindHallAngle.Pos[3] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
#ifdef MotorAngle_11
    FindHallAngle.Pos[3] = 10922 + HallOffset;
    FindHallAngle.Pos[2] = 21844 + HallOffset;
    FindHallAngle.Pos[6] = 32767 + HallOffset;
    FindHallAngle.Pos[4] = -21844 + HallOffset;
    FindHallAngle.Pos[5] = -10922 + HallOffset;
    FindHallAngle.Pos[1] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
#ifdef MotorAngle_12
    FindHallAngle.Pos[1] = 10922 + HallOffset;
    FindHallAngle.Pos[3] = 21844 + HallOffset;
    FindHallAngle.Pos[2] = 32767 + HallOffset;
    FindHallAngle.Pos[6] = -21844 + HallOffset;
    FindHallAngle.Pos[4] = -10922 + HallOffset;
    FindHallAngle.Pos[5] = 0 + HallOffset;
    uGF.DirectionDefault = 0;  // 0: For some hub motors
#endif
    HAL_MC1PWMEnableOutputs();
    // IOTestOutput = 0; // 移除測試輸出，因為在 userparms.h 中沒有對應定義
    //  Initialize control mode
    uGF.CtrlMode = CTRLMODE;
    uGF.ReGenMode = 2;  // Normal
    uGF.DriveMode = 1;  // Normal
    // Speed Mode (Control mode = 0)
    SpeedModeCtrlLimit = Q15_MAXSPEED_REF_LIMIT;
    // Torque mode
    SpeedCtrlLimit = Q15_MAXSPEED_CtrlMode_2;
    IqSquare.Sum = 0;
    IqSquare.RatedIq = RATED_CURRENT_Q15;

    g_stSystemData.sStepTime = (S_MOTOR_STEP_TIME_T){0, 0};
#if CODESW_MODBUS_SCHEDULER_ENABLE && CODESW_THROTTLE_ENABLE
    g_stSystemData.sSharedData.u8AssistLevel = THROTTLE_ASSIST_LEVEL_DEFAULT;
#endif

    // =======================================================================================
    // 初始化馬達參數
    // =======================================================================================
    logic_motor_configInit();
    g_stSystemData.bMotorDirection = I_FR_SWITCH_PIN;

    // =======================================================================================
    // 油門控制初始化
    // =======================================================================================

#if CODESW_THROTTLE_ENABLE == 1
    // 初始化油門, 讀取油門電壓並轉換
    g_stSystemData.u16ThrottleVRRaw = (ADCBUF_THROTTLE_VR >> 4);
    uint16_t u16InitialThrottleMv = logic_convert_adcToVoltageMv(g_stSystemData.u16ThrottleVRRaw,
                                                                 THROTTLE_DIVIDER_R1,
                                                                 THROTTLE_DIVIDER_R2);
    logic_throttle_initAndCheck(u16InitialThrottleMv);

#endif

    // =======================================================================================
    // VR控制初始化
    // =======================================================================================

#if CODESW_VR_ENABLE == 1
    // 初始化VR, 讀取VR電壓
    g_stSystemData.u16ThrottleVRRaw = (ADCBUF_THROTTLE_VR >> 4);
    logic_vr_initAndCheck(g_stSystemData.u16ThrottleVRRaw);
#endif

    // =======================================================================================
    // 電池初始化
    // =======================================================================================

#if CODESW_BATTERY_ENABLE
    // 初始化電池, 讀取電池電壓
    logic_battery_init(LOGIC_BATTERY_DEFAULT_NOMINAL_VOLTAGE);
#endif

    // =======================================================================================
    // 溫度初始化
    // =======================================================================================

#if CODESW_TEMPERATURE_CONTROLLER_ENABLE
    logic_temp_controller_init();
#endif
#if CODESW_EMBRAKER_ENABLE
    // =======================================================================================
    // 電磁煞車初始化
    // =======================================================================================
    // 初始化電磁煞車, 讀取電磁煞車電壓
#if CODESW_EMBRAKER_TEST
    bEM_BRAKE_SWITCH = 1;
    // 假設 IEMBMv 沒有問題
    if (bEM_BRAKE_SWITCH == 0) {
        g_stSystemData.u16IEMBMv = 0;  // 設定為低於閾值，觸發故障
    } else {
        g_stSystemData.u16IEMBMv = 3300;  // 設定為高於閾值
    }

    logic_embraker_init(g_stSystemData.u16IEMBMv);
#else
    // g_stSystemData.u16IEMBRaw = (ADCBUF_EMBRAKER >> 4);
    // g_stSystemData.u16IEMBMv = logic_convert_adcToVoltageMv(g_stSystemData.u16IEMBRaw,
    // EMBRAKER_DIVIDER_R1,
    // EMBRAKER_DIVIDER_R2);

    // 讀取 IEMB 數位腳位狀態
    if (EM_BRAKE_SWITCH == 0)  // 低電位時，代表故障或未安裝
    {
        g_stSystemData.u16IEMBMv = 0;  // 設定為低於閾值，觸發故障
    } else                             // 高電位時，代表正常
    {
        g_stSystemData.u16IEMBMv = 3300;  // 設定為高於閾值
    }
    logic_embraker_init(g_stSystemData.u16IEMBMv);
#endif
#endif

    // =======================================================================================
    // RS485 & Modbus 初始化
    // =======================================================================================

#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
#if CODESW_RS485_TEST_ENABLE == 1
    uint8_t u8Data_rx[128] = {0};
    uint8_t u8Data_tx[11] = {"HELLOWORLD\n"};
    static bool bTestInitialized = false;

    if (!bTestInitialized) {
        hal_rs485_init(getSystemTimeMs, 10);
        bTestInitialized = true;
    }
#else
    // 初始化正常的 Modbus 排程器
    static bool bModbusInitialized = false;

    if (!bModbusInitialized) {
        // 初始化 RS485 HAL
        hal_rs485_init(getSystemTimeMs, 20);  // 20ms 超時

        // 初始化 Modbus 服務
        modbusService_init(getSystemTimeMs, 20);  // 20ms 超時

        // ==================================================================
        // **【新增】** 預先將系統預設值載入 Modbus 緩衝區
        // ==================================================================
        S_MODBUS_ALL_DATA *pstModbusData = modbusService_getDataPtr();
        modbusDecode_encodeLcdSettings(&pstModbusData->uLcdData,
                                       &g_stSystemData.sSharedData,
                                       &g_stSystemData.sBatteryData, 1);  // u8Update 設為 1，確保所有欄位都被編碼
        modbusDecode_encodeGuiSettings(&pstModbusData->uPcGuiData,
                                       &g_stSystemData.sSharedData,
                                       &g_stSystemData.sBatteryData, 1);  // 同上
        // ==================================================================

        bModbusInitialized = true;
    }

#endif
#endif

    // =======================================================================================
    // 主程式
    // =======================================================================================

    while (1) {
        // -------------------------------------------------------------------------------------
        // RS485 & Modbus 執行
        // -------------------------------------------------------------------------------------

#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
        // 執行正常的 Modbus 排程器
        modbusService_process();
#if CODESW_RS485_TEST_ENABLE == 1
        static uint32_t u32LastTime = 0;

        // 處理狀態機
        hal_rs485_process();

        // 檢查接收完成
        if (hal_rs485_is_rx_complete()) {
            uint16_t u16Length = hal_rs485_get_rx_length();
            if (u16Length > 0) {
                memcpy(u8Data_rx, hal_rs485_get_rx_data(), u16Length);
            }
            hal_rs485_receive_reset();
            hal_rs485_send(u8Data_rx, u16Length);  // Echo
        }

        // 每3秒發送測試
        if ((millis() - u32LastTime) > 3000 && !hal_rs485_is_busy()) {
            hal_rs485_send(u8Data_tx, sizeof(u8Data_tx));
            u32LastTime = millis();
        }
#else
        // 從 modbus_master 取得資料，並透過 modbus_decode 更新資料
        S_MODBUS_ALL_DATA *pstModbusData = modbusService_getDataPtr();

        // 將這些資料更新到要主程式的變數
        modbusDecode_decodeBatteryData(&g_stSystemData.sBatteryData,
                                       &pstModbusData->uBatteryData.stRaw);

        // 獨立處理 GUI 的新資料
        if (modbusService_hasNewGuiData()) {
            modbusDecode_decodeGuiData(&g_stSystemData.sSharedData,
                                       &pstModbusData->uPcGuiData);
            modbusService_clearNewGuiDataFlag();
        }

        // 獨立處理 LCD 的新資料
        if (modbusService_hasNewLcdData()) {
            modbusDecode_decodeLcdData(&g_stSystemData.sSharedData,
                                       &pstModbusData->uLcdData);
            modbusService_clearNewLcdDataFlag();
        }
        {
            // ------------------------------------------------------------------
            // ? 電池裝置偵測與備用機制
            // 檢查外部電池裝置 (ID01) 是否正常回應，沒有的話採用內部偵測
            bool bExternalBatteryValid =
                checkExternalBatteryValidity(&pstModbusData->uBatteryData.stRaw);

            if (!bExternalBatteryValid) {
                // 外部電池裝置無效，使用本地電池數據作為備用
                updateLocalBatteryDataAsBackup(&g_stSystemData.sBatteryData);
            }
        }
        {
#if CODESW_SPEED_TEST == 0
            // --- Original Speed Calculation ---
            // ------------------------------------------------------------------
            // 計算車速
            // 從馬達邏輯模組獲取目前設定的輪徑 (單位: 吋 * 10)
            uint16_t u16WheelInches = logic_motor_getWheelDimension();
            uint8_t u8PolePairs = logic_motor_getPolePairs();
            uint16_t u16SensorPpr = logic_motor_getHallPPR();

            uint16_t u16ExternalRpm = logic_motor_LwfocGetExternalRpm(abs(Speed),
                                                                      u8PolePairs,
                                                                      u16SensorPpr);
            // 使用 RPM 和輪徑計算車速 (單位: KM/H * 100)
            uint16_t u16SpeedKmh_x100 = logic_motor_getSpeedKmhFromRpm(u16ExternalRpm,
                                                                       u16WheelInches);

            // 將結果轉換為 KM/H * 10 並存入共享資料結構中
            g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10 = u16SpeedKmh_x100 / 10;
#else
            // --- [NEW] Safe Speed Simulation ---
            static uint16_t s_u16SimulatedKmh_x10 = 0;
            static int8_t s_i8SpeedDirection = 1;  // 1 for ramp up, -1 for ramp down
            static uint32_t s_u32LastSpeedChangeTime = 0;

            // Update speed every 200ms
            if ((g_stSystemData.u32TimeMs - s_u32LastSpeedChangeTime) > 200) {
                s_u32LastSpeedChangeTime = g_stSystemData.u32TimeMs;

                if (s_i8SpeedDirection == 1) {
                    s_u16SimulatedKmh_x10 += 5;          // Ramp up by 0.5 km/h
                    if (s_u16SimulatedKmh_x10 >= 250) {  // Max speed 25.0 km/h
                        s_u16SimulatedKmh_x10 = 250;
                        s_i8SpeedDirection = -1;  // Change to ramp down
                    }
                } else {
                    s_u16SimulatedKmh_x10 -= 5;           // Ramp down by 0.5 km/h
                    if (s_u16SimulatedKmh_x10 > 30000) {  // Check for underflow
                        s_u16SimulatedKmh_x10 = 0;
                    }
                    if (s_u16SimulatedKmh_x10 == 0) {
                        s_i8SpeedDirection = 1;  // Change to ramp up
                    }
                }
            }
            // Overwrite the final display value directly
            g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10 = s_u16SimulatedKmh_x10;

#endif
        }
        // 將資料更新到 modbus_master, 如果update為1, 則將master的資料更新到slave
        modbusDecode_encodeLcdSettings(&pstModbusData->uLcdData,
                                       &g_stSystemData.sSharedData,
                                       &g_stSystemData.sBatteryData, 0);

        modbusDecode_encodeGuiSettings(&pstModbusData->uPcGuiData,
                                       &g_stSystemData.sSharedData,
                                       &g_stSystemData.sBatteryData, 0);

#endif
#endif
#if CODESW_SCOPE_ENABLE == 1
        // ------------------------------------------------------------------
        // 計算車速
        // 從馬達邏輯模組獲取目前設定的輪徑 (單位: 吋 * 10)
        uint16_t u16WheelInches = logic_motor_getWheelDimension();
        uint8_t u8PolePairs = logic_motor_getPolePairs();
        uint16_t u16SensorPpr = logic_motor_getHallPPR();

        uint16_t u16ExternalRpm = logic_motor_LwfocGetExternalRpm(abs(Speed),
                                                                  u8PolePairs,
                                                                  u16SensorPpr);
        // 使用 RPM 和輪徑計算車速 (單位: KM/H * 100)
        uint16_t u16SpeedKmh_x100 = logic_motor_getSpeedKmhFromRpm(u16ExternalRpm,
                                                                   u16WheelInches);

        // 將結果轉換為 KM/H * 10 並存入共享資料結構中
        // u16CurrentSpeedKmh_x10 = u16SpeedKmh_x100 / 10;
        u16CurrentSpeedKmh_x10 = u16SpeedKmh_x100 / 10;

#endif

        // SpeedRefHighLimit = SpeedCtrlLimit;
#if CODESW_SCOPE_ENABLE == 1
        DiagnosticsStepMain();
#endif

        if (PollingCntr >= 400)  // 20ms
        {
            PollingCntr = 0;

#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 1
            update_led_display();
#endif

// =======================================================================================
#if CODESW_RS485_TEST_ENABLE == 1
            // hal_rs485_send(u8Data_tx, sizeof(u8Data_tx));
#endif
            uGF.BrakeSWOn = !I_BRAKE_PIN;
#if 0
            uGF.DirSW = I_FR_SWITCH_PIN;
#endif

            MotorStallDetect();

#if CODESW_BATTERY_ENABLE
#if CODESW_BATTERY_TEST == 1
            // In test mode, cycle through predefined ADC values to simulate voltage changes.
            // ADC values for 24V system: Normal(25V), Dip(20V), Spike(28V)
            const uint16_t battery_test_adc_values[] = {1633, 1306, 1306, 1306, 1829, 1633, 1633, 1633};
            const int num_battery_test_values = sizeof(battery_test_adc_values) / sizeof(battery_test_adc_values[0]);
            static int test_battery_index = 0;
            static uint32_t last_battery_change_time = 0;

            if (last_battery_change_time == 0)  // Initialize on first run
            {
                last_battery_change_time = g_stSystemData.u32TimeMs;
            }

            // Change value every 3 seconds
            if ((g_stSystemData.u32TimeMs - last_battery_change_time) > 3000) {
                last_battery_change_time = g_stSystemData.u32TimeMs;
                test_battery_index = (test_battery_index + 1) % num_battery_test_values;
            }
            g_stSystemData.u16BatteryVoltageRaw = battery_test_adc_values[test_battery_index];
            logic_battery_updateVoltage(g_stSystemData.u16BatteryVoltageRaw);

            // --- DEBUG PRINTF ---
            // Get all voltage types for debugging the filter
            uint16_t inst_v = logic_battery_getInstantVoltage();
            uint16_t filt_v = logic_battery_getActualVoltage();  // This is the filtered value
            uint16_t chk_v = logic_battery_getVoltageForCheck();
            S_LOGIC_BATTERY_INFO_T info = logic_battery_getInfo();
            // --- END DEBUG ---
#else
            // In normal mode, read the real ADC value.
            g_stSystemData.u16BatteryVoltageRaw = (ADCBUF_BATTERY_VOLTAGE >> 4);
            logic_battery_updateVoltage(g_stSystemData.u16BatteryVoltageRaw);
#endif

            // Common logic for both test and normal mode
            g_stSystemData.u16BatteryVoltage = logic_battery_getActualVoltage();
            g_stSystemData.u16BatteryPercent = logic_battery_getSOCPercent();
            g_stSystemData.bBatteryShouldProhibit = logic_battery_shouldProhibitOutput();
#endif

#if CODESW_TEMPERATURE_CONTROLLER_ENABLE
#if CODESW_TEMPERATURE_CONTROLLER_TEST == 1
            // In test mode, cycle through predefined ADC values every 5 seconds.
            static int test_temp_index = 0;
            static uint32_t last_temp_change_time = 0;

            if (last_temp_change_time == 0)  // Initialize on first run
            {
                last_temp_change_time = g_stSystemData.u32TimeMs;
                g_stSystemData.u16ControllerTempRaw = temp_test_adc_values[0];
            }

            if ((g_stSystemData.u32TimeMs - last_temp_change_time) > 5000) {
                last_temp_change_time = g_stSystemData.u32TimeMs;
                test_temp_index = (test_temp_index + 1) % num_temp_test_values;
                g_stSystemData.u16ControllerTempRaw = temp_test_adc_values[test_temp_index];
            }
#else
            // In normal mode, read the real ADC value.
            g_stSystemData.u16ControllerTempRaw = (ADCBUF_CONTROLLER_TEMP >> 4);
#endif

            // 使用舊有的絕對電流限制邏輯
            int16_t i16CurrentLimitA = logic_temp_controller_updateTempAndGetCurrentLimit(g_stSystemData.u16ControllerTempRaw,
                                                                                          &g_stSystemData.bControllerIsOverTemp,
                                                                                          &g_stSystemData.bControllerIsOverLoad);

            // 更新系統資料，給其他模組參考
            g_stSystemData.u16ControllerTemp = (uint16_t)(logic_temp_controller_getTemp() / 10);  // 轉換為 °C * 10

            // 根據回傳的電流上限(A)，設定 FOC 的扭矩命令上限 (TorqMode_IqMax)
            // const int16_t default_TorqMode_IqMax = Q15(LOGIC_TEMP_CONTROLLER_RATIO_NORMAL);  // 系統預設的最大扭矩

            // 最終的扭矩上限，取「系統預設」和「溫度限制」中較小的一個
            // TorqMode_IqMax = i16CurrentLimitA;
            int16_t i16FinalCurrentLimit = i16CurrentLimitA;
#if CODESW_MOTOR_LOCK_TEST_ENABLE
            int16_t i16LockTestLimit = Q15(CODESW_MOTOR_LOCK_TEST_PHASE_CURRENT_A / RATED_CURRENT);
            if (i16FinalCurrentLimit > i16LockTestLimit) {
                i16FinalCurrentLimit = i16LockTestLimit;
            }
#endif
            if (MotorStallIsCurrentLimitActive()) {
                i16FinalCurrentLimit = (int16_t)(((int32_t)i16FinalCurrentLimit *
                                                  MOTOR_STALL_CURRENT_LIMIT_PERCENT) /
                                                 100);
            }
            piInputOmega.piState.outMax = i16FinalCurrentLimit;
            piInputOmega.piState.outMin = -i16FinalCurrentLimit;
#endif

#if CODESW_TEMPERATURE_MOTOR_ENABLE
            g_stSystemData.u16MotorTempRaw = (ADCBUF_MOTOR_TEMP >> 4);
#endif

            // --- [NEW] Aggregate all conditions that should stop the motor ---
            g_stSystemData.bMotorStop = false;  // Default to not stopped
#if CODESW_BATTERY_ENABLE
            if (g_stSystemData.bBatteryShouldProhibit) {
                g_stSystemData.bMotorStop = true;
            }
#endif
#if CODESW_TEMPERATURE_CONTROLLER_ENABLE
            if (g_stSystemData.bControllerIsOverTemp) {
                g_stSystemData.bMotorStop = true;
            }
#endif
            // --- End of Aggregation ---
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
            // --- [NEW & EXPANDED] Update Modbus error flags from Error Handler ---
            g_stSystemData.sSharedData.errors.bBatteryLowVoltage = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE);
            g_stSystemData.sSharedData.errors.bBatteryOverVoltage = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE);
            g_stSystemData.sSharedData.errors.bControllerOverTemp = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP);
            g_stSystemData.sSharedData.errors.bMotorOverload = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A05_MOTOR_OVERCURRENT);
            g_stSystemData.sSharedData.errors.bThrottleStuck = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A03_THROTTLE_FAULT);
            g_stSystemData.sSharedData.errors.bMotorOrControllerError = (logic_errorHandler_isAlarmActive(LOGIC_ALARM_A07_MOTOR_SHORT_CIRCUIT) ||
                                                                         logic_errorHandler_isAlarmActive(LOGIC_ALARM_A08_MOTOR_PHASE_LOSS) ||
                                                                         logic_errorHandler_isAlarmActive(LOGIC_ALARM_A12_CONTROLLER_HW_FAULT));
            g_stSystemData.sSharedData.errors.bMotorSensorError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A04_MOTOR_HALL_FAULT);
            g_stSystemData.sSharedData.errors.bBrakeSignalError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT);
            g_stSystemData.sSharedData.errors.bBrakeStuck = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT);  // Also map stuck to switch fault
            g_stSystemData.sSharedData.errors.bLsnError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A14_LSN_FAULT);
            g_stSystemData.sSharedData.errors.bCommLcdError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A10_CONTROLLER_COMM_TIMEOUT);
            g_stSystemData.sSharedData.errors.bEmbSensorFault = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A19_EMB_SENSOR_FAULT);
#endif
            // Note: bCommBatteryError, bCommGuiError, bCommAppError have no direct mapping in E_LOGIC_ALARM_CODE_T
            // --- End of Modbus Error Flag Update ---

#if CODESW_EMBRAKER_ENABLE
            // --- [REFACTORED] EMBRAKER logic now uses the master stop flag ---
            if (g_stSystemData.bMotorStop) {
                // 電池保護或控制器過溫已啟動，強制鎖定電磁剎車
                O_EM_BRAKE_CTRL_LAT = 0;  // 鎖定 (輸出LOW)
            } else {
                // 更新電磁煞車邏輯
#if CODESW_EMBRAKER_TEST
                if (bEM_BRAKE_SWITCH == 0) {
                    g_stSystemData.u16IEMBMv = 0;
                } else {
                    g_stSystemData.u16IEMBMv = 3300;
                }
#else
#if CODESW_EMBRAKER_USE_ADC_INPUT
                // 使用 RD8 ADC 輸入
                g_stSystemData.u16IEMBRaw = (ADCBUF_EMBRAKER >> 4);
                g_stSystemData.u16IEMBMv = logic_convert_adcToVoltageMv(g_stSystemData.u16IEMBRaw,
                                                                        EMBRAKER_DIVIDER_R1,
                                                                        EMBRAKER_DIVIDER_R2);
#else
                // 使用數位開關輸入
                if (EM_BRAKE_SWITCH == 0)  // 低電位時，代表故障或未安裝
                {
                    g_stSystemData.u16IEMBMv = 0;  // 設定為低於閾值，觸發故障
                } else                             // 高電位時，代表正常
                {
                    g_stSystemData.u16IEMBMv = 3300;  // 設定為高於閾值
                }
#endif  // CODESW_EMBRAKER_USE_ADC_INPUT
#endif  // CODESW_EMBRAKER_TEST
        // 煞車訊號偵測到
                if ((uGF.BrakeSWOn == 1)) {
                    g_stSystemData.i16TargetRpm = 0;
                    g_stSystemData.i16CurrentRpm = 0;
                    g_stSystemData.i16ActiveRpm = 0;
                    ReferenceRAW = 0;
                }

                // --- [Plan A/B] 準備 EMB 交接訊號 ---
                // uGF.Direction 由霍爾狀態序列即時更新(每個邊緣)。UVW 未短路(FOC驅動中)時
                // 持續鎖存行駛方向；UVW 短路(停車/將停)後，若實際轉向與鎖存方向相反，代表
                // 車輪被重力拉著倒溜 → 觸發 Plan B 立即鎖定。
                static unsigned char s_u8EmbTravelDir = 0;
                if (uGF.UVWLock == 0) {
                    s_u8EmbTravelDir = uGF.Direction;  // 驅動中持續鎖存行駛方向
                }
                bool bEmbUVWLockActive = (uGF.UVWLock != 0);
                bool bEmbReverseEdge = bEmbUVWLockActive && (uGF.Direction != s_u8EmbTravelDir);

                // [有動力倒溜/倒衝] 用速度環的帶號命令/回授(已同座標系,見 CNRead 的 Speed 符號校正)
                //   判斷「命令一個方向、實際往反方向動」。HallPulsesLatch 閘門排除靜止抖動/32767 尖峰。
                bool bEmbDirMismatch =
                    (abs(piInputOmega.inReference) >= EMB_ROLLBACK_CMD_THRESHOLD) &&
                    (abs(piInputOmega.inMeasure) >= EMB_ROLLBACK_SPEED_THRESHOLD) &&
                    (HallPulsesLatch >= EMB_ROLLBACK_MIN_PULSES) &&
                    (((piInputOmega.inReference > 0) && (piInputOmega.inMeasure < 0)) ||
                     ((piInputOmega.inReference < 0) && (piInputOmega.inMeasure > 0)));

#if CODESW_SCOPE_ENABLE == 1
                E_EMBRAKER_ACTION eBrakeAction = logic_embraker_update(g_stSystemData.u16IEMBMv,
                                                                       g_stSystemData.i16TargetRpm,  // 使用者意圖轉速
                                                                       ReferenceRAW,                 // 馬達實際執行轉速
                                                                       bEmbUVWLockActive,            // [Plan A] UVW短路生效中
                                                                       bEmbReverseEdge,              // [Plan B] 偵測到倒溜
                                                                       bEmbDirMismatch,              // [有動力倒溜] 命令/回授方向相反
                                                                       g_stSystemData.u32TimeMs);
#endif
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
                E_EMBRAKER_ACTION eBrakeAction = logic_embraker_update(g_stSystemData.u16IEMBMv,
                                                                       g_stSystemData.i16TargetRpm,  // 使用者意圖轉速
                                                                       ReferenceRAW,                 // 馬達實際執行轉速
                                                                       bEmbUVWLockActive,            // [Plan A] UVW短路生效中
                                                                       bEmbReverseEdge,              // [Plan B] 偵測到倒溜
                                                                       bEmbDirMismatch,              // [有動力倒溜] 命令/回授方向相反
                                                                       g_stSystemData.u32TimeMs);
#endif

                // --- [NEW] Final safety check to override brake action ---
                if (g_stSystemData.bMotorStop) {
                    eBrakeAction = EMBRAKER_ACTION_LOCK;
                }
#if CODESW_MOTOR_LOCK_TEST_ENABLE
                eBrakeAction = EMBRAKER_ACTION_LOCK;
#endif

                // 根據回傳的指令控制硬體
                switch (eBrakeAction) {
                    case EMBRAKER_ACTION_LOCK:
#if !CODESW_MOTOR_LOCK_TEST_ENABLE
                        MotorStallForceOutputZero();
#endif
                        O_EM_BRAKE_CTRL_LAT = 0;  // 鎖定 (輸出LOW)
                        break;
                    case EMBRAKER_ACTION_RELEASE:
                        O_EM_BRAKE_CTRL_LAT = 1;  // 釋放 (輸出HI)
                        break;
                    case EMBRAKER_ACTION_NONE:
                    default:
                        // 狀態無變化，不執行任何動作
                        break;
                }
            }
#endif
            // g_stSystemData.u16TorqueSensorRaw = (ADCBUF_TORQUE_SENSOR >> 4);

            // g_stSystemData.i16SpeedProtectRaw = (ADCBUF_SPEED_PROTECT >> 4);
        }
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
        // ? 馬達方向指示燈
        if (uGF.DirSW == 1) {
            HW_DirectionLed_On();
        } else {
            HW_DirectionLed_Off();
        }
#endif
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
        // ? 故障指示燈
        if (FaultLEDFLashCntr > 5000) {
            FaultLEDFLashCntr = 0;
            HW_FaultLed_Toggle();  // used for running indication
        }
#endif
        if (uGF.Fault == 1) {
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
            if (LEDFlashCntr > 2500) {
                LEDFlashCntr = 0;
                HW_RunLed_Toggle();
            }
#endif
            uGF.RunMotor = 0;
        }
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
        // ? 煞車燈
        if (HW_BrakeSignal_IsActive()) {
            HW_BrakeLight_On();
        } else {
            HW_BrakeLight_Off();
        }
#endif
        // ? 馬達運行指示燈
        if ((uGF.BrakeSWOn == 0)) {
            uGF.RunMotor = 1;
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
            if (uGF.Fault == 0 && uGF.RunMotor == 1) {
                O_LED_GREEN_LAT = 1;
            } else {
                O_LED_GREEN_LAT = 0;
            }
#endif

            //   HAL_MC1PWMEnableOutputs();
        } else if ((uGF.BrakeSWOn == 1)) {
            uGF.RunMotor = 0;
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
            O_LED_GREEN_LAT = 0;
#endif
            ClearAllFault();
#if CODESW_THROTTLE_ENABLE == 1
            // ? 煞車按下時，重置油門相關的轉速數值
            g_stSystemData.i16TargetRpm = 0;
            g_stSystemData.i16CurrentRpm = 0;
            g_stSystemData.i16ActiveRpm = 0;
            ReferenceRAW = 0;

            // ? 強制進入滑行模式，確保馬達停止
            uGF.Coast = 1;
#endif
        }

        if (Speed == 0)
            HallPeriod = 30000;  // Reset bugs workaround
#if 0
        if (CAN1_Receive(&My_CAN_RXMSG) == true)
        {
            CANRXCntr++;
            My_CAN_TXMSG.data = CANTXBuffer;
            My_CAN_TXMSG.msgId = 0x200;
            My_CAN_TXMSG.field.dlc = 5;
            if (*CANRXPtr == 0x10) // Battery voltage
            {
                CANTXBuffer[0] = 0x10;
                // TempVar = FracMpy (faultOvervoltage.monitor+1, Q15(0.051625));
                TempVar = faultOvervoltage.monitor + 1;
                CANTXBuffer[4] = TempVar;
                CANTXBuffer[3] = TempVar >> 8;
                CANTXBuffer[2] = 0;
                CANTXBuffer[1] = 0;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }

            else if (*CANRXPtr == 0x12) // REGEN Mode
            {
                CANTXBuffer[0] = 0x12;
                CANTXBuffer[4] = uGF.ReGenMode;
                CANTXBuffer[3] = 0xFF;
                CANTXBuffer[2] = 0xFF;
                CANTXBuffer[1] = 0xFF;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x13) // Assistance mode
            {
                CANTXBuffer[0] = 0x13;
                CANTXBuffer[4] = uGF.DriveMode;
                CANTXBuffer[3] = 0xFF;
                CANTXBuffer[2] = 0xFF;
                CANTXBuffer[1] = 0xFF;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x14) // Drive Temp
            {
                CANTXBuffer[0] = 0x14;
                CANTXBuffer[4] = faultOverTempMOSFET.monitor;
                CANTXBuffer[3] = faultOverTempMOSFET.monitor >> 8;
                CANTXBuffer[2] = 0;
                CANTXBuffer[1] = 0;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x15) // motor RPM
            {
                CANTXBuffer[0] = 0x15;
                // TempVar = FracMpy (FilteredSpeed, Q15(0.30517));
                // TempVar = FracMpy(HallPulsesLatch, Q15(0.83333));
                TempVar = HallPulsesLatch * 10;
                CANTXBuffer[4] = TempVar;
                CANTXBuffer[3] = TempVar >> 8;
                CANTXBuffer[2] = 0;
                CANTXBuffer[1] = 0;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x1f) // Drive status
            {
                CANTXBuffer[0] = 0x1f;
                CANTXBuffer[4] = FaultFlags.MOSOverHeat +
                                 (FaultFlags.Overvoltage << 1) +
                                 (FaultFlags.Undervoltage << 2);
                CANTXBuffer[3] = 0;
                CANTXBuffer[2] = 0;
                CANTXBuffer[1] = uGF.RunMotor;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x22) // ReGen mode
            {
                CANTXBuffer[0] = 0x22;
                if (*(CANRXPtr + 4) == 0)
                {
                    uGF.ReGenSet = 0;
                    uGF.ReGenMode = 0;
                    ReGenTorq = 0;
                }
                else if (*(CANRXPtr + 4) == 1)
                {
                    uGF.ReGenSet = 1;
                    uGF.ReGenMode = 1;
                    ReGenTorq = 2000;
                }
                else if (*(CANRXPtr + 4) == 2)
                {
                    uGF.ReGenSet = 1;
                    uGF.ReGenMode = 2;
                    ReGenTorq = 6553;
                }
                else if (*(CANRXPtr + 4) == 3)
                {
                    uGF.ReGenSet = 1;
                    uGF.ReGenMode = 3;
                    ReGenTorq = 9000;
                }
                CANTXBuffer[4] = *(CANRXPtr + 4);
                CANTXBuffer[3] = *(CANRXPtr + 3);
                CANTXBuffer[2] = *(CANRXPtr + 2);
                CANTXBuffer[1] = *(CANRXPtr + 1);
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x23) // Drive/Assist mode
            {
                CANTXBuffer[0] = 0x23;
                uGF.DriveMode = *(CANRXPtr + 4);
                CANTXBuffer[4] = *(CANRXPtr + 4);
                CANTXBuffer[3] = *(CANRXPtr + 3);
                CANTXBuffer[2] = *(CANRXPtr + 2);
                CANTXBuffer[1] = *(CANRXPtr + 1);
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x24) // Motor drive enable/disable
            {
                CANTXBuffer[0] = 0x24;
                uGF.CAN_Runmotor = *(CANRXPtr + 4);
                CANTXBuffer[4] = *(CANRXPtr + 4);
                CANTXBuffer[3] = *(CANRXPtr + 3);
                CANTXBuffer[2] = *(CANRXPtr + 2);
                CANTXBuffer[1] = *(CANRXPtr + 1);
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else if (*CANRXPtr == 0x30) // None or Error
            {
                CANTXBuffer[0] = 0x30;
                CANTXBuffer[4] = 1;
                CANTXBuffer[3] = 0;
                CANTXBuffer[2] = 0;
                CANTXBuffer[1] = 0;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
            else
            {
                CANTXBuffer[0] = 0x55;
                CAN1_Transmit(1, &My_CAN_TXMSG);
            }
        }
#endif

    }  // inner while loop

    // should never get here
    while (1) {
    }
}  // End of Main loop

// *****************************************************************************
/* Function:
    ResetParmeters()

  Summary:
    This routine resets all the parameters required for Motor

  Description:
    Reinitializes the duty cycle,resets all the counters when restarting motor

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void ResetParmeters(void) {
    /* Make sure ADC does not generate interrupt while initializing parameters*/
    DisableADCInterrupt();

    /* Re initialize the duty cycle to minimum value */
    INVERTERA_PWM_PDC3 = MIN_DUTY;
    INVERTERA_PWM_PDC2 = MIN_DUTY;
    INVERTERA_PWM_PDC1 = MIN_DUTY;
    HAL_MC1PWMDisableOutputs();

    /* enable FOC control loop   */
    uGF.RunMotor = 0;
    /* Set the reference speed value to 0 */
    ctrlParm.qVelRef = 0;

    /* Initialize PI control parameters */
    InitControlParameters();

    faultUndervoltage.counter = 0;
    faultOverCurrent.counter = 0;
    faultOverTempMCU.counter = 0;
    faultOverTempMOSFET.counter = 0;
    faultOverTempMCU.indicator = 0;

    piInputOmega.inReference = 0;
    piInputOmega.inMeasure = 0;

    mcappData.movingAvgFilterSpeed.avg = 0;
    readADCParm.qPrevAnRef = 0;
    pwmDutycycle.dutycycle = 0;

    //  motorStarted = 0;
    // throttleEnabled = 0;
    // fromRegenBrake = 0;
    // testBool = 1;

    // startUpCounter = 0;
    // openLoopCounter = 0;

    /* Enable ADC interrupt and begin main loop timing */
    ClearADCIF();
    adcDataBuffer = ClearADCIF_ReadADCBUF();
    EnableADCInterrupt();
}

// *****************************************************************************
/* Function:
    DoControl()

  Summary:
    Executes one PI iteration for each of the three loops Id,Iq,Speed

  Description:
    This routine executes one PI iteration for each of the three loops
    Id,Iq,Speed

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void DoControl(void) {
    /* 用於計算q參考值平方根的臨時變數 */
    int16_t temp_qref_pow_q15;
    static uint32_t s_u32LastSpeedProfileTimeMs = 0;
    bool bRunSpeedProfileTask = false;

    /* 每2000個計數(約100ms)更新一次霍爾脈衝計數 */
    if (++HallPulsesCntr >= 2000)  // 100ms更新一次
    {
        HallPulsesCntr = 0;
        HallPulsesLatch = HallPulses;  // 鎖存當前速度脈衝數用於輸出
        HallPulses = 0;                // 重置脈衝計數器
    }

    /* 使用一階低通濾波器對速度進行濾波
     * 濾波係數為0.01
     * FilteredSpeed = 0.01 * Speed + 0.99 * FilteredSpeed
     */
    FilteredSpeed =
        FracMpy(Q15(0.01), Speed) + FracMpy(Q15(1 - 0.01), FilteredSpeed);

    /* 確保濾波後的速度不為負值 */
    if (FilteredSpeed < 0)
        FilteredSpeed = 0;
#if CODESW_SPEED_CALCULATION_ENABLE == 1
    g_stSystemData.i16SpeedProtectRaw = abs(Speed);
    g_stSystemData.i16SpeedFiltered = abs(FilteredSpeed);
#endif
    // ----------------------------------------------------------------------------
    /* XIAOMI油門的ADC值範圍為8712~25584 */

    // XIAOMI throttle: 8712~25584
    // Longwin throttle:5912~26880

    // if(Speed < MotorStartSpeed)
    // Torque and Torque_Speed mode
#if 0
    if (uGF.CtrlMode != 0)
    {
        /* 將ADC值減去偏移量9100得到實際參考值 */
        ReferenceRAW = ReferenceRAWADC - 9100;

        /* 當參考值小於0時 */
        if (ReferenceRAW < 0)
        {
            /* 重置所有相關參數為0 */
            ReferenceRAW = 0;
            ReferenceRAWSet = 0;
            piInputOmega.piState.integrator = 0; // 重置速度PI控制器積分項
            piInputIq.inReference = 0;           // 重置扭矩參考值
            ctrlParm.qVqRef = 0;                 // 重置q軸電壓參考值

            /* 若再生制動未啟用,則進入慣性滑行模式 */
            if (uGF.ReGenEnable == 0)
            {
                uGF.Coast = 1;
            }
        }
        /* 當參考值大於0時 */
        else if (ReferenceRAW > 0)
        {
            /* 退出慣性滑行模式 */
            uGF.Coast = 0;

            /* 驅動模式2和3的特殊處理 */
            if (uGF.DriveMode == 2)
            {
                if (ReferenceRAW < Q15(0.999 - 0.0))
                    ReferenceRAW += Q15(0.0); // Bias with a boost
            }
            else if (uGF.DriveMode == 3)
            {
                /* 在參考值未達最大值時增加boost */
                if (ReferenceRAW < Q15(0.999 - 0.0))
                    ReferenceRAW += Q15(0.0); // Bias with a boost
            }
        }
        a_Reg = __builtin_mpy(ReferenceRAW, Q15(0.9), 0, 0, 0, 0, 0, 0);
        ReferenceRAW = __builtin_sacr(a_Reg, 0);
    }
    // Speed mode
    else
    {
        ReferenceRAW = ReferenceRAWADC - 9100;
        if (ReferenceRAW < 0)
            ReferenceRAW = 0;
    }
#endif
    // -------------------------------------------------------------------------------------
    // 油門執行
    // -------------------------------------------------------------------------------------

    if (g_stSystemData.u32TimeMs != s_u32LastSpeedProfileTimeMs) {
        s_u32LastSpeedProfileTimeMs = g_stSystemData.u32TimeMs;
        bRunSpeedProfileTask = true;
    }

    if (bRunSpeedProfileTask) {
    HW_DbgSpeedProfile_High();  // RD13: 標記速度命令處理開始 (油門/VR → 目標/加減速 → ReferenceRAW)

    if (g_stSystemData.bMotorStop == true) {
        // 電池保護已啟動，強制將所有目標歸零並進入滑行模式
        g_stSystemData.i16TargetRpm = 0;
        g_stSystemData.i16CurrentRpm = 0;
        g_stSystemData.i16ActiveRpm = 0;
        uGF.Coast = 1;
    } else {
#if CODESW_THROTTLE_ENABLE == 1
        // 判斷是否可以更新方向
#if CODESW_SCOPE_ENABLE == 1
        if (u16CurrentSpeedKmh_x10 <= CODESW_DIRECTION_CHANGE_SPEED_THRESHOLD) {
#endif
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
            if (g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10 <= CODESW_DIRECTION_CHANGE_SPEED_THRESHOLD) {
#endif
                // 當速度小於 速度限制，才進行方向更新
                // #if CODESW_MOTOR_DIRECTION_DEFAULT == 0

#if CODESW_IFR_DIRECTION_DEFAULT == 0
                g_stSystemData.bMotorDirection = I_FR_SWITCH_PIN;
#endif
#if CODESW_IFR_DIRECTION_DEFAULT == 1
                g_stSystemData.bMotorDirection = !I_FR_SWITCH_PIN;
#endif
                // #else
                // g_stSystemData.bMotorDirection = !I_FR_SWITCH_PIN;
                // #endif
            }
            // 讀取油門電壓並轉換
            g_stSystemData.u16ThrottleVRRaw = (ADCBUF_THROTTLE_VR >> 4);
            g_stSystemData.u16ThrottleVRMv = logic_convert_adcToVoltageMv(g_stSystemData.u16ThrottleVRRaw,
                                                                          THROTTLE_DIVIDER_R1,
                                                                          THROTTLE_DIVIDER_R2);

            // 油門控制邏輯
            uint16_t u16lTargetRpm;
            uint16_t u16lCurrentRpm = (uint16_t)g_stSystemData.i16CurrentRpm;
#if CODESW_SCOPE_ENABLE
            int8_t i8ThrottleResult = logic_throttle_getUpdateParams(&g_stSystemData.sStepTime,
                                                                     &u16lTargetRpm,
                                                                     u16lCurrentRpm,
                                                                     g_stSystemData.u16ThrottleVRMv,  // Pass mV value
                                                                     g_stSystemData.bMotorDirection,
                                                                     THROTTLE_ASSIST_LEVEL_DEFAULT);
#endif
#if CODESW_MODBUS_SCHEDULER_ENABLE
            int8_t i8ThrottleResult = logic_throttle_getUpdateParams(&g_stSystemData.sStepTime,
                                                                     &u16lTargetRpm,
                                                                     u16lCurrentRpm,
                                                                     g_stSystemData.u16ThrottleVRMv,  // Pass mV value
                                                                     g_stSystemData.bMotorDirection,
                                                                     g_stSystemData.sSharedData.u8AssistLevel);

#endif
            if (u16lTargetRpm > 32767) {
                g_stSystemData.i16TargetRpm = 32767;
            } else {
                g_stSystemData.i16TargetRpm = (int16_t)u16lTargetRpm;
            }

            // 處理油門控制結果
            if (i8ThrottleResult == 0)  // 成功
            {
                if ((u16lTargetRpm == 0) && MotorStallShouldForceOutputZeroOnThrottleRelease()) {
                    MotorStallForceOutputZero();
                } else {
                uGF.Coast = 0;
                // 使用馬達控制模組進行漸進式轉速調整
                int8_t i8MotorResult = 0;
                uint16_t u16MotorActiveRpm = 0;  // 使用帶正負號的區域變數
                uint16_t u16lThrottleOutputMax = 0;
                uint16_t u16lThrottleOutputMin = 0;

                if (g_stSystemData.bMotorDirection == 0) {
                    u16lThrottleOutputMax = LOGIC_THROTTLE_FWD_OUTPUT_MAX;
                    u16lThrottleOutputMin = LOGIC_THROTTLE_FWD_OUTPUT_MIN;
                } else {
                    u16lThrottleOutputMax = LOGIC_THROTTLE_REV_OUTPUT_MAX;
                    u16lThrottleOutputMin = LOGIC_THROTTLE_REV_OUTPUT_MIN;
                }

                // 更新馬達最後要執行的轉速
                // 注意：此處呼叫舊的無正負號函式，因此傳入 abs() 值並強制轉型指標

                i8MotorResult = logic_motor_getUpdateParams(&u16MotorActiveRpm,
                                                            g_stSystemData.u32TimeMs,
                                                            u16lCurrentRpm,
                                                            u16lTargetRpm,
                                                            u16lThrottleOutputMax,  // 最大轉速限制
                                                            u16lThrottleOutputMin,  // 最小轉速限制
                                                            &g_stSystemData.sStepTime);

                if (i8MotorResult == 0)  // 馬達控制成功
                {
                    // 更新當前轉速
                    g_stSystemData.i16CurrentRpm = (int16_t)u16MotorActiveRpm;
                    g_stSystemData.i16ActiveRpm = (int16_t)u16MotorActiveRpm;
                }
                }
            }
            // 將安全的方向設定應用到馬達控制器
#if CODESW_MOTOR_DIRECTION_DEFAULT == 0
            if (g_stSystemData.bMotorDirection == 0)
                uGF.DirSW = 0;
            else
                uGF.DirSW = 1;
#else
    if (g_stSystemData.bMotorDirection == 0)
        uGF.DirSW = 1;
    else
        uGF.DirSW = 0;
#endif
#endif
            // -------------------------------------------------------------------------------------
            // VR執行
            // -------------------------------------------------------------------------------------

#if CODESW_VR_ENABLE == 1
            // ===== VR 控制模式 (使用帶正負號RPM模型) =====

            // 1. 準備 `logic_vr_getUpdateParams` 所需的參數
            g_stSystemData.u16ThrottleVRRaw = (ADCBUF_THROTTLE_VR >> 4);
            uint16_t u16lCurrentSpeedKmh_x10;
#if CODESW_SCOPE_ENABLE == 1
            u16lCurrentSpeedKmh_x10 = u16CurrentSpeedKmh_x10;
#else
    u16lCurrentSpeedKmh_x10 = g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10;
#endif

            // 2. 呼叫VR邏輯模組，獲取經過安全檢查後的目標轉速 (i16TargetRpm) 和 Step/Time 參數
            int8_t i8VrResult = logic_vr_getUpdateParams(&g_stSystemData.sStepTime,
                                                         &g_stSystemData.i16TargetRpm,
                                                         g_stSystemData.i16CurrentRpm,
                                                         g_stSystemData.u16ThrottleVRRaw,
                                                         u16lCurrentSpeedKmh_x10);

            if (i8VrResult == 0)  // 成功讀取VR參數
            {
                uGF.Coast = 0;
                int16_t i16MotorActiveRpm = 0;

                // 3. 呼叫馬達斜率控制模組，讓當前轉速(i16CurrentRpm)平滑地趨近目標轉速(i16TargetRpm)
                //    注意：這裡假設 logic_motor_getUpdateParams 已被修改或重載以處理 int16_t
                int8_t i8MotorResult = logic_motor_getUpdateParamsSigned(&i16MotorActiveRpm,
                                                                         g_stSystemData.u32TimeMs,
                                                                         g_stSystemData.i16CurrentRpm,
                                                                         g_stSystemData.i16TargetRpm,
                                                                         &g_stSystemData.sStepTime);

                if (i8MotorResult == 0) {
                    // 4. 更新系統中的當前轉速和活動轉速
                    g_stSystemData.i16CurrentRpm = i16MotorActiveRpm;
                    g_stSystemData.i16ActiveRpm = i16MotorActiveRpm;
                }
            } else  // 讀取VR參數失敗或處於抑制狀態
            {
                uGF.Coast = 1;  // 進入滑行模式
                g_stSystemData.i16TargetRpm = 0;
                g_stSystemData.i16CurrentRpm = 0;
                g_stSystemData.i16ActiveRpm = 0;
            }

            // 5. 設定底層方向旗標 (uGF.DirSW)
            //    0 = 正轉, 1 = 反轉
            uGF.DirSW = (g_stSystemData.i16ActiveRpm < 0) ? 1 : 0;

#endif
        }

#if CODESW_THROTTLE_ENABLE == 1 || CODESW_VR_ENABLE == 1
        // 當(IEMB)被拉起時，強制將所有馬達指令歸零，防止馬達輸出
#if CODESW_EMBRAKER_ENABLE
        // 根據電磁煞車模組的狀態來決定是否要切斷馬達動力
        // 這將取代舊有的分散式檢查，讓 s_logic_embraker 成為唯一的邏輯來源
        {
            E_EMBRAKER_STATE emb_state = logic_embraker_getStatus();
            if (emb_state == EMBRAKER_STATE_FAULT) {
                MotorStallForceOutputZero();
            } else if ((emb_state == EMBRAKER_STATE_LOCKED) &&
                       (g_stSystemData.i16TargetRpm == 0)) {
                MotorStallForceOutputZero();
            }
        }
#endif

        // 將要執行的送到 ReferenceRAW 進行執行
        ReferenceRAW = g_stSystemData.i16ActiveRpm;

#endif

        HW_DbgSpeedProfile_Low();  // RD13: 標記速度命令處理結束

        // if(Speed < BrakeStopSpeed)
        if (HallPulsesLatch < BrakeStopSpeedPulses)
            uGF.ReGenBlock = 1;
        // else if(Speed > BrakeStartSpeed)
        else if (HallPulsesLatch > BrakeStartSpeedPulses)
            uGF.ReGenBlock = 0;
        uGF.ReGenEnable = uGF.ReGenBlock ^ uGF.ReGenSet;
        // #if CODESW_THROTTLE_ENABLE == 1
        if (uGF.DirSW == 1) {
            ReferenceRAW = -abs(ReferenceRAW);
        }
// #endif
// ---------------------------------------------------------
#if CODESW_THROTTLE_ENABLE == 1 || CODESW_VR_ENABLE == 1
        uGF.ReGenFlag = 0;  //

        // 注意：Real RPM 模式的 PI 控制器已在前面處理完成
        // 這裡只需要處理 Q15 模式和其他模式

        ctrlParm.qVelRef = ReferenceRAW;

        // 使用原有的 Q15 格式 Speed
        piInputOmega.inMeasure = Speed;
        piInputOmega.inReference = ctrlParm.qVelRef;

        // [FIX] 鬆油門邊緣(TargetRpm 非0→0)清一次速度環積分器：
        //   行駛時積分器累積正值以維持前進；鬆油門後若不清，低速時 Speed 逾時歸零
        //   → error=0 → PI 輸出=殘留正積分 → 微幅前進 creep。只在放開瞬間清一次即可，
        //   停車階段積分只會往煞車方向累積、不會再產生前進命令，故不需每 cycle 清。
        {
            static bool s_bPrevThrottleReleased = false;
            bool bThrottleReleased = (g_stSystemData.i16TargetRpm == 0);
            if (bThrottleReleased && !s_bPrevThrottleReleased) {
                piInputOmega.piState.integrator = 0;  // 速度環:每次放開都清(消前進 creep)
                // 註:電流環積分器不在此清(有速度放開會造成扭力瞬斷/抖動)。頂牆放開的反向
                //   驅動改由輸出級「近停放開強制 coast」處理(見下方 bStallReleaseCoast)。
            }
            s_bPrevThrottleReleased = bThrottleReleased;
        }

        MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                       piInputOmega.inMeasure, &piInputOmega.piState,
                                       &piOutputOmega.out);
        ctrlParm.qVqRef = piOutputOmega.out;
#if 0
#if CODESW_TEMPERATURE_CONTROLLER_ENABLE == 1
        /*
         * 套用溫度保護限制：
         * 速度環(Speed Loop)計算出的扭矩參考(ctrlParm.qVqRef)必須被限制在
         * 溫度保護模組所設定的最大允許扭矩(TorqMode_IqMax)之內。
         * 這可以防止在速度模式下，因溫度過高而損壞硬體。
         */
        if (ctrlParm.qVqRef > TorqMode_IqMax) {
            ctrlParm.qVqRef = TorqMode_IqMax;  // 限制正向扭矩
        } else if (ctrlParm.qVqRef < -TorqMode_IqMax) {
            ctrlParm.qVqRef = -TorqMode_IqMax;  // 限制反向扭矩 (例如煞車)
        }
#endif
#endif
        // 當速度過低時鎖定UVW相位
        // [FIX] 解除條件加入油門目標(i16TargetRpm)判斷：
        //   原本僅看「斜坡後的命令」(inReference = ActiveRpm) 與實際速度，
        //   導致減速近停時再催油門，必須等 ActiveRpm 沿斜坡慢慢爬過 UVWLockSpeed
        //   才會解除短路 → 表現為「油門沒反應，要等停車程序跑完」。
        //   改為：只要駕駛重新給出明顯的油門目標(TargetRpm >= 門檻)，立即解除 UVW
        //   短路，恢復即時加速反應。單純停車時 TargetRpm≈0，行為與原本相同。
#if CODESW_UVW_LOCK_ENABLE
        // [FIX] 進/出鎖改為命令驅動的遲滯 (hysteresis)，解決零速 hunting 死結：
        //   舊版把 idq.q/idq.d(量測電流)放進進場條件，但空載 0 命令下 FOC 會持續
        //   灌電流對抗左右抖動，idq 不斷超標 → 計數器每 cycle 歸零 → UVWLock 永遠
        //   latch 不了 → 短路接不上 → 抖動持續。唯一能讓馬達安靜的就是短路本身。
        //   新規則(依需求)：速度命令已到 0 且實際速度接近 0，即可進行 UVW lock。
        //     進鎖 — ReferenceRAW/qVelRef==0(命令已歸零) 且 已接近靜止，即立即接上
        //            短路(已移除 30ms debounce)；不再看 idq。
        //     出鎖 — 僅在駕駛重新給出明顯油門(|TargetRpm| >= UVW_LOCK_RELEASE_REF)時
        //            解除，形成遲滯，避免抖動的瞬間量測值把鎖解開。
        // [FIX] 靜止判斷改用霍爾脈衝數 HallPulsesLatch，不用瞬時 Speed：
        //   停車顫動時車輪在單一霍爾邊界來回，會不斷產生霍爾邊緣，
        //   (1) 每個邊緣把 T1INTCnt 清零 → Speed 的「逾時歸零」路徑永不觸發；
        //   (2) 極短的顫動週期 (HallPeriod<=HallMinPeriod) 會把 Speed 直接拉到 32767。
        //   → abs(Speed) 幾乎永遠大於門檻，進鎖條件永遠不成立，UVWLock
        //     永遠 latch 不上 → 相位接不上短路 → 持續前後抽動 (死結)。
        //   HallPulsesLatch(每 100ms 脈衝數)是邊緣「計數」而非「數值」，對 32767
        //   尖峰免疫；單邊界顫動的淨脈衝數很低，真正滑行才會高，剛好能區分。
        //   用專屬門檻 UVW_LOCK_STOP_PULSES 作為「已接近靜止」判斷 (獨立於 ReGen 的
        //   BrakeStopSpeedPulses，調整不會影響 ReGen 起煞點)。
        bool bUvwStopCommanded = (ReferenceRAW == 0) &&
                                 (ctrlParm.qVelRef == 0) &&
                                 (HallPulsesLatch < UVW_LOCK_STOP_PULSES);
        bool bUvwDriveRequested = (abs(g_stSystemData.i16TargetRpm) >= UVW_LOCK_RELEASE_REF);

        if (uGF.UVWLock == 1) {
            // 已鎖定：維持到駕駛重新給出明顯油門(遲滯)
            if (bUvwDriveRequested) {
                uGF.UVWLock = 0;
            }
        } else if (bUvwStopCommanded && !bUvwDriveRequested) {
            // 命令歸零且低速：立即進鎖 (已移除 30ms debounce)
            uGF.UVWLock = 1;
        }
#else
        uGF.UVWLock = 0;
#endif

#endif
    }
#if 0
    // 0:Speed Mode 1: Torque mode 2: Speed control, torque limit
    if (uGF.CtrlMode == 0)
    {
        uGF.ReGenFlag = 0;         // used in torque mode
        if (++SpeedLoopCntr >= 10) // 每10次執行一次速度控制迴圈
        {
            SpeedLoopCntr = 0; // 重置速度控制計數器

            // 速度斜率控制,用於平滑加減速
            if (++SpeedSlopCntr >= SpeedSlopCntrSet)
            {
                SpeedSlopCntr = 0;
                // 根據目標速度和當前速度參考值調整加減速
                if (ctrlParm.qVelRef < ReferenceRAW)
                    ctrlParm.qVelRef += AccSet; // 加速
                else
                    ctrlParm.qVelRef -= DeAccSet; // 減速
            }

            // 速度PI控制器的輸入設定
            piInputOmega.inMeasure = Speed;              // 實際速度
            piInputOmega.inReference = ctrlParm.qVelRef; // 目標速度

            // 限制速度參考值在允許範圍內
            if (piInputOmega.inReference > SpeedModeCtrlLimit)
                piInputOmega.inReference = SpeedModeCtrlLimit;
            else if (piInputOmega.inReference < -SpeedModeCtrlLimit)
                piInputOmega.inReference = -SpeedModeCtrlLimit;

            // 執行PI控制器運算
            MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                           piInputOmega.inMeasure,
                                           &piInputOmega.piState,
                                           &piOutputOmega.out);

            // 設定q軸電流參考值為PI控制器輸出
            ctrlParm.qVqRef = piOutputOmega.out;

            // 當速度過低時鎖定UVW相位
            if ((abs(piInputOmega.inReference) < UVWLockSpeed) &&
                (abs(piInputOmega.inMeasure) < UVWLockSpeed))
                uGF.UVWLock = 1;
            else
                uGF.UVWLock = 0;
        }
    }
    else if ((uGF.ReGenEnable == 1) && (ReferenceRAW == 0)) // ReGen braking
    {
        // Enabled when speed is positive
        if ((Speed >= BrakeStartSpeed) && (uGF.DirSW == 0)) // Faster response
        // if(HallPulsesLatch >= BrakeStartSpeedPulses)  // slow response !!
        {
            ctrlParm.qVqRef = -ReGenTorq;

            uGF.ReGenFlag = 0;
        }
        else if ((Speed <= -BrakeStartSpeed) && (uGF.DirSW == 1))
        {
            ctrlParm.qVqRef = ReGenTorq;
            uGF.ReGenFlag = 0;
        }
        // else if (HallPulsesLatch < BrakeStopSpeedPulses)
        else if ((Speed < BrakeStopSpeed) && (uGF.DirSW == 0))
        {
            ctrlParm.qVqRef = 0;
        }
        else if ((Speed > -BrakeStopSpeed) && (uGF.DirSW == 1))
        {
            ctrlParm.qVqRef = 0;
        }
#ifdef MotorReGenLock
        if (abs(Speed) < ReGenSpeed)
        {
            ctrlParm.qVqRef = 0;
            ReferenceRAW = 0;
            uGF.ReGenFlag = 1; // Short U V W for braking
        }
#endif
        ReferenceRAWSet = 0; // Reset slop control
    }
    else if (uGF.CtrlMode == 2) // Speed control, torque limit
    {
        // uGF.ReGenFlag = 0;      // used in torque mode
        if (++SpeedLoopCntr >= 10)
        {
            SpeedLoopCntr = 0;

            piInputOmega.inMeasure = Speed;
            if (uGF.DirSW == 0)
            {
                piInputOmega.inReference = SpeedCtrlLimit;
                if (piInputOmega.inMeasure > piInputOmega.inReference)
                    piInputOmega.piState.integrator = piInputOmega.piState.integrator >> 1;
                MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                               piInputOmega.inMeasure,
                                               &piInputOmega.piState,
                                               &piOutputOmega.out);
                // Torque slop control
                if (TorqAccCntr++ >= TorqAccCntrSet)
                {
                    TorqAccCntr = 0;
                    if (ReferenceRAWSet < ReferenceRAW)
                        ReferenceRAWSet += ReferenceRAWSetStep;
                    else
                        ReferenceRAWSet -= ReferenceRAWSetStep;
                }
                if (piOutputOmega.out > ReferenceRAWSet)
                    piOutputOmega.out = ReferenceRAWSet;

                ctrlParm.qVqRef = piOutputOmega.out;
                if (ctrlParm.qVqRef > TorqMode_IqMax)
                    ctrlParm.qVqRef = TorqMode_IqMax;
            }
            else
            {
                piInputOmega.inReference = -SpeedCtrlLimit;
                if (piInputOmega.inMeasure < piInputOmega.inReference)
                    piInputOmega.piState.integrator = piInputOmega.piState.integrator >> 1;
                MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                               piInputOmega.inMeasure,
                                               &piInputOmega.piState,
                                               &piOutputOmega.out);
                // Torque slop control
                if (TorqAccCntr++ >= TorqAccCntrSet)
                {
                    TorqAccCntr = 0;
                    if (ReferenceRAWSet < ReferenceRAW)
                        ReferenceRAWSet += ReferenceRAWSetStep;
                    else
                        ReferenceRAWSet -= ReferenceRAWSetStep;
                }

                if (piOutputOmega.out < ReferenceRAWSet)
                    piOutputOmega.out = ReferenceRAWSet;

                ctrlParm.qVqRef = piOutputOmega.out;
                if (ctrlParm.qVqRef < -TorqMode_IqMax)
                    ctrlParm.qVqRef = -TorqMode_IqMax;
            }

            if ((abs(piInputOmega.inReference) < UVWLockSpeed) && (abs(piInputOmega.inMeasure) < UVWLockSpeed))
                uGF.UVWLock = 1;
            else
                uGF.UVWLock = 0;
        }
    }
    else
    { // Normal torque control loop
        if (ctrlParm.qVqRef < ReferenceRAW)
        {
            if (TorqAccCntr++ >= TORQ_ACCDELAY)
            {
                if (uGF.DriveMode == 1)
                {
                    ctrlParm.qVqRef += 2;
                    TorqAccCntr = 0;
                    TorqMode_IqMax = Q15(0.6);
                }
                else if (uGF.DriveMode == 2)
                {
                    ctrlParm.qVqRef += 4;
                    TorqAccCntr = 0;
                    TorqMode_IqMax = Q15(0.65);
                }
                else if (uGF.DriveMode == 3)
                {
                    ctrlParm.qVqRef += 20;
                    TorqMode_IqMax = Q15(0.75);
                    TorqAccCntr = 0;
                }
            }
        }
        else
            ctrlParm.qVqRef -= 1;

        if (ctrlParm.qVqRef > TorqMode_IqMax)
            ctrlParm.qVqRef = TorqMode_IqMax;
        else if (ctrlParm.qVqRef < -TorqMode_IqMax)
            ctrlParm.qVqRef = -TorqMode_IqMax;
    }
    // Reduce output torque when MOSFET Temp. > 73`C
    if (faultOverTempMOSFET.monitor < OVERTEMP_MOSFET_73)
    {
        if (ctrlParm.qVqRef > Q15(0.3))
            ctrlParm.qVqRef = Q15(0.3);
        else if (ctrlParm.qVqRef < -Q15(0.3))
            ctrlParm.qVqRef = -Q15(0.3);
    }
#endif
        ctrlParm.qVdRef = 0;

        /* PI control for D */
        piInputId.inMeasure = idq.d;
        piInputId.inReference = ctrlParm.qVdRef;
        MC_ControllerPIUpdate_Assembly(piInputId.inReference, piInputId.inMeasure,
                                       &piInputId.piState, &piOutputId.out);
        vdq.d = piOutputId.out;

        /* Dynamic d-q adjustment with d component priority*/
        // Vector limitation
        // Vd is not limited
        // Vq is limited so the vector Vs is less than a maximum of 95%.
        // Vs = SQRT(Vd^2 + Vq^2) < 0.98
        // Vq = SQRT(0.98^2 - Vd^2)
        temp_qref_pow_q15 =
            (int16_t)(__builtin_mulss(piOutputId.out, piOutputId.out) >> 15);
        temp_qref_pow_q15 = Q15(MAX_VOLTAGE_VECTOR) - temp_qref_pow_q15;
        piInputIq.piState.outMax = Q15SQRT(temp_qref_pow_q15);
        piInputIq.piState.outMin = -piInputIq.piState.outMax;

        /* PI control for Q */
        piInputIq.inMeasure = idq.q;
        piInputIq.inReference = ctrlParm.qVqRef;
        MC_ControllerPIUpdate_Assembly(piInputIq.inReference, piInputIq.inMeasure,
                                       &piInputIq.piState, &piOutputIq.out);
        vdq.q = piOutputIq.out;

        IqSquareIntegral();  // Overheat by Iq Integral
    }
    // *****************************************************************************
    /* Function:
        _ADCAN17Interrupt()
      Summary:
        _ADCAN17Interrupt() ISR routine

      Description:
        Does speed calculation and executes the vector update loop
        The ADC sample and conversion is triggered by the PWM period.

      Precondition:
        None.

      Parameters:
        None

      Returns:
        None.

      Remarks:
        None.
     */

    void __attribute__((__interrupt__, no_auto_psv)) _ADCAN17Interrupt() {
        HW_DbgAdcIsr_High();  // RC13: 標記 ADC ISR 進入 (示波器量測執行時間)

        /* 微秒計數器遞增 */
        g_stSystemData.u16TimeUs++;

        /* 當微秒計數達到1毫秒時 */
        if (g_stSystemData.u16TimeUs >= TIMER_COUNT_1MS) {
            /* 重置微秒計數器 */
            g_stSystemData.u16TimeUs = 0;

            /* 毫秒計數器遞增 */
            g_stSystemData.u32TimeMs++;
        }

        // ? --------------------------------------------------------------
        PollingCntr++;
#if CODESW_UNIFIED_LED_LOGIC_ENABLE == 0
        LEDFlashCntr++;
        FaultLEDFLashCntr++;
#endif
        // Read ADC Buffer to Clear Flag
        adcDataBuffer = ClearADCIF_ReadADCBUF();

#if CODESW_THROTTLE_ENABLE
#else
    // Read unsigned values
    ReadADC0(ADCBUF_SPEED_REF_A, &readADCParm);
#endif

        CNRead_Inline();

        // Calculate qIa,qIb

        MeasCompCurr(ADCBUF_INV_A_IPHASE1, ADCBUF_INV_A_IPHASE2, &measCurrParm);
        // modified for LONGWIN H/W
        iabc.b = measCurrParm.qIa;
        iabc.c = measCurrParm.qIb;
        iabc.a = -(iabc.b + iabc.c);

        // Ibus = (long)(abs(iabc.a)+ abs(iabc.b)+abs(iabc.c)) >> 1;
        //  Calculate qIalpha,qIbeta from qIa,qIb
        IbusCalc();
        MC_TransformClarke_Assembly(&iabc, &ialphabeta);

        // Calculate qId,qIq from qSin,qCos,qIalpha,qIbeta
        MC_TransformPark_Assembly(&ialphabeta, &sincosTheta, &idq);

        // Calculate control values
        DoControl();

        CalculateParkAngleHall();

        // Calculate qSin,qCos from the thetaElectrical
        MC_CalculateSineCosine_Assembly_Ram(thetaElectrical, &sincosTheta);

        // Calculate qValpha,qVbeta from qSin,qCos,qVd,qVq
        MC_TransformParkInverse_Assembly(&vdq, &sincosTheta, &valphabeta);

        // Calculate qVa,qVb,qVc vectors from qValpha,qVbeta
        MC_TransformClarkeInverseSwappedInput_Assembly(&valphabeta, &vabc);

        // Generate SV-PWM from the voltage vectors and PWM frequency
        MC_CalculateSpaceVectorPhaseShifted_Assembly(&vabc, pwmPeriod, &pwmDutycycle);

        //    if (pwmDutycycle.dutycycle1 < MIN_DUTY)
        //    {
        //        pwmDutycycle.dutycycle1 = MIN_DUTY;
        //    }
        //    if (pwmDutycycle.dutycycle2 < MIN_DUTY)
        //    {
        //        pwmDutycycle.dutycycle2 = MIN_DUTY;
        //    }
        //    if (pwmDutycycle.dutycycle3 < MIN_DUTY)
        //    {
        //        pwmDutycycle.dutycycle3 = MIN_DUTY;
        //    }

        // [A] 頂牆/堵轉放開後,近停且尚未 UVW 短路時,強制 coast(關 PWM)不讓 FOC 主動輸出,
        //   避免「零速→反向」交界的換相把馬達往後主動驅動(runaway 後退 1m+)。UVW lock 一armed
        //   即由下方短路分支接手保持。只在速度模式、油門已放開、且近停時作用 → 不影響有速度時的煞車。
        bool bStallReleaseCoast = (uGF.CtrlMode == 0) &&
                                  (g_stSystemData.i16TargetRpm == 0) &&
                                  (uGF.UVWLock == 0) &&
                                  (HallPulsesLatch < UVW_LOCK_STOP_PULSES);
#if CODESW_BATTERY_PROTECTION_MODULE_ENABLE
        // 使用 s_logic_battery 模組的輸出禁制旗標來決定是否關閉 PWM
        if ((uGF.RunMotor == 0) || (uGF.Fault == 1) || (uGF.Coast == 1) || bStallReleaseCoast || logic_battery_shouldProhibitOutput()) {
#else
    if ((uGF.RunMotor == 0) || (uGF.Fault == 1) || (uGF.Coast == 1) || bStallReleaseCoast) {
#endif
            HAL_MC1PWMDisableOutputs();
            piInputOmega.piState.integrator = 0;
            piInputIq.piState.integrator = 0;
            piInputId.piState.integrator = 0;
        }
#if CODESW_UVW_LOCK_ENABLE
        else if ((uGF.UVWLock == 1) &&
                   (uGF.CtrlMode == 0))  // Works only in speed mode
        {
            // low side turns on, high side off
            PG3IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM3H,L, if Override is Enabled
            PG2IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM2H,L, if Override is Enabled
            PG1IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM1H,L, if Override is Enabled

            PG3IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM3H
            PG3IOCONLbits.OVRENL =
                1;                     // 1 = PWM Generator provides data for output on PWM3L
            PG2IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM2H
            PG2IOCONLbits.OVRENL =
                1;                     // 1 = PWM Generator provides data for output on PWM2L
            PG1IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM1H
            PG1IOCONLbits.OVRENL =
                1;  // 1 = PWM Generator provides data for output on PWM1L

            piInputOmega.piState.integrator = 0;
            piInputIq.piState.integrator = 0;
            piInputId.piState.integrator = 0;
        }
#endif
//------------------------------------------------------------------------------------
//   For ReGen mode, the motor will be locked when the speed is less than
//   ReGenSpeed
//------------------------------------------------------------------------------------
#ifdef MotorReGenLock
        else if ((uGF.ReGenFlag == 1) &&
                 (abs(Speed) < ReGenSpeed))  // Motor UVW shorted
        {

            // low side turns on, high side off
            PG3IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM3H,L, if Override is Enabled
            PG2IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM2H,L, if Override is Enabled
            PG1IOCONLbits.OVRDAT =
                1;  // 0b00 = State for PWM1H,L, if Override is Enabled

            PG3IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM3H
            PG3IOCONLbits.OVRENL =
                0;                     // 0 = PWM Generator provides data for output on PWM3L
            PG2IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM2H
            PG2IOCONLbits.OVRENL =
                0;                     // 0 = PWM Generator provides data for output on PWM2L
            PG1IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM1H
            PG1IOCONLbits.OVRENL =
                0;  // 0 = PWM Generator provides data for output on PWM1L

            INVERTERA_PWM_PDC3 = 0;  // "High side" duty
            INVERTERA_PWM_PDC2 = 0;
            INVERTERA_PWM_PDC1 = 0;

            piInputOmega.piState.integrator = 0;
            piInputIq.piState.integrator = 0;
            piInputId.piState.integrator = 0;
        }
#endif
        //------------------------------------------------------------------------------
        else {
            INVERTERA_PWM_PDC3 = pwmDutycycle.dutycycle1;  // Phase A
            INVERTERA_PWM_PDC2 = pwmDutycycle.dutycycle3;  // Phase C
            INVERTERA_PWM_PDC1 = pwmDutycycle.dutycycle2;  // Phase B
            HAL_MC1PWMEnableOutputs();
        }

#if !CODESW_BATTERY_PROTECTION_MODULE_ENABLE
        OvervoltageDetect();
        // if(startUpCounter > 1000)
        //     UndervoltageDetect();
#endif

        // OvertemperatureDetectMCU();
        OvertemperatureDetectMOSFET();
#if CODESW_SCOPE_ENABLE == 1
        X2CPG1Duty = INVERTERA_PWM_PDC1;
        X2CPG2Duty = INVERTERA_PWM_PDC2;
        X2CPG3Duty = INVERTERA_PWM_PDC3;
        DiagnosticsStepIsr();
#endif
        // Clear Interrupt Flag
        ClearADCIF();

        HW_DbgAdcIsr_Low();  // RC13: 標記 ADC ISR 離開 (示波器量測執行時間)
    }

    // *****************************************************************************
    /* Function:
        InitControlParameters()

      Summary:
        Function initializes control parameters

      Description:
        Initialize control parameters: PI coefficients, scaling constants etc.

      Precondition:
        None.

      Parameters:
        None

      Returns:
        None.

      Remarks:
        None.
     */
    void InitControlParameters(void) {
        /* ADC - Measure Current & Pot */
        /* Scaling constants: Determined by calibration or hardware design.*/
        readADCParm.qK = KPOT;
        measCurrParm.qKa = KCURRA;
        measCurrParm.qKb = KCURRB;

        //    ctrlParm.qRefRamp = SPEEDREFRAMP;
        //    ctrlParm.speedRampCount = SPEEDREFRAMP_COUNT;

        /* Set PWM period to Loop Time */
        pwmPeriod = LOOPTIME_TCY;

        /* PI - Id Current Control */
        piInputId.piState.kp = D_CURRCNTR_PTERM;
        piInputId.piState.ki = D_CURRCNTR_ITERM;
        piInputId.piState.kc = D_CURRCNTR_CTERM;
        piInputId.piState.outMax = D_CURRCNTR_OUTMAX;
        piInputId.piState.outMin = -piInputId.piState.outMax;
        piInputId.piState.integrator = 0;
        piOutputId.out = 0;

        /* PI - Iq Current Control */
        piInputIq.piState.kp = Q_CURRCNTR_PTERM;
        piInputIq.piState.ki = Q_CURRCNTR_ITERM;
        piInputIq.piState.kc = Q_CURRCNTR_CTERM;
        piInputIq.piState.outMax = Q_CURRCNTR_OUTMAX;
        piInputIq.piState.outMin = -piInputIq.piState.outMax;
        piInputIq.piState.integrator = 0;
        piOutputIq.out = 0;

        /* PI - Speed Control */
        piInputOmega.piState.kp = SPEEDCNTR_PTERM;
        piInputOmega.piState.ki = SPEEDCNTR_ITERM;
        piInputOmega.piState.kc = SPEEDCNTR_CTERM;
        piInputOmega.piState.outMax = SPEEDCNTR_OUTMAX;
        piInputOmega.piState.outMin = -piInputOmega.piState.outMax;
        piInputOmega.piState.integrator = 0;
        piOutputOmega.out = 0;
    }

    // ************************************************************************
    /* Function:
        MeasCurrOffset()

      Summary:
        Routine initializes Offset values of current
      Precondition:
        None.

      Parameters:
        None

      Returns:
        None.

      Remarks:
        None.
     */
    void MeasCurrOffset(int16_t *pOffseta, int16_t *pOffsetb) {
        int32_t adcOffsetIa = 0, adcOffsetIb = 0;
        uint16_t i = 0;

        /* Enable ADC interrupt and begin main loop timing */
        ClearADCIF();
        adcDataBuffer = ClearADCIF_ReadADCBUF();

        /* Taking multiple sample to measure voltage offset in all the channels */

        for (i = 0; i < (1 << CURRENT_OFFSET_SAMPLE_SCALER); i++) {
            while (!_ADCAN17IF);
            // measCurrOffsetFlag = 0;
            /* Wait for the conversion to complete */
            // while (measCurrOffsetFlag == 0);
            /* Sum up the converted results */
            adcOffsetIa += (int16_t)ADCBUF_INV_A_IPHASE1;
            adcOffsetIb += (int16_t)ADCBUF_INV_A_IPHASE2;
            ClearADCIF();
        }
        /* Averaging to find current Ia offset */
        *pOffseta = (int16_t)(adcOffsetIa >> CURRENT_OFFSET_SAMPLE_SCALER);
        /* Averaging to find current Ib offset*/
        *pOffsetb = (int16_t)(adcOffsetIb >> CURRENT_OFFSET_SAMPLE_SCALER);
        measCurrOffsetFlag = 0;
    }

    // void __attribute__((interrupt, no_auto_psv)) _CNCInterrupt()
    //{
    //     CNRead_Inline();
    // }

    void CMP1_ISR(void) {
        if (faultOverCurrent.counter > OVERCURRENT_COUNTER) {
            ResetParmeters();
            faultOverCurrent.counter = 0;
        }
        faultOverCurrent.counter++;
    }

    void UndervoltageDetect(void) {
        faultUndervoltage.measure = ADCBUF_VOLTAGE;
        faultUndervoltage.monitor = (faultUndervoltage.measure >> 6);

        if (faultUndervoltage.monitor < VOLTAGE_LIMITER) {
            faultUndervoltage.counter++;

            if (faultUndervoltage.counter > UNDERVOLTAGE_COUNTER) {
                ResetParmeters();
                faultUndervoltage.counter = 0;
            }
        }
    }
    void VoltageDetect(void) {
        faultUndervoltage.measure = ADCBUF_VOLTAGE;
    }
    void OvervoltageDetect(void) {
        faultOvervoltage.measure = ADCBUF_VOLTAGE;
        faultOvervoltage.monitor = (faultOvervoltage.measure >> 6);
        // Adjust the voltage ADC. value
        a_Reg =
            __builtin_mpy(faultOvervoltage.monitor, Q15(0.9807), 0, 0, 0, 0, 0, 0);
        faultOvervoltage.monitor = __builtin_sacr(a_Reg, 0);
        if (faultOvervoltage.monitor > OVERVOLTAGE_LIMITER) {
            faultOvervoltage.counter++;

            if (faultOvervoltage.counter > OVERVOLTAGE_COUNTER) {
                faultOvervoltage.counter = OVERVOLTAGE_COUNTER;
                FaultFlags.Overvoltage = 1;
                uGF.Fault = 1;
            }
        }
    }
    void OvertemperatureDetectMCU(void) {
        /*
        faultOverTempMCU.measure = ADCBUF_TEMPERATURE_MCU;
        faultOverTempMCU.monitor = (faultOverTempMCU.measure >> 6);

        if(faultOverTempMCU.monitor < OVERTEMP_LIMITER_57)
        {
            faultOverTempMCU.counter++;

            if(faultOverTempMCU.counter > OVERTEMP_COUNTER)
            {
                ResetParmeters();
                faultOverTempMCU.counter = 0;
            }
        }

        if(faultOverTempMCU.monitor < OVERTEMP_LIMITER_35)
        {
            faultOverTempMCU.counter++;

            if(faultOverTempMCU.counter> OVERTEMP_COUNTER)
            {
                faultOverTempMCU.indicator = (faultOverTempMCU.monitor >> 1);
            }
        }
        */
    }

    void OvertemperatureDetectMOSFET(void) {
        faultOverTempMOSFET.measure = ADCBUF_TEMPERATURE_MOSFET;
        faultOverTempMOSFET.monitor = (faultOverTempMOSFET.measure >> 6);

        if (faultOverTempMOSFET.monitor < MOSFET_OverTemp) {
            faultOverTempMOSFET.counter++;

            if (faultOverTempMOSFET.counter > OVERTEMP_COUNTER) {
                // ResetParmeters();
                faultOverTempMOSFET.counter = 0;
                FaultFlags.MOSOverHeat = 1;
                uGF.Fault = 1;
            }
        }
    }

    void CNRead_Inline(void) {
        HallState = (unsigned int)I_HALL_U_PIN + ((unsigned int)I_HALL_V_PIN * 2) +
                    ((unsigned int)I_HALL_W_PIN * 4);
        if (OldHallState != HallState) {
            TMRLatch = TMR1;
            TMR1 = 0;
            T1INTCnt = 0;
            HallPeriod = TMRLatch;
            GetHallAngleAuto_Inline();  // OldHallState updates here.
            // PeriodTemp += ((long int)TMR2Latch - (long int)PeriodTemp) * ;

            DeltaT_Array[DeltaT_Index++] = HallPeriod;
            if (DeltaT_Index >= 8)
                DeltaT_Index = 0;
            DeltaT_Sum = DeltaT_Array[0] + DeltaT_Array[1] + DeltaT_Array[2] +
                         DeltaT_Array[3] + DeltaT_Array[4] + DeltaT_Array[5] +
                         DeltaT_Array[6] + DeltaT_Array[7];
            PeriodAverage = DeltaT_Sum >> 3;

            // Used by SVPWN angle
            HallPeriodFiltered = PeriodAverage;
            // Test another filter
            // FilteredHallPeriod= FracMpy(HallPeriod, Q15(0.1)) +
            // FracMpy(FilteredHallPeriod,Q15(0.9));
            //        HallPeriodFiltered= FracMpy(HallPeriod, Q15(0.1)) +
            //        FracMpy(HallPeriodFiltered,Q15(0.9)); if(HallPeriodFiltered <
            //        HallMinPeriod)
            //            HallPeriodFiltered = HallMinPeriod;

            if ((HallPeriodFiltered > HallMinPeriod) && (T1INTCnt == 0)) {
                Speed = __builtin_divf(HallMinPeriod, HallPeriodFiltered);
            } else if (HallPeriod <= HallMinPeriod) {
                Speed = 32767;
            }

            //========= Codes below is optional, need to check actual speed
            // direction=======
            //          Choose one of the codes below
            //------------------------------------------------------------------------------
            //        if(uGF.Direction != uGF.DirectionDefault)
            //            Speed = -Speed;
            //------------------------------------------------------------------------------
            if (uGF.Direction == uGF.DirectionDefault)
                Speed = -Speed;
            //==============================================================================
            HallPulses++;
        }
        if (HallPulsesLatch < MotorStartSpeedPulses)
            Speed = 0;
    }
    //------------------------------------------------------------------------------
    //  1:64 scale
    //  interrupt: 65535/(100MIPS/64) = 0.042 sec
    //  if T1INTCnt = 10, 0.42 sec --> No hall edge happens --> speed = 0;
    //------------------------------------------------------------------------------
    void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
        /* Interrupt Service Routine code goes here */
        IFS0bits.T1IF = 0;  // Clear Timer2 interrupt flag
        if (++T1INTCnt >= 20) {
            T1INTCnt = 20;
            Speed = 0;
        }
    }
    void SetupTimer1(void) {
        T1CONbits.TON = 0;    // Disable Timer
        T1CONbits.TCS = 0;    // Select internal instruction cycle clock
        T1CONbits.TGATE = 0;  // Disable Gated Timer mode
                              //    T1CONbits.TCKPS = 0b11; // Select 1:256 Pre-scaler
        T1CONbits.TCKPS = 2;  // 1:64
                              //    T1CONbits.TCKPS = 1; //1:8
        TMR1 = 0x00;          // Clear timer register
        PR1 = 0xFFFF;         //
        IPC0bits.T1IP = 2;    // Set Timer Interrupt Priority Level
        IFS0bits.T1IF = 0;    // Clear Timer Interrupt Flag
        IEC0bits.T1IE = 1;    // Enable Timer interrupt
        T1CONbits.TON = 1;    // Start Timer
    }
    //------------------------------------------------------------------------------
    // Motor Stall Detect
    // Executed in main() @50Hz
    //------------------------------------------------------------------------------
    static bool s_bMotorStallCurrentLimited = false;
    static uint16_t s_u16MotorStallReleaseCounter = 0;

    bool MotorStallIsCurrentLimitActive(void) {
        return (FaultFlags.MotorStall == 0) && s_bMotorStallCurrentLimited;
    }

    bool MotorStallShouldForceOutputZeroOnThrottleRelease(void) {
        return (FaultFlags.MotorStall != 0) ||
               s_bMotorStallCurrentLimited ||
               (faultMotorStall.counter >= MOTOR_STALL_CURRENT_LIMIT_CNTR);
    }

    void MotorStallForceOutputZero(void) {
        g_stSystemData.i16TargetRpm = 0;
        g_stSystemData.i16CurrentRpm = 0;
        g_stSystemData.i16ActiveRpm = 0;
        ReferenceRAW = 0;
        ctrlParm.qVelRef = 0;
        ctrlParm.qVqRef = 0;
        ctrlParm.qVdRef = 0;
        piInputOmega.inReference = 0;
        piInputOmega.piState.integrator = 0;
        piOutputOmega.out = 0;
        piInputIq.inReference = 0;
        piInputIq.piState.integrator = 0;
        piOutputIq.out = 0;
        piInputId.inReference = 0;
        piInputId.piState.integrator = 0;
        piOutputId.out = 0;
        uGF.Coast = 1;
    }

    void MotorStallDetect(void) {
        bool bThrottleReleased = (g_stSystemData.i16TargetRpm == 0);
        bool bMotorCommandActive = (abs(g_stSystemData.i16TargetRpm) >= MOTOR_STALL_COMMAND_THRESHOLD);
        bool bNearZeroSpeed = (abs(Speed) <= MOTOR_STALL_SPEED_THRESHOLD);
        bool bClearlyMoving = (abs(Speed) >= MOTOR_STALL_SPEED_RELEASE_THRESHOLD);

        if (FaultFlags.MotorStall != 0) {
            if (bThrottleReleased) {
                MotorStallForceOutputZero();
                FaultFlags.MotorStall = 0;
                faultMotorStall.counter = 0;
                s_bMotorStallCurrentLimited = false;
                s_u16MotorStallReleaseCounter = 0;
                if ((FaultFlags.Overvoltage == 0) &&
                    (FaultFlags.MOSOverHeat == 0) &&
                    (FaultFlags.MCUOverHeat == 0) &&
                    (FaultFlags.Undervoltage == 0)) {
                    uGF.Fault = 0;
                }
            } else {
                faultMotorStall.counter = MOTOR_STALL_LOCK_CNTR;
            }
            return;
        }

        if (uGF.CtrlMode != 0) {
            faultMotorStall.counter = 0;
            s_bMotorStallCurrentLimited = false;
            s_u16MotorStallReleaseCounter = 0;
            return;
        }

        if (!bMotorCommandActive) {
            if (MotorStallShouldForceOutputZeroOnThrottleRelease()) {
                MotorStallForceOutputZero();
            }
            faultMotorStall.counter = 0;
            s_bMotorStallCurrentLimited = false;
            s_u16MotorStallReleaseCounter = 0;
            return;
        }

        if (bNearZeroSpeed) {
            s_u16MotorStallReleaseCounter = 0;
            if (faultMotorStall.counter < MOTOR_STALL_LOCK_CNTR) {
                faultMotorStall.counter++;
            }

            if (faultMotorStall.counter >= MOTOR_STALL_CURRENT_LIMIT_CNTR) {
                s_bMotorStallCurrentLimited = true;
            }

            if (faultMotorStall.counter >= MOTOR_STALL_LOCK_CNTR) {
                faultMotorStall.counter = MOTOR_STALL_LOCK_CNTR;
                FaultFlags.MotorStall = 1;
                uGF.Fault = 1;
            }
        } else if (bClearlyMoving) {
            if (s_u16MotorStallReleaseCounter < MOTOR_STALL_RELEASE_CNTR) {
                s_u16MotorStallReleaseCounter++;
            } else {
                faultMotorStall.counter = 0;
                s_bMotorStallCurrentLimited = false;
                s_u16MotorStallReleaseCounter = 0;
            }
        } else {
            s_u16MotorStallReleaseCounter = 0;
            if (!s_bMotorStallCurrentLimited && (faultMotorStall.counter > 0)) {
                faultMotorStall.counter--;
            }
        }
    }
    void ClearAllFault(void) {
        uGF.Fault = 0;
        FaultFlags.MotorStall = 0;
        FaultFlags.Overvoltage = 0;
        FaultFlags.MOSOverHeat = 0;
        faultMotorStall.counter = 0;
        faultOvervoltage.counter = 0;
    }

    void IqSquareIntegral(void) {
        IqSquare.OverCurrent = piInputIq.inMeasure - IqSquare.RatedIq;
        if (IqSquare.OverCurrent > 0)
            IqSquare.Sum += FracMpy(IqSquare.OverCurrent, IqSquare.OverCurrent);
        else
            IqSquare.Sum -= FracMpy(IqSquare.OverCurrent, IqSquare.OverCurrent);
        if (IqSquare.Sum < 0)
            IqSquare.Sum = 0;
    }
    void IbusCalc2(void) {
        TempVar16 = HALF_PWMDUTY;
        Temp2Var16 = PG1DC - TempVar16;
        if (Temp2Var16 >= 0)
            V_PhaseA = __builtin_divf(Temp2Var16, TempVar16);
        else
            V_PhaseA = -__builtin_divf(-Temp2Var16, TempVar16);

        Temp2Var16 = PG2DC - TempVar16;
        if (Temp2Var16 >= 0)
            V_PhaseB = __builtin_divf(Temp2Var16, TempVar16);
        else
            V_PhaseB = -__builtin_divf(-Temp2Var16, TempVar16);

        Temp2Var16 = PG3DC - TempVar16;
        if (Temp2Var16 >= 0)
            V_PhaseC = __builtin_divf(Temp2Var16, TempVar16);
        else
            V_PhaseC = -__builtin_divf(-Temp2Var16, TempVar16);
    }
    void IbusCalc(void) {
        Temp2Var16 = (signed int)PG3DC - HALF_PWMDUTY;
        a_Reg = __builtin_mpy(Temp2Var16, iabc.a, 0, 0, 0, 0, 0, 0);
        V_PhaseA = __builtin_sacr(a_Reg, 0);
        Temp2Var16 = (signed int)PG1DC - HALF_PWMDUTY;
        a_Reg = __builtin_mpy(Temp2Var16, iabc.b, 0, 0, 0, 0, 0, 0);
        V_PhaseB = __builtin_sacr(a_Reg, 0);
        Temp2Var16 = (signed int)PG2DC - HALF_PWMDUTY;
        a_Reg = __builtin_mpy(Temp2Var16, iabc.c, 0, 0, 0, 0, 0, 0);
        V_PhaseC = __builtin_sacr(a_Reg, 0);

        TempVar16 = (long)(V_PhaseA + V_PhaseB + V_PhaseC);
        //
        // 1/(2500/32768) = 13.1072 = 26843 in Q11
        // HALF_PWMDUTY: 2500
        a_Reg = __builtin_mpy(TempVar16, 26843, 0, 0, 0, 0, 0, 0);
        Ibus = __builtin_sacr(a_Reg, -4);

        a_Reg = __builtin_mpy(IbusAVG, Q15(0.99), 0, 0, 0, 0, 0, 0);
        IbusAVG = __builtin_sacr(a_Reg, 0);
        a_Reg = __builtin_mpy(Ibus, Q15(0.01), 0, 0, 0, 0, 0, 0);
        TempVar16 = __builtin_sacr(a_Reg, 0);
        IbusAVG += TempVar16;
    }

    //=================================================================================================
    // Modbus 電池裝置偵測與備用機制
    //=================================================================================================

#if CODESW_MODBUS_SCHEDULER_ENABLE == 1

    /**
     * @brief 檢查外部電池裝置 (ID01) 是否正常回應
     * @param pstBatteryData 外部電池裝置的原始數據
     * @return true: 外部電池裝置正常, false: 外部電池裝置無效
     * @note 此函式用於判斷是否需要啟用本地電池數據備用機制
     */
    bool checkExternalBatteryValidity(
        const S_MODBUS_BATTERY_DATA_RAW *pstBatteryData) {
        // 檢查電池容量是否為有效值 (非0且非0xFFFF)
        if (pstBatteryData->u16Regs[0x01] == 0 ||
            pstBatteryData->u16Regs[0x01] == 0xFFFF) {
            return false;
        }

        // 檢查電池百分比是否為有效值 (0-100)
        uint8_t u8Percent = pstBatteryData->u16Regs[0x02];
        if (u8Percent > 100) {
            return false;
        }

        // 檢查電池電壓是否為有效值 (非0且非0xFFFF)
        if (pstBatteryData->u16Regs[0x03] == 0 ||
            pstBatteryData->u16Regs[0x03] == 0xFFFF) {
            return false;
        }

        // 檢查製造資訊是否為有效值
        uint16_t u16MfgInfo = pstBatteryData->u16Regs[0x04];
        if (u16MfgInfo == 0 || u16MfgInfo == 0xFFFF) {
            return false;
        }

        return true;
    }

    /**
     * @brief 使用本地電池數據更新電池結構體作為備用
     * @param pstBatteryData 目標電池數據結構體
     * @note 當外部電池裝置無效時，此函式將本地ADC讀取的電池數據
     *       轉換為標準的電池數據格式，用於更新其他Slave裝置
     */
    void updateLocalBatteryDataAsBackup(S_BATTERY_DATA * pstBatteryData) {
// 使用本地電池監控模組的數據
#if CODESW_BATTERY_ENABLE
        // 電池容量 (根據電壓估算，假設為48V電池)
        pstBatteryData->u16CapacityAh_x10 = 120;  // 12.0 AH

        // 電池百分比 (從本地電池監控模組取得)
        pstBatteryData->u8Percent = logic_battery_getSOCPercent();

        // 電池電壓 (從本地ADC讀取)
        pstBatteryData->u16Voltage_x10 = logic_battery_getActualVoltage() / 10;

        // 製造資訊 (使用預設值)
        pstBatteryData->u8MfgCode = 0x01;  // 製造商代碼
        pstBatteryData->u8MfgYear = 24;    // 2024年
        pstBatteryData->u8MfgMonth = 1;    // 1月
        pstBatteryData->u8MfgWeek = 1;     // 第1週

        // 序號 (使用預設值)
        pstBatteryData->u16SerialNumber = 0x1234;

        // 充電次數 (使用預設值)
        pstBatteryData->u16ChargeCycles = 0;

        // 最後充電時間 (使用當前時間或預設值)
        pstBatteryData->u16LastChargeYear = 2024;
        pstBatteryData->u8LastChargeMonth = 1;
        pstBatteryData->u8LastChargeDay = 1;
        pstBatteryData->u8LastChargeHour = 0;
        pstBatteryData->u8LastChargeMinute = 0;

#else
    // 如果本地電池監控未啟用，使用預設值
    pstBatteryData->u16CapacityAh_x10 = 120;  // 12.0 AH
    pstBatteryData->u8Percent = 50;           // 50%
    pstBatteryData->u16Voltage_x10 = 480;     // 48.0V
    pstBatteryData->u8MfgCode = 0x01;
    pstBatteryData->u8MfgYear = 24;
    pstBatteryData->u8MfgMonth = 1;
    pstBatteryData->u8MfgWeek = 1;
    pstBatteryData->u16SerialNumber = 0x1234;
    pstBatteryData->u16ChargeCycles = 0;
    pstBatteryData->u16LastChargeYear = 2024;
    pstBatteryData->u8LastChargeMonth = 1;
    pstBatteryData->u8LastChargeDay = 1;
    pstBatteryData->u8LastChargeHour = 0;
    pstBatteryData->u8LastChargeMinute = 0;
#endif
    }

#endif  // CODESW_MODBUS_SCHEDULER_ENABLE == 1
