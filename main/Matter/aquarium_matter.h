/**
 * @file aquarium_matter.h
 * @brief Matter/HomeKit integration for Aquarium Temperature Monitor
 * 
 * Features:
 * - Temperature Sensor (Matter cluster)
 * - HomeKit compatibility
 * - QR code pairing
 * - iPhone integration
 */

#ifndef AQUARIUM_MATTER_H
#define AQUARIUM_MATTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Matter stack and create Temperature Sensor device
 * 
 * @return true if successful, false otherwise
 */
bool aquarium_matter_init(void);

/**
 * Update Matter temperature attribute
 * 
 * @param temp_celsius Temperature in Celsius
 */
void aquarium_matter_update_temperature(float temp_celsius);

/**
 * Start Matter commissioning (pairing mode)
 * Display QR code for iPhone pairing
 */
void aquarium_matter_start_commissioning(void);

/**
 * Print Matter pairing QR code to console
 */
void aquarium_matter_print_qr_code(void);

#ifdef __cplusplus
}
#endif

#endif // AQUARIUM_MATTER_H
