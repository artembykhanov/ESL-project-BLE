#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

// DONE: 1. Generate random BLE UUID (Version 4 UUID) and define it in the following format:
#define RANDOM_BASE_UUID {0x41, 0xC1, 0xB0, 0x2D, 0x56, 0x32, /* - */ 0x1B, 0xA2, /* - */ 0xF9, 0x4A, /* - */ 0x3A, 0x6B, /* - */ 0xF1, 0xB1, 0x69, 0xA9}
// UUID: a969b1f1-6b3a-4af9-a21b-32562db0c141

// DONE: 2. Pick a random service 16-bit UUID and define it:
#define RANDOM_SERVICE_UUID 0x180D

typedef struct
{
    uint16_t service_handle;
    uint8_t uuid_type;
} ble_estc_service_t;

ret_code_t estc_ble_service_init(ble_estc_service_t *service);

#endif /* ESTC_SERVICE_H__ */