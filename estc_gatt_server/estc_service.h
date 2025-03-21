#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

// DONE: 1. Generate random BLE UUID (Version 4 UUID) and define it in the following format:
#define RANDOM_BASE_UUID {0x41, 0xC1, 0xB0, 0x2D, 0x56, 0x32, /* - */ 0x1B, 0xA2, /* - */ 0xF9, 0x4A, /* - */ 0x3A, 0x6B, /* - */ 0xF1, 0xB1, 0x69, 0xA9}
// UUID: a969b1f1-6b3a-4af9-a21b-32562db0c141

// DONE: 2. Pick a random service 16-bit UUID and define it:
#define RANDOM_SERVICE_UUID 0x9000

// Done: 3. Pick a characteristic UUID and define it:
#define RANDOM_CHARACTERISTIC_UUID_1 0x9001
#define RANDOM_CHARACTERISTIC_UUID_2 0x9002

#define INITIAL_VALUE_1 0
#define INITIAL_VALUE_2 0
#define CHARACTERISTIC_1_DESC "Characteristic 1: 2-byte read/write"
#define CHARACTERISTIC_2_DESC "Characteristic 2: 4-byte only read"
#define CHARACTERISTIC_1_PROPERTIES_NOTIFY 1
#define CHARACTERISTIC_2_PROPERTIES_INDICATE 1

typedef struct
{
    uint16_t service_handle;
    uint16_t connection_handle;
    uint8_t uuid_type;

    // Done: 6.3. Add handles for characterstic (type: ble_gatts_char_handles_t)
    ble_gatts_char_handles_t characteristic_handles_1;
    ble_gatts_char_handles_t characteristic_handles_2;
} ble_estc_service_t;

ret_code_t estc_ble_service_init(ble_estc_service_t *service);

void estc_ble_service_on_ble_event(const ble_evt_t *ble_evt, void *ctx);

void estc_update_characteristic_1_value(ble_estc_service_t *service, int32_t *value);

#endif /* ESTC_SERVICE_H__ */