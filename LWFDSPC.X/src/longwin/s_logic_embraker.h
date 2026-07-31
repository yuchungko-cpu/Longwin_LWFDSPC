#ifndef S_LOGIC_EMBRAKER_H_
#define S_LOGIC_EMBRAKER_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 電磁煞車的內部狀態
 */
typedef enum {
    EMBRAKER_STATE_FAULT,           // 故障狀態
    EMBRAKER_STATE_LOCKED,          // 鎖定狀態 (煞車作用中)
    EMBRAKER_STATE_RELEASED,        // 釋放狀態 (馬達可運轉)
    EMBRAKER_STATE_WAITING_TO_LOCK  // 等待鎖定 (馬達停止，等待速度或時間觸發)
} E_EMBRAKER_STATE;

/**
 * @brief 電磁煞車模組回傳的動作指令
 */
typedef enum {
    EMBRAKER_ACTION_NONE,    // 無需改變狀態
    EMBRAKER_ACTION_LOCK,    // 指令：鎖定煞車 (將OEMB設為LOW)
    EMBRAKER_ACTION_RELEASE  // 指令：釋放煞車 (將OEMB設為HI)
} E_EMBRAKER_ACTION;

// --- 硬體相關參數定義 ---

// EM-Braker Input (IEMB) on Pin RD8
#define EMBRAKER_DIVIDER_R1 10000UL   // NOTE: Assuming same as throttle for now
#define EMBRAKER_DIVIDER_R2 100000UL  // NOTE: Assuming same as throttle for now

// --- 行為相關參數定義 ---
#define EMBRAKER_FAULT_THRESHOLD_MV 1500     // IEMB 故障檢測電壓閾值 (mV)
#define EMBRAKER_RELEASE_THRESHOLD_FWD 1600  // 正轉時，釋放煞車的馬達命令閾值
#define EMBRAKER_RELEASE_THRESHOLD_REV 1600  // 反轉時，釋放煞車的馬達命令閾值 (可設為不同)

// [EMB 動作準則]
//  (1) UVW lock 生效後，經過 EMBRAKER_SHORT_TO_LOCK_DELAY_MS(可設定) → 強制鎖定 EMB。
//      計時從「UVW lock 生效」起算。延遲長度應足夠讓馬達由 UVW 短路減速到完全停止，
//      避免帶速鎖定造成騎乘震動。上機依實際停止時間微調。
//  (2) [Plan B] 偵測到倒溜(反向霍爾邊緣) → 立即鎖定。
//  (3) [failsafe] 速度命令(ReferenceRAW)歸零後超過 EMBRAKER_LOCK_TIMEOUT_MS 仍未由 UVW lock
//      鎖定(例如霍爾異常導致 UVW lock 從未生效) → 強制鎖定,確保 EMB 不會永遠不鎖。
//      計時從「命令歸零」起算(非鬆油門瞬間)，避免 timeout 在減速斜坡期間被吃光而帶速硬鎖。
//  (4) [有動力倒溜 failsafe] 油門作用中(RELEASED),若「命令方向與實際帶號回授異號」(命令前進卻
//      後退、或命令後退卻前進)持續 EMBRAKER_ROLLBACK_LOCK_MS → 強制鎖定並閂鎖至鬆油門。
//      涵蓋陡坡上有動力卻被重力拉著反向、且尚未近停(UVW lock/Plan B 到不了)的情形。
//      方向由 main.c 用 piInputOmega.inReference/inMeasure(同座標系)判斷後以 bDirMismatch 傳入。
#define EMBRAKER_SHORT_TO_LOCK_DELAY_MS 50  // UVW lock 後延遲鎖定 EMB 的時間 (ms)，可設定
#define EMBRAKER_LOCK_TIMEOUT_MS 3000         // 命令歸零後逾時強制鎖定的故障安全網 (ms)
#define EMBRAKER_ROLLBACK_LOCK_MS 2000        // 有動力下方向相反持續此時間 → 強制鎖定 (ms)

// --- 以下參數目前未作為 EMB 動作條件 (保留定義供參考) ---
#define EMBRAKER_LOCK_SPEED_KMH_X10 5   // (停用) 舊版低於此車速(km/h×10)則鎖定

/**
 * @brief 初始化電磁煞車模組
 * @note 應在系統啟動時呼叫。此函式不直接存取硬體。
 * @param u16IembMv IEMB 腳位的初始電壓值 (mV)
 * @return bool true: 初始化成功 / false: 檢測到煞車故障 (A19)
 */
bool logic_embraker_init(uint16_t u16IembMv);

/**
 * @brief 更新電磁煞車的狀態機，並回傳應執行的動作
 * @note 應在主迴圈中定期呼叫。此函式不直接存取硬體。
 *       內部會根據 i16MotorCommand 的正負號自動判斷方向。
 * @param u16IembMv         IEMB 腳位的即時電壓值 (mV)
 * @param i16MotorCommand   目前的馬達驅動命令值 (例如: ReferenceRAW, 帶正負號)
 * @param i16ActualMotorCommand 實際發送給馬達的命令值 (用來確認是否為0)
 * @param bUVWLockActive    [Modified Plan A] UVW三相短路(平順停車)是否生效中
 * @param bReverseEdgeDetected [Plan B] 是否偵測到倒溜(與行駛方向相反的霍爾邊緣)
 * @param bDirMismatch      [有動力倒溜] 命令方向與實際帶號回授異號且確實在滾動(main.c 算好傳入)
 * @param bBrakeSwOn        [IBKS] 手剎車/充電中訊號作用中 (RC12 為 Low → uGF.BrakeSWOn==1)。
 *                          為 true 時**立即鎖定**，不經 UVW 延遲也不等 timeout failsafe。
 * @param u32CurrentTimeMs  目前的系統時間 (毫秒)
 * @return E_EMBRAKER_ACTION 應對煞車硬體執行的動作
 */
E_EMBRAKER_ACTION logic_embraker_update(uint16_t u16IembMv,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
                                        bool bDirMismatch,
                                        bool bBrakeSwOn,
                                        uint32_t u32CurrentTimeMs);

/**
 * @brief 獲取目前電磁煞車的內部狀態
 * @return E_EMBRAKER_STATE 目前的狀態
 */
E_EMBRAKER_STATE logic_embraker_getStatus(void);

#endif  // S_LOGIC_EMBRAKER_H_