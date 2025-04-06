#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "sdk_errors.h"

/**@brief   Macro for defining a ble_lbs instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_LBS_DEF(_name)                          \
    static ble_estc_service_t _name;                \
    NRF_SDH_BLE_OBSERVER(_name##_obs,               \
                         BLE_LBS_BLE_OBSERVER_PRIO, \
                         ble_lbs_on_ble_evt, &_name)

#define RANDOM_BASE_UUID {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, \
                          0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}
#define RANDOM_SERVICE_UUID 0x1523 // Service UUID

#define RANDOM_CHARACTERISTIC_UUID_RGB_STATE 0x1525 // RGB STATE characteristic UUID
#define RANDOM_CHARACTERISTIC_UUID_RGB_VALUE 0x1526 // RGB VALUE characteristic UUID

#define CHARACTERISTIC_RGB_STATE_SIZE sizeof(uint8_t)
#define CHARACTERISTIC_RGB_VALUE_SIZE (sizeof(uint8_t) * 3)

struct ble_estc_service_s;

// RGB state characteristic write event handler type
typedef void (*ble_lbs_rgb_state_write_handler_t)(uint16_t conn_handle, struct ble_estc_service_s *p_lbs, uint8_t new_state);

// RGB value characteristic write event handler type
typedef void (*ble_lbs_rgb_value_write_handler_t)(uint16_t conn_handle, struct ble_estc_service_s *p_lbs, uint8_t r, uint8_t g, uint8_t b);

/** @brief LED Button Service init structure. This structure contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_lbs_rgb_state_write_handler_t rgb_state_write_handler; // Event handler to be called when RGB state characteristic is written.
    ble_lbs_rgb_value_write_handler_t rgb_value_write_handler; // Event handler to be called when RGB value characteristic is written.
} ble_lbs_init_t;

typedef struct ble_estc_service_s
{
    uint16_t service_handle;
    uint8_t uuid_type;
    uint16_t connection_handle;
    ble_lbs_rgb_state_write_handler_t rgb_state_write_handler;
    ble_lbs_rgb_value_write_handler_t rgb_value_write_handler;

    ble_gatts_char_handles_t rgb_state_characteristic_handles;
    ble_gatts_char_handles_t rgb_value_characteristic_handles;
} ble_estc_service_t;

ret_code_t estc_ble_service_init(ble_estc_service_t *service, const ble_lbs_init_t *lbs_init);
void estc_characteristic_init_values(uint8_t rgb_state, uint8_t r, uint8_t g, uint8_t b);

void ble_lbs_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

void estc_update_characteristic_1_value(ble_estc_service_t *service, int32_t *value);

#endif /* ESTC_SERVICE_H__ */