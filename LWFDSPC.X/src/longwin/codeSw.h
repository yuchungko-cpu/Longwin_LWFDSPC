#ifndef _CODESW_H_
#define _CODESW_H_
#include <stddef.h>  // For NULL
#include <stdint.h>
//
void delayMicroseconds(uint32_t us);

// 六步方波 開關
#define SixStepStart

// 關閉滑行
// #define GEAR_RATIO (1.0f)                 // 齒輪比，根據實際硬體調整
// #define WHEEL_CIRCUMFERENCE_MM (1500.0f)  // 輪胎周長 (mm)，根據實際輪胎調整
// #define WHEEL_DIAMETER_MM (7f * 2.54f)    // 輪胎直徑 (mm)，根據實際輪胎調整
//
#define RATED_CURRENT (50.0f)  // 額定電流 (A)，根據實際馬達規格調整

#define DEFAULT_SCALE_FACTOR (100)  // 保留 10 倍率作為備用

// 增加速度所帶入的輪徑計算（輪徑還是輪週長） 0: 使用輪週長計算,1: 使用輪徑計算
#define CODESW_SPEED_CALCULATION_USE_DIAMETER (1)
#define CODESW_SPEED_CALCULATION_USE_CIRCUMFERENCE (!CODESW_SPEED_CALCULATION_USE_DIAMETER)

/**
 * @brief 設定是否使用修正後的 100MHz FCY 時脈設定。
 * @note 1: 使用修正後的 100MHz FCY 設定 (用於正常運作與 UART 通訊)
 *       0: 使用舊的、未修正的時脈設定 (用於與舊版韌體行為進行比對測試)
 */
#define USE_CORRECTED_100MHZ_CLOCK_SETTING (1)

// ===== 統一倍率定義 =====
#define CODESW_EMBRAKER_ENABLE (1)
#define CODESW_EMBRAKER_USE_ADC_INPUT (0) // 1: 使用 RD8 ADC 輸入, 0: 使用數位開關輸入
#define CODESW_EMBRAKER_TEST (0)            // 測試未有實際接上手煞車的裝置時
                                            //
#define CODESW_IFR_DIRECTION_DEFAULT (1)    // 0: 不變 1: 反轉
#define CODESW_MOTOR_DIRECTION_DEFAULT (0)  // 0: 不變 1: 反轉

// 增加速度計算
#define CODESW_SPEED_CALCULATION_ENABLE (1)

// 建議使用 SCALE_FACTOR_100 作為統一標準
// 提供 0.01 精度，平衡運算效率與精度需求
//
// #define CODESW_UART_BRG_VALUE (53) // For 115200 baud with FCY=100MHz (based
// on hal/clock.h). BRG = (100000000 / (16 * 115200)) - 1 = 53.25

// =============================================================================
// == X2CScope 與 Modbus/RS485 ==
// == X2CScope 走專屬 UART2 (RB8=U2TX / RB9=U2RX)，Modbus/RS485 走 UART1，      ==
// == 兩者硬體上完全獨立，可同時運作。                                          ==
// =============================================================================

// Modbus/RS485 排程器開關。
// 【2026-07-29】原本定義為 (!CODESW_SCOPE_ENABLE)，因為舊設計裡 X2CScope 與 RS485 共用
// UART1、硬體上不可能並存。X2CScope 改用專屬 UART2 之後兩者已完全獨立，那個互補關係
// 沒有意義且造成誤導 (「關掉 Modbus」會連帶改動 X2C 觀測變數的宣告)，故 CODESW_SCOPE_ENABLE
// 已移除，X2CScope 一律改用 CODESW_X2C_SCOPE_ENABLE。
#define CODESW_MODBUS_SCHEDULER_ENABLE (1)  // 1: 啟用 Modbus 排程器, 0: 停用

// Modbus/RS485 執行暫停開關 (單一變因，除錯用)。
// 設 1 時只跳過主迴圈裡的 modbusService_process() —— 那是唯一會真正驅動 RS485 收發與
// 狀態機的入口，所以等於「RS485 完全靜止」，但其餘 Modbus 程式碼照常編譯，不會踩到
// CODESW_MODBUS_SCHEDULER_ENABLE 設 0 時的編譯錯誤。
// 副作用：電池/LCD/GUI 資料不再更新 (decode 讀到舊值)，僅適合平台測試，測完設回 0。
#define CODESW_MODBUS_PROCESS_SUSPEND (0)

// X2CScope 開關 (走專屬 UART2：RB8=U2TX / RB9=U2RX，原 CAN1 腳位，CAN IC 已實體移除)。
// 接線：MCU pin48(RB8) → 轉接器 RXD；MCU pin49(RB9) ← 轉接器 TXD；必須共地、3.3V 準位。
// baud 115741 (divider 53)，GUI 端設 115200。
// 接收為中斷驅動 (_U2RXInterrupt, IPL 3 —— 低於 ADC/PWM 的 7 與 Modbus U1RX 的 4)
// 搭配 128 byte 軟體 FIFO，故接收不依賴主迴圈頻率。
// 硬體鏈路已於 2026-07-28 完整驗證 (RB8 腳位 10kHz、TX 57.9kHz、pin48/49 短接自我回路、
// 經真實轉接器的收發回音)，四個暫時性自測開關已於 2026-07-29 全部移除。
#define CODESW_X2C_SCOPE_ENABLE (1)

#define CODESW_MODBUS_SCHEDULER_BRG_VALUE (107)                // For 57600 baud with FCY=100MHz (based on hal/clock.h). BRG =
                                                               // (100000000 / (16 * 57600)) - 1 = 108.5

#define CODESW_RS485_TEST_ENABLE 0  // 1: 啟用 RS485 任務管理器測試模式, 0: 正常模式

#define CODESW_UART_TEST_ENABLE (0)

// ===== 控制模式配置 =====
// THROTTLE 和 VR 互斥配置 - 只能選擇其中一種
#define CODESW_THROTTLE_ENABLE (1)                  // 油門控制啟用 (1: 啟用, 0: 停用)
#define CODESW_VR_ENABLE (!CODESW_THROTTLE_ENABLE)  // VR控制啟用 (當 THROTTLE 停用時自動啟用)

// 正逆轉轉換速度門檻值（throttle 模式 and VR 模式）
#define CODESW_DIRECTION_CHANGE_SPEED_THRESHOLD (10)  // 1.0 km/h
#define CODESW_DIRECTION_CHANGE_SPEED_VR (10)         // 1.0 Km/h

// 控制模式識別宏 (自動化配置)
// #define CODESW_CONTROL_MODE_THROTTLE (CODESW_THROTTLE_ENABLE)
// #define CODESW_CONTROL_MODE_VR (CODESW_VR_ENABLE)

// ===== 其他功能配置 =====
// 加入電池的偵測
#define CODESW_BATTERY_ENABLE (1)
#define CODESW_BATTERY_PROTECTION_MODULE_ENABLE (1) // 1: 使用 s_logic_battery 模組進行電壓保護, 0: 使用 main.c 中的舊有邏輯
#define CODESW_BATTERY_TEST (0) // 1: 啟用電池電壓模擬測試, 0: 正常讀取 ADC

#define CODESW_UNIFIED_LED_LOGIC_ENABLE (1) // 1: 啟用統一的 LED 顯示邏輯, 0: 使用舊版分散邏輯

// 醫療代步車機種：定義後，RD7(綠)/RD6(黃) 改作前進/後退方向指示燈，
// 取代無故障時的電量長條顯示（紅燈 RD5 仍作電量/警示用）。
// 註解掉此行即恢復一般電量長條顯示。
#define CODESW_MEDICAL_SCOOTER

#define CODESW_SPEED_TEST (0) // 1: 啟用速度模擬測試, 0: 正常讀取霍爾速度

#define CODESW_TEMPERATURE_CONTROLLER_ENABLE (1)  // 啟用舊的、但功能完整的溫度保護邏輯
#define CODESW_TEMPERATURE_CONTROLLER_TEST (0)

#define CODESW_TEMPERATURE_MOTOR_ENABLE (1)

// ===== 除錯功能配置 =====
// 示波器量測腳位：1: 啟用 RC13(ADC ISR 執行時間) / RD13(速度命令處理時間) 量測輸出, 0: 停用
#define CODESW_DEBUG_ISR_PROFILE_ENABLE (1)

// UVW low-speed lock. Keep enabled; entry is delayed until speed/current are quiet.
#define CODESW_UVW_LOCK_ENABLE (1)

// Motor lock / stall validation mode. Keep disabled for production builds.
#define CODESW_MOTOR_LOCK_TEST_ENABLE (0)
#define CODESW_MOTOR_LOCK_TEST_PHASE_CURRENT_A (5.0f)

#define DEBUG_MAIN_LEVEL 4

#if DEBUG_MAIN_LEVEL >= 1

#define MAIN_LOG_E(fmt, ...)                                                     \
    printf("[E][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, \
           ##__VA_ARGS__);
#else
#define MAIN_LOG_E(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 2
#define MAIN_LOG_W(fmt, ...)                                                     \
    printf("[W][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, \
           ##__VA_ARGS__);
#else
#define MAIN_LOG_W(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 3
#define MAIN_LOG_I(fmt, ...)                                                     \
    printf("[I][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, \
           ##__VA_ARGS__);
#else
#define MAIN_LOG_I(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 4
#define MAIN_LOG(fmt, ...)                                                  \
    printf("[ ][%10lu][   ]%25s:%-5d > " fmt, millis(), __FILE__, __LINE__, \
           ##__VA_ARGS__);
#define MAIN_LOG_S(fmt, ...) printf(fmt, ##__VA_ARGS__);
#define MAIN_LOG_V(fmt, ...)                                                     \
    printf("[V][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, \
           ##__VA_ARGS__);
#else
#define MAIN_LOG(fmt, ...)
#define MAIN_LOG_S(fmt, ...)
#define MAIN_LOG_V(fmt, ...) ;
#endif

#endif /* CODESW_H */
