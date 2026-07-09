#ifndef S_LOGIC_SPEED_OVER_H_
#define S_LOGIC_SPEED_OVER_H_

#include <stdbool.h>
#include <stdint.h>

// --- 常數定義 (整數版本 * 100) ---
// 正轉超速保護參數
#define LOGIC_SPEED_OVER_FORWARD_NORMAL_LIMIT_KMH (2500) // 正轉正常限速 25.00 KM/H * 100
#define LOGIC_SPEED_OVER_FORWARD_WARNING_DIFF_KMH (300)  // 正轉警告區間差值 (25-3=22 KM/H) * 100
#define LOGIC_SPEED_OVER_FORWARD_CRITICAL_DIFF_KMH (150) // 正轉臨界區間差值 (25-1.5=23.5 KM/H) * 100

// 逆轉超速保護參數
#define LOGIC_SPEED_OVER_REVERSE_NORMAL_LIMIT_KMH (200)  // 逆轉正常限速 2.00 KM/H * 100
#define LOGIC_SPEED_OVER_REVERSE_WARNING_DIFF_KMH (50)   // 逆轉警告區間差值 (2-0.5=1.5 KM/H) * 100
#define LOGIC_SPEED_OVER_REVERSE_CRITICAL_DIFF_KMH (25)  // 逆轉臨界區間差值 (2-0.25=1.75 KM/H) * 100

// 計算實際閾值
#define LOGIC_SPEED_OVER_FORWARD_WARNING_THRESHOLD_KMH (LOGIC_SPEED_OVER_FORWARD_NORMAL_LIMIT_KMH - LOGIC_SPEED_OVER_FORWARD_WARNING_DIFF_KMH)
#define LOGIC_SPEED_OVER_FORWARD_CRITICAL_THRESHOLD_KMH (LOGIC_SPEED_OVER_FORWARD_NORMAL_LIMIT_KMH - LOGIC_SPEED_OVER_FORWARD_CRITICAL_DIFF_KMH)

#define LOGIC_SPEED_OVER_REVERSE_WARNING_THRESHOLD_KMH (LOGIC_SPEED_OVER_REVERSE_NORMAL_LIMIT_KMH - LOGIC_SPEED_OVER_REVERSE_WARNING_DIFF_KMH)
#define LOGIC_SPEED_OVER_REVERSE_CRITICAL_THRESHOLD_KMH (LOGIC_SPEED_OVER_REVERSE_NORMAL_LIMIT_KMH - LOGIC_SPEED_OVER_REVERSE_CRITICAL_DIFF_KMH)

// 超速保護相關參數
#define LOGIC_SPEED_OVER_WARNING_STEP (1)   // 警告區間遞增值
#define LOGIC_SPEED_OVER_WARNING_TIME (20)  // 警告區間遞增時間
#define LOGIC_SPEED_OVER_CRITICAL_STEP (1)  // 臨界區間遞增值
#define LOGIC_SPEED_OVER_CRITICAL_TIME (30) // 臨界區間遞增時間
#define LOGIC_SPEED_OVER_ACTIVE_STEP (2)    // 超速區間遞增值
#define LOGIC_SPEED_OVER_ACTIVE_TIME (20)   // 超速區間遞增時間

// --- 枚舉定義 ---

/**
 * @brief 定義馬達運轉方向。
 */
typedef enum {
    E_LOGIC_SPEED_OVER_DIRECTION_FORWARD = 0,     /**< 正轉方向 */
    E_LOGIC_SPEED_OVER_DIRECTION_REVERSE          /**< 逆轉方向 */
} E_LOGIC_SPEED_OVER_DIRECTION_T;

/**
 * @brief 定義超速保護的級別狀態。
 */
typedef enum {
    E_LOGIC_SPEED_OVER_LEVEL_NORMAL = 0,          /**< 速度正常，未觸發任何超速條件 */
    E_LOGIC_SPEED_OVER_LEVEL_WARNING,             /**< 速度進入警告區間 */
    E_LOGIC_SPEED_OVER_LEVEL_CRITICAL,            /**< 速度進入臨界區間 */
    E_LOGIC_SPEED_OVER_LEVEL_LIMIT_EXCEEDED       /**< 速度超過正常限速，觸發超速保護 */
} E_LOGIC_SPEED_OVER_LEVEL_T;

// --- 結構體定義 ---

/**
 * @brief 超速保護配置參數結構體
 */
typedef struct {
    uint32_t u32NormalLimitKmh;      /**< 正常限速值 * 100 (KM/H) */
    uint32_t u32WarningThresholdKmh; /**< 警告閾值 * 100 (KM/H) */
    uint32_t u32CriticalThresholdKmh;/**< 臨界閾值 * 100 (KM/H) */
    uint16_t u16WarningStep;         /**< 警告區間遞增值 */
    uint16_t u16WarningTime;         /**< 警告區間遞增時間 */
    uint16_t u16CriticalStep;        /**< 臨界區間遞增值 */
    uint16_t u16CriticalTime;        /**< 臨界區間遞增時間 */
    uint16_t u16ActiveStep;          /**< 超速區間遞增值 */
    uint16_t u16ActiveTime;          /**< 超速區間遞增時間 */
} S_LOGIC_SPEED_OVER_CONFIG_T;

// --- 函數原型 ---

/**
 * @brief 初始化超速保護邏輯模組。
 * @note 應在系統啟動時調用一次，以設定初始狀態。
 * @example
 * \\code
 * logic_speedOver_init();
 * \\endcode
 */
void logic_speedOver_init(void);

/**
 * @brief 設定正轉方向的超速保護參數
 * @param pstConfig 指向配置參數結構體的指標
 * @return bool 設定成功返回 true，失敗返回 false
 */
bool logic_speedOver_setForwardConfig(const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig);

/**
 * @brief 設定逆轉方向的超速保護參數
 * @param pstConfig 指向配置參數結構體的指標
 * @return bool 設定成功返回 true，失敗返回 false
 */
bool logic_speedOver_setReverseConfig(const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig);

/**
 * @brief 計算超速保護模式下，馬達輸出的調整參數，內部管理時間間隔和漸進式變化
 *
 * @details
 * 此函式根據當前的車速、目標速度和運轉方向，判斷是否需要調整馬達輸出以維持安全速度。
 * - 如果當前速度超過目標速度，會計算減速所需的 step 和 time。
 * - 如果當前速度在安全範圍內，則 step 和 time 為 0。
 * - 內部管理時間間隔和漸進式變化。
 * - 支援正轉和逆轉兩種方向的超速保護。
 *
 * @param u32CurrentSpeedKmh 當前速度 * 100 (KM/H)。
 * @param u32TargetSpeedKmH 目標參考速度 * 100 (KM/H)，通常是騎士設定的速度或系統允許的最高速度。
 * @param u16CurrentOutput 目前的馬達輸出值。
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param eDirection 馬達運轉方向
 * @param pu16TargetOutput 指向儲存計算出的目標輸出值的指標。
 *                         此目標輸出值是基於超速保護邏輯計算得出的理想輸出，
 *                         但不一定立即達到，而是透過內部時間管理逐步趨近。
 *
 * @return bool 如果輸出需要更新，則返回 true，否則返回 false。
 *
 * @example
 * uint32_t target_speed = 2500; // 假設限速25 * 100
 * uint16_t current_output = getCurrentOutput();
 * uint16_t target_output_val;
 * uint32_t system_time = get_system_time_ms();
 * bool needs_update;
 *
 * needs_update = logic_speedOver_getUpdateParams(currentSpeed, target_speed, 
 *                                              current_output, system_time, 
 *                                              E_LOGIC_SPEED_OVER_DIRECTION_FORWARD, 
 *                                              &target_output_val);
 * if (needs_update) {
 *     apply_motor_output(target_output_val);
 * }
 */
bool logic_speedOver_getUpdateParams(uint32_t u32CurrentSpeedKmh,
                                     uint32_t u32TargetSpeedKmH,
                                     uint16_t u16CurrentOutput,
                                     uint32_t u32SystemTimeMs,
                                     E_LOGIC_SPEED_OVER_DIRECTION_T eDirection,
                                     uint16_t *pu16TargetOutput);

/**
 * @brief 獲取當前超速保護狀態
 * @param eDirection 馬達運轉方向
 * @return E_LOGIC_SPEED_OVER_LEVEL_T 當前保護等級
 */
E_LOGIC_SPEED_OVER_LEVEL_T logic_speedOver_getCurrentLevel(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection);

/**
 * @brief 檢查是否處於超速保護狀態
 * @param eDirection 馬達運轉方向
 * @return bool 如果處於超速保護狀態返回 true，否則返回 false
 */
bool logic_speedOver_isProtectionActive(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection);

#endif // S_LOGIC_SPEED_OVER_H_