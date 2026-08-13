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
// 有動力下的倒溜/倒衝：latched 表示已因此上鎖、須待鬆油門才解
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
    s_bRollbackLatched = false;

    // 檢查初始故障狀態
    if (u16IembMv < EMBRAKER_FAULT_THRESHOLD_MV) {
        s_eCurrentState = EMBRAKER_STATE_FAULT;
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A04_EMB_SENSOR_FAULT, true);
        return false;  // 初始化失敗
    }
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A04_EMB_SENSOR_FAULT, false);

    // 初始化為鎖定狀態
    s_eCurrentState = EMBRAKER_STATE_LOCKED;
    return true;  // 初始化成功
}

E_EMBRAKER_ACTION logic_embraker_update(uint16_t u16IembMv,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
                                        bool bRollbackDetected,
                                        bool bDownhillSlideDetected,
                                        bool bBrakeSwOn,
                                        bool bELockActive,
                                        uint32_t u32CurrentTimeMs) {
    E_EMBRAKER_ACTION eAction = EMBRAKER_ACTION_NONE;
    E_EMBRAKER_STATE eNextState = s_eCurrentState;

    bool bIsActive = _isMotorCommandActive(i16MotorCommand);
    bool bIsActualMotorCommandZero = (i16ActualMotorCommand == 0);

    // 優先處理故障恢復：如果處於故障狀態但電壓已恢復正常，則轉移到安全的鎖定狀態
    if (s_eCurrentState == EMBRAKER_STATE_FAULT) {
        if (u16IembMv >= EMBRAKER_FAULT_THRESHOLD_MV) {
            // 電壓恢復，轉移到LOCKED狀態以便下次重新檢查
            logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A04_EMB_SENSOR_FAULT, false);
            eNextState = EMBRAKER_STATE_LOCKED;
        } else {
            // 仍處於故障，保持鎖定
            eAction = EMBRAKER_ACTION_LOCK;
        }
    } else if (bBrakeSwOn) {
        // [IBKS 立即鎖定] 手剎車/充電中 (RC12 Low) → 立即鎖定 EMB，不經任何延遲。
        //   必須是獨立的最高優先分支，不能只靠「把馬達命令歸零」間接觸發，原因:
        //   (1) 油門仍踩住時 DoControl 的 1kHz 速度命令任務會持續重算 i16TargetRpm,
        //       使 bUvwDriveRequested 成立 → uGF.UVWLock 永遠 latch 不上 → UVW 主路徑(延遲
        //       EMBRAKER_SHORT_TO_LOCK_DELAY_MS 後鎖定)整條失效;
        //   (2) 只剩 EMBRAKER_LOCK_TIMEOUT_MS(3000ms) 的 failsafe 能鎖 → 完全不是「立即」,
        //       且 IBKS 在 3 秒內解除就從未鎖過;
        //   (3) 該 3 秒計時還會被競態清掉: 1kHz 任務若插在 main.c 對 i16TargetRpm 歸零與本
        //       函式讀取之間,本函式會看到非零命令 → WAITING_TO_LOCK 的 bIsActive 分支把狀態
        //       退回 RELEASED 並清掉 s_bFailsafeTiming,3 秒重新從頭計。
        //   同時把狀態強制設為 LOCKED,IBKS 解除後才能走正常的「運轉前檢查 → 釋放」流程,
        //   避免硬體被鎖住卻沒有任何分支負責解鎖。
        eAction = EMBRAKER_ACTION_LOCK;
        eNextState = EMBRAKER_STATE_LOCKED;
        s_bUvwLockTiming = false;
        s_bFailsafeTiming = false;
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
                } else if (bIsActive && !bELockActive) {
                    // [e-lock] 段位 0 時不進行運轉前檢查、不放開煞車 —— 電子鎖車。
                    //   正常情況下段位 0 的命令已在油門模組歸零，bIsActive 本就是 false;
                    //   這道 !bELockActive 是防護性的第二層，確保任何來源的非零命令都無法解鎖。
                    //   段位切回 1~5 後 bELockActive 轉為 false，此分支恢復正常運轉前檢查流程。
                    // 收到運轉指令，現在檢查IEMB
                    if (u16IembMv < EMBRAKER_FAULT_THRESHOLD_MV) {
                        // 運轉前檢查失敗，進入故障狀態
                        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A04_EMB_SENSOR_FAULT, true);
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
                } else if (bRollbackDetected) {
                    // [有動力倒溜] 偵測到即鎖 —— 不計時。偵測訊號是「與命令方向相反的淨霍爾邊緣
                    //   數達 N」(main.c 的 g_u8EmbRevEdgeCnt)，N 個邊緣就是固定的車輪位移
                    //   (1 邊緣 = 1.75 mm)，與速度無關，故慢速潛行倒溜也會在規格內的位移被攔下。
                    //   鎖定後閂鎖至鬆油門(避免坡上仍有油門時被 LOCKED 的運轉前檢查放開又倒溜)。
                    //   馬達斷力由 main.c 在 LOCK 動作時呼叫 MotorStallForceOutputZero() 處理。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                    s_bRollbackLatched = true;
                } else if (bDownhillSlideDetected) {
                    // [下坡滑動] 命令已歸零、車卻仍在移動 → 立即鎖定，不計時。
                    //   在 RELEASED 檢查是防禦性的：本狀態需要 bIsActive(未平滑的油門命令
                    //   > 門檻)，而偵測的閘門是「平滑後的命令已歸零」，兩者步調不一致，
                    //   理論上重疊窗口極短。不設閂鎖，理由見 s_logic_embraker.h。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                }
                break;

            case EMBRAKER_STATE_WAITING_TO_LOCK:
                if (bIsActive && !bELockActive) {
                    // [e-lock] 段位 0 時不得由此分支放開煞車，否則「切到 0 段 → 減速中」的途中
                    //   若有非零命令殘留，會在即將鎖定前又把煞車放掉。理由同 LOCKED 分支。
                    // 在鎖定前又重新給予指令，直接回到釋放狀態 (無需再次檢查IEMB)
                    eAction = EMBRAKER_ACTION_RELEASE;
                    eNextState = EMBRAKER_STATE_RELEASED;
                    s_bUvwLockTiming = false;   // 重置 UVW lock 延遲計時
                    s_bFailsafeTiming = false;  // 重置 failsafe 計時
                } else if (bRollbackDetected) {
                    // [有動力倒溜，遲到但有效] RELEASED 離開的判斷用未平滑的原始油門命令，
                    //   放油門瞬間就會歸零；但反向邊緣計數器是用平滑後的 inReference 武裝/
                    //   累加，衰減比較慢。兩者步調不一致時，計數達標的時間點可能落在狀態已經
                    //   切到 WAITING_TO_LOCK 之後 —— 這裡補上同一個判斷，避免那段時間漏接、
                    //   一路掉到 EMBRAKER_LOCK_TIMEOUT_MS 才鎖。處理方式與 RELEASED 分支相同
                    //   (立即鎖定 + 閂鎖)，維持行為一致。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                    s_bRollbackLatched = true;
                    s_bUvwLockTiming = false;
                    s_bFailsafeTiming = false;
                } else if (bDownhillSlideDetected) {
                    // [下坡滑動] 命令已歸零、車卻仍在移動達門檻 → 立即鎖定，不計時。
                    //   **這是本狀態的主要著力點** —— 下坡點放時後面三條路全都不通:
                    //     bReverseEdgeDetected(Plan B) 前提是 bUVWLockActive;
                    //     bUVWLockActive 需要車已近靜止,下坡達不到;
                    //     failsafe 要等 EMBRAKER_LOCK_TIMEOUT_MS,屆時車速已高 → 帶速硬夾。
                    //   在此攔下,車剛開始滑、動能極小,不適感最低。
                    //   不設閂鎖 (與 bRollbackDetected 不同)，理由見 s_logic_embraker.h。
                    eAction = EMBRAKER_ACTION_LOCK;
                    eNextState = EMBRAKER_STATE_LOCKED;
                    s_bUvwLockTiming = false;
                    s_bFailsafeTiming = false;
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