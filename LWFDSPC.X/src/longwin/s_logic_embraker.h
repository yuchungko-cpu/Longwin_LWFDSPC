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
#define EMBRAKER_LOCK_SPEED_KMH_X10 5       // 低於此速度(1.0km/h * 10)則鎖定煞車
#define EMBRAKER_LOCK_TIMEOUT_MS 1000         // 油門鬆開後，等待此時間後強制鎖定 (ms) (故障安全網)
// [Modified Plan A] UVW三相短路(平順停車)生效後，延遲此時間才鎖定EMB (ms)。
// 可調整以對齊「機械煞車實際夾緊」與「車速到0」的時機，避免帶速鎖定或鎖定過晚倒溜。
#define EMBRAKER_SHORT_TO_LOCK_DELAY_MS 300

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
 * @param u16SpeedKmhx10    目前車速 (單位: KM/H x 10)
 * @param i16MotorCommand   目前的馬達驅動命令值 (例如: ReferenceRAW, 帶正負號)
 * @param i16ActualMotorCommand 實際發送給馬達的命令值 (用來確認是否為0)
 * @param bUVWLockActive    [Modified Plan A] UVW三相短路(平順停車)是否生效中
 * @param bReverseEdgeDetected [Plan B] 是否偵測到倒溜(與行駛方向相反的霍爾邊緣)
 * @param u32CurrentTimeMs  目前的系統時間 (毫秒)
 * @return E_EMBRAKER_ACTION 應對煞車硬體執行的動作
 */
E_EMBRAKER_ACTION logic_embraker_update(uint16_t u16IembMv,
                                        uint16_t u16SpeedKmhx10,
                                        int16_t i16MotorCommand,
                                        int16_t i16ActualMotorCommand,
                                        bool bUVWLockActive,
                                        bool bReverseEdgeDetected,
                                        uint32_t u32CurrentTimeMs);

/**
 * @brief 獲取目前電磁煞車的內部狀態
 * @return E_EMBRAKER_STATE 目前的狀態
 */
E_EMBRAKER_STATE logic_embraker_getStatus(void);

#endif  // S_LOGIC_EMBRAKER_H_