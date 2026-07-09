#ifndef S_LOGIC_MOTOR_H_
#define S_LOGIC_MOTOR_H_

#include <stdint.h>
#include <stdbool.h>

// --- Merged from s_logic_motor_config.h ---

// --- Default Motor Parameter Values ---
// Based on longwinConrtrolFunction_chunchi.md
#define LOGIC_MOTOR_DEFAULT_WHEEL_DIMENSION_INCH (80)                    // 預設輪徑 (7 吋 * 10)
#define LOGIC_MOTOR_DEFAULT_POLE_PAIRS (12)                               // 預設馬達極對數 (Doc: 2~20, NOPOLESPAIRS is 10)
#define LOGIC_MOTOR_DEFAULT_HALL_PPR (610)                                // 預設馬達霍爾每轉脈衝數 (51.7 Pulse/R * 10)
#define LOGIC_MOTOR_DEFAULT_EXTERNAL_SENSOR_PPR (6)                       // 預設外部輪速感測器每轉脈衝數 (1~6 Pulse/R)
#define LOGIC_MOTOR_DEFAULT_SPEED_SOURCE (LOGIC_MOTOR_SPEED_SOURCE_HALL)  // 預設速度來源 (0:IHU/Hall)

// --- Calculation Constants (Integer versions) ---
// Based on formulas in longwinConrtrolFunction_chunchi.md
#define LOGIC_MOTOR_PROGRAM_SPEED_MAX_VALUE (32768)  // FOC 程式 Speed 參數最大值 32768
#define LOGIC_MOTOR_PROGRAM_SPEED_RPM_FACTOR (3000)  // FOC 程式 Speed 參數轉 RPM 因子 3000
#define LOGIC_MOTOR_KMH_CONVERSION_FACTOR (440)      // RPM 與輪徑轉 KM/H 的轉換因子 (0.0047878 * 100000)
#define LOGIC_MOTOR_RPM_FROM_HALL_HZ_FACTOR (60)     // Hall 頻率轉 RPM 的轉換因子 (60)

/**
 * @brief 速度感測器來源選擇
 */
typedef enum {
    LOGIC_MOTOR_SPEED_SOURCE_HALL = 0,    /**< 使用馬達霍爾感測器 (IHU) */
    LOGIC_MOTOR_SPEED_SOURCE_EXTERNAL = 1 /**< 使用外部輪徑感測器 (ILSN) */
} E_LOGIC_MOTOR_SPEED_SOURCE_T;

/**
 * @brief 馬達參數配置結構
 */
typedef struct {
    uint16_t u16WheelDimensionInches;          /**< 輪徑 (吋 * 10) */
    uint8_t u8PolePairs;                       /**< 馬達極對數 */
    uint16_t u16HallPPR;                       /**< 馬達霍爾感測器每轉脈衝數 (PPR * 10) */
    uint8_t u8ExternalSensorPPR;               /**< 外部輪速感測器每轉脈衝數 (PPR) */
    E_LOGIC_MOTOR_SPEED_SOURCE_T eSpeedSource; /**< 當前使用的速度感測器來源 */
} S_LOGIC_MOTOR_CONFIG_T;

// --- Original from s_logic_motor.h ---

/**
 * @brief 馬達步進時間參數結構
 */
typedef struct
{
    int8_t i8Step;  // 步進大小 (正值為加速，負值為減速)
    int8_t i8Time;  // 時間間隔 (毫秒)
} S_MOTOR_STEP_TIME_T;

// --- Merged Function Prototypes ---

// --- Configuration functions ---
/**
 * @brief 初始化馬達參數配置模組
 * @note 將所有參數設定為預設值，並清除相關的錯誤狀態。應在系統啟動時調用。
 */
void logic_motor_configInit(void);

/**
 * @brief 設定輪徑尺寸
 * @param u16Inches 輪徑，單位：吋 * 10
 */
void logic_motor_setWheelDimension(uint16_t u16Inches);

/**
 * @brief 取得目前設定的輪徑尺寸
 * @return uint16_t 輪徑，單位：吋 * 10
 */
uint16_t logic_motor_getWheelDimension(void);

/**
 * @brief 設定馬達極對數
 * @param u8Pairs 馬達的極對數
 */
void logic_motor_setPolePairs(uint8_t u8Pairs);

/**
 * @brief 取得目前設定的馬達極對數
 * @return uint8_t 馬達的極對數
 */
uint8_t logic_motor_getPolePairs(void);

/**
 * @brief 設定馬達霍爾感測器每轉脈衝數 (PPR)
 * @param u16ppr Hall PPR值 * 10
 */
void logic_motor_setHallPPR(uint16_t u16ppr);

/**
 * @brief 取得馬達霍爾感測器每轉脈衝數 (PPR)
 * @return uint16_t Hall PPR值 * 10
 */
uint16_t logic_motor_getHallPPR(void);

/**
 * @brief 設定外部輪速感測器每轉脈衝數 (PPR)
 * @param u8ppr 外部感測器PPR值。若在外部感測器模式下設為0，會觸發A14錯誤。
 */
void logic_motor_setExternalSensorPPR(uint8_t u8ppr);

/**
 * @brief 取得外部輪速感測器每轉脈衝數 (PPR)
 * @return uint8_t 外部感測器PPR值
 */
uint8_t logic_motor_getExternalSensorPPR(void);

/**
 * @brief 設定速度感測器來源
 * @param eSource 速度感測器來源 (霍爾或外部)
 */
void logic_motor_setSpeedSource(E_LOGIC_MOTOR_SPEED_SOURCE_T eSource);

/**
 * @brief 取得目前設定的速度感測器來源
 * @return E_LOGIC_MOTOR_SPEED_SOURCE_T 速度感測器來源
 */
E_LOGIC_MOTOR_SPEED_SOURCE_T logic_motor_getSpeedSource(void);

/**
 * @brief 取得目前的完整馬達參數配置
 * @return S_LOGIC_MOTOR_CONFIG_T 包含所有當前馬達參數的結構副本
 */
S_LOGIC_MOTOR_CONFIG_T logic_motor_getCurrentConfig(void);

// --- Calculation functions (Integer versions) ---

/**
 * @brief (LWFOC) 將FOC內部ProgramSpeed值轉換為內部RPM
 * @param s16ProgramSpeed FOC提供的ProgramSpeed值 (通常為int16_t)
 * @return uint16_t 內部RPM值 * 10
 */
uint16_t logic_motor_LwfocGetInternalRpm(int16_t s16ProgramSpeed);

/**
 * @brief (LWFOC) 將內部RPM轉換為外部RPM
 * @param u16LwfocInternalRpm 內部RPM * 10
 * @param u8PolePairs 馬達極對數
 * @param u16SensorPpr 感測器每轉脈衝數 * 10 (根據eSpeedSource選擇HallPPR或ExternalSensorPPR)
 * @return uint16_t 外部RPM值 * 10。如果u16SensorPpr為0，則回傳0。
 */
uint16_t logic_motor_LwfocGetExternalRpm(uint16_t u16LwfocInternalRpm, uint8_t u8PolePairs, uint16_t u16SensorPpr);

/**
 * @brief 將RPM值和輪徑轉換為速度 (KM/H)
 * @param u16GenericRpm RPM值 * 10 (可以是馬達RPM或輪子RPM)
 * @param u16WheelDimensionInches 輪徑 * 10 (吋)
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H
 */
uint16_t logic_motor_getSpeedKmhFromRpm(uint16_t u16GenericRpm, uint16_t u16WheelDimensionInches);

/**
 * @brief (LWFOC) 複合計算：從FOC ProgramSpeed 計算速度 (KM/H)
 * @param s16ProgramSpeed FOC提供的ProgramSpeed值
 * @param psConfig 指向當前馬達配置的指標
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H。若psConfig為NULL，回傳0。
 */
uint16_t logic_motor_getSpeedKmhViaLwfoc(int16_t s16ProgramSpeed, const S_LOGIC_MOTOR_CONFIG_T *psConfig);

/**
 * @brief 從霍爾感測器頻率計算馬達機械RPM
 * @param u16HallFrequencyHz 霍爾感測器頻率 * 10 (Hz)
 * @param u8PolePairs 馬達極對數
 * @return uint16_t 馬達機械RPM * 10。如果u8PolePairs為0，則回傳0。
 */
uint16_t logic_motor_getMotorMechanicalRpmFromHallHz(uint16_t u16HallFrequencyHz, uint8_t u8PolePairs);

/**
 * @brief 從馬達機械RPM計算車輪RPM
 * @param u16MotorMechanicalRpm 馬達機械RPM * 10
 * @param u8PolePairs 馬達極對數
 * @param u16ExternalSensorPprAtWheel 外部輪速感測器每轉脈衝數 * 10 (PPR)
 * @return uint16_t 車輪RPM * 10。如果u16ExternalSensorPprAtWheel為0，則回傳0。
 */
uint16_t logic_motor_getWheelRpmFromMotorMechanicalRpm(uint16_t u16MotorMechanicalRpm, uint8_t u8PolePairs, uint16_t u16ExternalSensorPprAtWheel);

/**
 * @brief (Hall Hz) 複合計算：從霍爾感測器頻率計算速度 (KM/H)
 * @param u16HallFrequencyHz 霍爾感測器頻率 * 10 (Hz)
 * @param psConfig 指向當前馬達配置的指標
 * @return uint16_t 計算得到的速度 * 100，單位 KM/H。若psConfig為NULL，回傳0。
 */
uint16_t logic_motor_getSpeedKmhViaHallHz(uint16_t u16HallFrequencyHz, const S_LOGIC_MOTOR_CONFIG_T *psConfig);

// --- Original Function Prototypes ---

/**
 * @brief 根據步進參數更新馬達轉速，實現平滑加減速
 * @param pu16ActiveRpm 指向實際轉速的指標 (輸出)
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param u16CurrentRpm 當前轉速
 * @param u16TargetRpm 目標轉速
 * @param u16MaxRpm 最大轉速限制
 * @param u16MinRpm 最小轉速限制
 * @param psStepTime 步進時間參數結構指標
 * @return 0: 成功, -1: 時間未到或指標無效, -2: 時間參數無效, -3: 無需調整
 */
int8_t logic_motor_getUpdateParams(uint16_t *pu16ActiveRpm,
                                   uint32_t u32SystemTimeMs,
                                   uint16_t u16CurrentRpm,
                                   uint16_t u16TargetRpm,
                                   uint16_t u16MaxRpm,
                                   uint16_t u16MinRpm,
                                   S_MOTOR_STEP_TIME_T *psStepTime);

/**
 * @brief 將霍爾脈衝計數轉換為真實的 RPM 值
 * @param u16HallPulsesLatch 霍爾脈衝計數 (例如：100ms 內的計數)
 * @return uint16_t 真實 RPM 值
 */
uint16_t logic_motor_getRealRpm(uint16_t u16HallPulsesLatch);

/**
 * @brief (未使用) 根據真實 RPM 進行馬達控制
 */
int8_t logic_motor_controlByRealRpm(uint16_t *pu16ActiveRpm,
                                    uint32_t u32SystemTimeMs,
                                    uint16_t u16CurrentRpm,
                                    uint16_t u16TargetRpm,
                                    uint16_t u16MaxRpm,
                                    uint16_t u16MinRpm,
                                    S_MOTOR_STEP_TIME_T *psStepTime);

/**
 * @brief (未使用) 將真實 RPM 轉換為控制系統的參考值
 */
int16_t logic_motor_rpmToControlValue(uint16_t u16Rpm, uint16_t u16MaxRpm);

/**
 * @brief (未使用) 將真實 RPM 轉換為 Q15 格式
 */
int16_t logic_motor_convertRealRpmToQ15(uint16_t u16RealRpm);

/**
 * @brief (Signed RPM Model) 根據步進參數更新馬達轉速，實現平滑加減速
 * @param pi16ActiveRpm 指向實際轉速的指標 (輸出, 帶正負號)
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param i16CurrentRpm 當前轉速 (帶正負號)
 * @param i16TargetRpm 目標轉速 (帶正負號)
 * @param psStepTime 步進時間參數結構指標
 * @return 0: 成功, -1: 時間未到或指標無效, -2: 時間參數無效, -3: 無需調整
 */
int8_t logic_motor_getUpdateParamsSigned(int16_t *pi16ActiveRpm,
                                         uint32_t u32SystemTimeMs,
                                         int16_t i16CurrentRpm,
                                         int16_t i16TargetRpm,
                                         S_MOTOR_STEP_TIME_T *psStepTime);


#endif  // S_LOGIC_MOTOR_H_