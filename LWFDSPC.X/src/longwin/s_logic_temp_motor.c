#include "s_logic_temp_motor.h"
#include <stddef.h> // For NULL
#include "s_logic_error_handler.h"

// --- 模組內部狀態變數 ---
static E_LOGIC_MOTOR_TEMP_STATUS_T seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_NORMAL;
static uint8_t su8ConsecutiveReadings = 0;

/**
 * @brief 初始化馬達溫度保護邏輯模組。
 */
void logic_motorTemp_init(void)
{
    seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_NORMAL;
    su8ConsecutiveReadings = 0;
    logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A20_MOTOR_OVER_TEMP, false);
}

/**
 * @brief 根據馬達溫度更新保護狀態，並通過 s_logic_error_handler 設定或清除 LOGIC_ALARM_A20_MOTOR_OVER_TEMP 警報。
 */
E_LOGIC_MOTOR_TEMP_STATUS_T logic_motorTemp_update(uint32_t u32MotorTempC)
{
    E_LOGIC_MOTOR_TEMP_STATUS_T _ePreviousStatus = seCurrentMotorTempStatus;

    switch (seCurrentMotorTempStatus)
    {
        case E_LOGIC_MOTOR_TEMP_STATUS_NORMAL:
            if (u32MotorTempC >= LOGIC_MOTOR_TEMP_OVERHEAT_THRESHOLD_C)
            {
                seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_PENDING;
                su8ConsecutiveReadings = 1; // 開始計數
            }
            else
            {
                su8ConsecutiveReadings = 0; // 溫度正常，重置計數
            }
            break;

        case E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_PENDING:
            if (u32MotorTempC >= LOGIC_MOTOR_TEMP_OVERHEAT_THRESHOLD_C)
            {
                su8ConsecutiveReadings++;
                if (su8ConsecutiveReadings >= LOGIC_MOTOR_TEMP_OVERHEAT_REQUIRED_READINGS)
                {
                    seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE;
                    su8ConsecutiveReadings = 0; // 重置計數器供恢復狀態使用
                }
            }
            else
            {
                // 溫度在確認期間降回正常範圍
                seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_NORMAL;
                su8ConsecutiveReadings = 0;
            }
            break;

        case E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE:
            if (u32MotorTempC < LOGIC_MOTOR_TEMP_RECOVERY_THRESHOLD_C)
            {
                seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_RECOVERY_PENDING;
                su8ConsecutiveReadings = 1; // 開始計數
            }
            else
            {
                su8ConsecutiveReadings = 0; // 溫度仍高，重置恢復計數
            }
            break;

        case E_LOGIC_MOTOR_TEMP_STATUS_RECOVERY_PENDING:
            if (u32MotorTempC < LOGIC_MOTOR_TEMP_RECOVERY_THRESHOLD_C)
            {
                su8ConsecutiveReadings++;
                if (su8ConsecutiveReadings >= LOGIC_MOTOR_TEMP_RECOVERY_REQUIRED_READINGS)
                {
                    seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_NORMAL;
                    su8ConsecutiveReadings = 0; // 重置計數器
                }
            }
            else
            {
                // 溫度在恢復確認期間再次升高超過恢復閾值
                // 甚至可能再次超過過熱閾值，應回到 OVERHEAT_ACTIVE 以確保安全
                seCurrentMotorTempStatus = E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE;
                su8ConsecutiveReadings = 0;
            }
            break;

        default:
            // 未知狀態，重置為正常以策安全
            logic_motorTemp_init();
            break;
    }

    // 狀態轉移後，處理 A20 警報
    if (seCurrentMotorTempStatus == E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE && _ePreviousStatus != E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE)
    {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A20_MOTOR_OVER_TEMP, true);
    }
    else if (seCurrentMotorTempStatus != E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE && _ePreviousStatus == E_LOGIC_MOTOR_TEMP_STATUS_OVERHEAT_ACTIVE)
    {
        logic_errorHandler_setAlarmStatus(LOGIC_ALARM_A20_MOTOR_OVER_TEMP, false);
    }

    return seCurrentMotorTempStatus;
}

/**
 * @brief 獲取目前馬達溫度保護的詳細狀態。
 *
 * @return E_MOTOR_TEMP_STATUS 目前的保護狀態枚舉值。
 */
E_LOGIC_MOTOR_TEMP_STATUS_T logic_motorTemp_getStatus(void)
{
    return seCurrentMotorTempStatus;
}
