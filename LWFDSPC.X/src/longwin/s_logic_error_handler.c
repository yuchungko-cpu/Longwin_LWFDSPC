#include "s_logic_error_handler.h"

static uint32_t su32_err_activeAlarmsBitmask = 0;

/**
 * @brief 初始化錯誤處理模組
 * 
 * 清除所有已活動的警報，將位元遮罩設為預設值。
 */
void logic_errorHandler_init(void)
{
    su32_err_activeAlarmsBitmask = LOGIC_ERROR_HANDLER_DEFAULT_ACTIVE_ALARMS_BITMASK;
}

/**
 * @brief 設定或清除指定警報的活動狀態
 * 
 * @param eAlarmCode 要設定/清除的警報代碼
 * @param bIsActive  true 表示設為活動，false 表示清除
 */
void logic_errorHandler_setAlarmStatus(E_LOGIC_ALARM_CODE_T eAlarmCode,
                                       bool bIsActive)
{
    if (eAlarmCode < LOGIC_ALARM_CODE_COUNT)
    { // 確保警報碼在有效範圍內
        if (bIsActive)
        {
            su32_err_activeAlarmsBitmask |= (1UL << eAlarmCode);
        }
        else
        {
            su32_err_activeAlarmsBitmask &= ~(1UL << eAlarmCode);
        }
    }
}

/**
 * @brief 檢查指定的警報代碼當前是否活動
 * 
 * @param eAlarmCode 要檢查的警報代碼
 * @return true 如果警報活動中，否則為 false
 */
bool logic_errorHandler_isAlarmActive(E_LOGIC_ALARM_CODE_T eAlarmCode)
{
    if (eAlarmCode < LOGIC_ALARM_CODE_COUNT)
    {
        return (su32_err_activeAlarmsBitmask & (1UL << eAlarmCode)) != 0;
    }
    return false; // 無效的警報碼
}

/**
 * @brief 獲取當前活動的最高優先級警報
 * 
 * 優先級由 E_LOGIC_ALARM_CODE_T 枚舉值的順序決定 (值越小優先級越高)。
 * 
 * @return E_LOGIC_ALARM_CODE_T 最高優先級的活動警報代碼，如果沒有警報活動則返回 LOGIC_ALARM_NONE
 */
E_LOGIC_ALARM_CODE_T logic_errorHandler_getHighestPriorityActiveAlarm(void)
{
    for (uint8_t u8AlarmIndex = 0; u8AlarmIndex < LOGIC_ALARM_CODE_COUNT; ++u8AlarmIndex)
    {
        if ((su32_err_activeAlarmsBitmask & (1UL << u8AlarmIndex)) != 0)
        {
            if (u8AlarmIndex != LOGIC_ALARM_NONE)
            {
                return (E_LOGIC_ALARM_CODE_T)u8AlarmIndex;
            }
        }
    }
    return LOGIC_ALARM_NONE;
}

/**
 * @brief 獲取所有當前活動警報的位元遮罩
 * 
 * 位元遮罩中的每一位對應 E_LOGIC_ALARM_CODE_T 中的一個警報碼，
 * 警報碼的枚舉值即為其在遮罩中的位元索引。
 * 
 * @return uint32_t 代表所有活動警報的位元遮罩
 */
uint32_t logic_errorHandler_getAllActiveAlarmsMask(void)
{
    return su32_err_activeAlarmsBitmask;
}