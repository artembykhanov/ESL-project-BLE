#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"

void flash_storage_init(void);
void flash_storage_update_values(bool update_state, uint32_t new_state,
                                 bool update_rgb, uint8_t r, uint8_t g, uint8_t b);

#endif // FLASH_STORAGE_H
