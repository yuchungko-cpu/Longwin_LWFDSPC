#ifndef S_HAL_RS485_H
#define S_HAL_RS485_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化 RS485
 * @param pfnGetTime 獲取系統時間的函式指標（可為 NULL）
 */
void hal_rs485_init(uint32_t (*pfnGetTime)(void),
                    uint32_t u32TimeoutMs);

/**
 * @brief 發送資料
 * @param pu8Data 資料指標
 * @param u16Length 資料長度
 */
void hal_rs485_send(const uint8_t *pu8Data, uint16_t u16Length);

/**
 * @brief 發送處理（在主迴圈中呼叫）
 */
void hal_rs485_process(void);

/**
 * @brief 檢查是否忙碌
 * @return true 忙碌中
 */
bool hal_rs485_is_busy(void);

/**
 * @brief 檢查接收是否完成
 * @return true 接收完成
 */
bool hal_rs485_is_rx_complete(void);

/**
 * @brief 獲取接收到的資料長度
 * @return 資料長度
 */
uint16_t hal_rs485_get_rx_length(void);

/**
 * @brief 重置接收狀態
 */
void hal_rs485_receive_reset(void);

/**
 * @brief 取得接收到的資料
 * @return 資料指標
 */
uint8_t *hal_rs485_get_rx_data(void);

#endif // S_HAL_RS485_H