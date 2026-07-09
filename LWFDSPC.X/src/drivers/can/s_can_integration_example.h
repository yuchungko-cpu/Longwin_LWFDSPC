/*******************************************************************************
  Longwin CAN 整合範例標頭檔 (CAN Integration Example Header File)

  檔案名稱:
    s_can_integration_example.h

  摘要:
    此檔案包含 CAN 整合範例的函式宣告和介面定義

  描述:
    此檔案提供了完整的 CAN 整合介面，包含：
    1. 初始化函式
    2. 訊息處理函式
    3. 統計資訊函式
    4. 主程式整合範例

*******************************************************************************/
#ifndef _S_CAN_INTEGRATION_EXAMPLE_H
#define _S_CAN_INTEGRATION_EXAMPLE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// CAN 整合函式宣告 (CAN Integration Function Declarations)
// *****************************************************************************

/**
 * @brief 初始化 CAN 整合模組
 * @return bool true=成功, false=失敗
 * @note 此函式會初始化 CAN 驅動器並註冊所有回調函式
 * 
 * @example
 * // 在 main() 函式中初始化
 * if (!canIntegration_init()) {
 *     // 處理初始化失敗
 *     return;
 * }
 */
bool canIntegration_init(void);

/**
 * @brief 處理 CAN 訊息 (主迴圈中呼叫)
 * @return 無
 * @note 此函式應在主迴圈中定期呼叫，處理接收到的 CAN 訊息
 * 
 * @example
 * // 在主迴圈中呼叫
 * while (1) {
 *     canIntegration_processMessages();
 *     // 其他主迴圈邏輯...
 * }
 */
void canIntegration_processMessages(void);

/**
 * @brief 取得 CAN 統計資訊
 * @param pu32RxCount 接收計數指標
 * @param pu32TxCount 傳送計數指標
 * @param pu32ErrorCount 錯誤計數指標
 * @return bool true=成功, false=失敗
 * 
 * @example
 * uint32_t u32RxCount, u32TxCount, u32ErrorCount;
 * if (canIntegration_getStatistics(&u32RxCount, &u32TxCount, &u32ErrorCount)) {
 *     // 處理統計資訊
 *     printf("CAN Stats: RX=%lu, TX=%lu, Errors=%lu\n", 
 *            u32RxCount, u32TxCount, u32ErrorCount);
 * }
 */
bool canIntegration_getStatistics(uint32_t* pu32RxCount, uint32_t* pu32TxCount, uint32_t* pu32ErrorCount);

/**
 * @brief 重置 CAN 統計資訊
 * @return 無
 * @note 此函式會重置所有計數器
 */
void canIntegration_resetStatistics(void);

/**
 * @brief 主程式 CAN 整合範例
 * @note 此函式展示如何在主程式中整合 CAN 功能
 * 
 * 使用方式：
 * 1. 在 main() 函式中呼叫 canIntegration_init()
 * 2. 在主迴圈中呼叫 canIntegration_processMessages()
 * 3. 可選擇性地呼叫統計資訊函式
 * 
 * @example
 * // 在 main() 函式中呼叫範例
 * canIntegration_mainExample();
 */
void canIntegration_mainExample(void);

#ifdef __cplusplus
}
#endif

#endif // _S_CAN_INTEGRATION_EXAMPLE_H 