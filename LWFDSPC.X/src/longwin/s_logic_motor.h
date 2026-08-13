#ifndef S_LOGIC_MOTOR_H_
#define S_LOGIC_MOTOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "codeSw.h"          // CODESW_THROTTLE_ACCEL_FILTER_ENABLE
#include "../motor_scale.h"  // MOTOR_POLE_PAIRS / SPEED_FS_RPM / GEAR_RATIO_X100

// --- Merged from s_logic_motor_config.h ---

// --- Default Motor Parameter Values ---
// Based on longwinConrtrolFunction_chunchi.md
#define LOGIC_MOTOR_DEFAULT_WHEEL_DIMENSION_INCH (80)                     // 預設輪徑 (8.0 吋 * 10)
#define LOGIC_MOTOR_DEFAULT_POLE_PAIRS (MOTOR_POLE_PAIRS)                 // 馬達極對數 (實測 3；6 極馬達)
// [DEPRECATED] 舊版用此欄位隱含代表減速齒比 (610/120 = 5.083，再配合當時 Speed 刻度
//   偏小 4 倍，合成 ÷20.33 = 真實齒比)。齒比已改由 motor_scale.h 的 GEAR_RATIO_X100
//   明確定義，本欄位與 logic_motor_setHallPPR/getHallPPR 已無 live 消費者。
#define LOGIC_MOTOR_DEFAULT_HALL_PPR (610)                                // [DEPRECATED] 見上方說明
#define LOGIC_MOTOR_DEFAULT_EXTERNAL_SENSOR_PPR (6)                       // 預設外部輪速感測器每轉脈衝數 (1~6 Pulse/R)
#define LOGIC_MOTOR_DEFAULT_SPEED_SOURCE (LOGIC_MOTOR_SPEED_SOURCE_HALL)  // 預設速度來源 (0:IHU/Hall)

// --- Calculation Constants (Integer versions) ---
// Based on formulas in longwinConrtrolFunction_chunchi.md
#define LOGIC_MOTOR_PROGRAM_SPEED_MAX_VALUE (32768)          // FOC 程式 Speed 參數最大值 32768
#define LOGIC_MOTOR_PROGRAM_SPEED_RPM_FACTOR (SPEED_FS_RPM)  // Q15 滿刻度對應的機械 RPM (12000)
#define LOGIC_MOTOR_KMH_CONVERSION_FACTOR (479)              // RPM 與輪徑轉 KM/H 的轉換因子 (0.0047878 * 100000)
#define LOGIC_MOTOR_RPM_FROM_HALL_HZ_FACTOR (60)             // Hall 頻率轉 RPM 的轉換因子 (60)

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

// ============================================================================
//  命令濾波曲線 (CODESW_THROTTLE_ACCEL_FILTER_ENABLE == 1 時生效)
// ============================================================================
//  結構：目標 → [限斜率 R] → [一階低通 τ] → 速度命令
//    限斜率決定「到達時間」，濾波只把折角磨圓(消掉起步/到達/換段的階躍)。
//    純一階濾波不能單獨用：它的加速率與剩餘誤差成正比 → 起步最猛、尾段最慢，
//    要把起步猛度壓到現行水準 (1.38 km/h/s) 需 τ≈4.1s，到達時間會變 19s。
//    加上限斜率後，到達時間與原本線性斜坡幾乎相同 (5.20s → 5.65s)，但無階躍。
//
//  四象限獨立：正轉/反轉 × 加速/減速，四組參數互不共用。
//    加速側早先已有濾波;減速側原本被排除(走 step/time 表)，因速度環增益調高後
//    (SPEEDCNTR_PTERM 3000→5000、ITERM 10→20) 放油門與換段的命令階躍被放大成頓挫，
//    故補上減速濾波。舊註解「若讓濾波接手減速會慢 10 倍以上」的前提是「只用一階濾波」;
//    本實作減速同樣是「限斜率 + 濾波」兩級，斜率直接對齊原減速表的量級，故無此問題。
//
//  ⚠ 為何減速的 K 必須遠小於加速的 K：兩邊全程時間差一個數量級。
//    加速全程約 3200 ms，τ = 2^7 × 2 = 256 ms 只佔 8%，磨圓了但延遲感覺不出來;
//    減速全程僅 240~600 ms，同樣的 τ=256 ms 會佔 50~85%，停車時間會加倍(安全問題)。
//    故減速取 K=3~5 (τ=16~64 ms)，維持與加速相近的「τ／全程」比例。
//
//  單位：命令 count (1 count = 0.366 機械 RPM = 0.00069 km/h)，見 s_logic_throttle.h 檔頭。
// ============================================================================
#define ACCEL_FILTER_TICK_MS 2u      // 斜坡/濾波的更新週期
#define ACCEL_FILTER_FRAC_BITS 10u   // 濾波狀態的小數位元數
                                     //   ⚠ 必要：天真的 `cmd += (target-cmd)>>K` 在誤差
                                     //   小於 2^K 時位移結果為 0 會永久卡住 (K=9 實測停在
                                     //   距目標 511 counts = 0.35 km/h 處)。帶小數位元後殘差 <1 count。
#define ACCEL_FILTER_CATCHUP_MAX 8u  // 主迴圈延遲時最多補算幾個 tick (避免無界迴圈)
#define ACCEL_FILTER_RESYNC_TOL 64u  // 命令被外部強制改寫(如失速保護歸零)時的重同步門檻

#if CODESW_X2C_SCOPE_ENABLE == 1
// ---------------------------------------------------------------------------
//  X2CScope 觀測：速度命令在斜坡/濾波「前 / 中 / 後」三個取樣點
//  單位皆為 Q15 速度命令 count (1 count = 0.366 機械 RPM = 0.00069 km/h)
//    g_i16ScopeCmdTarget  油門算出的目標，未經任何斜坡 → 轉油門時是階躍
//    g_i16ScopeCmdRateLim 限斜率後、一階濾波前 (只有新加速方式會更新；舊方式維持 0)
//    g_i16ScopeCmdOut     實際送往速度環的命令 (= i16ActiveRpm)
//  兩種加速方式都會更新 Target / Out，可直接 A/B 對照。
//  X2CScope 的取樣點在 ADC ISR (50us) 內，遠快於曲線 τ=0.26s，波形足夠清楚。
// ---------------------------------------------------------------------------
extern int16_t g_i16ScopeCmdTarget;
extern int16_t g_i16ScopeCmdRateLim;
extern int16_t g_i16ScopeCmdOut;
#endif

typedef struct
{
    uint8_t u8RateCountsPerTick;  // 限斜率：每 tick 增加的命令 count
    uint8_t u8FilterShift;        // K：時間常數 τ = 2^K x ACCEL_FILTER_TICK_MS
    uint16_t u16StartCmd;         // 起始速度 (命令 count)；加速起步時直接預載，不用爬上去
} S_ACCEL_FILTER_CURVE_T;

// 7 段。這個值同時是曲線表的陣列維度與 logic_motor_getUpdateParamsFiltered() 的索引
//   夾住上界(s_logic_motor.c)，改它不會動到任何一段曲線的內容。
#define ACCEL_FILTER_CURVE_COUNT 7u

// --- 曲線選擇 ---
// 原本的 step/time 曲線是「純程式內設定」：門檻與 {Step,Time} 全是 #define，
//   執行時只依油門電壓/當前輸出/方向查表，與 Modbus 或 EEPROM 完全無關。
//   新曲線沿用同一原則 —— 預設在程式內選定，不吃外部參數。
// 註：協定裡的 eAccelCurve (ID02/ID03 reg 0x09)、u8AccelMs_Ax1~Ax5 (reg 0x0F~0x11) 與
//   EEPROM 的 u8AccelCurveGroup 目前都只被解碼/回寫，沒有任何控制邏輯讀取，
//   且其語意未經實機驗證(LCD 送 0 就會落到最緩的曲線 1)，故預設不使用。
#define ACCEL_FILTER_CURVE_SELECT (5u)  // 1~7；曲線 3 = 與原本 step/time 同手感

// 1 = 改由 Modbus 的 eAccelCurve 選 (A0 = 第 1 組)；0 = 用上面的 ACCEL_FILTER_CURVE_SELECT
//   ⚠ 啟用前要注意：eAccelCurve 取自 (u16ControlMode & 0xFF)，是未經驗證的 8-bit 值，
//     直接當索引傳入 logic_motor_getUpdateParamsFiltered()。該函式對越界的處理是
//     **夾到最後一段(最快)**,不是夾到最緩:
//         if (u8CurveIndex >= ACCEL_FILTER_CURVE_COUNT) u8CurveIndex = COUNT - 1u;
//     追加曲線 6、7 之後,「最後一段」已由 R=8 變成 R=12(加速率 1.5 倍),
//     所以任何異常值會落到更猛的曲線上。啟用此開關前應改成夾到最緩或加白名單。
#define ACCEL_FILTER_CURVE_FROM_MODBUS (0)

#define ACCEL_FILTER_CURVE_INDEX_DEFAULT ((uint8_t)((ACCEL_FILTER_CURVE_SELECT) - 1u))

#if ((ACCEL_FILTER_CURVE_SELECT) < 1u) || ((ACCEL_FILTER_CURVE_SELECT) > ACCEL_FILTER_CURVE_COUNT)
#error "ACCEL_FILTER_CURVE_SELECT 必須是 1~7"
#endif

// 起始速度預設值 = 原本的 OUTPUT_MIN (549.3 RPM = 1.04 km/h)
#define ACCEL_FILTER_START_CMD_DEFAULT RPMX10_TO_CMD(5493)

// 曲線索引 0~6 對應曲線 1~7 (eAccelCurve A0 = 第 1 組)。
// R (counts/tick, tick=2ms) → 加速率 / 由起始速度到 8.29 km/h 的到達時間：
//   曲線1  R=2   0.69 km/h/s  10.7s      曲線5  R=8   2.76 km/h/s  3.2s
//   曲線2  R=3   1.04 km/h/s   7.3s      曲線6  R=10  3.45 km/h/s  2.6s  ← 追加
//   曲線3  R=4   1.38 km/h/s   5.7s      曲線7  R=12  4.14 km/h/s  2.1s  ← 追加
//   曲線4  R=6   2.07 km/h/s   4.0s
//   (曲線3 = 與原本 step/time 的 ENTRY_0 {2,1} 同手感)
//
// [2026-08-11] 追加曲線 6、7。**接在後面而非插在前面** —— 減速側當初追加兩段更柔的是插在
//   最前面(原 1~5 順移為 3~7)，加速側若照做，現行的 ACCEL_FILTER_CURVE_SELECT = 5 會從
//   R=8 變成 R=4，加速率悄悄砍半。接在後面則原本的曲線 1~5 編號與內容**完全不動**。
//
// K 全部維持 7 (τ=0.26s)：折角磨圓但不明顯延遲；要更軟可個別加大。
//   追加的兩段不必降 K —— τ／全程比為 9.8% (R=10) 與 12.2% (R=12)，仍在減速表所採用的
//   7~13% 區間內(見上方減速曲線說明)，濾波不會吃掉太多到達時間。
//
// ⚠ 安全：曲線 7 的加速率是 4.14 km/h/s = 1.15 m/s²，約為曲線 3(原本手感)的 3 倍。
//   醫療代步車起步時乘客未必抓穩，且高加速度會增加後傾風險 —— 曲線 6、7 上車前務必
//   實測起步衝擊與載人穩定性，確認可接受後再開放給使用者選。
#define ACCEL_FILTER_CURVE_TABLE_INIT                                           \
    {                                                                           \
        {2, 7, ACCEL_FILTER_START_CMD_DEFAULT},  /* 曲線 1 最緩 */               \
        {3, 7, ACCEL_FILTER_START_CMD_DEFAULT},  /* 曲線 2 */                    \
        {4, 7, ACCEL_FILTER_START_CMD_DEFAULT},  /* 曲線 3 = 原本手感 */         \
        {6, 7, ACCEL_FILTER_START_CMD_DEFAULT},  /* 曲線 4 */                    \
        {8, 7, ACCEL_FILTER_START_CMD_DEFAULT},  /* 曲線 5 */                    \
        {10, 7, ACCEL_FILTER_START_CMD_DEFAULT}, /* 曲線 6 (追加) */             \
        {12, 7, ACCEL_FILTER_START_CMD_DEFAULT}, /* 曲線 7 最快 (追加) */        \
    }

// ============================================================================
//  減速濾波曲線 (與加速完全獨立)
// ============================================================================
//  第三欄與加速的 u16StartCmd 位於量程的相反兩端，功能無法共用：
//    加速需要「起步直接跳到起始速度」省掉爬升;
//    減速需要「接近 0 時直接歸 0」殺掉一階濾波的漸近尾巴。
//  ⚠ 歸零吸附是必要的，不是選配。兩個理由：
//    (1) 延遲：K=4 時濾波輸出落後斜降約 τ×R ≈ 1000 counts，之後幾何衰減到能
//        四捨五入成 0 還需約 7.6τ ≈ 240 ms。而 main.c 的 bUvwStopCommanded 要求
//        ReferenceRAW 與 ctrlParm.qVelRef **恰好為 0**，UVW 短路與 EMB 上鎖才會啟動,
//        不吸附就是白等這 240 ms。
//    (2) 整數陷阱：`state -= state >> K` 在 state < 2^K 時位移結果為 0 而停止衰減。
//        以 FRAC_BITS=10 計，K >= 10 會讓命令永遠卡在 1 count 到不了 0，UVW 就再也
//        鎖不上(只剩 3 秒 failsafe)。吸附門檻讓這件事不可能發生。
// ============================================================================
typedef struct
{
    uint8_t u8RateCountsPerTick;  // 限斜率：每 tick 遞減的命令 count
    uint8_t u8FilterShift;        // K：時間常數 τ = 2^K x ACCEL_FILTER_TICK_MS
    uint16_t u16SnapToZeroCmd;    // 目標為 0 且輸出低於此值 → 直接歸零 (見上方說明)
} S_DECEL_FILTER_CURVE_T;

#define DECEL_FILTER_CURVE_COUNT 7u

// 減速曲線選擇。**獨立於 ACCEL_FILTER_CURVE_SELECT** —— 減速是安全相關行為,
//   不應跟著使用者選的加速段位走。
// ⚠ 曲線 1、2 是後來追加的「更柔」段,插在最前面以維持「編號越大越猛」的順序,
//   因此原本的曲線 1~5 已順移為 3~7。選 5 = 原本的曲線 3 (≈原 step/time 表中間值)。
#define DECEL_FILTER_CURVE_SELECT (2u)  // 1~7；愈小愈柔。3 = 追加前的曲線 1 (R=40)

#define DECEL_FILTER_CURVE_INDEX_DEFAULT ((uint8_t)((DECEL_FILTER_CURVE_SELECT) - 1u))

#if ((DECEL_FILTER_CURVE_SELECT) < 1u) || ((DECEL_FILTER_CURVE_SELECT) > DECEL_FILTER_CURVE_COUNT)
#error "DECEL_FILTER_CURVE_SELECT 必須是 1~7"
#endif

// 歸零吸附門檻預設 100 counts ≈ 0.069 km/h (體感上已完全停住)
#define DECEL_FILTER_SNAP_TO_ZERO_DEFAULT 100u

// 曲線索引 0~6 對應曲線 1~7。全程 = 12000 counts (8.29 km/h = 2.30 m/s) 降到 0。
// 原本的 step/time 減速表是 -25~-40 counts/ms，等效 50~80 counts/tick (tick=2ms)：
//   曲線  R    counts/ms  全程     K(τ)        制動距離*  備註
//   1     20   10         1200ms   6(128ms)    1.38 m     最柔
//   2     30   15          800ms   5( 64ms)    0.92 m
//   3     40   20          600ms   5( 64ms)    0.69 m     追加前的曲線 1
//   4     50   25          480ms   5( 64ms)    0.55 m     = 原表最慢段 (-25)
//   5     64   32          375ms   4( 32ms)    0.43 m     ≈ 原表中間值
//   6     80   40          300ms   4( 32ms)    0.35 m     = 原表最快段 (-40)
//   7    100   50          240ms   3( 16ms)    0.28 m     最猛
//   * 制動距離 = ½ x 2.30 m/s x 全程,假設車速跟得上命令(理論值,實車會更長)。
// K 隨 R 增大而減小,維持「τ／全程」比例在 7~13% (與加速側的 8% 相當)。
//
// ⚠ 安全：曲線 1 的制動距離約為曲線 5 的 3.2 倍。柔順度與制動距離是直接的取捨,
//   選用曲線 1~3 前務必實測制動距離並確認可接受。
//
// 註：原減速表是**速度相依**的(低速 -25、高速 -40,越接近停止越柔和)。改為單一 R
//   會失去該特性,但一階濾波本身就在磨圓尾段,很大程度替代了它。此取捨已確認接受。
#define DECEL_FILTER_CURVE_TABLE_INIT                                             \
    {                                                                             \
        {20, 6, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 1 最柔 1200ms */      \
        {30, 5, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 2      800ms */       \
        {40, 5, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 3      600ms */       \
        {50, 5, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 4 = 原表最慢 */       \
        {64, 4, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 5 ≈ 原表中間 */       \
        {80, 4, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT},  /* 曲線 6 = 原表最快 */       \
        {100, 3, DECEL_FILTER_SNAP_TO_ZERO_DEFAULT}, /* 曲線 7 最猛 */             \
    }

// ============================================================================
//  反轉專屬濾波參數 (與正轉完全獨立)
// ============================================================================
//  不做 5 段：反轉上限是固定值 (LOGIC_THROTTLE_REV_OUTPUT_MAX = 4000 counts =
//  2.76 km/h)、不隨段位變,per-level 曲線沒有意義。
//
//  ⚠ 改動前的實況：濾波呼叫沒有方向閘門,反轉一直在吃**正轉**的加速曲線
//    (R=8、預載 1500)。後果是反轉 0→2.76 km/h 由原 step/time 表意圖的 5~8 秒
//    ({1,2}~{3,4}) 變成約 0.63 秒,快了約 8 倍,且一踩就跳到反轉滿速的 38%。
//    這是加速濾波導入時的連帶效果,先前未被注意。
//  下列預設值刻意**維持該現況**(R=8/K=7/預載 1500),避免這次改動又動到反轉手感;
//    若要調回原 step/time 表的 5~8 秒,把 REV_ACCEL_FILTER_RATE 降到 1~2、
//    REV_ACCEL_FILTER_START_CMD 設 0 即可。
// ============================================================================
#define REV_ACCEL_FILTER_RATE 8u                                      // 每 tick 遞增 count
#define REV_ACCEL_FILTER_SHIFT 7u                                     // τ = 256 ms
#define REV_ACCEL_FILTER_START_CMD ACCEL_FILTER_START_CMD_DEFAULT     // 起始預載 1500

// 反轉減速：原反轉減速表為 -10~-22 counts/ms,等效 20~44 counts/tick,取中段 32。
//   4000 counts 全程 250 ms,τ=32 ms 佔 13%。
// [實車調校 2026-08-10] 32 → 23。反轉獨立減速參數實測效果良好,確定採用此模式。
//   23 counts/tick = 11.5 counts/ms → 4000 counts 全程 348 ms (原 32 為 250 ms)。
#define REV_DECEL_FILTER_RATE 23u
#define REV_DECEL_FILTER_SHIFT 4u
#define REV_DECEL_FILTER_SNAP DECEL_FILTER_SNAP_TO_ZERO_DEFAULT

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

#if CODESW_THROTTLE_ACCEL_FILTER_ENABLE == 1
/**
 * @brief 以「限斜率 + 一階濾波」更新馬達速度命令。加速與減速皆走濾波，四象限獨立參數。
 * @param pu16ActiveRpm    指向實際速度命令的指標 (輸出)
 * @param u32SystemTimeMs  系統時間戳記 (毫秒)
 * @param u16CurrentRpm    當前速度命令
 * @param u16TargetRpm     目標速度命令
 * @param u16MaxRpm        上限 (方向決定的固定上限；段位上限已在油門模組內套用)
 * @param u16MinRpm        下限 —— **只在加速時作為地板**。減速時絕不套用,否則命令會被
 *                         釘在 OUTPUT_MIN(1500 count) 永遠到不了 0。
 * @param psStepTime       step/time 參數；本函式已不使用(保留參數以維持呼叫端相容,
 *                         且濾波停用時的 fallback 路徑仍需要它)
 * @param u8CurveIndex     加速曲線索引 0~4 (對應曲線 1~5)，超出範圍自動夾住
 * @param bReverse         true = 反轉,改用 REV_*_FILTER_* 那組獨立參數
 * @param u8DecelCurveIndex 減速曲線索引 0~4，超出範圍自動夾住。bReverse 時忽略
 * @return 0: 成功, -1: 時間未到或指標無效, -3: 無需調整
 *
 * @note 濾波狀態在加速↔減速之間**共用且連續** —— 那是同一條命令訊號,切換時只換參數,
 *       狀態不重置,否則命令會跳。
 */
int8_t logic_motor_getUpdateParamsFiltered(uint16_t *pu16ActiveRpm,
                                           uint32_t u32SystemTimeMs,
                                           uint16_t u16CurrentRpm,
                                           uint16_t u16TargetRpm,
                                           uint16_t u16MaxRpm,
                                           uint16_t u16MinRpm,
                                           S_MOTOR_STEP_TIME_T *psStepTime,
                                           uint8_t u8CurveIndex,
                                           bool bReverse,
                                           uint8_t u8DecelCurveIndex);
#endif

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