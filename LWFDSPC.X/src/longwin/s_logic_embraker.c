#include "s_logic_embraker.h"
#include "s_logic_error_handler.h"
#include <stddef.h>
#include <stdlib.h>  // For abs()

// --- Module-Private Variables ---

static E_EMBRAKER_STATE s_eCurrentState = EMBRAKER_STATE_LOCKED;

// 「速度命令歸零(ReferenceRAW==0)」起算的計時，用於逾時 failsafe
static uint32_t s_u32LockTimerMs = 0;
static bool s_bFailsafeTiming = false;
// UVW lock 生效起算的計時，用於「UVW lock 後延遲鎖定 EMB」
static uint32_t s_u32UvwLockTimerMs = 0;
static bool s_bUvwLockTiming = false;
// 有動力下方向相反(倒溜/倒衝)起算的計時；latched 表示已因此上鎖、須待鬆油門才解
static uint32_t s_u32RollbackTimerMs = 0;
static bool s_bRollbackTiming = false;
static bool s_bRollbackLatched = false;

// --- Private Helper Functions ---

/**
 * @brief 根據方向(從馬達命令正負號判斷)檢查馬達命令是否有效(超過閾值)
 */
static bool _isMotorCommandActive(int16_t i16MotorCommand) {
    if (i16MotorCommand > 0) {
        // 正轉
        return (i16MotorCommand > EMBRAKER_RELEASE_THRESHOLD_FWD);
    } else if (i16MotorCommand < 0) {
        // 反轉
        return (i16MotorCommand < -EMBRAKER_RELEASE_THRESHOLD_REV);
    } else {
        // 命令為 0
        return false;
    }
}

// --- Public Function Implementation ---

bool logic_embraker_init(uint16_t u16IembMv) {
    // 重置所有計時/閂鎖旗標
    s_bUvwLockTiming = false;
    s_bFailsafeTiming = false;
    s_bRollbackTiming = false;
    s_bRollbackLatched = false;

    // 檢查初始故障狀態
    if (u16IembMv < EMBRAKER_FAULT_THRESHOLD_MV) {
        s_eCurrentState = EMBRAKER_STATE_FAULT;
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A19_EMB_SENSOR_FAULT, true);
        return false;  // 初始化失敗
    }
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A19_EMB_SENSOR_FAULT, false);

    // 初始化為鎖定狀態
    s_eCurrentState = EMBRAKER_STATE_LOCKED;
    return true;  // 初始化成功
}

E_EMBRAKER_ACTION logic_embraker_update(uint16_t u16IembMv,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
                                        bool bDirMismatch,
                                        uint32_t u32CurrentTimeMs) {
    E_EMBRAKER_ACTION eAction = EMBRAKER_ACTION_NONE;
    E_EMBRAKER_STATE eNextState = s_eCurrentState;

    bool bIsActive = _isMotorCommandActive(i16MotorCommand);
    bool bIsActualMotorCommandZero = (i16ActualMotorCommand == 0);

    // 優先處理故障恢復：如果處於故障狀態但電壓已恢復正常，則轉移到安全的鎖定狀態
    if (s_eCurrentState == EMBRAKER_STATE_FAULT) {
        if (u16IembMv >= EMBRAKER_FAULT_THRESHOLD_MV) {
            // 電壓恢復，轉移到LOCKED狀態以便下次重新檢查
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A19_EMB_SENSOR_FAULT, false);
            eNextState = EMBRAKER_STATE_LOCKED;
        } else {
            // 仍處於故障，保持鎖定
            eAction = EMBRAKER_ACTION_LOCK;
        }
    } else { // 非故障狀態，執行正常狀態機
        switch (s_eCurrentState) {
            case EMBRAKER_STATE_LOCKED:
                // 此狀態為馬達靜止，是進行「運轉前檢查」的地方
                if (s_bRollbackLatched) {
                    // [有動力倒溜閂鎖] 保持鎖定,直到駕駛鬆油門才解閂(避免坡上有油門時放開→立即又倒溜)。
                    eAction = EMBRAKER_ACTION_LOCK;
                    if (!bIsActive) {
                        s_bRollbackLatched = false;  // 鬆油門解閂;之後回歸正常運轉前檢查流程
                    }
                } else if (bIsActive) {
                    // 收到運轉指令，現在檢查IEMB
                    if (u16IembMv < EMBRAKER_FAULT_THRESHOLD_MV) {
                        // 運轉前檢查失敗，進入故障狀態
                        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A19_EMB_SENSOR_FAULT, true);
                        eNextState = EMBRAKER_STATE_FAULT;
                        eAction = EMBRAKER_ACTION_LOCK;
                    } else {
                        // 運轉前檢查通過，釋放煞車
                        eAction = EMBRAKER_ACTION_RELEASE;
                        eNextState = EMBRAKER_STATE_RELEASED;
                    }
                }
                break;

            case EMBRAKER_STATE_RELEASED:
                // 馬達正在運轉中，不檢查IEMB訊號
                if (!bIsActive) {
                    // 運轉指令已停止，準備鎖定煞車
                    eNextState = EMBRAKER_STATE_WAITING_TO_LOCK;
                    s_bUvwLockTiming = false;   // 重置 UVW lock 延遲計時
                    s_bFailsafeTiming = false;  // 重置 failsafe 計時 (改為命令歸零後才起算)
                    s_bRollbackTiming = false;  // 重置有動力倒溜計時
                } else if (bDirMismatch) {
                    // [有動力倒溜 failsafe] 命令方向與實際帶號回授異號且確實在滾動 → 計時,
                    //   持續 EMBRAKER_ROLLBACK_LOCK_MS 即強制鎖定並閂鎖(待鬆油門才解)。
                    //   馬達斷力交由既有 stall 保護處理(輪被鎖住→近零速+有命令→MotorStall 斷力並閂鎖)。
                    if (!s_bRollbackTiming) {
                        s_bRollbackTiming = true;
                        s_u32RollbackTimerMs = u32CurrentTimeMs;
                    } else if ((u32CurrentTimeMs - s_u32RollbackTimerMs) >= EMBRAKER_ROLLBACK_LOCK_MS) {
                        eAction = EMBRAKER_ACTION_LOCK;
                        eNextState = EMBRAKER_STATE_LOCKED;
                        s_bRollbackTiming = false;
                        s_bRollbackLatched = true;
                    }
                } else {
                    // 方向一致(正常運轉) → 重置倒溜計時
                    s_bRollbackTiming = false;
                }
                break;

            case EMBRAKER_STATE_WAITING_TO_LOCK:
                if (bIsActive) {
                    // 在鎖定前又重新給予指令，直接回到釋放狀態 (無需再次檢查IEMB)
                    eAction = EMBRAKER_ACTION_RELEASE;
                    eNextState = EMBRAKER_STATE_RELEASED;
                    s_bUvwLockTiming = false;   // 重置 UVW lock 延遲計時
                    s_bFailsafeTiming = false;  // 重置 failsafe 計時
                } else if (bReverseEdgeDetected) {
                    // [Plan B] 偵測到倒溜(反向霍爾邊緣) → 立即鎖定，繞過延遲。
                    // 適用於 UVW 短路在靜止時撐不住重力、車已開始倒溜的瞬間搶救。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                    s_bUvwLockTiming = false;
                } else if (bUVWLockActive) {
                    // [動作準則] UVW 三相短路(平順停車)生效後，計時達可設定延遲才鎖定 EMB。
                    //   計時從「UVW lock 生效」起算，延遲足夠讓馬達由短路減速到停止，
                    //   避免帶速鎖定造成騎乘震動。
                    s_bFailsafeTiming = false;  // UVW 主路徑接手，failsafe 不計時
                    if (!s_bUvwLockTiming) {
                        s_bUvwLockTiming = true;
                        s_u32UvwLockTimerMs = u32CurrentTimeMs;  // UVW lock 生效起算
                    } else if ((u32CurrentTimeMs - s_u32UvwLockTimerMs) >= EMBRAKER_SHORT_TO_LOCK_DELAY_MS) {
                        eAction = EMBRAKER_ACTION_LOCK;
                        eNextState = EMBRAKER_STATE_LOCKED;
                        s_bUvwLockTiming = false;
                    }
                } else {
                    // UVW lock 尚未生效 → 重置 UVW 延遲計時；由「命令歸零起算」的逾時 failsafe 保底。
                    s_bUvwLockTiming = false;

                    // [failsafe] 計時從「速度命令(ReferenceRAW)真正歸零」起算，而非鬆油門瞬間。
                    //   若從鬆油門起算，減速斜坡較長時 timeout 會在斜坡期間被吃光，命令一到 0
                    //   就立即帶速硬鎖 → 震動。改為命令歸零後才起算，才有真正的 settle 緩衝窗，
                    //   期間車可繼續減速、UVW lock 也有機會先生效走主路徑。
                    //   命令歸零後持續 EMBRAKER_LOCK_TIMEOUT_MS 仍未由 UVW lock 鎖定(例如霍爾
                    //   異常使 UVW lock 從未生效) → 強制鎖定，確保 EMB 不會永遠不鎖。
                    if (bIsActualMotorCommandZero) {
                        if (!s_bFailsafeTiming) {
                            s_bFailsafeTiming = true;
                            s_u32LockTimerMs = u32CurrentTimeMs;  // 命令歸零起算
                        } else if ((u32CurrentTimeMs - s_u32LockTimerMs) > EMBRAKER_LOCK_TIMEOUT_MS) {
                            eAction = EMBRAKER_ACTION_LOCK;
                            eNextState = EMBRAKER_STATE_LOCKED;
                            s_bFailsafeTiming = false;
                        }
                    } else {
                        // 命令尚未歸零(仍在減速斜坡上) → 不計時
                        s_bFailsafeTiming = false;
                    }
                }
                break;

            default:
                // 安全保護，任何未知狀態都回到鎖定狀態
                s_eCurrentState = EMBRAKER_STATE_LOCKED;
                eAction = EMBRAKER_ACTION_LOCK;
                break;
        }
    }

    s_eCurrentState = eNextState;
    return eAction;
}

E_EMBRAKER_STATE logic_embraker_getStatus(void) {
    return s_eCurrentState;
}