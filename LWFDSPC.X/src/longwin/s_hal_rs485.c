#include <xc.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "s_hal_rs485.h"
#include "../../hal/uart1.h"
#include "../userparms.h"

#include "codeHw.h"
#include "codeSw.h"

// --- Modbus RTU 幀間隔定義 ---
// 由於 LCD 不遵循標準的 3.5 字元間隔，我們使用較短的超時
// 在 57600 波特率下，1 字元 = 0.1736 ms
// 設定為 0.5ms，適應 LCD 的快速回應特性
#define MODBUS_T3_5_TIMEOUT_MS 0  // 0 表示不等待 3.5 字元間隔

// --- 簡單的狀態定義 ---
typedef enum {
    RS485_IDLE,
    RS485_SET_TX,
    RS485_SET_TX_DELAY,
    RS485_SENDING,
    RS485_WAIT_TX_COMPLETE,
    RS485_SET_RX,
    RS485_SET_RX_DELAY,
    RS485_RECEIVING
} E_RS485_STATE_T;

// 定義一個buffer 用於接收資料與發送資料
static uint8_t s_u8Buffer[128] = {0};

// --- 控制變數 ---
static E_RS485_STATE_T s_eState = RS485_IDLE;
static uint16_t s_u16TxLength = 0;
static uint16_t s_u16TxIndex = 0;

static volatile uint16_t s_u16RxCount = 0;
static volatile uint32_t s_u32LastRxTime = 0;  // 用於偵測幀間超時
static volatile bool s_bRxComplete = false;    // 新增: 接收完成旗標

static uint32_t (*s_pfnGetTime)(void) = NULL;

// --- 中斷處理 ---
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) {
    while (UART1_IsReceiveBufferDataReady()) {
        uint8_t u8Data = UART1_DataRead();

        // 只有在處於接收或空閒狀態時才處理進來的資料
        if (s_eState == RS485_RECEIVING || s_eState == RS485_IDLE) {
            if (s_eState == RS485_IDLE) {
                // 收到第一個位元組，開始進入接收狀態
                s_u16RxCount = 0;
                s_bRxComplete = false;
                s_eState = RS485_RECEIVING;
            }

            if (s_u16RxCount < sizeof(s_u8Buffer)) {
                s_u8Buffer[s_u16RxCount++] = u8Data;
                if (s_pfnGetTime != NULL) {
                    // 每收到一個字元，就更新最後接收時間戳
                    s_u32LastRxTime = s_pfnGetTime();
                }
            }
            // 如果 buffer 滿了，則忽略後續資料，等待幀超時來處理
        }
    }
    UART1_InterruptReceiveFlagClear();
}

// --- 公開函式 ---

void hal_rs485_init(uint32_t (*pfnGetTime)(void),
                    uint32_t u32TimeoutMs)  // u32TimeoutMs 暫時保留，但不再使用
{
    (void)u32TimeoutMs;  // 標記為未使用
    s_pfnGetTime = pfnGetTime;

    // 設定 RE 引腳
    O_RS485_RE_TRIS = 0;
    HW_RS485_SetRxMode();

    // 初始化 UART 中斷
    UART1_InterruptReceiveEnable();

    hal_rs485_receive_reset();
}

void hal_rs485_send(const uint8_t *pu8Data, uint16_t u16Length) {
    // 必須在 IDLE 狀態才能發送
    if (pu8Data == NULL || u16Length == 0 || s_eState != RS485_IDLE)
        return;

    // 發送前確保接收緩衝區是乾淨的
    hal_rs485_receive_reset();

    memcpy(s_u8Buffer, pu8Data, u16Length);
    s_u16TxLength = u16Length;
    s_u16TxIndex = 0;
    s_eState = RS485_SET_TX;
}

void hal_rs485_process(void) {
    // 使用時間戳來實現非阻塞延遲
    // 注意：假設 s_pfnGetTime() 的單位是毫秒 (ms)
    static uint32_t u32DelayStartTime = 0;

    switch (s_eState) {
        case RS485_IDLE:
            // 在 IDLE 狀態下無事可做
            break;

        case RS485_SET_TX:
            HW_RS485_SetTxMode();
            u32DelayStartTime = s_pfnGetTime(); // 記錄開始延遲的時間
            s_eState = RS485_SET_TX_DELAY;
            break;

        case RS485_SET_TX_DELAY:
            // 等待至少 1ms，確保驅動器穩定
            if (s_pfnGetTime() - u32DelayStartTime >= 1) {
                s_eState = RS485_SENDING;
            }
            break;

        case RS485_SENDING:
            if (s_u16TxIndex < s_u16TxLength) {
                if (!UART1_StatusBufferFullTransmitGet()) {
                    UART1_DataWrite(s_u8Buffer[s_u16TxIndex++]);
                }
            } else {
                s_eState = RS485_WAIT_TX_COMPLETE;
            }
            break;

        case RS485_WAIT_TX_COMPLETE:
            if (UART1_IsTransmissionComplete()) {
                s_eState = RS485_SET_RX;
            }
            break;

        case RS485_SET_RX:
            HW_RS485_SetRxMode();
            u32DelayStartTime = s_pfnGetTime(); // 記錄開始延遲的時間
            s_eState = RS485_SET_RX_DELAY;
            break;

        case RS485_SET_RX_DELAY:
            // 等待至少 1ms，確保方向切換完成
            if (s_pfnGetTime() - u32DelayStartTime >= 1) {
                // [關鍵修正]：完成發送流程後，應返回 IDLE 狀態。
                // 後續的接收將由 UART RX 中斷自動觸發，將狀態從 IDLE 切換到 RECEIVING。
                s_eState = RS485_IDLE;
            }
            break;

        case RS485_RECEIVING:
            // 檢查是否收到完整的 Modbus 封包 (此處邏輯不變)
            if (s_u16RxCount >= 4)
            {
                uint8_t funcCode = s_u8Buffer[1];
                uint8_t expectedLength = 0;

                if (funcCode == 0x03) {
                    if (s_u16RxCount >= 3) {
                        uint8_t byteCount = s_u8Buffer[2];
                        expectedLength = 3 + byteCount + 2;
                    }
                } else if (funcCode == 0x10) {
                    expectedLength = 8;
                }

                if (expectedLength > 0 && s_u16RxCount >= expectedLength) {
                    s_bRxComplete = true;
                    s_eState = RS485_IDLE;
                }
            }

            // 幀間超時 (此處邏輯不變)
            if (s_pfnGetTime != NULL && s_u16RxCount > 0 &&
                (s_pfnGetTime() - s_u32LastRxTime) > 1)
            {
                s_bRxComplete = true;
                s_eState = RS485_IDLE;
            }
            break;

        default:
            s_eState = RS485_IDLE;
            break;
    }
}

bool hal_rs485_is_busy(void) {
    // 只要不是 IDLE 狀態，或是有資料待處理，就認為是忙碌
    return (s_eState != RS485_IDLE || s_bRxComplete);
}

bool hal_rs485_is_rx_complete(void) {
    // 直接返回接收完成旗標
    return s_bRxComplete;
}

uint16_t hal_rs485_get_rx_length(void) {
    return s_u16RxCount;
}

void hal_rs485_receive_reset(void) {
    s_eState = RS485_IDLE;
    s_u16RxCount = 0;
    s_bRxComplete = false;
    // 清空硬體 FIFO
    while (UART1_IsReceiveBufferDataReady()) {
        (void)UART1_DataRead();
    }
    UART1_InterruptReceiveFlagClear();
}

uint8_t *hal_rs485_get_rx_data(void) {
    return s_u8Buffer;
}
