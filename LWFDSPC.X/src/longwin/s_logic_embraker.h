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

/**
 * @brief 最後一次回傳 EMBRAKER_ACTION_LOCK 的原因
 * @note  純診斷用，不參與任何控制判斷。實車無法即時觀測時，事後以 debugger/X2CScope
 *        讀 main.c 的 g_u8EmbLockReason 與各原因計數器，即可知道停車是走哪條路徑夾的
 *        (正常停車應為 UVW_DELAY;若看到 FAILSAFE 或 DOWNHILL，代表停車鏈有路徑失效)。
 */
typedef enum {
    EMBRAKER_LOCK_REASON_NONE = 0,      // 尚未夾過
    EMBRAKER_LOCK_REASON_UVW_DELAY,     // 1 = 正常停車：UVW 短路生效 + SHORT_TO_LOCK_DELAY
    EMBRAKER_LOCK_REASON_FAILSAFE,      // 2 = 命令歸零後 LOCK_TIMEOUT 逾時 (帶速夾，異常)
    EMBRAKER_LOCK_REASON_DOWNHILL,      // 3 = 下坡滑動偵測 (硬夾)
    EMBRAKER_LOCK_REASON_ROLLBACK,      // 4 = 有動力倒溜偵測 (硬夾)
    EMBRAKER_LOCK_REASON_REVERSE_EDGE,  // 5 = Plan B：UVW 生效後偵測到反向邊緣
    EMBRAKER_LOCK_REASON_IBKS,          // 6 = 手剎車/充電中訊號 (硬夾)
    EMBRAKER_LOCK_REASON_ROLLBACK_HOLD, // 7 = LOCKED 狀態下倒溜閂鎖保持鎖定
    EMBRAKER_LOCK_REASON_FAULT,         // 8 = A04 IEMB 故障保持鎖定
    EMBRAKER_LOCK_REASON_PRERUN_FAIL    // 9 = 運轉前檢查 IEMB 不合格
} E_EMBRAKER_LOCK_REASON;

// --- 硬體相關參數定義 ---

// EM-Braker Input (IEMB) on Pin RD8
#define EMBRAKER_DIVIDER_R1 10000UL   // NOTE: Assuming same as throttle for now
#define EMBRAKER_DIVIDER_R2 100000UL  // NOTE: Assuming same as throttle for now

// --- 行為相關參數定義 ---
#define EMBRAKER_FAULT_THRESHOLD_MV 1500     // IEMB 故障檢測電壓閾值 (mV)
#define EMBRAKER_RELEASE_THRESHOLD_FWD 800  // 正轉時，釋放煞車的馬達命令閾值
#define EMBRAKER_RELEASE_THRESHOLD_REV 800  // 反轉時，釋放煞車的馬達命令閾值 (可設為不同)

// [EMB 動作準則]
//  (1) UVW lock 生效後，經過 EMBRAKER_SHORT_TO_LOCK_DELAY_MS(可設定) → 強制鎖定 EMB。
//      計時從「UVW lock 生效」起算。延遲長度應足夠讓馬達由 UVW 短路減速到完全停止，
//      避免帶速鎖定造成騎乘震動。上機依實際停止時間微調。
//      [V0.24] UVW lock 的進鎖條件是「油門目標歸零 且 HallPulsesLatch < UVW_LOCK_STOP_PULSES」
//      (main.c 的 bUvwStopCommanded)。V0.23 以前看的是減速斜坡**後**的命令(ReferenceRAW)，
//      比放油門晚了整條斜坡(最高速約 700~800ms)，而同樣看車速的 coast(PWM 全關)卻在放油門
//      瞬間就成立 → 中間那段「PWM 全關、零制動」窗口會讓車在微傾斜路面潛行、latch 回不到
//      門檻下 → UVW 永遠進不去 → 本準則整條失效，只剩 (3) 的 failsafe 帶速夾停(V0.23 實車
//      回報的平地減速頓挫)。改用油門目標後低速區一律由三相短路接手。
//      [V0.24] **speed mode 已完全取消 coast(PWM 全關)**，且 **EMB 夾住(LOCKED/FAULT)的全程
//      UVW 短路保持生效**(main.c 的 bUvwEmbClamped)。因此煞車夾住期間馬達一定是短路狀態:
//      靜止時零電流、被外力推動時提供與 EMB 同向的制動力、且結構上不可能被 FOC 驅動 ——
//      連「倒溜閂鎖/段位 0 時駕駛握著油門」也不會讓馬達對著夾緊的煞車出力。
//      PWM 全關只剩 RunMotor==0(含 IBKS/充電)、Fault 閂鎖、電池禁制三個系統理由。
//  (2) [Plan B] 偵測到倒溜(反向霍爾邊緣) → 立即鎖定。
//  (3) [failsafe] 速度命令(ReferenceRAW)歸零後超過 EMBRAKER_LOCK_TIMEOUT_MS 仍未由 UVW lock
//      鎖定(例如霍爾異常導致 UVW lock 從未生效) → 強制鎖定,確保 EMB 不會永遠不鎖。
//      **不看車速**,時間到就夾 —— 刻意如此,理由見該巨集處的 ⚠⚠。
//      計時從「命令歸零」起算(非鬆油門瞬間)，避免 timeout 在減速斜坡期間被吃光而帶速硬鎖。
//  (4) [有動力倒溜] RELEASED 與 WAITING_TO_LOCK 兩個狀態下,偵測到「命令一個方向、車卻往
//      反方向動」→ **立即**鎖定並閂鎖至鬆油門。涵蓋陡坡上有動力卻被重力拉著反向、尚未近停
//      (UVW lock/Plan B 到不了)的情形。規格:倒溜量不得超過 1/4 車輪(91 個霍爾邊緣 / 159 mm)。
//      偵測訊號由 main.c 的 CNRead_Inline 以「與命令方向相反的淨霍爾邊緣數」(反向 +1、
//      正向 -1、地板 0) (g_u8EmbRevEdgeCnt >= EMB_ROLLBACK_REV_EDGES) 產生後以
//      bRollbackDetected 傳入 ——
//      用邊緣計數而非速度,是因為 N 個邊緣等於固定的車輪位移(1 邊緣 = 1.75 mm),與速度無關,
//      再慢的潛行倒溜也會在規格內被攔下(舊版靠 0.45 km/h 速度門檻 + 2 秒計時,慢速倒溜
//      永遠不觸發、快速倒溜也已滑行數十公分)。門檻與抑制窗見 userparms.h。
//      WAITING_TO_LOCK 也要看這個訊號,是因為「油門是否算作用中(RELEASED↔WAITING_TO_LOCK)」
//      用的是未平滑的原始油門命令(放油門瞬間歸零),而反向邊緣計數器的武裝訊號是平滑後的
//      inReference(走減速斜坡,衰減較慢)——兩者步調不一致時,計數達標可能發生在狀態已經
//      切到 WAITING_TO_LOCK 之後,若只在 RELEASED 檢查會漏接,一路掉到 (3) 的 timeout 才鎖。
//  (5) [下坡滑動] 與 (4) 是**不同的物理現象**,不可混為一談:
//        (4) 倒溜   = 車往與命令**相反**的方向動 → 方向問題 → 規格管位移(≤1/4 車輪)
//        (5) 下坡滑動 = 車往與命令**相同**的方向動但比命令快 → 速度問題 → 無位移規格
//      (4) 的偵測是「反向 +1、正向 -1、**地板 0**」的淨計數,下坡往前滑每個邊緣都是 -1 被
//      夾在 0 → **結構上永遠偵測不到下坡滑動**,不是門檻調不調的問題。
//      情境:停在坡上「點油門後立刻放掉」→ EMB 釋放、車被重力拉動、命令立刻歸零。此時
//      UVW 短路扭矩 ∝ 轉速(ω≈0 幾乎無扭矩)、速度環 PI 在零誤差時輸出零扭矩且積分爬升慢、
//      (4) 又結構上不涵蓋 → 三條路全不通,只剩 (3) 的 failsafe 帶速硬夾。
//      本判斷在「車剛開始滑動」時即鎖定,動能極小,不適感最低。
//      偵測訊號由 main.c 產生後以 bDownhillSlideDetected 傳入,RELEASED 與 WAITING_TO_LOCK
//      兩個狀態都檢查(理由同 (4))。判別方式與門檻見 userparms.h 的 EMB_DOWNHILL_*：
//      「命令歸零 + 霍爾週期真的變短(= 車在加速) + 車速低於上限 + 位移達門檻」。
//      ⚠ 自由滑行的阻力恆為正 ⇒ 平路與上坡的週期必然逐步變長,故本偵測在平路正常停車時
//        結構上不成立 —— 這是它不再需要 V0.20 那個「武裝旗標」的原因(該旗標的解除門檻
//        低於油門最低命令速度,反而讓功能整條失效,已移除)。
//      [V0.24] 判準由「週期沒變長」改為「週期真的變短」。前者的隱含減速度門檻帶 v² 項
//        (a_th = v²/(2^TOL_SHIFT x d_LOOKBACK))，在 3 km/h 上限附近只有 0.26 m/s²、低於平路
//        自由滑行阻力的量級 → 平路在 2.6~3.0 km/h 這段其實會誤觸(而且走硬夾)。改成「必須
//        變短」後,上面那句「平路結構上不成立」才真正成立於**所有車速**。
// UVW lock 後延遲鎖定 EMB 的時間 (ms)，可設定。
// [實車調校 2026-08-14] 50 → 150。50ms 時機械夾緊仍略早於車速真正到 0，停車瞬間有應力
//   頓挫感;150ms 讓 UVW 短路有足夠時間把馬達帶到靜止再夾，實測頓挫消除。
//   ⚠ 這顆沒有安全代價 (延遲期間 UVW 短路仍在制動)，是停車舒適性的首選旋鈕;
//     不要為了同樣目的去改 UVW_LOCK_STOP_PULSES 或 EMBRAKER_LOCK_TIMEOUT_MS，那兩個都有
//     保護職責 (見各自的說明)。上限建議不超過 300ms，否則坡上會有一段無機械煞車的空窗。
#define EMBRAKER_SHORT_TO_LOCK_DELAY_MS 150

// =============================================================================
//  軟夾時長 (以 20ms tick 為單位) —— 停車路徑的 EMB 漸降時間
// =============================================================================
//  由「數位瞬間鎖住」改為「PWM duty 100% → 0% 線性漸降」的動能耗散時間。目的是
//  削減 EMB 一次夾住時的機械衝擊(客戶回報下坡/上坡點放時的震動與翹前輪)。
//
//  單位是 tick(每 tick = 20ms),不是 ms。EMB 任務本來就是 20ms 觸發,實體粒度就是 20ms,
//  以 tick 為單位可省一次除法且不會誤導讀者以為可精細到 ms。實際時長 = ticks × 20ms。
//
//  適用範圍:僅 `WAITING_TO_LOCK` 走完 UVW + SHORT_TO_LOCK_DELAY 到期的**正常停車**路徑。
//  以下情境仍走硬夾(立即 0%),不受本值影響:
//    (1) IBKS(uGF.BrakeSWOn)         語意上駕駛要求立即
//    (2) bMotorStop(電池/過溫/A04)   系統禁制,不可延遲
//    (3) bEmbRollbackDetected         已滑到 28mm,再軟夾會佔用倒溜預算
//    (4) bEmbDownhillSlide             同 (3)
//  硬夾/軟夾分派見 main.c 的 EMB action switch 分支。
//
//  取值指引 (tick × 20ms = 實際時長):
//    0       停用(等同硬夾,行為與 V0.21 之前完全相同)
//    2~3     40~60ms   極短,只削峰值不留延遲感
//    4~6     80~120ms  推薦區間,明顯柔化但不失效感 ← 實測 6 = 120ms 已驗證
//    7~10    140~200ms 較軟,體感開始有延遲
//    11~20   220~400ms 明顯柔化,但已進入需要注意的區間 ↓
//    >20     禁止(見下方 #error)
//
//  ⚠ tick > 10 的注意事項:LOCK 觸發時倒溜/滑動偵測會**被解除武裝**(參見 main.c LOCK
//    分支的計數器清零),ramp 期間偵測不再重新武裝。若車在 ramp 過程中被外力推動或滑動,
//    只有 UVW 短路提供制動力(車速愈低愈弱)。粗估在 30% 坡上、400ms ramp 期間,
//    最多可能滑動 ~80mm(仍在 1/4 車輪 = 159mm 規格內,但接近半數)。設 tick > 10 前
//    請確認實際使用坡度不會超過此範圍。若需要更長 ramp 且安全,較好的作法是「ramp 期間
//    保留偵測武裝」(尚未實作,可視需求規劃)。
//
// [實車調校 2026-08-19] 10 → 5 (200ms → 100ms)。V0.23 實車回報:PWM 軟夾使上下坡的夾煞
//   應力大幅下降、EMB 動作時不再有應力過大把前輪抬起的問題,而 100ms 已足夠削掉衝擊峰值,
//   不需要 200ms 的延遲感。5 落在上面的推薦區間內,且遠低於「tick > 10」的注意事項門檻。
#define EMB_SOFT_CLAMP_TICKS 5

#if (EMB_SOFT_CLAMP_TICKS) > 20
#error "EMB_SOFT_CLAMP_TICKS > 20 (>400ms): 陡坡倒溜可能違反 1/4 車輪規格,且 ramp 期間偵測已解武裝無保底 (見本註解與 main.c LOCK 分支)"
#endif
// 命令歸零後逾時強制鎖定的故障安全網 (ms)。[實車調校 2026-08-10] 3000 → 1000。
//   縮短的理由：下坡滑行時三條正常路徑全部不可達 —— UVW lock 進不去(HallPulsesLatch 遠大於
//   UVW_LOCK_STOP_PULSES)、Plan B 必須先有 UVW、有動力倒溜偵測又因命令降到
//   EMB_ROLLBACK_CMD_THRESHOLD 以下而解除武裝 —— 只剩本 failsafe。下坡車速持續上升,
//   3000ms 時的車速遠高於 1000ms,縮短計時等於在較低車速時才夾,衝擊較小。
//   原本設 3000ms 是擔心「命令已歸零但車還在滾」時被帶速硬夾,但實車驗證 1000ms 下
//   UVW lock 都能及時接手(配合 UVW_LOCK_STOP_PULSES),平地停車無頓挫。
//
//   ⚠⚠ 本計時**刻意不設車速閘門** —— 時間到就夾,不管當時車速。這是設計決定,不是缺陷:
//     命令已歸零卻仍持續移動達 EMBRAKER_LOCK_TIMEOUT_MS,代表所有正常的減速與保持路徑
//     都已失效。此時「車還在繼續移動」本身才是安全疑慮,必須強制停止;帶速夾造成的頓挫
//     與煞車片磨耗是可接受的代價。
//     ⇒ **不要為了舒適性而加車速閘門。** 那會讓最後一道保底在最需要它的時候放行。
//     舒適性要從「讓更早的路徑先接手」解決 —— 那是上面 (5) 下坡滑動偵測的職責
//     (EMB_DOWNHILL_*,26mm 就攔下,此時動能極小)。本計時只負責「無論如何都要停下來」。
// [實車調校 2026-08-15] 1000 → 1500 ms。同步自 LWFDSPC_V0.22f/LWFDSPC_V0.2 分支的實車調校
//   結果。V0.21 有 26mm 觸發的下坡/倒溜偵測作為早期接手，此值只在該偵測也失靈時作用;
//   放寬 500ms 給正常路徑更多時間 latch 上 (UVW lock / 下坡偵測)。⚠ 見上方 "不要為了
//   舒適性調它" 的警告 —— 本次放寬是給正常路徑更多接手時間，不是替代早期接手。
// [實車調校 2026-08-21] 1500 → 1000 ms。V0.24 客戶測試:上/下坡動作正常，但要求**縮短下坡
//   滑行距離** —— 本計時是下坡滑行的最後接手者,少 500ms 直接少一段滑行距離(1 km/h 時約
//   14 cm、2 km/h 時約 28 cm)。
//   ⚠ 這不是為了舒適性放寬,是為了**位移**收緊,與上方警告的方向一致(收緊永遠安全)。
//   為何 V0.24 可以放心收回 1000ms:2026-08-15 放寬到 1500ms 的理由是「給正常路徑更多時間
//   latch 上」，而當時 UVW lock 的進鎖要等整條減速斜坡(最長 700~800ms)走完，1000ms 的餘裕
//   確實吃緊。V0.24 起 UVW 進鎖改看油門目標(放油門即成立,見 main.c bUvwStopCommanded)，
//   正常停車在放油門後幾十 ms 內就 latch 上,不再需要那 500ms 的緩衝。
#define EMBRAKER_LOCK_TIMEOUT_MS 1000

// --- 以下參數目前未作為 EMB 動作條件 (保留定義供參考) ---
#define EMBRAKER_LOCK_SPEED_KMH_X10 5   // (停用) 舊版低於此車速(km/h×10)則鎖定

/**
 * @brief 初始化電磁煞車模組
 * @note 應在系統啟動時呼叫。此函式不直接存取硬體。
 * @param u16IembMv IEMB 腳位的初始電壓值 (mV)
 * @return bool true: 初始化成功 / false: 檢測到煞車故障 (A04)
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
 * @param bRollbackDetected [有動力倒溜] 連續反向霍爾邊緣達 EMB_ROLLBACK_REV_EDGES(main.c 算好傳入)
 *                          → 立即鎖定並閂鎖至鬆油門。RELEASED、WAITING_TO_LOCK 兩個狀態都會檢查。
 * @param bDownhillSlideDetected [下坡滑動] 命令歸零後車仍被重力加速且位移達門檻
 *                          (main.c 算好傳入，含車速上限閘門) → **立即鎖定**，不經 UVW 延遲也不等
 *                          timeout failsafe。與 bRollbackDetected **刻意不同**：不設閂鎖。
 *                          倒溜閂鎖是為了擋「坡上仍握著油門 → LOCKED 的運轉前檢查又放開 →
 *                          再次倒溜」的循環；下坡滑動的觸發前提是駕駛已經放掉油門，
 *                          bIsActive 本就是 false，LOCKED 會自然保持，該循環不存在。
 * @param bBrakeSwOn        [IBKS] 手剎車/充電中訊號作用中 (RC12 為 Low → uGF.BrakeSWOn==1)。
 *                          為 true 時**立即鎖定**，不經 UVW 延遲也不等 timeout failsafe。
 * @param bELockActive      [e-lock] 助力段位 0 (電子鎖車) 作用中。
 *                          與 bBrakeSwOn **刻意不同**：不做立即鎖定，只禁止 LOCKED 狀態放開煞車。
 *                          段位 0 時油門命令已在 s_logic_throttle.c 被歸零，停車鏈會自行平順地
 *                          減速→UVW 短路→鎖定；若在此改成立即鎖定，騎行中切到 0 段就會帶速硬鎖。
 *                          本旗標的作用是防護性的：即使未來有其他路徑注入非零命令，段位 0 也絕不
 *                          放開煞車。
 * @param u32CurrentTimeMs  目前的系統時間 (毫秒)
 * @return E_EMBRAKER_ACTION 應對煞車硬體執行的動作
 */
E_EMBRAKER_ACTION logic_embraker_update(uint16_t u16IembMv,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
                                        bool bRollbackDetected,
                                        bool bDownhillSlideDetected,
                                        bool bBrakeSwOn,
                                        bool bELockActive,
                                        uint32_t u32CurrentTimeMs);

/**
 * @brief 獲取目前電磁煞車的內部狀態
 * @return E_EMBRAKER_STATE 目前的狀態
 */
E_EMBRAKER_STATE logic_embraker_getStatus(void);

/**
 * @brief 獲取最後一次回傳 LOCK 的原因 (純診斷用，不參與控制)
 * @return E_EMBRAKER_LOCK_REASON
 */
E_EMBRAKER_LOCK_REASON logic_embraker_getLastLockReason(void);

#endif  // S_LOGIC_EMBRAKER_H_