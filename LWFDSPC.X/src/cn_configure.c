/*
 * File:   interrupt.c
 * Author: A20692
 *
 * Created on June 23, 2020, 10:35 AM
 */


#include <xc.h>
#include "userparms.h"

void CN_Configure(void) {
    CNCONC = 0;
    /*  ON: Change Notification (CN) Control for PORTx On bit
        1 = CN is enabled
        0 = CN is disabled   */
    CNCONCbits.ON = 0;
    /*    CNSTYLE: Change Notification Style Selection bit
        1 = Edge style (detects edge transitions, bits are used for a CNE)
        0 = Mismatch style (detects change from last port read event)       */
    CNCONCbits.CNSTYLE = 0;

    CNEN0C = 0;
    CNEN0Cbits.CNEN0C4 = 1;
    CNEN0Cbits.CNEN0C5 = 1;
    CNEN0Cbits.CNEN0C10 = 1;

    _CNCIF = 0;
    _CNCIE = 0;
    _CNCIP = 5;
}

/**
 * @brief 初始化數位和類比輸入腳位
 * @details 啟用重要數位輸入腳位的內部上拉電阻，以改善訊號穩定性
 *          特別針對 RD14 (前進/後退開關) 的電壓下降問題
 *          類比輸入腳位僅設定為輸入模式，不啟用上拉電阻以避免影響 ADC 讀值
 * 
 * @note 此函式應在 main() 中的 GPIO 初始化後呼叫
 * @note 數位輸入：先設定為輸入，再啟用上拉電阻
 * @note 類比輸入：僅設定為輸入，不啟用上拉電阻
 */
void InitDigitalInputPullups(void)
{
    // 先設定所有重要數位輸入腳位為輸入模式
    I_BRAKE_TRIS = 1;           // 煞車訊號輸入
    I_FR_SWITCH_TRIS = 1;       // 前進/後退開關輸入
    I_CRUISE_TRIS = 1;          // 啟動/停止開關輸入
    I_SPEED_SENSOR_A_TRIS = 1;  // 方向開關A輸入
    I_SPEED_SENSOR_B_TRIS = 1;  // 方向開關B輸入
    I_EXT_SPEED_SENSOR_TRIS = 1; // 外部速度感測輸入

    // 延遲一小段時間確保 TRIS 設定生效
    { volatile uint16_t i; for(i = 0; i < 1000; i++); }
    
    // 啟用重要數位輸入腳位的內部上拉電阻
    I_FR_SWITCH_PULLUP = 1;     // 啟用 RD14 上拉電阻 (最重要)
    I_BRAKE_PULLUP = 1;         // 啟用煞車訊號上拉電阻
    I_CRUISE_PULLUP = 1;        // 啟用啟動/停止開關上拉電阻
    I_SPEED_SENSOR_A_PULLUP = 1; // 啟用方向開關A上拉電阻
    I_SPEED_SENSOR_B_PULLUP = 1; // 啟用方向開關B上拉電阻
    I_EXT_SPEED_SENSOR_PULLUP = 1; // 啟用外部速度感測上拉電阻
    
    // 注意：類比輸入腳位不啟用上拉電阻，避免影響 ADC 讀值
}
