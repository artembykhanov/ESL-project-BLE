#ifndef FLASH_STORAGE_H__
#define FLASH_STORAGE_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"

/**
 * @brief Initialize flash storage
 */
void flash_storage_init(void);

/**
 * @brief Update the RGB state (on/off)
 * @param new_state The new state value
 */
void flash_storage_update_state(uint8_t new_state);

/**
 * @brief Update the RGB color values
 * @param r The new red value
 * @param g The new green value
 * @param b The new blue value
 */
void flash_storage_update_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Update the values of the RGB state and color (legacy function)
 * @param update_state True if the state should be updated, false if not
 * @param new_state The new state value
 * @param update_rgb True if the RGB color should be updated, false if not
 * @param r The new red value
 * @param g The new green value
 * @param b The new blue value
 */
void flash_storage_update_values(bool update_state, uint32_t new_state,
                                 bool update_rgb, uint8_t r, uint8_t g, uint8_t b);

#endif // FLASH_STORAGE_H__
