#ifndef S_LOGIC_ERROR_HANDLER_H_
#define S_LOGIC_ERROR_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>

// 基於 "FUNCTION - ALM 異常顯示表" 從 longwinConrtrolFunction_chunchi.md

/**
 * @brief 系統警告/錯誤代碼枚舉
 *
 * 定義了系統中所有可能的警告和錯誤代碼。
 * 這些代碼將由 s_logic_error_handler 模組進行管理。
 * @note 枚舉值的順序隱含了優先級 (值越小，優先級越高，LOGIC_ALARM_NONE 除外)。
 */
typedef enum {
  LOGIC_ALARM_NONE = 0, // 無警報

  // 油門相關 (Throttle)
  LOGIC_ALARM_A03_THROTTLE_FAULT, // 油門異常 (開機或運行時電壓超出範圍)

  // 煞車相關 (Brake System)
  LOGIC_ALARM_A02_BRAKE_SWITCH_FAULT, // 一般煞車開關異常
                                      // (IBKS，例如開機時為低電位)
  LOGIC_ALARM_A04_EMB_SENSOR_FAULT,   // 電磁煞車感測器異常 (Embarker
                                    // Sensor，例如開機時電壓超出範圍)

  // 馬達相關 (Motor)
  LOGIC_ALARM_A19_MOTOR_HALL_FAULT,    // 馬達霍爾信號異常 (運行時檢測)
  LOGIC_ALARM_A05_MOTOR_OVERCURRENT,   // 馬達電流過大
  LOGIC_ALARM_A06_MOTOR_STALL,         // 馬達堵轉
  LOGIC_ALARM_A07_MOTOR_SHORT_CIRCUIT, // 馬達短路 (MOSFET 短路，控制器硬體故障)
  LOGIC_ALARM_A08_MOTOR_PHASE_LOSS,    // 馬達缺相 (控制器硬體故障)
  LOGIC_ALARM_A20_MOTOR_OVER_TEMP,     // 馬達溫度過高

  // 電池相關 (Battery)
  LOGIC_ALARM_A01_BATTERY_UNDER_VOLTAGE, // 電池低電壓保護 (放電低壓)
  LOGIC_ALARM_A15_BATTERY_OVER_VOLTAGE,  // 電池過電壓保護

  // 控制器相關 (Controller)
  LOGIC_ALARM_A09_CONTROLLER_OVER_TEMP,    // 控制器溫度過高
  LOGIC_ALARM_A10_CONTROLLER_COMM_TIMEOUT, // 控制器與儀表通信超時
  LOGIC_ALARM_A11_CONTROLLER_CONFIG_ERROR, // 控制器參數錯誤
  LOGIC_ALARM_A12_CONTROLLER_HW_FAULT,     // 控制器硬體故障 (MOSFET 故障)

  // 外部感測器/輸入 (External Sensors/Inputs)
  LOGIC_ALARM_A14_LSN_FAULT, // LSN 外部速度感測器PPR設置無效或信號異常

  // 系統/其他
  LOGIC_ALARM_A13_SYSTEM_STARTUP_CHECK_FAIL, // 系統啟動自檢失敗
  LOGIC_ALARM_A16_EEPROM_FAULT,              // EEPROM 故障
  LOGIC_ALARM_A17_LOW_VOLTAGE_FORBID_OUTPUT, // 低電壓禁止輸出
                                             // (特定邏輯，可能與A01關聯)

  LOGIC_ALARM_CODE_COUNT // 用於計數，表示總的警報類型數量 (不含NONE)

} E_LOGIC_ALARM_CODE_T;

// --- 預設錯誤處理器配置值 ---
#define LOGIC_ERROR_HANDLER_DEFAULT_ACTIVE_ALARMS_BITMASK                      \
  (0UL) // 預設無活動警報

/**
 * @brief 初始化錯誤處理模組
 *
 * 清除所有已活動的警報，將位元遮罩設為預設值。
 */
void logic_errorHandler_init(void);

/**
 * @brief 設定或清除指定警報的活動狀態
 *
 * @param eAlarmCode 要設定/清除的警報代碼
 * @param bIsActive  true 表示設為活動，false 表示清除
 */
void logic_errorHandler_setAlarmStatus(E_LOGIC_ALARM_CODE_T eAlarmCode,
                                       bool bIsActive);

/**
 * @brief 檢查指定的警報代碼當前是否活動
 *
 * @param eAlarmCode 要檢查的警報代碼
 * @return true 如果警報活動中，否則為 false
 */
bool logic_errorHandler_isAlarmActive(E_LOGIC_ALARM_CODE_T eAlarmCode);

/**
 * @brief 獲取當前活動的最高優先級警報
 *
 * 優先級由 E_LOGIC_ALARM_CODE_T 枚舉值的順序決定 (值越小優先級越高)。
 *
 * @return E_LOGIC_ALARM_CODE_T 最高優先級的活動警報代碼，如果沒有警報活動則返回
 * LOGIC_ALARM_NONE
 */
E_LOGIC_ALARM_CODE_T logic_errorHandler_getHighestPriorityActiveAlarm(void);

/**
 * @brief 獲取一個包含所有當前活動警報的位元遮罩
 *
 * 位元遮罩中的每一位對應 E_LOGIC_ALARM_CODE_T 中的一個警報碼，
 * 警報碼的枚舉值即為其在遮罩中的位元索引。
 * 例如，如果 (E_LOGIC_ALARM_CODE_T)1 (即 LOGIC_ALARM_A03_THROTTLE_FAULT)
 * 活動，則遮罩的第1位為1。
 *
 * @return uint32_t 代表所有活動警報的位元遮罩
 */
uint32_t logic_errorHandler_getAllActiveAlarmsMask(void);

#endif // S_LOGIC_ERROR_HANDLER_H_