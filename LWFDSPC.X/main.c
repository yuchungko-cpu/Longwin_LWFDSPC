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

#if CODESW_X2C_SCOPE_ENABLE == 1
#include "diagnostics/diagnostics.h"
#endif

/*
 * 【診斷】主迴圈實際頻率 (Hz)，每秒更新一次；可直接在 X2CScope Watch View 觀察。
 * 參考值：Modbus 停用時實測 42835Hz (23.3us/圈)。
 * 無條件編譯 —— 見下方「觀測變數一律存在」的說明。
 */
volatile uint32_t g_u32MainLoopHz = 0;

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
// Modbus 相關 header 一律 include：對應的 .c 本來就無條件編譯，這裡只是型別與函式宣告，
// 不含任何程式碼成本。這樣 S_SHARED_DEVICE_DATA / S_BATTERY_DATA 等型別在
// CODESW_MODBUS_SCHEDULER_ENABLE = 0 時依然可見。
#include "src/longwin/s_hal_rs485.h"
#include "src/longwin/s_modbus_decode.h"
#include "src/longwin/s_modbus_master.h"
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
    // 一律存在：這兩份資料是全系統共用的狀態 (車速、助力等級、電池)，
    // 主迴圈與 X2CScope 觀測都會讀，不應隨 Modbus 開關出現/消失。
    S_BATTERY_DATA sBatteryData;
    S_SHARED_DEVICE_DATA sSharedData;

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

// =============================================================================
// == X2CScope 觀測變數：一律無條件編譯，不隨 CODESW_X2C_SCOPE_ENABLE 出現/消失。 ==
// ==                                                                          ==
// == 理由：開關只應決定「X2CScope 要不要執行」，不該改變 main.c 編譯出哪些程式碼。 ==
// == 過去把這些變數掛在開關下，導致開關一翻符號就從 elf 消失，GUI 存下的 Scope   ==
// == 通道解析不到位址 → Scope View 報 null 而 Watch View 卻正常，極難診斷。      ==
// == 代價僅約 20 bytes RAM 與 ADC ISR 內 3 個賦值 (遠低於 0.1% CPU)。            ==
// =============================================================================

// 車速 (KM/H * 10)。值由主迴圈自 g_stSystemData.sSharedData 鏡射而來。
uint16_t u16CurrentSpeedKmh_x10;
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
// [有動力倒溜] 與命令方向相反的「淨」霍爾邊緣數。CNRead_Inline (50us ISR) 反向 +1 / 正向 -1
// (地板 0)，20ms 的 EMB 任務讀取；8-bit 存取在 16-bit MCU 上為原子，不需臨界區。
// 1 邊緣 = 車輪 1.75 mm，門檻見 userparms.h 的 EMB_ROLLBACK_REV_EDGES。
// ⚠ 只在霍爾邊緣時更新 → 車靜止時凍結，故 EMB 的 LOCK/RELEASE 動作處必須清零(見該處註解)。
volatile uint8_t g_u8EmbRevEdgeCnt = 0;
// [下坡滑動] 命令歸零後的「淨」位移邊緣數，**帶號**：一個轉向 +1、另一個 -1。
// 與 g_u8EmbRevEdgeCnt 的差別 (兩者不可合併，見 userparms.h 的說明)：
//   倒溜計數需要「命令方向」來定義什麼叫反向，故必須靠 |inReference| > 門檻 武裝;
//   而本計數的閘門本來就是「命令已歸零」—— 那時沒有命令方向可比，所以改用帶號淨計數，
//   由 20ms 的 EMB 任務以 |cnt| >= EMB_DOWNHILL_SLIDE_EDGES 判斷，不需鎖存方向。
// 帶號淨計數的兩個附帶好處：
//   (1) 靜止抖動時方向交替 → 正負相抵停在 0 附近 → 自動免疫誤觸 (與倒溜同一原理);
//   (2) 前後對稱 → 順帶接住「命令已歸零後的無動力倒溜」，那正是倒溜偵測解除武裝後的空窗。
// 命令非零時持續歸零 → 計數天然從「命令歸零」那一刻起算，不必另外清。
// 16-bit 存取在 16-bit MCU 上為原子，不需臨界區。
volatile int16_t g_i16EmbZeroCmdEdgeCnt = 0;
// [有動力倒溜] 武裝旗標。true = EMB 已 RELEASE 過(車曾在動力狀態)，才允許倒溜偵測累計。
// 目的:避免車停著、EMB LOCKED、駕駛剛切排檔時，人為推車後退立刻觸發 EMB LOCK 動作
// (煞車本來就鎖著，該動作無效但會影響觸感/log)。EMB RELEASE 時置位、LOCK 時清零。
// 讀取端見 CNRead_Inline 的計數邏輯與主迴圈的 bEmbRollbackDetected 合成。
// 8-bit / bool 存取原子，不需臨界區。
volatile bool g_bEmbRollbackArmed = false;
// [下坡滑動] 「命令歸零後車在加速」的連續確認次數。CNRead_Inline 以霍爾週期比較累加，
// 20ms 的 EMB 任務讀取。自由滑行的阻力恆為正 → 平路與上坡的週期必然逐步變長，因此
// 「週期真的變短」等價於「重力已超過全部阻力，車不會自己停」。這是唯一不依賴車速門檻、
// 也不依賴跨迴圈狀態的判別，故它取代了 V0.20 的武裝旗標 (s_bEmbDownhillArmed，為何移除
// 見 userparms.h 的 EMB_DOWNHILL_* 說明)。
// ⚠ 名稱沿用 V0.21 的 NoDecel(當時判準是「週期沒變長」),語意已於 V0.24 改為「在加速」——
//   舊判準的隱含減速度門檻帶 v² 項,在 3 km/h 附近會把平路自由滑行誤判成沒減速。
// 車速上限閘門 (EMB_DOWNHILL_MIN_PERIOD) 也在 ISR 內一併判斷 —— 計數只在上限以下累加，
// 因此「本計數達標」已內含「車速夠低、夾煞可接受」。
// ⚠ 與上面兩個計數器同理：只在霍爾邊緣時更新 → 車靜止時凍結，故 EMB 的 LOCK/RELEASE
//   動作處必須清零 (見該處註解)。8-bit 存取在 16-bit MCU 上為原子，不需臨界區。
volatile uint8_t g_u8EmbNoDecelCnt = 0;
// [e-lock] 助力段位 0 (電子鎖車) 是否作用中。1 = 段位 0，油門已被歸零且 EMB 不會放開。
// 純觀測用 (X2CScope)，不參與任何控制判斷 —— 控制邏輯各自直接讀 u8AssistLevel。
volatile uint8_t g_u8ELockActive = 0;
// [EMB 夾煞診斷] 純觀測用 (X2CScope / debugger)，不參與任何控制判斷。
// 實車無法即時觀測時，路試後保持通電接上 debugger 讀這幾顆即可判斷停車走的是哪條路徑：
//   正常停車應該只有 g_u16EmbLockCntUvwDelay 在累加;
//   g_u16EmbLockCntFailsafe > 0  ⇒ 停車鏈有路徑失效,被 1500ms 逾時帶速夾停;
//   g_u16EmbLockCntDownhill > 0  ⇒ 下坡滑動偵測動作(平路不該出現);
//   g_u16EmbLockCntRollback > 0  ⇒ 倒溜偵測動作。
// 計數只在「進入 LOCK 的那一次」累加(保持鎖定的 tick 不重複計)，見 EMB LOCK 分支。
volatile uint8_t  g_u8EmbLockReason = 0;        // 最後一次的原因碼 (E_EMBRAKER_LOCK_REASON)
volatile int16_t  g_i16EmbLockPulses = 0;       // 夾煞當下的 HallPulsesLatch (邊緣/100ms)
volatile uint16_t g_u16EmbLockPeriod = 0;       // 夾煞當下的 HallPeriod (Timer1 ticks)
volatile uint16_t g_u16EmbLockCntUvwDelay = 0;  // 原因 1：正常停車 (UVW + 延遲)
volatile uint16_t g_u16EmbLockCntFailsafe = 0;  // 原因 2：逾時 failsafe
volatile uint16_t g_u16EmbLockCntDownhill = 0;  // 原因 3：下坡滑動
volatile uint16_t g_u16EmbLockCntRollback = 0;  // 原因 4：有動力倒溜
volatile uint16_t g_u16EmbLockCntOther = 0;     // 其餘 (Plan B / IBKS / 故障 / 運轉前檢查)
const int16_t PhaseValues[8] = {0, 0, -21844, -10922, 21844, 10922, 32767, 0};
signed int TempVar;     // main loop
signed int Ibus = 0;     // 瞬時 DC bus 電流 (Q15，與 iabc 同刻度；313.3 counts/A)
// 快速 IIR 平均 (τ=6.4ms)，供 X2CScope 觀測動態；上報一律用下面的 IbusMeanQ15。
// 內部狀態多帶 IBUS_AVG_FRAC_BITS 個小數位元，因此沒有舊版 Q15 乘法實作的小訊號凍結
// 死區(舊版 |IbusAVG| < 50 時衰減項與輸入項雙雙捨為 0)。純位移，不用累加器。
// 狀態上界 = 32767 << 8 = 8.4e6，int32 安全。
signed int IbusAVG = 0;
#define IBUS_AVG_FRAC_BITS 8  // 內部小數位元數
#define IBUS_IIR_SHIFT     7  // τ = 2^7 = 128 取樣 x 50us = 6.4ms
static int32_t s_i32IbusAvgAcc = 0;
// 上報用的精確平均：ISR 累加 IBUS_AVG_SAMPLES 筆後取平均寫入 IbusMeanQ15。
// 用 2 的幕次讓除法變成位移；IbusMeanQ15 是單一 16-bit 全域，主迴圈可原子讀取，
// 不需要關中斷或臨界區。
#define IBUS_AVG_SAMPLES 512  // 512 x 50us = 25.6ms 平均窗
#define IBUS_AVG_SHIFT 9      // log2(IBUS_AVG_SAMPLES)
signed int IbusMeanQ15 = 0;   // 過去 25.6ms 的 DC bus 電流平均 (Q15，帶正負號)
// IbusMeanQ15 的安培表示 (單位 0.1 A，帶正負號：回充為負)，僅供 X2CScope 觀測，
// 免得在 scope 上心算 ÷313.3。上限 1046 (=104.6A)，int16 內安全。
// 在主迴圈換算(見 Modbus 上報處)，不在 ISR 裡做 32-bit 除法。
signed int IbusAmpX10 = 0;
static int32_t s_i32IbusAccum = 0;   // 累加器，最大 512 x 32767 = 1.7e7，int32 安全
static uint16_t s_u16IbusSamples = 0;
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
// PWM 佔空比觀測變數 (見上方「觀測變數一律存在」的說明)，於 ADC ISR 內更新。
unsigned int X2CPG1Duty = 0;
unsigned int X2CPG2Duty = 0;
unsigned int X2CPG3Duty = 0;
// 1966 count = 720 馬達RPM = 1.36 km/h 以下啟動 Lock。
// 註：僅被 main.c 的 #if 0 死碼區 (CtrlMode 0/2 舊路徑) 使用；live 路徑用
//     UVW_LOCK_STOP_PULSES / UVW_LOCK_RELEASE_REF。
signed int UVWLockSpeed = Q15(0.06);
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
// ReGen 起/停煞門檻，用 HallPulsesLatch (每 100ms Hall 邊緣數) 而非 Speed。
// 換算 (18 邊緣/機械轉, 齒比 20.3)：1 pulse/100ms = 33.3 馬達RPM = 0.063 km/h
signed int BrakeStartSpeedPulses = 15;  // 500 馬達RPM = 0.95 km/h 以上才允許 ReGen
signed int BrakeStopSpeedPulses = 7;    // 233 馬達RPM = 0.44 km/h 以下停止 ReGen
// [BUGFIX] 原本初始化為 0 且全檔沒有任何執行期賦值，於是 main.c 的
//   `if (HallPulsesLatch < MotorStartSpeedPulses) Speed = 0;` 恆為假 ——
//   「靜止時把 Speed 歸零」這道保護從來沒有生效過。
//   後果 (X2CScope 實測)：停車時最後一個霍爾邊緣若被解碼成反向 (夾煞車回彈很容易造成)，
//   Speed 就保持該負值 (實測 -900 = -0.62 km/h)，唯一能清掉它的是 T1 逾時 20x42ms = 840ms。
//   這 840ms 內命令已是 0 而 inMeasure 為負 → 速度環看到正誤差 → 命令馬達往前出力,
//   只能靠 UVW 短路/EMB 先接手壓住,是時序運氣而非設計。
//   取 3 的理由：與上面兩個 ReGen 門檻同一刻度 (1 脈衝/100ms = 33.3 馬達RPM)，
//   3 是能可靠濾掉靜止抖動的最小值。不影響 EMB 倒溜偵測 (那個看 uGF.Direction 逐邊緣，
//   不看 Speed)，也不影響 UVW lock (看 HallPulsesLatch)。
signed int MotorStartSpeedPulses = 3;   // 100 馬達RPM = 0.19 km/h 以下視為靜止
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
        case LOGIC_ALARM_A19_MOTOR_HALL_FAULT:
            return 6;  // Doc: A07
        case LOGIC_ALARM_A05_MOTOR_OVERCURRENT:
            return 7;  // Doc: A09
        case LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP:
            return 9;  // Doc: A06
        case LOGIC_ALARM_A14_LSN_FAULT:
            return 16;  // A14
        case LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE:
            return 15;  // A15
        case LOGIC_ALARM_A04_EMB_SENSOR_FAULT:
            return 17;  // was A19's flash count -- TODO confirm correct LED flash count for A04
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
    // Q15 速度刻度基準：Speed = HallMinPeriod / HallPeriodFiltered
    // HALL_MIN_PERIOD 由 motor_scale.h 依極對數與 SPEED_FS_RPM 推導 (= 434)，
    // 該檔的 #error 護欄保證此值不會在未同步重算命令域/PI 增益的情況下被改動。
    HallMinPeriod = HALL_MIN_PERIOD;
    HallPeriod = 30000;

    /* Initialize Peripherals */
    Init_Peripherals();
    SW_12V = TURN_ON;  // must before CAN_Initialize();
    // CAN1 已停用：RB8/RB9 (原 CAN1TX/RX) 改配置給 X2CScope 專屬 UART2。
    // CAN1_Initialize();
    // SCCP3_TMR_Initialize();
    // CN_Configure();
    OverCurrentEnable();
    /* Initializing Current offsets in structure variable */
    measCurrOffsetFlag = 1;
    MeasCurrOffset(&measCurrParm.Offseta, &measCurrParm.Offsetb);
    // measCurrParm.Offseta = 384;
    // measCurrParm.Offsetb = 192;
    // HAL_MC1PhaseStateChangeMaxPeriodSet(PERIOD_CONSTANT);

    // DSP 累加器飽和設定。影響範圍：main.c 內直接使用 a_Reg / __builtin_mpy / __builtin_sacr
    //   的程式，以及 SpeedCalc.s 的 FracMpy。motor_control 函式庫的組語函式會在進入時自行
    //   覆寫 CORCON、離開時還原，故不受此處設定影響。
    CORCONbits.SATA = 1;    // ACCA 飽和
    CORCONbits.SATB = 1;    // ACCB 飽和
    CORCONbits.SATDW = 1;   // 存回資料空間時飽和：sac/sacr 寫入 16-bit 變數時「夾住」而非繞回
                            //   (SATA 只管 MAC 運算對 40-bit 累加器本身的飽和)。
    CORCONbits.ACCSAT = 1;  // 9.31 飽和模式：保留 8 個 guard bits，中間和不會被提早夾在 ±1.0
    // [FIX] 原本此處第 4 行是 `CORCONbits.SATA = 0;`，把前一行剛開啟的 ACCA 飽和又關掉
    //   (明顯是遺留的除錯痕跡)。已逐條審過 main.c 全部直接累加器運算：
    //   IbusCalc() 改寫後每個 sacr 的結果都有 int16 餘量(duty 偏移 ≤ 16387、每相貢獻 ≤ 16387)，
    //   三相加總改在 int32 內做並顯式夾制，故不再依賴這裡的設定；其餘運算(過壓監測、
    //   FracMpy)皆無溢位可能。此處保留飽和設定純粹作為防護網。
    //   __builtin_divf 與 Q15SQRT 不經累加器，均不受影響。

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
    // O_EM_BRAKE_CTRL 由 SCCP2 PWM 驅動 (emb_pwm_init 已在 board_service 呼叫)。
    // port_config 已設 TRISD1=0、CN 內部下拉、PPS 到 SCCP2:OCM2;此處不再直接寫 LAT。
#if CODESW_EMBRAKER_ENABLE
    emb_pwm_hardLock();      // EMB enabled: start locked
#else
    emb_pwm_hardRelease();   // EMB disabled for test: keep released
#endif

// 通訊相關初始化
#if CODESW_MODBUS_SCHEDULER_ENABLE == 1
    O_UART_TX_TRIS = 0;   // UART TX設為輸出
    I_UART_RX_TRIS = 1;   // UART RX設為輸入
    O_RS485_RE_TRIS = 0;  // RS485方向控制設為輸出
    O_RS485_RE_LAT = 0;   // 初始設為接收模式
#endif
    // CAN 已停用：RB8/RB9 (原 CAN_TX/CAN_RX) 改由 X2CScope 專屬 UART2 使用，
    // 其 TRIS/PPS 已於 port_config.c 的 MapGPIOHWFunction() 設定。
    // O_CAN_STB_TRIS = 0;  // CAN待機控制設為輸出 (RB7)
    // O_CAN_STB_LAT = 0;   // 初始啟動(非待機)
    // O_CAN_TX_TRIS = 0;   // CAN TX設為輸出 (RB8 -> 現為 U2TX)
    // I_CAN_RX_TRIS = 1;   // CAN RX設為輸入 (RB9 -> 現為 U2RX)

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
#if CODESW_X2C_SCOPE_ENABLE == 1
    DiagnosticsInit();  // 初始化 X2CScope 專屬 UART2 (RB8/RB9) 與 X2C 連線
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
#if CODESW_THROTTLE_ENABLE
    // 不再與 Modbus 綁定：油門邏輯一律讀 sSharedData.u8AssistLevel，
    // 若 Modbus 停用而這裡沒初始化，助力等級會是 0，油門行為就會不對。
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
#if CODESW_MODBUS_PROCESS_SUSPEND == 0
        modbusService_process();
#endif
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
            // ------------------------------------------------------------------
            // 計算車速：Q15 Speed -> 車輪 RPM -> km/h
            //
            // [FIX] 舊版是三個互相補償的錯誤湊出來的結果：
            //   (1) 把原始 Q15 Speed 當成「RPM*10」直接餵進 LwfocGetExternalRpm()
            //       (真正做 Q15->RPM 的 LwfocGetInternalRpm() 反而沒被呼叫)；
            //   (2) 用 HALL_PPR=610 與極對數 12 湊出 ÷5.083，再乘上當時 Speed 刻度
            //       偏小 4 倍，合成 ÷20.33 剛好等於真實齒比 20.3；
            //   (3) 吋->km/h 係數用 440 (正確為 479)，−8.1% 又把 (1)(2) 殘留的
            //       +9.0% 抵掉，淨誤差 +0.2%，所以顯示看起來是對的。
            //   現改為明確的兩段換算：SPEED_FS_RPM 定義 Q15 刻度，
            //   GEAR_RATIO_X100 定義齒比，係數改回 479。
            // ------------------------------------------------------------------
            // 從馬達邏輯模組獲取目前設定的輪徑 (單位: 吋 * 10)
            uint16_t u16WheelInches = logic_motor_getWheelDimension();

            // Q15 Speed -> 車輪 RPM * 10 (含齒比，取絕對值)
            uint16_t u16WheelRpmX10 = scale_speedToWheelRpmX10(Speed);

            // 使用輪速 RPM 和輪徑計算車速 (單位: KM/H * 100)
            uint16_t u16SpeedKmh_x100 = logic_motor_getSpeedKmhFromRpm(u16WheelRpmX10,
                                                                       u16WheelInches);

            // 轉換為 KM/H * 10 存入共享資料結構。改用四捨五入：舊版直接截斷，
            // 在新的 (更精確的) 換算下 7.19 km/h 會被截成 7.1，與實機顯示不符。
            g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10 = (u16SpeedKmh_x100 + 5) / 10;

            // ------------------------------------------------------------------
            // DC bus 電流上報 (單位 0.1 A) → ID02 reg 0x04 / ID03 reg 0x05
            //   來源 IbusMeanQ15 是 ISR 算好的 25.6ms 算術平均，單一 16-bit 全域，
            //   16-bit MCU 上讀取本身即為原子，無需關中斷。
            //   換算 313.3 counts/A 由 userparms.h 的 IABC_Q15_TO_A_X10() 依
            //   R_SHUNT_Ohm / CURRENT_GAIN_OPAMP / KCURRA 推導。
            //   負值(再生制動時電流回灌)以 0 表示，因上報欄位為 unsigned 的「負載電流」。
            {
                signed int i16IbusMean = IbusMeanQ15;  // 原子讀取

                // X2CScope 觀測用：同一個值的安培 x10 表示，保留正負號(回充為負)。
                //   主迴圈是自由跑的，而 IbusMeanQ15 每 25.6ms 才更新一次，所以只在值
                //   真的變了才做那次 32-bit 除法。
                static signed int s_i16LastIbusMean = 1;  // 與合法初值 0 不同 → 開機必算一次
                if (i16IbusMean != s_i16LastIbusMean) {
                    s_i16LastIbusMean = i16IbusMean;
                    if (i16IbusMean >= 0) {
                        IbusAmpX10 = (signed int)IABC_Q15_TO_A_X10(i16IbusMean);
                    } else {
                        // -32768 取負會溢位 → 夾到 32767 (= 104.6A，即感測器軌到軌極限)
                        signed int i16Mag = (i16IbusMean == -32768)
                                ? 32767 : (signed int)(-i16IbusMean);
                        IbusAmpX10 = -(signed int)IABC_Q15_TO_A_X10(i16Mag);
                    }
                }

                if (i16IbusMean < 0) {
                    i16IbusMean = 0;
                }
                g_stSystemData.sSharedData.u16LoadCurrentA_x10 =
                    IABC_Q15_TO_A_X10(i16IbusMean);
            }
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
        // 車速本體算在 g_stSystemData.sSharedData 裡 (見上方 Modbus 區塊)，這裡鏡射一份
        // 到獨立全域變數供 X2CScope 觀測。無條件執行，符號不隨開關消失。
        u16CurrentSpeedKmh_x10 = g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10;

        // SpeedRefHighLimit = SpeedCtrlLimit;
#if CODESW_X2C_SCOPE_ENABLE == 1
        DiagnosticsStepMain();  // X2CScope 背景通訊 (UART2)
#endif
        {
            // 【診斷】每秒統計一次主迴圈圈數 = 主迴圈頻率，供 Watch View 觀察。
            static uint32_t u32LoopRateLastMs = 0;
            static uint32_t u32LoopPasses = 0;
            uint32_t u32NowMs = g_stSystemData.u32TimeMs;

            u32LoopPasses++;
            if ((u32NowMs - u32LoopRateLastMs) >= 1000UL) {
                u32LoopRateLastMs = u32NowMs;
                g_u32MainLoopHz = u32LoopPasses;
                u32LoopPasses = 0;
            }
        }

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
            g_stSystemData.sSharedData.errors.bMotorSensorError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A19_MOTOR_HALL_FAULT);
            g_stSystemData.sSharedData.errors.bBrakeSignalError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT);
            g_stSystemData.sSharedData.errors.bBrakeStuck = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT);  // Also map stuck to switch fault
            g_stSystemData.sSharedData.errors.bLsnError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A14_LSN_FAULT);
            g_stSystemData.sSharedData.errors.bCommLcdError = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A10_CONTROLLER_COMM_TIMEOUT);
            g_stSystemData.sSharedData.errors.bEmbSensorFault = logic_errorHandler_isAlarmActive(LOGIC_ALARM_A04_EMB_SENSOR_FAULT);
#endif
            // Note: bCommBatteryError, bCommGuiError, bCommAppError have no direct mapping in E_LOGIC_ALARM_CODE_T
            // --- End of Modbus Error Flag Update ---

#if CODESW_EMBRAKER_ENABLE
            // --- [REFACTORED] EMBRAKER logic now uses the master stop flag ---
            if (g_stSystemData.bMotorStop) {
                // 電池保護或控制器過溫已啟動，強制鎖定電磁剎車(緊急路徑,硬夾)
                emb_pwm_hardLock();
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

                // [有動力倒溜/倒衝] 偵測訊號來自 CNRead_Inline 的連續反向霍爾邊緣計數
                //   (g_u8EmbRevEdgeCnt)。與速度無關 → 再慢的潛行倒溜也會在固定位移內被抓到，
                //   這是「倒溜不超過 1/4 車輪(91 邊緣)」規格的基礎。1 邊緣 = 車輪 1.75 mm。
                //   [2026-08-15] 武裝來源由命令方向改為排檔方向 + EMB 曾 RELEASE，
                //   完整理由見 CNRead_Inline 內的說明。
                //
                // [F/R 切換的抑制窗] 排檔硬體開關若在車還在滾時被切換，滾動方向與新的排檔方向
                //   會不一致 → 立即觸發鎖定。但這是**排檔剛切、車尚未回應**的必然過渡態，不是
                //   真正的倒溜。因此在排檔訊號翻轉時起算抑制窗，期間不觸發鎖定。
                //   由 0 段(靜止)進入 F/R 段不抑制 —— 那個情境是起步，不是換向。
                static int8_t s_i8EmbLastGearSign = 0;
                static uint32_t s_u32EmbFlipMs = 0;
                int8_t i8EmbGearSign = (uGF.DirSW == 0) ? +1 : -1;  // 排檔:+1=前進、-1=倒退
                if ((s_i8EmbLastGearSign != 0) && (i8EmbGearSign != s_i8EmbLastGearSign)) {
                    s_u32EmbFlipMs = g_stSystemData.u32TimeMs;  // 排檔翻轉
                }
                s_i8EmbLastGearSign = i8EmbGearSign;
                bool bEmbFlipHoldoff =
                    (EMB_ROLLBACK_FLIP_HOLDOFF_MS > 0) &&
                    ((g_stSystemData.u32TimeMs - s_u32EmbFlipMs) < EMB_ROLLBACK_FLIP_HOLDOFF_MS);

                bool bEmbRollbackDetected = (g_u8EmbRevEdgeCnt >= EMB_ROLLBACK_REV_EDGES) &&
                                            !bEmbFlipHoldoff &&
                                            (HallPulsesLatch >= EMB_ROLLBACK_MIN_PULSES);

                // --- [下坡滑動] 命令歸零後車卻在加速 → 立即上鎖 ---
                //   完整理由見 userparms.h 的 EMB_DOWNHILL_* 說明。三個閘門：
                //     (1) 命令已歸零   —— g_i16EmbZeroCmdEdgeCnt 的計數閘門天然提供
                //     (2) 車在加速     —— g_u8EmbNoDecelCnt (ISR 內比較霍爾週期有沒有變短;
                //                        滑行阻力恆為正 ⇒ 變短就是重力已超過全部阻力。
                //                        名稱沿用 V0.21 的 NoDecel，語意已於 V0.24 改掉)
                //     (3) 車速低於上限 —— 同在 ISR 內，以週期下限實作，已內含於 (2) 的累加
                //   此處只讀 ISR 的結果，再加位移門檻作為雜訊裕度。
                //   ⚠ V0.20 的武裝旗標 (s_bEmbDownhillArmed) 已移除，**不要加回來**：它的
                //     解除門檻 (0.5 km/h) 低於油門的最低命令速度 (1.04 km/h)，「按油門」本身
                //     就會在駕駛放開之前解除武裝 → 下坡點放永遠偵測不到 (V0.20 實車確認)。
                //     現在改由 (2) 的物理判別擋平路誤鎖、(3) 的速度上限限制不適感，兩者都不
                //     需要跨迴圈狀態，因此也沒有「旗標卡住 → 平路停車帶速硬夾」的單點失效。
                bool bEmbDownhillSlide =
                        (g_u8EmbNoDecelCnt >= EMB_DOWNHILL_NODECEL_CONFIRM) &&
                        (abs(g_i16EmbZeroCmdEdgeCnt) >= EMB_DOWNHILL_SLIDE_EDGES);

                // [e-lock] 助力段位 0 = 電子鎖車。段位由 LCD 經 Modbus 在執行期更新
                //   (modbusDecode_decodeLcdData)，故騎行中切段位會即時生效。
                //   油門命令的歸零在 s_logic_throttle.c 完成；此旗標只是禁止 EMB 放開煞車的
                //   第二層防護，刻意不做立即鎖定(否則帶速切 0 段會硬鎖)。
                bool bEmbELockActive = (g_stSystemData.sSharedData.u8AssistLevel == 0u);
                g_u8ELockActive = bEmbELockActive ? 1u : 0u;  // X2CScope 觀測

                E_EMBRAKER_ACTION eBrakeAction = logic_embraker_update(g_stSystemData.u16IEMBMv,
                                                                       g_stSystemData.i16TargetRpm,  // 使用者意圖轉速
                                                                       ReferenceRAW,                 // 馬達實際執行轉速
                                                                       bEmbUVWLockActive,            // [Plan A] UVW短路生效中
                                                                       bEmbReverseEdge,              // [Plan B] 偵測到倒溜
                                                                       bEmbRollbackDetected,         // [有動力倒溜] 連續反向霍爾邊緣達門檻
                                                                       bEmbDownhillSlide,            // [下坡滑動] 命令歸零後仍在移動達門檻
                                                                       (uGF.BrakeSWOn == 1),         // [IBKS] 手剎車/充電中 → 立即鎖定
                                                                       bEmbELockActive,              // [e-lock] 段位 0 → 禁止放開煞車
                                                                       g_stSystemData.u32TimeMs);

                // --- [NEW] Final safety check to override brake action ---
                if (g_stSystemData.bMotorStop) {
                    eBrakeAction = EMBRAKER_ACTION_LOCK;
                }
#if CODESW_MOTOR_LOCK_TEST_ENABLE
                eBrakeAction = EMBRAKER_ACTION_LOCK;
#endif

                // 根據回傳的指令控制硬體
                //
                // [必要] 兩個分支都要清零 g_u8EmbRevEdgeCnt。該計數器只在霍爾邊緣中斷內更新
                //   (整段包在 OldHallState != HallState 裡)，車被煞車夾住不動時**完全不會執行**,
                //   連「命令歸零 → 解除武裝歸零」的分支也不會跑 → 計數凍結在 >= 門檻。若不在此
                //   清零：鎖定→鬆油門解閂→重新給油門→RELEASE→下一個 20ms tick 因計數仍達門檻而
                //   立刻重鎖，車一個邊緣都沒動就被鎖死，而要扣掉計數又必須讓車動 → 卡死。
                //   LOCK 清零 = 偵測已被執行，位移預算用掉;RELEASE 清零 = 每次放煞車都從零起算。
                //   8-bit 寫入在 16-bit MCU 上為原子,且是單純覆寫(非讀改寫),與 ISR 無競態。
                //
                // [夾煞診斷] LOCK 的上升緣 —— FAULT 與倒溜閂鎖分支每個 20ms tick 都會回傳
                //   LOCK(保持鎖定),診斷計數器只在第一次進 LOCK 時累加才有意義。
                static bool s_bPrevEmbLockAction = false;
                bool bEmbLockEdge = (eBrakeAction == EMBRAKER_ACTION_LOCK) && !s_bPrevEmbLockAction;
                s_bPrevEmbLockAction = (eBrakeAction == EMBRAKER_ACTION_LOCK);

                switch (eBrakeAction) {
                    case EMBRAKER_ACTION_LOCK:
#if !CODESW_MOTOR_LOCK_TEST_ENABLE
                        MotorStallForceOutputZero();
#endif
                        // [軟夾／硬夾分派]
                        //   四個情境走硬夾(立即 0% duty),其餘走軟夾:
                        //     (1) IBKS(uGF.BrakeSWOn)      駕駛主動命令,語意上要立即
                        //     (2) bMotorStop                系統安全禁制,不可延遲
                        //     (3) bEmbRollbackDetected      已滑到 28mm,再軟夾佔用倒溜預算
                        //     (4) bEmbDownhillSlide         已滑到 26mm,同理
                        //   (2) 在本 switch 外的 g_stSystemData.bMotorStop 分支已處理硬鎖,
                        //     但那邊只覆蓋 O_EM_BRAKE 硬體,logic_embraker_update 仍會回傳 LOCK
                        //     動作跑進本 switch。為保險,(2) 也列入本判斷條件。
                        //   軟夾只用在**正常停車**路徑:WAITING_TO_LOCK 走完 UVW + SHORT_TO_LOCK_DELAY
                        //     到期。此時車已停或近停,軟夾無安全代價、只是為了消震動。
                        if ((uGF.BrakeSWOn == 1) || g_stSystemData.bMotorStop ||
                            bEmbRollbackDetected || bEmbDownhillSlide) {
                            emb_pwm_hardLock();
                        } else {
                            emb_pwm_startSoftClamp(EMB_SOFT_CLAMP_TICKS);
                        }
                        // [夾煞診斷] 純觀測,不影響控制。只在 LOCK 的**上升緣**記錄(見上方
                        //   bEmbLockEdge)：FAULT 與倒溜閂鎖那兩個分支每個 tick 都回傳 LOCK
                        //   (保持鎖定),不擋掉會把計數器灌爆而失去診斷意義。
                        E_EMBRAKER_LOCK_REASON eLockReason = logic_embraker_getLastLockReason();
                        g_u8EmbLockReason = (uint8_t)eLockReason;
                        if (bEmbLockEdge) {
                            g_i16EmbLockPulses = HallPulsesLatch;  // 夾煞當下的車速
                            g_u16EmbLockPeriod = HallPeriod;
                            switch (eLockReason) {
                                case EMBRAKER_LOCK_REASON_UVW_DELAY:
                                    g_u16EmbLockCntUvwDelay++;
                                    break;
                                case EMBRAKER_LOCK_REASON_FAILSAFE:
                                    g_u16EmbLockCntFailsafe++;
                                    break;
                                case EMBRAKER_LOCK_REASON_DOWNHILL:
                                    g_u16EmbLockCntDownhill++;
                                    break;
                                case EMBRAKER_LOCK_REASON_ROLLBACK:
                                    g_u16EmbLockCntRollback++;
                                    break;
                                default:
                                    g_u16EmbLockCntOther++;
                                    break;
                            }
                        }
                        g_u8EmbRevEdgeCnt = 0;
                        // [有動力倒溜] EMB 已上鎖 → 解除武裝。煞車夾住後車不該再動,
                        //   下次 RELEASE 才重新武裝。避免煞車鎖住狀態下人為推車後退觸發鎖定
                        //   (該動作無效)。
                        g_bEmbRollbackArmed = false;
                        // [下坡滑動] 偵測已被執行,位移預算用掉;煞車夾住後車不該再動,
                        //   兩個計數器一併清零,理由同上方 g_u8EmbRevEdgeCnt。
                        g_i16EmbZeroCmdEdgeCnt = 0;
                        g_u8EmbNoDecelCnt = 0;
                        break;
                    case EMBRAKER_ACTION_RELEASE:
                        emb_pwm_hardRelease();  // 釋放 (100% duty,若 ramp 進行中則中斷)
                        g_u8EmbRevEdgeCnt = 0;
                        // [有動力倒溜] 武裝：機械煞車打開的這一刻起，車若倒溜就該立即上鎖。
                        //   舊版靠命令 > 100 count 武裝,命令降到 100 以下就解除 → 上坡點放
                        //   的減速斜坡期間漏接,見 CNRead_Inline 的說明。改為 RELEASE 起武裝,
                        //   由排檔方向與滾動方向的一致性判定倒溜。
                        g_bEmbRollbackArmed = true;
                        // [下坡滑動] 每次放煞車都從零起算。ISR 在「命令非零」時本來就會清這
                        //   兩個計數器,但車被夾住不動時 ISR 完全不執行 → 會凍結在達標值,
                        //   放開煞車的下一個 20ms tick 就立刻重鎖。理由同 g_u8EmbRevEdgeCnt。
                        g_i16EmbZeroCmdEdgeCnt = 0;
                        g_u8EmbNoDecelCnt = 0;
                        break;
                    case EMBRAKER_ACTION_NONE:
                    default:
                        // 狀態無變化，不執行任何動作
                        break;
                }

                // [EMB 軟夾 ramp tick] 若正在漸降,推進一格;無 ramp 時 no-op。
                //   放在 switch 之後:若本 tick 剛觸發 startSoftClamp,tick 下一週期才開始遞減
                //   (讓完整的 duration 都是遞減時間,不會被本 tick 立刻扣掉一步)。
                emb_pwm_tick20ms();
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
            if (g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10 <= CODESW_DIRECTION_CHANGE_SPEED_THRESHOLD) {
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
            int8_t i8ThrottleResult = logic_throttle_getUpdateParams(&g_stSystemData.sStepTime,
                                                                     &u16lTargetRpm,
                                                                     u16lCurrentRpm,
                                                                     g_stSystemData.u16ThrottleVRMv,  // Pass mV value
                                                                     g_stSystemData.bMotorDirection,
                                                                     g_stSystemData.sSharedData.u8AssistLevel);
            if (u16lTargetRpm > 32767) {
                g_stSystemData.i16TargetRpm = 32767;
            } else {
                g_stSystemData.i16TargetRpm = (int16_t)u16lTargetRpm;
            }
#if CODESW_X2C_SCOPE_ENABLE == 1
            // 斜坡/濾波「前」的取樣點：油門直接算出的目標命令(轉油門時為階躍)
            g_i16ScopeCmdTarget = g_stSystemData.i16TargetRpm;
#endif

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

                // 注意：這兩個是方向決定的**固定**上下限，段位上限只在 s_logic_throttle.c 內生效。
                //   OutputMin 只在「加速中」的分支作為地板 (見 logic_motor_getUpdateParams)，
                //   目標為 0 時走減速分支，不會被墊高 —— e-lock 因此不需要在這裡另外處理。
                if (g_stSystemData.bMotorDirection == 0) {
                    u16lThrottleOutputMax = LOGIC_THROTTLE_FWD_OUTPUT_MAX;
                    u16lThrottleOutputMin = LOGIC_THROTTLE_FWD_OUTPUT_MIN;
                } else {
                    u16lThrottleOutputMax = LOGIC_THROTTLE_REV_OUTPUT_MAX;
                    u16lThrottleOutputMin = LOGIC_THROTTLE_REV_OUTPUT_MIN;
                }

                // 更新馬達最後要執行的轉速
                // 注意：此處呼叫舊的無正負號函式，因此傳入 abs() 值並強制轉型指標

#if CODESW_THROTTLE_ACCEL_FILTER_ENABLE == 1
                // 限斜率 + 一階濾波。曲線在程式內選定，與原本的 step/time 曲線一樣不吃外部參數。
                //   加速用 ACCEL_FILTER_CURVE_SELECT、減速用 DECEL_FILTER_CURVE_SELECT
                //   (兩者獨立 —— 減速是安全相關行為,不跟著使用者選的加速段位走)。
                //   反轉走 REV_*_FILTER_* 那組獨立參數,不再意外沿用正轉的加速曲線。
#if ACCEL_FILTER_CURVE_FROM_MODBUS == 1
                uint8_t u8lAccelCurveIndex = (uint8_t)g_stSystemData.sSharedData.eAccelCurve;
#else
                uint8_t u8lAccelCurveIndex = ACCEL_FILTER_CURVE_INDEX_DEFAULT;
#endif
                i8MotorResult = logic_motor_getUpdateParamsFiltered(
                        &u16MotorActiveRpm,
                        g_stSystemData.u32TimeMs,
                        u16lCurrentRpm,
                        u16lTargetRpm,
                        u16lThrottleOutputMax,  // 最大轉速限制 (方向決定的固定上限,非段位)
                        u16lThrottleOutputMin,  // 最小轉速限制 (僅加速時作為地板)
                        &g_stSystemData.sStepTime,
                        u8lAccelCurveIndex,
                        (g_stSystemData.bMotorDirection != 0),  // 反轉 → 用 REV_* 獨立參數
                        DECEL_FILTER_CURVE_INDEX_DEFAULT);      // 減速曲線,獨立於加速段位
#else
                i8MotorResult = logic_motor_getUpdateParams(&u16MotorActiveRpm,
                                                            g_stSystemData.u32TimeMs,
                                                            u16lCurrentRpm,
                                                            u16lTargetRpm,
                                                            u16lThrottleOutputMax,  // 最大轉速限制
                                                            u16lThrottleOutputMin,  // 最小轉速限制
                                                            &g_stSystemData.sStepTime);
#endif

                if (i8MotorResult == 0)  // 馬達控制成功
                {
                    // 更新當前轉速
                    g_stSystemData.i16CurrentRpm = (int16_t)u16MotorActiveRpm;
                    g_stSystemData.i16ActiveRpm = (int16_t)u16MotorActiveRpm;
#if CODESW_X2C_SCOPE_ENABLE == 1
                    // 斜坡/濾波「後」的取樣點：實際送往速度環的命令
                    g_i16ScopeCmdOut = (int16_t)u16MotorActiveRpm;
#endif
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
            u16lCurrentSpeedKmh_x10 = g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10;

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
                //   驅動由 UVW 三相短路處理 —— 放油門且近停即進鎖,PWM 被 override 成下橋全開,
                //   FOC 的輸出到不了馬達 (V0.24 前是靠輸出級的 bStallReleaseCoast 關 PWM,
                //   見下方 PWM 輸出分支的「已移除」說明)。
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
        // [FIX 2026-08-19] 進鎖的命令條件由「斜坡後的命令」(ReferenceRAW/qVelRef == 0) 改為
        //   「油門目標」(i16TargetRpm == 0)，與出鎖條件 bUvwDriveRequested 統一成**同一個訊號
        //   的遲滯對** (0 進鎖 / >= UVW_LOCK_RELEASE_REF 出鎖)。車速門檻完全不動。
        //   [為何] 舊寫法讓進鎖比「放油門」晚了整條減速斜坡 (曲線 2 從最高速約 700~800ms)，
        //   而同樣看車速的 coast(PWM 全關) 卻是放油門瞬間就成立 → 兩者之間出現一段
        //   「PWM 全關、零制動」的窗口。車若在該窗口因路面微傾斜或殘餘動量以 0.2~0.5 km/h
        //   潛行,HallPulsesLatch 就回不到門檻以下 → UVW 永遠進不去 → V0.23 起 coast 是閂鎖
        //   也出不來 → 只剩 EMB 的 1500ms failsafe 帶速夾停。實車症狀:「平地放油門減速，
        //   UVW lock 有時未啟動、車子繼續低速前進，最後被 EMB 夾停而震動」。
        //   改用油門目標後,低速區一律由三相短路接手 (有制動力、且結構上不可能驅動馬達)，
        //   speed mode 不再需要也不再有 PWM 全關狀態 —— bStallReleaseCoast 因此一併移除
        //   (見下方 PWM 輸出分支的說明)。
        //   附帶效果:堵轉/頂牆放油門時 (命令很大、latch=0) 也會立即進短路,那正是舊
        //   bStallReleaseCoast 要守的空窗。
        bool bUvwStopCommanded = (g_stSystemData.i16TargetRpm == 0) &&
                                 (HallPulsesLatch < UVW_LOCK_STOP_PULSES);
        bool bUvwDriveRequested = (abs(g_stSystemData.i16TargetRpm) >= UVW_LOCK_RELEASE_REF);

        // [V0.24] **EMB 機械上夾住時，UVW 短路一律保持生效** —— 煞車夾住的期間馬達沒有任何
        //   理由被 FOC 驅動,也沒有理由浮接:
        //     (1) 短路在靜止時零電流(制動扭矩 ∝ 反電動勢 ∝ 轉速)，不耗電不發熱;
        //     (2) 車若被外力推動,短路立刻提供制動力,和 EMB 同方向;
        //     (3) 結構上不可能驅動馬達 → 取代舊 coast(浮接)所有的保護職責。
        //   包含 LOCKED 與 FAULT(A04) 兩種夾住狀態。**進鎖不看車速門檻** —— EMB 已經夾住,
        //   車速的顧慮由夾煞路徑自己負責(見 s_logic_embraker.h 的動作準則)。
        //   **出鎖也被它擋住**:倒溜閂鎖或段位 0 時駕駛握著油門,EMB 仍夾住,此時放行 FOC 只會
        //   讓馬達對著夾緊的煞車出力(堵轉發熱)。EMB 一放開(RELEASED)本旗標即消失,
        //   同一個 tick 內 bUvwDriveRequested 就會解鎖,加速反應不受影響。
#if CODESW_EMBRAKER_ENABLE
        E_EMBRAKER_STATE eUvwEmbState = logic_embraker_getStatus();
        bool bUvwEmbClamped = (eUvwEmbState == EMBRAKER_STATE_LOCKED) ||
                              (eUvwEmbState == EMBRAKER_STATE_FAULT);
#else
        bool bUvwEmbClamped = false;
#endif

        if (uGF.UVWLock == 1) {
            // 已鎖定：維持到駕駛重新給出明顯油門(遲滯)，但 EMB 還夾著就不放
            if (bUvwDriveRequested && !bUvwEmbClamped) {
                uGF.UVWLock = 0;
            }
        } else if ((bUvwStopCommanded && !bUvwDriveRequested) || bUvwEmbClamped) {
            // 命令歸零且低速 (已移除 30ms debounce)，或 EMB 已夾住：立即進鎖
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

        IbusCalc();
        //  Calculate qIalpha,qIbeta from qIa,qIb
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

        // [設計準則 2026-08-19] **speed mode 不使用 coast(PWM 全關)**：
        //   馬達只有兩種通電狀態 —— (1) FOC 以 reference 0 制動 / 驅動、(2) UVW 三相短路。
        //   低速段(HallPulsesLatch < UVW_LOCK_STOP_PULSES)與**EMB 夾住的全程**一律是 (2)。
        //   PWM 全關只剩三個系統層面的理由,全部與停車路徑無關:
        //     (1) uGF.RunMotor == 0  未運轉 —— 含 IBKS(手剎車/充電中)按下,充電時不可短路;
        //     (2) uGF.Fault == 1     故障閂鎖(過流/霍爾異常/60 秒堵轉) —— 故障的橋不該短路;
        //     (3) 電池模組輸出禁制   過壓/低壓保護。
        //   [已退役:uGF.Coast] 它原本讓 bMotorStop(電池/過溫)、VR 讀取失敗、堵轉歸零、
        //     以及「EMB 已夾住且油門為 0」都走 PWM 全關。這些情境的共同需求是「馬達不要出力」,
        //     而三相短路同樣滿足(結構上不可能驅動馬達),還多了制動力與明確的橋狀態,
        //     因此不再需要浮接。命令歸零的動作(MotorStallForceOutputZero 等)全部保留;
        //     旗標本身仍在 control2.h(見該處註解),但**已無任何讀取者**。
        //   ⚠ 本設計**依賴 CODESW_UVW_LOCK_ENABLE == 1**:短路分支是「馬達不要出力」的唯一
        //     實作者。若為了除錯把 UVW lock 關掉,停車與各保護路徑就只剩「命令歸零」一層
        //     (FOC 仍在通電),請自行評估風險或暫時把 uGF.Coast 加回本判斷式。
        //
        // [已移除:bStallReleaseCoast / s_bCoastLatched]
        //   舊機制:「頂牆/堵轉放開後,近停且尚未 UVW 短路時,強制關 PWM 不讓 FOC 主動輸出」,
        //   V0.23 又為了修上坡震動(coast 反覆進出造成 5~10Hz 扭矩階躍)把它改成閂鎖。
        //   它守的那個空窗之所以存在,是因為 UVW 進鎖看的是**斜坡後的命令**(ReferenceRAW),
        //   堵轉/放油門時命令要走完整條斜坡才歸零,而 coast 看的是**油門目標**(立刻歸零)。
        //   V0.24 把 UVW 進鎖改看油門目標後(見上方 bUvwStopCommanded 的說明):
        //     - 空窗消失,低速區改由三相短路占據 —— 短路結構上不可能驅動馬達,又有制動力,
        //       比全關浮接更強,原本要防的「零速↔反向換相往後驅動」保護只增不減;
        //     - 上坡震動的機制(coast 進出)整條不存在,不需要閂鎖;
        //     - V0.23 閂鎖帶來的副作用一併消失:平路潛行時 coast 閂住 → 零制動 → latch 回不到
        //       門檻下 → UVW 進不去也出不來 → 只剩 1500ms failsafe 帶速夾停(實車回報的
        //       平地減速頓挫)。
        //   其根因(HallPeriod <= HallMinPeriod 時把 Speed 誤斷言成 32767)已於 2026-08-11
        //   在 CNRead_Inline 修掉(見該處註解),本移除即該註解所說的「等實車驗證過再談」。
#if CODESW_BATTERY_PROTECTION_MODULE_ENABLE
        // 使用 s_logic_battery 模組的輸出禁制旗標來決定是否關閉 PWM
        if ((uGF.RunMotor == 0) || (uGF.Fault == 1) || logic_battery_shouldProhibitOutput()) {
#else
    if ((uGF.RunMotor == 0) || (uGF.Fault == 1)) {
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
        // X2CScope 觀測用的佔空比取樣。無條件執行，符號不隨開關消失。
        X2CPG1Duty = INVERTERA_PWM_PDC1;
        X2CPG2Duty = INVERTERA_PWM_PDC2;
        X2CPG3Duty = INVERTERA_PWM_PDC3;
#if CODESW_X2C_SCOPE_ENABLE == 1
        DiagnosticsStepIsr();  // X2CScope 取樣更新 (UART2)
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
            // [下坡滑動] T1INTCnt 在下一行就被歸零，但「Timer1 這段期間是否溢位過」決定了
            //   TMRLatch 可不可信 (溢位 = 週期 > 65535 ticks，即車速低於約 0.15 km/h，
            //   TMRLatch 只是取模後的殘值)。加速判別必須比較真實週期，故在歸零前取樣。
            //   附帶效果：車停過 (>42ms 無邊緣) 必然溢位 → 正好用來作廢跨越停車的舊週期。
            unsigned int u16T1OvfAtEdge = T1INTCnt;
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

            // [FIX 2026-08-11] 移除原本的 `else if (HallPeriod <= HallMinPeriod) Speed = 32767;`
            //   —— 那是把「物理上不可能的量測」轉換成「最大確信的極端值」。
            //
            //   HALL_MIN_PERIOD = 434 是 SPEED_FS_RPM(12000 馬達RPM) 對應的 Timer1 週期
            //   (見 motor_scale.h)，而 12000 = 2.96 x MOTOR_MAX_RPM。所以
            //   `HallPeriod <= HallMinPeriod` 代表「量到馬達物理上限 3 倍的轉速」——
            //   它只可能來自停車顫動/接點彈跳/雜訊，絕不可能是真實速度。
            //   刪掉分支後沒有 else，Speed 自然保持前值,這正是「本次取樣無效」該有的行為。
            //   不改成 0(會謊稱靜止,可能在真實速度下誤觸 UVW lock 或堵轉偵測)，
            //   也不改成上限(仍是在垃圾取樣上斷言高速)。
            //
            //   這一行是三個症狀的共同根因,前兩個過去分別被繞過而非修復:
            //   (1) [堵轉放開後倒行 1m+] CalculateParkAngleHall() 的
            //       `if (abs(Speed) <= Q15(0.02))` 低速分支刻意**不依賴 uGF.Direction**
            //       (內層 if/else 兩邊完全相同) —— 因為近零速時方向由邊界顫動決定、不可信。
            //       Speed=32767 讓這道防護失效 → 改走內插分支 → 吃不可信的 Direction 且
            //       HallPeriodFiltered 塌小使角度步長爆大 → 電流向量最多錯置 120°
            //       (HallAngle 的 ±60° 偏移 + 往錯方向推進撞上 ±60° 夾限) → 扭矩反向。
            //       同時速度環 P 項因 32767 飽和到電流上限(清積分器擋不住,P 項不需歷史)。
            //       → 過去用 bStallReleaseCoast 關 PWM 圍堵。
            //   (2) [UVW lock 永遠 latch 不上] 見下方 bUvwStopCommanded 處的註解。
            //       → 過去改用 HallPulsesLatch 取代 Speed 圍堵。
            //   (3) [堵轉保護失效] bClearlyMoving = abs(Speed) >= Q15(0.06) 因 32767 恆為真
            //       → 釋放計數器 500ms 後把 faultMotorStall.counter 歸零 → 顫動時堵轉計數
            //       永遠累積不到 8 秒 → 電流砍半與 60 秒閂鎖都不啟動。此條尚未實車確認。
            //
            //   [2026-08-19 更新] (1) 的圍堵措施 bStallReleaseCoast 已移除 —— 那個「近停但
            //     UVW 還沒短路」的空窗來自 UVW 進鎖看錯訊號(等斜坡後的命令),進鎖改看油門目標
            //     後空窗消失,低速區改由三相短路占據(結構上不可能驅動馬達,保護只增不減)。
            //     UVW_LOCK_STOP_PULSES 仍保留為「已接近靜止」的判斷門檻。
            if ((HallPeriodFiltered > HallMinPeriod) && (T1INTCnt == 0)) {
                Speed = __builtin_divf(HallMinPeriod, HallPeriodFiltered);
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

            // --- [有動力倒溜] 反向霍爾邊緣「淨」計數 (供 EMB 立即鎖定) ---
            //   每個邊緣比對「滾動方向」與「排檔方向」：反向 +1、正向 -1 (地板 0)。
            //   用邊緣「計數」而非速度，是為了讓偵測與速度無關 —— 再慢的潛行倒溜也會在固定
            //   位移(1 邊緣 = 車輪 1.75 mm)內被抓到，才能保證「不超過 1/4 車輪」的規格。
            //
            //   [2026-08-15] 參考訊號由「命令方向 (piInputOmega.inReference)」改為
            //     「排檔方向 (uGF.DirSW，來自 F/R 硬體開關)」。
            //   [為何改] 舊版以命令方向為武裝來源，命令降到 EMB_ROLLBACK_CMD_THRESHOLD(100)
            //     以下就解除武裝、計數歸零。上坡點放油門的實際時序:
            //       t=0     點油門     命令 = 1183 count，武裝生效
            //       t=T     放油門     TargetRpm=0，進入減速斜坡
            //       t=T~T+90ms 命令由 1183 降到 0，中途跨過 100 → **解除武裝、計數歸零**
            //     倒溜位移一大半發生在 t=T~T+90ms 這段，舊版完全接不到 → 只能等 V0.21 的
            //     沒減速偵測 (26mm 位移門檻) 接手。實車回報「上坡下滑 EMB 上鎖時前輪翹起」
            //     的根因即在此:動能累積到 26mm 時峰值減速度已能翻覆。
            //   [改用排檔訊號的三個理由]
            //     (1) F/R 硬體開關訊號可信，不會像命令一樣在減速斜坡上「消失」
            //     (2) 排檔=前進但車在倒溜，本來就是**駕駛意圖**與**實際運動**不符,語意最強
            //     (3) 覆蓋窗口從「命令 > 100」延伸到「排檔=F 且 EMB 曾 RELEASE」全程
            //   [為何加「EMB 曾 RELEASE」] 避免車停著、EMB LOCKED、駕駛剛切 F 開關時
            //     人為推車後退立刻上鎖(硬體上煞車已在鎖住狀態，此鎖定沒意義但會影響觸感)。
            //     此條件由 EMB LOCK 動作處清零、RELEASE 動作處置位(見該處)。
            //
            //   為何用「淨」而非「連續」：規格管的是淨倒溜位移，而連續計數只能界定單調位移。
            //   上坡與重力拉鋸時 (倒溜→積分器充飽→推前一小段→再倒溜)，連續計數每次都被歸零，
            //   淨位移卻早已超過 1/4 車輪 → 永遠不鎖。正向抵扣可讓拉鋸下的計數仍單調爬到門檻。
            //   地板取 0 而不讓它變負，是為了不讓長距離正常前進「預存額度」而延誤後續偵測。
            //   靜止抖動時方向交替 → 正負相抵停在 0 附近，故仍自動免疫。
            {
                // 排檔方向：uGF.DirSW 由 I_FR_SWITCH_PIN 直接讀取(見 main.c bMotorDirection 賦值處)。
                //   0=前進、1=倒退。約定符號:bEmbGearReverse=true 表示排檔在倒退位置。
                bool bEmbGearReverse = (uGF.DirSW != 0);

                if (g_bEmbRollbackArmed) {
                    bool bEmbRollingNeg = (uGF.Direction == uGF.DirectionDefault);
                    if (bEmbRollingNeg != bEmbGearReverse) {
                        // 滾動方向與排檔方向不一致 → 倒溜
                        if (g_u8EmbRevEdgeCnt < 255u) {
                            g_u8EmbRevEdgeCnt++;
                        }
                    } else if (g_u8EmbRevEdgeCnt > 0u) {
                        g_u8EmbRevEdgeCnt--;  // 方向一致 → 抵扣一個邊緣的倒溜位移
                    }
                } else {
                    g_u8EmbRevEdgeCnt = 0;  // 未武裝 → 保持歸零
                }
            }

            // --- [下坡滑動] 命令歸零後的「淨」位移計數 (帶號，供 EMB 立即鎖定) ---
            //   閘門是「命令已歸零」，與倒溜計數的「命令有方向」正好互補：
            //   命令非零時本計數持續歸零，故它天然從命令歸零那一刻起算。
            //   符號慣例沿用上方 Speed 那行：Direction == DirectionDefault ⇒ 往負方向滾。
            //   飽和在 ±32000 而非讓它回捲 —— 回捲會讓 |cnt| 掃過 0 而漏掉已達標的位移。
            //
            //   同一個閘門下並行「車在加速判別 + 車速上限」，產出 g_u8EmbNoDecelCnt
            //   (名稱沿用 V0.21 的 NoDecel，語意已於 V0.24 改為「在加速」)：
            //   自由滑行的阻力恆為正 ⇒ 平路/上坡的霍爾週期必然逐步變長，「週期變短」就代表
            //   重力已超過全部阻力、車不會自己停。這是區分「平路正常滑行到停」與「下坡
            //   溜車」的唯一物理依據，且此推論不含車速項 —— 平路/上坡在任何車速、任何阻力下
            //   都不可能觸發 (完整理由與參數取值見 userparms.h 的 EMB_DOWNHILL_* 說明)。
            {
                // 相隔 EMB_DOWNHILL_NODECEL_LOOKBACK 個邊緣的週期環形緩衝。取 6 的倍數使
                //   比較的兩筆是同一組霍爾狀態轉換，感測器裝配不等距的誤差因此對消。
                static uint16_t su16EmbPeriodRing[EMB_DOWNHILL_NODECEL_LOOKBACK] = {0};
                static uint8_t su8EmbPeriodIdx = 0;
                static uint8_t su8EmbPeriodFill = 0;

                if (piInputOmega.inReference == 0) {
                    if (uGF.Direction == uGF.DirectionDefault) {
                        if (g_i16EmbZeroCmdEdgeCnt > -32000) {
                            g_i16EmbZeroCmdEdgeCnt--;
                        }
                    } else {
                        if (g_i16EmbZeroCmdEdgeCnt < 32000) {
                            g_i16EmbZeroCmdEdgeCnt++;
                        }
                    }

                    if (u16T1OvfAtEdge != 0u) {
                        // Timer1 溢位過 → 本次 HallPeriod 是取模殘值，不可用於比較；
                        //   且這代表車曾靜止或極慢 (<0.15 km/h)，緩衝內的舊週期已跨越那段
                        //   空白，同樣不可信 → 整組作廢重新累積。
                        su8EmbPeriodFill = 0u;
                        g_u8EmbNoDecelCnt = 0u;
                    } else {
                        uint16_t u16Old = su16EmbPeriodRing[su8EmbPeriodIdx];  // LOOKBACK 個邊緣前
                        su16EmbPeriodRing[su8EmbPeriodIdx] = HallPeriod;
                        if (++su8EmbPeriodIdx >= EMB_DOWNHILL_NODECEL_LOOKBACK) {
                            su8EmbPeriodIdx = 0u;
                        }
                        if (su8EmbPeriodFill < EMB_DOWNHILL_NODECEL_LOOKBACK) {
                            su8EmbPeriodFill++;  // 緩衝未滿，還沒有可比較的對象
                        } else if ((HallPeriod > EMB_DOWNHILL_MIN_PERIOD) &&
                                   (u16Old > HallPeriod) &&
                                   ((uint16_t)(u16Old - HallPeriod) >=
                                    (uint16_t)(u16Old >> EMB_DOWNHILL_NODECEL_TOL_SHIFT))) {
                            // [V0.24] 週期**真的變短**(超過雜訊裕度 1/2^TOL_SHIFT) = 車在加速
                            //   = 重力已超過全部阻力,車不會自己停;且車速仍在夾煞可接受的
                            //   上限以下(週期下限 = 速度上限;上限之上交回 UVW 短路/回充處理)。
                            // ⚠ 不要改回舊版的「週期沒變長就算沒減速」:那個判準的隱含減速度
                            //   門檻是 a_th = v²/(2^S x d_LOOKBACK),帶 v² 項 → 車速愈高愈鬆,
                            //   在 3 km/h 上限附近只有 0.26 m/s²,低於平路自由滑行阻力的量級
                            //   → 平路 2.6~3.0 km/h 這段會被判成「沒減速」而硬夾(V0.23 實車
                            //   回報的平地減速頓挫)。改成「必須變短」後判準只剩重力 vs 阻力,
                            //   平路/上坡在任何車速下結構上都不可能觸發。完整推導見 userparms.h。
                            // ⚠ 寫成「先比大小再相減」而非「HallPeriod <= u16Old - 容忍」:
                            //   後者在 16-bit 上會下溢 → 門檻回捲成極大值 → 該速度帶靜默失效。
                            //   本寫法全程留在 16-bit 且不可能溢位(與倒溜計數同一個紀律)。
                            if (g_u8EmbNoDecelCnt < 255u) {
                                g_u8EmbNoDecelCnt++;
                            }
                        } else {
                            g_u8EmbNoDecelCnt = 0u;  // 要求「連續」成立，斷一次就重算
                        }
                    }
                } else {
                    g_i16EmbZeroCmdEdgeCnt = 0;  // 命令非零 → 重新起算
                    su8EmbPeriodFill = 0u;       // 驅動中的週期不可拿來與滑行期比較
                    g_u8EmbNoDecelCnt = 0u;
                }
            }
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
    //=============================================================================
    // 母線電流量測    Ibus = Σ dᵢ·iᵢ        (dᵢ = PGxDC / MPER，0~1 的工作比)
    //
    // 為什麼減 HALF_PWMDUTY：數學上「不減」也完全正確 ——
    //   Σdᵢiᵢ = Σ(0.5 + (dᵢ−0.5))iᵢ = 0.5·Σiᵢ + Σ(dᵢ−0.5)iᵢ
    //   而 iabc.a = -(iabc.b + iabc.c) 使 Σiᵢ ≡ 0(整數層面精確，不是近似)，共模項消失，
    //   兩種寫法給出同一個 Ibus。減它的理由純粹是定點數的實作餘量：
    //     dᵢ ∈ [0,1] 的 Q15 表示在 dᵢ = 1.0(PGxDC = MPER，即 100% duty)時是 32768，
    //     放不進 int16 → 會飽和成 32767 而損失增益；
    //     (dᵢ−0.5) ∈ [−0.5, +0.5] → ±16387，餘量加倍，任何工作點都不會碰到邊界。
    //   順帶讓每相乘積與答案同數量級(不必用大數相減得小數)，精度略好一點而已
    //   (整個電氣週期的最大誤差 1.27 vs 1.47 LSB)。
    //   ⚠ 它對「電流量測共模偏移」沒有任何幫助：Σiᵢ ≡ 0 是由 iabc.a 導出來的，
    //   兩種寫法對偏移的敏感度完全相同(ε 的係數同為 d_b+d_c−2d_a)。
    //   註：HALF_PWMDUTY = 2500 而真正的半週期是 4999/2 = 2499.5，差 0.5 count 的誤差
    //   等於 0.5·Σiᵢ/MPER —— 同樣被 Σiᵢ ≡ 0 消掉，故無影響。
    //
    // 相位對應：LONGWIN 硬體把三相接線交換過(見 duty 寫入處)→ PG3=A、PG1=B、PG2=C。
    // 呼叫時機：必須在本週期新 duty 寫入「之前」，讀到的才是 ADC 取樣期間實際生效的 duty。
    //
    // 刻度：Ibus 為 Q15，與 iabc 同刻度(313.3 counts/A，Q15 1.0 ↔ 104.6 A = 感測器軌到軌)。
    //   物理上界 |Ibus| ≤ max|iᵢ| ≤ 32768，也就是 Q15 滿刻度 —— 極端情況(量測貼軌)會剛好
    //   踩到 int16 邊界，所以下面的 int32 加總與顯式夾制是必要的，不只是防護網。
    //=============================================================================

    // Δduty(原始計數) → (dᵢ−0.5) 的 Q15 表示。等效乘 32768/MPER = 6.5547；
    //   sacr(-4) 是左移 4 位(×16)，配合 Q11 常數 → 乘 IBUS_NORM_Q11/2048。
    //   |2500 × 6.5547| = 16387 < 32767，不會飽和。
    static inline signed int IbusDutyDevQ15(signed int i16DeltaDuty) {
        a_Reg = __builtin_mpy(i16DeltaDuty, IBUS_NORM_Q11, 0, 0, 0, 0, 0, 0);
        return __builtin_sacr(a_Reg, -4);
    }

    static inline signed int IbusFracMpy(signed int i16A, signed int i16B) {
        a_Reg = __builtin_mpy(i16A, i16B, 0, 0, 0, 0, 0, 0);
        return __builtin_sacr(a_Reg, 0);
    }

#if IBUS_DEADTIME_COMP_Q15 != 0
    // 回傳 int32：iabc.a 在極端量測下可達 -32768，int16 的 -(-32768) 會溢位。
    static inline int32_t IbusAbs(signed int i16Val) {
        return (i16Val < 0) ? -(int32_t)i16Val : (int32_t)i16Val;
    }
#endif

    void IbusCalc(void) {
        // 先正規化 duty、再乘電流 —— 順序決定精度：
        //   舊版是「先乘 → 在原始計數刻度捨位 → 最後才 ×6.55」，捨位誤差被放大 6.55 倍
        //   (每相 ±3.3 LSB，三相最壞 ±10 LSB)；先正規化後每相只剩 ±0.5 LSB。
        const signed int i16DevA = IbusDutyDevQ15((signed int)PG3DC - HALF_PWMDUTY);
        const signed int i16DevB = IbusDutyDevQ15((signed int)PG1DC - HALF_PWMDUTY);
        const signed int i16DevC = IbusDutyDevQ15((signed int)PG2DC - HALF_PWMDUTY);

        // 每相對母線電流的貢獻，已是 Q15 且與 Ibus 同刻度。
        // 用 int32 加總：感測器軌到軌 = Q15 滿刻度，所以單相貢獻可達 ±16387、
        //   三相同號會到 49161 —— int16 的 C 加法會「繞回」變號，故先加寬再夾。
        int32_t i32Ibus = (int32_t)IbusFracMpy(i16DevA, iabc.a)
                + (int32_t)IbusFracMpy(i16DevB, iabc.b)
                + (int32_t)IbusFracMpy(i16DevC, iabc.c);

#if IBUS_DEADTIME_COMP_Q15 != 0
        // 死區補償(見 hal/pwm.h 的 IBUS_DEADTIME_COMP_Q15)：Ibus_真實 = Σdᵢiᵢ − δ·Σ|iᵢ|。
        //   補償量隨電流自然趨於 0，零電流附近不需另設門檻。
        {
            const int32_t i32AbsSum = (int32_t)IbusAbs(iabc.a)
                    + (int32_t)IbusAbs(iabc.b) + (int32_t)IbusAbs(iabc.c);
            i32Ibus -= (i32AbsSum * IBUS_DEADTIME_COMP_Q15) >> 15;
        }
#endif
        if (i32Ibus > 32767) {
            i32Ibus = 32767;
        } else if (i32Ibus < -32768) {
            i32Ibus = -32768;
        }
        Ibus = (signed int)i32Ibus;

        // 上報用的定窗算術平均 (25.6ms) → IbusMeanQ15 → Modbus / IbusAmpX10
        s_i32IbusAccum += Ibus;
        if (++s_u16IbusSamples >= IBUS_AVG_SAMPLES) {
            IbusMeanQ15 = (signed int)(s_i32IbusAccum >> IBUS_AVG_SHIFT);
            s_i32IbusAccum = 0;
            s_u16IbusSamples = 0;
        }

        // 快速 IIR (τ=6.4ms) → IbusAVG，供 X2CScope 觀測動態
        s_i32IbusAvgAcc += (((int32_t)Ibus << IBUS_AVG_FRAC_BITS) - s_i32IbusAvgAcc)
                >> IBUS_IIR_SHIFT;
        IbusAVG = (signed int)(s_i32IbusAvgAcc >> IBUS_AVG_FRAC_BITS);
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
