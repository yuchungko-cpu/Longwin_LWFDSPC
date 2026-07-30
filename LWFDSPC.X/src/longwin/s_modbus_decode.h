#ifndef S_MODBUS_DECODE_H
#define S_MODBUS_DECODE_H

#include <stdint.h>
#include <stdbool.h>
#include "s_modbus_master.h"

//=================================================================================================
// 主控器韌體版本號 (回報於 ID03 0x0015;LongWin 格式 類別號.主版號次版號,兩位小數)
//   打包: (CATEGORY<<8)|(MAJOR<<4)|MINOR  ex: V4.75 -> 0x0475
//   目前 V0.15 -> Category=0, Major=1, Minor=5 -> 0x0015 (GUI 顯示 V0.15)
//   ※ 每次出版時更新此三個數字,並在 git 打對應 tag (ex: v0.15)。
//=================================================================================================
#define FW_VER_CATEGORY 0   // 類別號 (00~99) -> 高位元組
#define FW_VER_MAJOR    1   // 主版號 (0~9)   -> D7-D4
#define FW_VER_MINOR    5   // 次版號 (0~9)   -> D3-D0

//=================================================================================================
// Application-Layer Data Structures (High-Level)
//=================================================================================================

// --- Enums for Control Mode, Accel Curve, Battery Type ---
typedef enum
{
    E_CONTROL_MODE_ASSIST = 0,
    E_CONTROL_MODE_ELECTRIC = 1,
    E_CONTROL_MODE_AUTO = 2,
    E_CONTROL_MODE_WALK_ASSIST = 3,
    E_CONTROL_MODE_TORQUE_SENSOR = 4,
    E_CONTROL_MODE_VR_DIRECTION = 5,
    E_CONTROL_MODE_CRUISE = 6
} E_CONTROL_MODE;

typedef enum
{
    E_ACCEL_CURVE_A0 = 0, // External (LCD/APP)
    E_ACCEL_CURVE_A1 = 1, // Internal 1
    E_ACCEL_CURVE_A2 = 2, // Internal 2
    E_ACCEL_CURVE_A3 = 3, // Internal 3
    E_ACCEL_CURVE_A4 = 4, // Internal 4
    E_ACCEL_CURVE_A5 = 5, // Internal 5
} E_ACCEL_CURVE;

typedef enum
{
    E_BATTERY_TYPE_LI_24V = 0,
    E_BATTERY_TYPE_LI_36V = 1,
    E_BATTERY_TYPE_LI_48V = 2,
    E_BATTERY_TYPE_LEAD_ACID_24V = 5,
    E_BATTERY_TYPE_LEAD_ACID_36V = 6,
    E_BATTERY_TYPE_LEAD_ACID_48V = 7
} E_BATTERY_TYPE;

/**
 * @brief Decoded data for Battery (ID01)
 */
typedef struct
{
    uint16_t u16CapacityAh_x10; // Capacity in 0.1 AH
    uint8_t u8Percent;          // Percentage (0-100)
    uint16_t u16Voltage_x10;    // Voltage in 0.1 V
    uint8_t u8MfgCode;          // Manufacturer Code
    uint8_t u8MfgYear;          // Manufacturing Year (00-99)
    uint8_t u8MfgMonth;         // Manufacturing Month (1-12)
    uint8_t u8MfgWeek;          // Manufacturing Week (1-52)
    uint16_t u16SerialNumber;   // Serial number
    uint16_t u16ChargeCycles;   // Charge cycle count
    uint16_t u16LastChargeYear; // Last charge year (e.g., 2024)
    uint8_t u8LastChargeMonth;  // Last charge month
    uint8_t u8LastChargeDay;    // Last charge day
    uint8_t u8LastChargeHour;   // Last charge hour
    uint8_t u8LastChargeMinute; // Last charge minute
} S_BATTERY_DATA;

/**
 * @brief Decoded data for LCD/APP/PC-GUI (ID02, ID03)
 */
typedef struct
{
    // Mirrored values from other devices (e.g., Battery)
    // uint16_t u16LcdMirror_BatteryCapacityAh_x10; // From LCD Reg 0x0001
    // uint8_t  u8LcdMirror_BatteryPercent;         // From LCD Reg 0x0002
    // uint16_t u16LcdMirror_BatteryVoltage_x10;    // From LCD Reg 0x000C

    // Real-time values
    uint16_t u16CurrentSpeedKmh_x10; // Speed in 0.1 km/h
    uint16_t u16LoadCurrentA_x10;    // Current in 0.1 A
    uint16_t u16ControllerTempC_x10; // Temperature in 0.1 °C
    uint16_t u16MotorRpm;            // Motor RPM
    uint16_t u16PedalRpm;            // Pedal RPM
    uint16_t u16TorqueSensorAdValue; // TQ sensor AD value
    uint16_t u16Password;            // Password

    // Decoded bit-fields
    struct
    {
        bool bHeadlight : 1;
        bool bBrakeLight : 1;
        bool bRightTurnSignal : 1;
        bool bLeftTurnSignal : 1;
    } lights;

    struct
    {
        bool bBrakeSignalError : 1;
        bool bBatteryLowVoltage : 1;
        bool bBrakeStuck : 1;
        bool bThrottleStuck : 1;
        bool bEmbSensorFault : 1;            // D4, A19
        bool bMotorOverload : 1;
        bool bControllerOverTemp : 1;
        bool bMotorOrControllerError : 1;
        bool bUnused_D8 : 1;
        bool bMotorSensorError : 1;
        bool bLsnError : 1;
        bool bBatteryOverVoltage : 1;
        bool bCommBatteryError : 1;
        bool bCommLcdError : 1;
        bool bCommGuiError : 1;
        bool bCommAppError : 1;
    } errors;

    // User settings
    uint8_t u8WheelDiameterInches;
    bool bBacklightOn;
    bool bRegenOn;
    bool bCruiseOn;
    bool bHeadlightOn;
    bool bUnitIsMile;
    uint8_t u8AssistLevel;
    uint16_t u16ForwardSpeedLimitKmh_x10;
    uint16_t u16ReverseSpeedLimitKmh_x10;
    E_CONTROL_MODE eControlMode;
    E_ACCEL_CURVE eAccelCurve;
    uint8_t u8PulsePerRev;
    uint16_t u16ThrottleAdValue;
    bool bWalkModeActive;
    E_BATTERY_TYPE eBatteryType;

    // Acceleration times (ms/step)
    uint8_t u8AccelMs_Ax1;
    uint8_t u8AccelMs_Ax2;
    uint8_t u8AccelMs_Ax3;
    uint8_t u8AccelMs_Ax4;
    uint8_t u8AccelMs_Ax5;

    // T-Curve Torque Percentages (PC-GUI)
    uint8_t u8TorqueT1_pct;
    uint8_t u8TorqueT2_pct;
    uint8_t u8TorqueT3_pct;
    uint8_t u8TorqueT4_pct;
    uint8_t u8TorqueT5_pct;

    // Firmware Version (PC-GUI)
    uint8_t u8FwCategory;
    uint8_t u8FwMajor;
    uint8_t u8FwMinor;

    // Mirrored raw values from Battery for GUI (Regs 0x0004-0x0006, 0x0007-0x0009)
    uint16_t u16GuiMirror_BatMfgInfo1; // Corresponds to Battery reg 0x0004
    uint16_t u16GuiMirror_BatMfgInfo2; // Corresponds to Battery reg 0x0005
    uint16_t u16GuiMirror_BatSerial;   // Corresponds to Battery reg 0x0006
    uint16_t u16GuiMirror_BatCycles;   // Corresponds to Battery reg 0x0007
    uint16_t u16GuiMirror_BatDate;     // Corresponds to Battery reg 0x0008
    uint16_t u16GuiMirror_BatTime;     // Corresponds to Battery reg 0x0009

} S_SHARED_DEVICE_DATA;

/**
 * @brief Communication status flags
 */
typedef struct
{
    bool bBatteryValid : 1;
    bool bLCDValid : 1;
    bool bPCGUIValid : 1;
    bool bTestMirrorValid : 1;
} S_COMM_STATUS;

/**
 * @brief Main application data structure, containing all decoded Modbus data.
 */
typedef struct
{
    S_BATTERY_DATA battery;
    S_SHARED_DEVICE_DATA sharedData;
    S_COMM_STATUS commStatus;
} S_MODBUS_DATA;

//=================================================================================================
// Public API Functions
//=================================================================================================

/**
 * @brief Decodes raw data block from Battery (ID01).
 * @param pDest Pointer to S_BATTERY_DATA to store decoded application data.
 * @param pSrc Pointer to s_modbus_battery_data_t, the raw data from the master.
 */
void modbusDecode_decodeBatteryData(S_BATTERY_DATA *pDest, const S_MODBUS_BATTERY_DATA_RAW *pSrc);

/**
 * @brief Decodes raw data block from LCD (ID02).
 * @param pDest Pointer to S_APP_DATA to store decoded application data.
 * @param pSrc Pointer to s_modbus_lcd_app_data_t, the raw data from the master.
 */
void modbusDecode_decodeLcdData(S_SHARED_DEVICE_DATA *pDest, const U_MODBUS_LCD_APP_DATA *pSrc);

/**
 * @brief Decodes raw data block from PC-GUI (ID03).
 * @param pDest Pointer to S_APP_DATA to store decoded application data.
 * @param pSrc Pointer to s_modbus_pc_gui_data_t, the raw data from the master.
 */
void modbusDecode_decodeGuiData(S_SHARED_DEVICE_DATA *pDest, const U_MODBUS_PC_GUI_DATA *pSrc);

/**
 * @brief Encodes application settings into the raw LCD data block for writing.
 * @param pDest Pointer to s_modbus_lcd_app_data_t, the destination for raw data.
 * @param pSrc Pointer to S_APP_DATA, the source of application settings.
 */
void modbusDecode_encodeLcdSettings(U_MODBUS_LCD_APP_DATA *pDest,
                                    const S_SHARED_DEVICE_DATA *pSrc,
                                    const S_BATTERY_DATA *pSrcBattery,
                                    uint8_t u8Update);

/**
 * @brief Encodes application settings into the raw PC-GUI data block for writing.
 * @param pDest Pointer to s_modbus_pc_gui_data_t, the destination for raw data.
 * @param pSrc Pointer to S_APP_DATA, the source of application settings.
 */
void modbusDecode_encodeGuiSettings(U_MODBUS_PC_GUI_DATA *pDest,
                                    const S_SHARED_DEVICE_DATA *pSrc,
                                    const S_BATTERY_DATA *pSrcBattery,
                                    uint8_t u8Update);

/**
 * @brief Gets a value from the test mirror block using the mapping formula.
 * @param pSrc Pointer to s_modbus_test_block_t, the raw data from the master.
 * @param u8SourceDeviceId The original device ID (1, 2, 3) of the data.
 * @param u16SourceRegAddr The original register address from the source device.
 * @return The 16-bit value stored at the mirrored address.
 */
uint16_t modbusDecode_getMirroredValue(const S_MODBUS_ALL_DATA *pSrc,
                                       uint8_t u8SourceDeviceId,
                                       uint16_t u16SourceRegAddr);

/**
 * @brief Sets a value in the test mirror block using the mapping formula.
 * @param pDest Pointer to s_modbus_test_block_t, the destination for the raw data.
 * @param u8SourceDeviceId The original device ID (1, 2, 3) of the data.
 * @param u16SourceRegAddr The original register address from the source device.
 * @param u16Value The 16-bit value to write to the mirrored address.
 */
void modbusDecode_setMirroredValue(S_MODBUS_ALL_DATA *pDest,
                                   uint8_t u8SourceDeviceId,
                                   uint16_t u16SourceRegAddr,
                                   uint16_t u16Value);

#endif // S_MODBUS_DECODE_H