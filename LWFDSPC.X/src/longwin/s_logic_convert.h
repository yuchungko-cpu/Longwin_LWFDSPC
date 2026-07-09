#ifndef S_LOGIC_CONVERT_H_
#define S_LOGIC_CONVERT_H_

#include <stdint.h>

// --- System-Wide ADC Parameters ---
#define LOGIC_CONVERT_ADC_RESOLUTION_BITS 12
#define LOGIC_CONVERT_VREF_MV             3300

/**
 * @brief Converts a raw ADC value to millivolts based on a specific voltage divider circuit.
 * @param u16AdcRawValue The raw value from the ADC buffer.
 * @param u32R1_ohms The value of the R1 resistor (top resistor) in the divider.
 * @param u32R2_ohms The value of the R2 resistor (bottom resistor, to GND) in the divider.
 * @return The calculated voltage in millivolts (mV).
 */
uint16_t logic_convert_adcToVoltageMv(uint16_t u16AdcRawValue, uint32_t u32R1_ohms, uint32_t u32R2_ohms);

#endif // S_LOGIC_CONVERT_H_