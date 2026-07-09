#include "s_logic_embraker.h"
#include "s_logic_error_handler.h"
#include <stddef.h>
#include <stdlib.h>  // For abs()

// --- Module-Private Variables ---

static E_EMBRAKER_STATE s_eCurrentState = EMBRAKER_STATE_LOCKED;
static uint32_t s_u32LockTimerMs = 0;

// [Modified Plan A] UVW 短路持續時間計時 (用於「短路生效後延遲鎖定 EMB」)
static uint32_t s_u32UvwLockTimerMs = 0;
static bool s_bUvwLockTiming = false;

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
                                        uint16_t u16SpeedKmhx10,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
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
                if (bIsActive) {
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
                    s_u32LockTimerMs = u32CurrentTimeMs;  // 啟動計時器 (故障安全網用)
                    s_bUvwLockTiming = false;             // 重置 Plan A 計時
                }
                break;

            case EMBRAKER_STATE_WAITING_TO_LOCK:
                if (bIsActive) {
                    // 在鎖定前又重新給予指令，直接回到釋放狀態 (無需再次檢查IEMB)
                    eAction = EMBRAKER_ACTION_RELEASE;
                    eNextState = EMBRAKER_STATE_RELEASED;
                    s_bUvwLockTiming = false;  // 重置 Plan A 計時
                } else if (bReverseEdgeDetected) {
                    // [Plan B] 偵測到倒溜(反向霍爾邊緣) → 立即鎖定，繞過延遲。
                    // 適用於 UVW 短路在靜止時撐不住重力、車已開始倒溜的瞬間搶救。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                    s_bUvwLockTiming = false;
                } else if (bUVWLockActive) {
                    // [Modified Plan A] UVW 三相短路(平順停車)生效後，計時達可調延遲才鎖定，
                    // 讓機械煞車的夾緊時機對齊車速到0，避免帶速鎖定或鎖定過晚倒溜。
                    if (!s_bUvwLockTiming) {
                        s_bUvwLockTiming = true;
                        s_u32UvwLockTimerMs = u32CurrentTimeMs;  // UVW 短路起算
                    } else if ((u32CurrentTimeMs - s_u32UvwLockTimerMs) >= EMBRAKER_SHORT_TO_LOCK_DELAY_MS) {
                        eAction = EMBRAKER_ACTION_LOCK;
                        eNextState = EMBRAKER_STATE_LOCKED;
                        s_bUvwLockTiming = false;
                    }
                } else {
                    // UVW 短路尚未生效 → 重置 Plan A 計時；保留原速度/逾時作為故障安全網。
                    s_bUvwLockTiming = false;

                    bool bShouldLockBySpeed = (u16SpeedKmhx10 < EMBRAKER_LOCK_SPEED_KMH_X10);
                    bool bShouldLockByTimeout = ((u32CurrentTimeMs - s_u32LockTimerMs) > EMBRAKER_LOCK_TIMEOUT_MS);

                    if (bIsActualMotorCommandZero && (bShouldLockBySpeed || bShouldLockByTimeout)) {
                        eAction = EMBRAKER_ACTION_LOCK;
                        eNextState = EMBRAKER_STATE_LOCKED; // 回到鎖定狀態，準備下一次的運轉前檢查
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