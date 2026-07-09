#include "s_logic_convert.h"

uint16_t logic_convert_adcToVoltageMv(uint16_t u16AdcRawValue, uint32_t u32R1_ohms, uint32_t u32R2_ohms)
{
    // 1. Calculate the maximum possible ADC value based on resolution
    const uint32_t u32AdcMax = (1UL << LOGIC_CONVERT_ADC_RESOLUTION_BITS) - 1;
    if (u32AdcMax == 0) {
        return 0; // Avoid division by zero
    }

    // 2. Convert the raw ADC value to the voltage at the ADC pin (in mV)
    // Formula: V_adc = (ADC_raw / ADC_max) * V_ref
    uint32_t u32VAdcMv = ((uint32_t)u16AdcRawValue * (uint32_t)LOGIC_CONVERT_VREF_MV) / u32AdcMax;

    // 3. Calculate the original voltage before the voltage divider
    // Formula: V_in = V_adc * (R1 + R2) / R2
    if (u32R2_ohms == 0) {
        return 0; // Avoid division by zero
    }

    uint32_t u32R1PlusR2 = u32R1_ohms + u32R2_ohms;
    
    // Check for overflow before multiplication
    if (u32R1PlusR2 < u32R1_ohms) { 
        return 65535; // Return max value on overflow
    }

    uint32_t u32VInMv = (u32VAdcMv * u32R1PlusR2) / u32R2_ohms;

    // Clamp to 16-bit max value if result exceeds it
    if (u32VInMv > 65535) {
        return 65535;
    }

    return (uint16_t)u32VInMv;
}