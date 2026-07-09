#include "s_logic_speed_over.h"

// --- 內部時間管理 ---
static uint32_t su32_lastUpdateTime[2] = {0, 0}; // [0]: 正轉, [1]: 逆轉
static uint32_t su32_updateInterval = 20; // 20ms 更新間隔

// --- 內部狀態管理 ---
static uint16_t su16_currentOutput[2] = {0, 0}; // [0]: 正轉, [1]: 逆轉
static uint16_t su16_targetOutput[2] = {0, 0};  // [0]: 正轉, [1]: 逆轉
static E_LOGIC_SPEED_OVER_LEVEL_T se_currentLevel[2] = {E_LOGIC_SPEED_OVER_LEVEL_NORMAL, E_LOGIC_SPEED_OVER_LEVEL_NORMAL};

// --- 內部配置管理 ---
static S_LOGIC_SPEED_OVER_CONFIG_T sst_forwardConfig = {
    .u32NormalLimitKmh = LOGIC_SPEED_OVER_FORWARD_NORMAL_LIMIT_KMH,
    .u32WarningThresholdKmh = LOGIC_SPEED_OVER_FORWARD_WARNING_THRESHOLD_KMH,
    .u32CriticalThresholdKmh = LOGIC_SPEED_OVER_FORWARD_CRITICAL_THRESHOLD_KMH,
    .u16WarningStep = LOGIC_SPEED_OVER_WARNING_STEP,
    .u16WarningTime = LOGIC_SPEED_OVER_WARNING_TIME,
    .u16CriticalStep = LOGIC_SPEED_OVER_CRITICAL_STEP,
    .u16CriticalTime = LOGIC_SPEED_OVER_CRITICAL_TIME,
    .u16ActiveStep = LOGIC_SPEED_OVER_ACTIVE_STEP,
    .u16ActiveTime = LOGIC_SPEED_OVER_ACTIVE_TIME
};

static S_LOGIC_SPEED_OVER_CONFIG_T sst_reverseConfig = {
    .u32NormalLimitKmh = LOGIC_SPEED_OVER_REVERSE_NORMAL_LIMIT_KMH,
    .u32WarningThresholdKmh = LOGIC_SPEED_OVER_REVERSE_WARNING_THRESHOLD_KMH,
    .u32CriticalThresholdKmh = LOGIC_SPEED_OVER_REVERSE_CRITICAL_THRESHOLD_KMH,
    .u16WarningStep = LOGIC_SPEED_OVER_WARNING_STEP,
    .u16WarningTime = LOGIC_SPEED_OVER_WARNING_TIME,
    .u16CriticalStep = LOGIC_SPEED_OVER_CRITICAL_STEP,
    .u16CriticalTime = LOGIC_SPEED_OVER_CRITICAL_TIME,
    .u16ActiveStep = LOGIC_SPEED_OVER_ACTIVE_STEP,
    .u16ActiveTime = LOGIC_SPEED_OVER_ACTIVE_TIME
};

// --- 內部函數宣告 ---
static E_LOGIC_SPEED_OVER_LEVEL_T _speedOver_calculateProtectionLevel(uint32_t u32CurrentSpeedKmh, 
                                                                      const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig);
static uint16_t _speedOver_calculateTargetOutput(uint16_t u16CurrentOutput, 
                                                 E_LOGIC_SPEED_OVER_LEVEL_T eLevel,
                                                 const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig);
static void _speedOver_updateInternalTime(uint32_t u32SystemTimeMs, E_LOGIC_SPEED_OVER_DIRECTION_T eDirection);
static uint8_t _speedOver_getDirectionIndex(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection);

// --- 內部輔助函數 ---
static void _speedOver_copyConfig(S_LOGIC_SPEED_OVER_CONFIG_T* pstDest, const S_LOGIC_SPEED_OVER_CONFIG_T* pstSrc)
{
    if (pstDest != ((void*)0) && pstSrc != ((void*)0))
    {
        pstDest->u32NormalLimitKmh = pstSrc->u32NormalLimitKmh;
        pstDest->u32WarningThresholdKmh = pstSrc->u32WarningThresholdKmh;
        pstDest->u32CriticalThresholdKmh = pstSrc->u32CriticalThresholdKmh;
        pstDest->u16WarningStep = pstSrc->u16WarningStep;
        pstDest->u16WarningTime = pstSrc->u16WarningTime;
        pstDest->u16CriticalStep = pstSrc->u16CriticalStep;
        pstDest->u16CriticalTime = pstSrc->u16CriticalTime;
        pstDest->u16ActiveStep = pstSrc->u16ActiveStep;
        pstDest->u16ActiveTime = pstSrc->u16ActiveTime;
    }
}

// --- 函數實作 ---

void logic_speedOver_init(void)
{
    // 初始化內部時間管理
    su32_lastUpdateTime[0] = 0;
    su32_lastUpdateTime[1] = 0;
    su32_updateInterval = 20; // 20ms 更新間隔
    
    // 初始化輸出值
    su16_currentOutput[0] = 0;
    su16_currentOutput[1] = 0;
    su16_targetOutput[0] = 0;
    su16_targetOutput[1] = 0;
    
    // 初始化保護等級
    se_currentLevel[0] = E_LOGIC_SPEED_OVER_LEVEL_NORMAL;
    se_currentLevel[1] = E_LOGIC_SPEED_OVER_LEVEL_NORMAL;
}

/**
 * @brief 設定正轉方向的超速保護參數
 * @param pstConfig 指向配置參數結構體的指標
 * @return bool 設定成功返回 true，失敗返回 false
 */
bool logic_speedOver_setForwardConfig(const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig)
{
    if (pstConfig == ((void*)0))
    {
        return false;
    }
    
    // 複製配置參數
    _speedOver_copyConfig(&sst_forwardConfig, pstConfig);
    return true;
}

/**
 * @brief 設定逆轉方向的超速保護參數
 * @param pstConfig 指向配置參數結構體的指標
 * @return bool 設定成功返回 true，失敗返回 false
 */
bool logic_speedOver_setReverseConfig(const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig)
{
    if (pstConfig == ((void*)0))
    {
        return false;
    }
    
    // 複製配置參數
    _speedOver_copyConfig(&sst_reverseConfig, pstConfig);
    return true;
}

/**
 * @brief 獲取方向索引
 * @param eDirection 馬達運轉方向
 * @return uint8_t 方向索引 (0: 正轉, 1: 逆轉)
 */
static uint8_t _speedOver_getDirectionIndex(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection)
{
    return (eDirection == E_LOGIC_SPEED_OVER_DIRECTION_FORWARD) ? 0 : 1;
}

/**
 * @brief 更新內部時間管理變數
 * @param u32SystemTimeMs 系統時間戳記 (毫秒)
 * @param eDirection 馬達運轉方向
 */
static void _speedOver_updateInternalTime(uint32_t u32SystemTimeMs, E_LOGIC_SPEED_OVER_DIRECTION_T eDirection)
{
    uint8_t u8Index = _speedOver_getDirectionIndex(eDirection);
    su32_lastUpdateTime[u8Index] = u32SystemTimeMs;
}

/**
 * @brief 計算保護等級
 * @param u32CurrentSpeedKmh 當前速度 * 100 (KM/H)
 * @param pstConfig 指向配置參數的指標
 * @return E_LOGIC_SPEED_OVER_LEVEL_T 保護等級
 */
static E_LOGIC_SPEED_OVER_LEVEL_T _speedOver_calculateProtectionLevel(uint32_t u32CurrentSpeedKmh, 
                                                                      const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig)
{
    if (pstConfig == ((void*)0))
    {
        return E_LOGIC_SPEED_OVER_LEVEL_NORMAL;
    }
    
    // 根據速度判斷保護等級
    if (u32CurrentSpeedKmh >= pstConfig->u32NormalLimitKmh)
    {
        return E_LOGIC_SPEED_OVER_LEVEL_LIMIT_EXCEEDED;
    }
    else if (u32CurrentSpeedKmh >= pstConfig->u32CriticalThresholdKmh)
    {
        return E_LOGIC_SPEED_OVER_LEVEL_CRITICAL;
    }
    else if (u32CurrentSpeedKmh >= pstConfig->u32WarningThresholdKmh)
    {
        return E_LOGIC_SPEED_OVER_LEVEL_WARNING;
    }
    else
    {
        return E_LOGIC_SPEED_OVER_LEVEL_NORMAL;
    }
}

/**
 * @brief 根據保護等級計算目標輸出值
 * @param u16CurrentOutput 當前輸出值
 * @param eLevel 保護等級
 * @param pstConfig 指向配置參數的指標
 * @return uint16_t 目標輸出值
 */
static uint16_t _speedOver_calculateTargetOutput(uint16_t u16CurrentOutput, 
                                                 E_LOGIC_SPEED_OVER_LEVEL_T eLevel,
                                                 const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig)
{
    if (pstConfig == ((void*)0))
    {
        return u16CurrentOutput;
    }
    
    uint16_t u16TargetOutput = u16CurrentOutput;

    switch (eLevel)
    {
    case E_LOGIC_SPEED_OVER_LEVEL_LIMIT_EXCEEDED:
        // 最高保護等級：立即停止
        u16TargetOutput = 0;
        break;

    case E_LOGIC_SPEED_OVER_LEVEL_CRITICAL:
        // 臨界保護等級：快速減速
        if (u16CurrentOutput > pstConfig->u16CriticalStep)
        {
            u16TargetOutput = u16CurrentOutput - pstConfig->u16CriticalStep;
        }
        else
        {
            u16TargetOutput = 0;
        }
        break;

    case E_LOGIC_SPEED_OVER_LEVEL_WARNING:
        // 警告保護等級：緩慢減速
        if (u16CurrentOutput > pstConfig->u16WarningStep)
        {
            u16TargetOutput = u16CurrentOutput - pstConfig->u16WarningStep;
        }
        else
        {
            u16TargetOutput = 0;
        }
        break;

    case E_LOGIC_SPEED_OVER_LEVEL_NORMAL:
    default:
        // 正常狀態：保持當前輸出
        u16TargetOutput = u16CurrentOutput;
        break;
    }

    return u16TargetOutput;
}

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
 */
bool logic_speedOver_getUpdateParams(uint32_t u32CurrentSpeedKmh,
                                     uint32_t u32TargetSpeedKmH,
                                     uint16_t u16CurrentOutput,
                                     uint32_t u32SystemTimeMs,
                                     E_LOGIC_SPEED_OVER_DIRECTION_T eDirection,
                                     uint16_t *pu16TargetOutput)
{
    // 參數有效性檢查
    if (pu16TargetOutput == ((void*)0))
    {
        return false;
    }
    
    uint8_t u8Index = _speedOver_getDirectionIndex(eDirection);
    
    // 更新當前輸出值
    su16_currentOutput[u8Index] = u16CurrentOutput;

    // 檢查是否達到更新時間間隔
    if ((u32SystemTimeMs - su32_lastUpdateTime[u8Index]) < su32_updateInterval)
    {
        // 未到更新時間，回傳當前目標值
        *pu16TargetOutput = su16_targetOutput[u8Index];
        return false;
    }

    // 更新內部時間
    _speedOver_updateInternalTime(u32SystemTimeMs, eDirection);

    // 根據方向選擇配置參數
    const S_LOGIC_SPEED_OVER_CONFIG_T* pstConfig = (eDirection == E_LOGIC_SPEED_OVER_DIRECTION_FORWARD) 
                                                   ? &sst_forwardConfig : &sst_reverseConfig;

    // 計算保護等級
    E_LOGIC_SPEED_OVER_LEVEL_T eCurrentLevel = _speedOver_calculateProtectionLevel(u32CurrentSpeedKmh, pstConfig);
    se_currentLevel[u8Index] = eCurrentLevel;

    // 計算目標輸出值
    su16_targetOutput[u8Index] = _speedOver_calculateTargetOutput(u16CurrentOutput, eCurrentLevel, pstConfig);

    // 設定目標輸出值
    *pu16TargetOutput = su16_targetOutput[u8Index];

    // 檢查是否需要更新（目標值與當前值不同）
    return (su16_targetOutput[u8Index] != u16CurrentOutput);
}

/**
 * @brief 獲取當前超速保護狀態
 * @param eDirection 馬達運轉方向
 * @return E_LOGIC_SPEED_OVER_LEVEL_T 當前保護等級
 */
E_LOGIC_SPEED_OVER_LEVEL_T logic_speedOver_getCurrentLevel(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection)
{
    uint8_t u8Index = _speedOver_getDirectionIndex(eDirection);
    return se_currentLevel[u8Index];
}

/**
 * @brief 檢查是否處於超速保護狀態
 * @param eDirection 馬達運轉方向
 * @return bool 如果處於超速保護狀態返回 true，否則返回 false
 */
bool logic_speedOver_isProtectionActive(E_LOGIC_SPEED_OVER_DIRECTION_T eDirection)
{
    uint8_t u8Index = _speedOver_getDirectionIndex(eDirection);
    return (se_currentLevel[u8Index] != E_LOGIC_SPEED_OVER_LEVEL_NORMAL);
}