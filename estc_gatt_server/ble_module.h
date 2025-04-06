#ifndef BLE_MODULE_H
#define BLE_MODULE_H

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "estc_service.h"

ble_estc_service_t *get_estc_service(void);
nrf_ble_gatt_t *get_gatt(void);
nrf_ble_qwr_t *get_qwr(void);
ble_advertising_t *get_advertising(void);

void ble_stack_init(void);
void gap_params_init(void);
void gatt_init(void);
void services_init(void);
void conn_params_init(void);
void advertising_init(void);
void advertising_start(void);
void buttons_leds_init(void);

void rgb_state_write_handler(uint16_t conn_handle, ble_estc_service_t *p_lbs, uint8_t new_state);
void rgb_value_write_handler(uint16_t conn_handle, ble_estc_service_t *p_lbs, uint8_t r, uint8_t g, uint8_t b);

#endif // BLE_MODULE_H
