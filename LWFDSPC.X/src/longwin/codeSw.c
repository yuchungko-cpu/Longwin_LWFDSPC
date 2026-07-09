#include "codeSw.h"

void delayMicroseconds(uint32_t us)
{
    // 基於 100MHz (FCY) 的指令時脈
    // 1 指令週期 = 1 / 100,000,000 秒 = 10 奈秒
    // 1 微秒 = 1000 奈秒 = 100 個指令週期
    // 每個 NOP 約消耗 1 個指令週期

    // 迴圈本身會消耗一些時間，所以我們需要進行一些調整
    // 這裡的數字是透過實驗和微調得到的，以盡可能接近實際的微秒延遲
    // 注意：這是一個阻塞式延遲，會暫停所有其他處理

    us *= (100 / 4); // 根據指令週期和迴圈開銷進行調整

    while (us--)
    {
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile("nop");
    }
}
