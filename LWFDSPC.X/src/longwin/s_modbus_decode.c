#include "s_modbus_decode.h"
#include <string.h> // For memset

//=================================================================================================
// Helper Constants
//=================================================================================================

#define MIRROR_BASE_ADDR 0x0100
#define MIRROR_BLOCK_SIZE 32

//=================================================================================================
// Public API Functions - DECODING
//=================================================================================================

void modbusDecode_decodeBatteryData(S_BATTERY_DATA *pDest,
                                    const S_MODBUS_BATTERY_DATA_RAW *pSrc)
{
    pDest->u16CapacityAh_x10 = pSrc->u16Regs[0x01]; // Addr 0x0001
    pDest->u8Percent = pSrc->u16Regs[0x02];         // Addr 0x0002
    pDest->u16Voltage_x10 = pSrc->u16Regs[0x03];    // Addr 0x0003

    uint16_t u16Info1 = pSrc->u16Regs[0x04]; // Addr 0x0004
    pDest->u8MfgCode = (u16Info1 >> 8) & 0xFF;
    pDest->u8MfgYear = u16Info1 & 0xFF;

    uint16_t u16Info2 = pSrc->u16Regs[0x05]; // Addr 0x0005
    pDest->u8MfgMonth = (u16Info2 >> 8) & 0xFF;
    pDest->u8MfgWeek = u16Info2 & 0xFF;

    pDest->u16SerialNumber = pSrc->u16Regs[0x06]; // Addr 0x0006
    pDest->u16ChargeCycles = pSrc->u16Regs[0x07]; // Addr 0x0007

    uint16_t u16Date = pSrc->u16Regs[0x08]; // Addr 0x0008
    pDest->u16LastChargeYear = ((u16Date >> 9) & 0x7F) + 2000;
    pDest->u8LastChargeMonth = (u16Date >> 5) & 0x0F;
    pDest->u8LastChargeDay = u16Date & 0x1F;

    uint16_t u16Time = pSrc->u16Regs[0x09]; // Addr 0x0009
    pDest->u8LastChargeHour = (u16Time >> 8) & 0xFF;
    pDest->u8LastChargeMinute = u16Time & 0xFF;
}

void modbusDecode_decodeLcdData(S_SHARED_DEVICE_DATA *pDest,
                                const U_MODBUS_LCD_APP_DATA *pSrc)
{
    // Decode data received from LCD (to_device blocks)
    // 電池容量 , 不用更新，因為是battery裝置提供
    // pDest->u16LcdMirror_BatteryCapacityAh_x10 = pSrc->u16Regs[0x01]; // Addr 0x0001
    // 電池百分比，不用更新，因為是battery裝置提供
    // pDest->u8LcdMirror_BatteryPercent = pSrc->u16Regs[0x02];         // Addr 0x0002
    // 速度，不用更新，因為是master的資料
    // pDest->u16CurrentSpeedKmh_x10 = pSrc->u16Regs[0x03]; // Addr 0x0003
    // 電流，不用更新，因為是master的資料
    // pDest->u16LoadCurrentA_x10 = pSrc->u16Regs[0x04];    // Addr 0x0004

    // 燈光，更新
    uint16_t u16Lights = pSrc->u16Regs[0x05]; // Addr 0x0005
    pDest->lights.bHeadlight = (u16Lights >> 0) & 1;
    pDest->lights.bBrakeLight = (u16Lights >> 1) & 1;
    pDest->lights.bRightTurnSignal = (u16Lights >> 2) & 1;
    pDest->lights.bLeftTurnSignal = (u16Lights >> 3) & 1;

    // 錯誤，不用更新，因為是master的資料
    /*
    uint16_t u16Errors = pSrc->u16Regs[0x06]; // Addr 0x0006
    pDest->errors.bBrakeSignalError = (u16Errors >> 0) & 1;
    pDest->errors.bBatteryLowVoltage = (u16Errors >> 1) & 1;
    pDest->errors.bBrakeStuck = (u16Errors >> 2) & 1;
    pDest->errors.bThrottleStuck = (u16Errors >> 3) & 1;
    pDest->errors.bMotorOverload = (u16Errors >> 5) & 1;
    pDest->errors.bControllerOverTemp = (u16Errors >> 6) & 1;
    pDest->errors.bMotorOrControllerError = (u16Errors >> 7) & 1;
    pDest->errors.bMotorSensorError = (u16Errors >> 9) & 1;
    pDest->errors.bLsnError = (u16Errors >> 10) & 1;
    pDest->errors.bBatteryOverVoltage = (u16Errors >> 11) & 1;
    pDest->errors.bCommBatteryError = (u16Errors >> 12) & 1;
    pDest->errors.bCommLcdError = (u16Errors >> 13) & 1;
    pDest->errors.bCommGuiError = (u16Errors >> 14) & 1;
    pDest->errors.bCommAppError = (u16Errors >> 15) & 1;
    */

    // Decode settings sent from the device
    // 輪徑、背光、再生、頭燈、定速、單位、輔助段數，更新
    uint16_t u16WheelCfg = pSrc->u16Regs[0x07]; // Addr 0x0007
    uint8_t u8WheelCfgHi = (u16WheelCfg >> 8) & 0xFF;
    uint8_t u8WheelCfgLo = u16WheelCfg & 0xFF;
    pDest->u8WheelDiameterInches = u8WheelCfgHi & 0x1F;
    pDest->bBacklightOn = (u8WheelCfgHi >> 6) & 1;
    pDest->bRegenOn = (u8WheelCfgLo >> 7) & 1;
    pDest->bHeadlightOn = (u8WheelCfgLo >> 6) & 1;
    pDest->bCruiseOn = (u8WheelCfgLo >> 5) & 1;
    pDest->bUnitIsMile = (u8WheelCfgLo >> 4) & 1;
    pDest->u8AssistLevel = u8WheelCfgLo & 0x0F;

    // 最高速度，更新
    pDest->u16ForwardSpeedLimitKmh_x10 = pSrc->u16Regs[0x08]; // Addr 0x0008

    // 控制模式、加速度曲線，更新
    uint16_t u16ControlMode = pSrc->u16Regs[0x09]; // Addr 0x0009
    pDest->eControlMode = (E_CONTROL_MODE)((u16ControlMode >> 8) & 0xFF);
    pDest->eAccelCurve = (E_ACCEL_CURVE)(u16ControlMode & 0xFF);

    // 脈衝、油門、助推模式，更新
    uint16_t u16Pulse = pSrc->u16Regs[0x0A]; // Addr 0x000A
    pDest->u8PulsePerRev = u16Pulse & 0xFF;
    pDest->u16ThrottleAdValue = (u16Pulse >> 8) & 0xFF;
    pDest->bWalkModeActive = pDest->u16ThrottleAdValue == 0xCC;

    // Decode Block 2 (0x000B - 0x000C)

    // 控制氣溫度，不用更新，因為是master的資料
    // pDest->u16ControllerTempC_x10 = pSrc->u16Regs[0x0B]; // Addr 0x000B
    // pDest->u16LcdMirror_BatteryVoltage_x10 = pSrc->u16Regs[0x0C]; // Addr 0x000C

    // 電池電壓，不用更新，因為是battery裝置提供
    // pDest->u16LcdMirror_BatteryVoltage_x10 = pSrc->u16Regs[0x0C];    // Addr 0x000C

    // 密碼，更新
    pDest->u16Password = pSrc->u16Regs[0x0D]; // Addr 0x000D

    // 倒車速度限制，更新
    pDest->u16ReverseSpeedLimitKmh_x10 = pSrc->u16Regs[0x0E]; // Addr 0x000E

    // 加速度曲線1、2，更新
    uint16_t u16Accel12 = pSrc->u16Regs[0x0F]; // Addr 0x000F
    pDest->u8AccelMs_Ax2 = (u16Accel12 >> 8) & 0xFF;
    pDest->u8AccelMs_Ax1 = u16Accel12 & 0xFF;

    // 加速度曲線3、4，更新
    uint16_t u16Accel34 = pSrc->u16Regs[0x10]; // Addr 0x0010
    pDest->u8AccelMs_Ax4 = (u16Accel34 >> 8) & 0xFF;
    pDest->u8AccelMs_Ax3 = u16Accel34 & 0xFF;

    // 電池類型、加速度曲線5，更新
    uint16_t u16Accel5 = pSrc->u16Regs[0x11]; // Addr 0x0011
    pDest->eBatteryType = (E_BATTERY_TYPE)((u16Accel5 >> 8) & 0x07);
    pDest->u8AccelMs_Ax5 = u16Accel5 & 0xFF;
}

void modbusDecode_decodeGuiData(S_SHARED_DEVICE_DATA *pDest,
                                const U_MODBUS_PC_GUI_DATA *pSrc)
{
    // This function decodes settings received FROM the PC-GUI.
    // It reads the "FromDevice" blocks in the raw data structure.

    // Decode Block 1 (0x0007 - 0x000A)
    uint16_t u16WheelCfg = pSrc->u16Regs[0x07];
    uint8_t u8WheelCfgHi = (u16WheelCfg >> 8) & 0xFF;
    uint8_t u8WheelCfgLo = u16WheelCfg & 0xFF;
    // 更新 輪徑
    pDest->u8WheelDiameterInches = u8WheelCfgHi & 0x1F;
    // 更新 大燈on/off
    pDest->bBacklightOn = (u8WheelCfgHi >> 6) & 1;
    // 更新 回充on/off
    pDest->bRegenOn = (u8WheelCfgLo >> 7) & 1;
    // 更新 頭燈on/off
    pDest->bHeadlightOn = (u8WheelCfgLo >> 6) & 1;
    // 更新 定速功能On/off
    pDest->bCruiseOn = (u8WheelCfgLo >> 5) & 1;
    // 更新 單位 km/h or mile/h
    pDest->bUnitIsMile = (u8WheelCfgLo >> 4) & 1;
    // 更新 輔助段數
    pDest->u8AssistLevel = u8WheelCfgLo & 0x0F;

    // 更新 最高速度
    pDest->u16ForwardSpeedLimitKmh_x10 = pSrc->u16Regs[0x08];

    // 更新 控制模式
    uint16_t u16ControlMode = pSrc->u16Regs[0x09];
    pDest->eControlMode = (E_CONTROL_MODE)((u16ControlMode >> 8) & 0xFF);
    // 更新 加速度曲線
    pDest->eAccelCurve = (E_ACCEL_CURVE)(u16ControlMode & 0xFF);

    // 更新 脈衝、油門、助推模式
    uint16_t u16Pulse = pSrc->u16Regs[0x0A];
    pDest->u8PulsePerRev = u16Pulse & 0xFF;
    pDest->u16ThrottleAdValue = (u16Pulse >> 8) & 0xFF;
    pDest->bWalkModeActive = pDest->u16ThrottleAdValue == 0xCC;

    // Decode Block 2 (0x000C - 0x000E)
    // 更新 加速度曲線1、2
    uint16_t u16Accel12 = pSrc->u16Regs[0x0C];
    pDest->u8AccelMs_Ax2 = (u16Accel12 >> 8) & 0xFF;
    pDest->u8AccelMs_Ax1 = u16Accel12 & 0xFF;

    // 更新 加速度曲線3、4
    uint16_t u16Accel34 = pSrc->u16Regs[0x0D];
    pDest->u8AccelMs_Ax4 = (u16Accel34 >> 8) & 0xFF;
    pDest->u8AccelMs_Ax3 = u16Accel34 & 0xFF;

    // 更新 電池類型、加速度曲線5
    uint16_t u16Accel5 = pSrc->u16Regs[0x0E];
    pDest->eBatteryType = (E_BATTERY_TYPE)((u16Accel5 >> 8) & 0x07);
    pDest->u8AccelMs_Ax5 = u16Accel5 & 0xFF;

    // 更新 扭矩1、2
    // Decode Block 3 (0x000F - 0x0011)
    uint16_t t12 = pSrc->u16Regs[0x0F];
    pDest->u8TorqueT2_pct = (t12 >> 8) & 0xFF;
    pDest->u8TorqueT1_pct = t12 & 0xFF;

    // 更新 扭矩3、4
    uint16_t t34 = pSrc->u16Regs[0x10];
    pDest->u8TorqueT4_pct = (t34 >> 8) & 0xFF;
    pDest->u8TorqueT3_pct = t34 & 0xFF;

    // 更新 扭矩5
    pDest->u8TorqueT5_pct = pSrc->u16Regs[0x11] & 0xFF;

    // Decode Block 4 (0x0012)
    // 更新 設定密碼
    pDest->u16Password = pSrc->u16Regs[0x12];

    // Decode Block 5 (0x0014)
    // 更新 倒車速度限制
    pDest->u16ReverseSpeedLimitKmh_x10 = pSrc->u16Regs[0x14];
}

//=================================================================================================
// Public API Functions - ENCODING
//=================================================================================================

void modbusDecode_encodeLcdSettings(U_MODBUS_LCD_APP_DATA *pDest,
                                    const S_SHARED_DEVICE_DATA *pSrc,
                                    const S_BATTERY_DATA *pSrcBattery,
                                    uint8_t u8Update)
{
    // Encode settings into the 'from_device' blocks, which are writable by the master.
    //
    uint8_t u8WheelCfgHi = (pSrc->u8WheelDiameterInches & 0x1F) | ((pSrc->bBacklightOn & 1) << 6);
    uint8_t u8WheelCfgLo = (pSrc->u8AssistLevel & 0x0F) |
                           ((pSrc->bUnitIsMile & 1) << 4) |
                           ((pSrc->bCruiseOn & 1) << 5) |
                           ((pSrc->bHeadlightOn & 1) << 6) |
                           ((pSrc->bRegenOn & 1) << 7);
    // ------------------------------------------------------------
    // Battery data
    // 0x0001 -> Battery capacity
    pDest->u16Regs[0x01] = pSrcBattery->u16CapacityAh_x10;

    // 0x0002 -> Battery percent
    pDest->u16Regs[0x02] = pSrcBattery->u8Percent;

    // 0x0003 -> speed
    pDest->u16Regs[0x03] = pSrc->u16CurrentSpeedKmh_x10;

    // 0x0004 -> load current
    pDest->u16Regs[0x04] = pSrc->u16LoadCurrentA_x10;

    // 0x0005 -> Light status
    uint16_t u16Lights = (pSrc->lights.bHeadlight << 0) |
                         (pSrc->lights.bBrakeLight << 1) |
                         (pSrc->lights.bRightTurnSignal << 2) |
                         (pSrc->lights.bLeftTurnSignal << 3);
    pDest->u16Regs[0x05] = u16Lights;

    // 0x0006 -> Error status
    uint16_t u16Errors = (pSrc->errors.bBrakeSignalError << 0) |
                         (pSrc->errors.bBatteryLowVoltage << 1) |
                         (pSrc->errors.bBrakeStuck << 2) |
                         (pSrc->errors.bThrottleStuck << 3) |
                         (pSrc->errors.bMotorOverload << 5) |
                         (pSrc->errors.bControllerOverTemp << 6) |
                         (pSrc->errors.bMotorOrControllerError << 7) |
                         (pSrc->errors.bMotorSensorError << 9) |
                         (pSrc->errors.bLsnError << 10) |
                         (pSrc->errors.bBatteryOverVoltage << 11) |
                         (pSrc->errors.bCommBatteryError << 12) |
                         (pSrc->errors.bCommLcdError << 13) |
                         (pSrc->errors.bCommGuiError << 14) |
                         ((pSrc->errors.bCommAppError || pSrc->errors.bEmbSensorFault) << 15); // Bit 15 represents CommAppError or EmbSensorFault (A19)
    pDest->u16Regs[0x06] = u16Errors;
    // ------------------------------------------------------------
    if (u8Update & 0x01)
    {
        // 0x0007 -> Wheel diameter
        pDest->u16Regs[0x07] = ((uint16_t)u8WheelCfgHi << 8) | u8WheelCfgLo; // Addr 0x0007

        // 0x0008 -> Forward speed limit
        pDest->u16Regs[0x08] = pSrc->u16ForwardSpeedLimitKmh_x10; // Addr 0x0008

        // 0x0009 -> Control mode
        pDest->u16Regs[0x09] = ((uint16_t)pSrc->eControlMode << 8) | pSrc->eAccelCurve; // Addr 0x0009

        // 0x000A -> Pulse
        uint8_t u8PulseHiByte = pSrc->bWalkModeActive ? 0xCC : pSrc->u16ThrottleAdValue;
        pDest->u16Regs[0x0A] = ((uint16_t)u8PulseHiByte << 8) | pSrc->u8PulsePerRev; // Addr 0x000A
    }
    // 0x000B -> Controller Temp
    pDest->u16Regs[0x0B] = pSrc->u16ControllerTempC_x10;

    // 0x000C -> Battery voltage
    pDest->u16Regs[0x0C] = pSrcBattery->u16Voltage_x10;

    if (u8Update == 1)
    {
        // 0x000D -> Password
        pDest->u16Regs[0x0D] = pSrc->u16Password; // Addr 0x000D

        // 0x000E -> Reverse speed limit
        pDest->u16Regs[0x0E] = pSrc->u16ReverseSpeedLimitKmh_x10; // Addr 0x000E

        // 0x000F -> Accel 1
        pDest->u16Regs[0x0F] = ((uint16_t)pSrc->u8AccelMs_Ax2 << 8) | pSrc->u8AccelMs_Ax1; // Addr 0x000F

        // 0x0010 -> Accel 2
        pDest->u16Regs[0x10] = ((uint16_t)pSrc->u8AccelMs_Ax4 << 8) | pSrc->u8AccelMs_Ax3; // Addr 0x0010

        // 0x0011 -> Battery type
        pDest->u16Regs[0x11] = ((uint16_t)pSrc->eBatteryType << 8) | pSrc->u8AccelMs_Ax5; // Addr 0x0011
    }
}

void modbusDecode_encodeGuiSettings(U_MODBUS_PC_GUI_DATA *pDest,
                                    const S_SHARED_DEVICE_DATA *pSrc,
                                    const S_BATTERY_DATA *pSrcBattery,
                                    uint8_t u8Update)
{
    // Update 的作用是將  slave (read/write) 的資料更新到 slave 上
    /*
        uint16_t u16ToDeviceBlock1[6];   // Addr 0x0001 - 0x0006
        uint16_t u16ToDeviceBlock2;      // Addr 0x000B
        uint16_t u16ToDeviceBlock3;      // Addr 0x0013
        uint16_t u16ToDeviceBlock4;      // Addr 0x0015
        uint16_t u16ToDeviceBlock5[6];   // Addr 0x0016 - 0x001B
        uint16_t u16ToDeviceBlock6;      // Addr 0x001C
        uint16_t u16ToDeviceBlock7;      // Addr 0x001D
        uint16_t u16FromDeviceBlock1[4]; // Addr 0x0007 - 0x000A
        uint16_t u16FromDeviceBlock2[3]; // Addr 0x000C - 0x000E
        uint16_t u16FromDeviceBlock3[3]; // Addr 0x000F - 0x0011
        uint16_t u16FromDeviceBlock4;    // Addr 0x0012
        uint16_t u16FromDeviceBlock5;    // Addr 0x0014
     */
    // This function populates the 'ToDevice' blocks of the PC_GUI data structure.
    // This is data that the master WRITES TO the GUI (e.g., status information).

    // Block 1: Status (Addr 0x0001 - 0x0006)
    // 0x0001 -> Battery capacity
    pDest->u16Regs[0x01] = pSrcBattery->u16CapacityAh_x10;
    // 0x0002 -> Battery percent
    pDest->u16Regs[0x02] = pSrcBattery->u8Percent;
    // 0x0003 -> Battery voltage
    pDest->u16Regs[0x03] = pSrcBattery->u16Voltage_x10;
    // 0x0004 -> Speed
    pDest->u16Regs[0x04] = pSrc->u16CurrentSpeedKmh_x10;
    // 0x0005 -> Load current
    pDest->u16Regs[0x05] = pSrc->u16LoadCurrentA_x10;

    // 0x0006 -> Error status
    uint16_t u16Errors = (pSrc->errors.bBrakeSignalError << 0) |
                         (pSrc->errors.bBatteryLowVoltage << 1) |
                         (pSrc->errors.bBrakeStuck << 2) |
                         (pSrc->errors.bThrottleStuck << 3) |
                         (pSrc->errors.bMotorOverload << 5) |
                         (pSrc->errors.bControllerOverTemp << 6) |
                         (pSrc->errors.bMotorOrControllerError << 7) |
                         (pSrc->errors.bMotorSensorError << 9) |
                         (pSrc->errors.bLsnError << 10) |
                         (pSrc->errors.bBatteryOverVoltage << 11) |
                         (pSrc->errors.bCommBatteryError << 12) |
                         (pSrc->errors.bCommLcdError << 13) |
                         (pSrc->errors.bCommGuiError << 14) |
                         (pSrc->errors.bCommAppError << 15);
    pDest->u16Regs[0x06] = u16Errors;

    // u16FromDeviceBlock1[4]; // Addr 0x0007 - 0x000A
    if (u8Update & 0x01)
    {
        // 0x0007 -> Light status
        uint16_t u16LightStatusBits = (pSrc->bRegenOn << 7) |
                                      (pSrc->bHeadlightOn << 6) |
                                      (pSrc->bCruiseOn << 5) |
                                      (pSrc->bUnitIsMile << 4) |
                                      (pSrc->u8AssistLevel & 0x0F);
        pDest->u16Regs[0x07] = u16LightStatusBits;

        // 0x0008 -> speed limit
        pDest->u16Regs[0x08] = pSrc->u16ForwardSpeedLimitKmh_x10;

        // 0x0009 -> control mode
        pDest->u16Regs[0x09] = ((uint16_t)pSrc->eControlMode << 8) |
                               pSrc->eAccelCurve;
        // 0x000A -> pulse
        pDest->u16Regs[0x0A] = ((uint16_t)pSrc->u8PulsePerRev << 8) |
                               pSrc->u16ThrottleAdValue;
    }
    // u16ToDeviceBlock2;      // Addr 0x000B
    // 0x000B -> Controller Temp
    pDest->u16Regs[0x0B] = pSrc->u16ControllerTempC_x10;

    if (u8Update == 1)
    {
        // u16FromDeviceBlock2[3]; // Addr 0x000C - 0x000E
        // 0x000C -> Accel 1
        pDest->u16Regs[0x0C] = ((uint16_t)pSrc->u8AccelMs_Ax2 << 8) | pSrc->u8AccelMs_Ax1;
        // 0x000D -> Accel 2
        pDest->u16Regs[0x0D] = ((uint16_t)pSrc->u8AccelMs_Ax4 << 8) | pSrc->u8AccelMs_Ax3;
        // 0x000E -> Accel 3
        pDest->u16Regs[0x0E] = ((uint16_t)pSrc->u8AccelMs_Ax5 << 8) | pSrc->u8AccelMs_Ax5;

        // u16FromDeviceBlock3[3]; // Addr 0x000F - 0x0011
        // 0x000F -> Torque 1
        pDest->u16Regs[0x0F] = ((uint16_t)pSrc->u8TorqueT1_pct << 8) | pSrc->u8TorqueT2_pct;
        // 0x0010 -> Torque 2
        pDest->u16Regs[0x10] = ((uint16_t)pSrc->u8TorqueT3_pct << 8) | pSrc->u8TorqueT4_pct;
        // 0x0011 -> Torque 3
        pDest->u16Regs[0x11] = ((uint16_t)pSrc->u8TorqueT5_pct << 8) | pSrc->u8TorqueT5_pct;

        //  u16FromDeviceBlock4;    // Addr 0x0012
        pDest->u16Regs[0x12] = pSrc->u16Password;
    }

    // u16ToDeviceBlock3;      // Addr 0x0013
    // 0x0013 -> Motor RPM
    pDest->u16Regs[0x13] = pSrc->u16MotorRpm;

    if (u8Update == 1)
    {
        pDest->u16Regs[0x14] = pSrc->u16ReverseSpeedLimitKmh_x10;
    }

    //
    // Block 4: FW Version (Addr 0x0015)
    uint16_t u16Version = ((uint16_t)pSrc->u8FwCategory << 8) | ((uint16_t)pSrc->u8FwMajor << 4) | pSrc->u8FwMinor;
    pDest->u16Regs[0x15] = u16Version;

    // Block 5: Mirrored Battery Info (Addr 0x0016 - 0x001B)
    pDest->u16Regs[0x16] = pSrc->u16GuiMirror_BatMfgInfo1;
    pDest->u16Regs[0x17] = pSrc->u16GuiMirror_BatMfgInfo2;
    pDest->u16Regs[0x18] = pSrc->u16GuiMirror_BatSerial;
    pDest->u16Regs[0x19] = pSrc->u16GuiMirror_BatCycles;
    pDest->u16Regs[0x1A] = pSrc->u16GuiMirror_BatDate;
    pDest->u16Regs[0x1B] = pSrc->u16GuiMirror_BatTime;

    // Block 6: Pedal RPM (Addr 0x001C)
    pDest->u16Regs[0x1C] = pSrc->u16PedalRpm;

    // Block 7: TQ Sensor AD (Addr 0x001D)
    pDest->u16Regs[0x1D] = pSrc->u16TorqueSensorAdValue;
}

//=================================================================================================
// Public API Functions - TEST MIRROR
//=================================================================================================

uint16_t modbusDecode_getMirroredValue(const S_MODBUS_ALL_DATA *pSrc,
                                       uint8_t u8SourceDeviceId,
                                       uint16_t u16SourceRegAddr)
{
    if (u8SourceDeviceId < 1 || u8SourceDeviceId > 3 || u16SourceRegAddr >= MIRROR_BLOCK_SIZE)
    {
        return 0; // Invalid arguments
    }
    // This is a simplified example and might need adjustment based on the final U_MODBUS_ALL_DATA_RAW layout
    const uint16_t *basePtr = (const uint16_t *)pSrc;
    uint16_t index = ((u8SourceDeviceId - 1) * sizeof(S_MODBUS_LCD_APP_DATA_RAW) / 2) + u16SourceRegAddr;
    if (index < 128)
    {
        return basePtr[index];
    }
    return 0; // Index out of bounds
}

void modbusDecode_setMirroredValue(S_MODBUS_ALL_DATA *pDest,
                                   uint8_t u8SourceDeviceId,
                                   uint16_t u16SourceRegAddr,
                                   uint16_t u16Value)
{
    if (u8SourceDeviceId < 1 || u8SourceDeviceId > 3 || u16SourceRegAddr >= MIRROR_BLOCK_SIZE)
    {
        return; // Invalid arguments
    }
    // This is a simplified example and might need adjustment
    uint16_t *basePtr = (uint16_t *)pDest;
    uint16_t index = ((u8SourceDeviceId - 1) * sizeof(S_MODBUS_LCD_APP_DATA_RAW) / 2) + u16SourceRegAddr;
    if (index < 128)
    {
        basePtr[index] = u16Value;
    }
}