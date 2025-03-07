#include "estc_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

ret_code_t estc_ble_service_init(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;
    service->service_handle = BLE_CONN_HANDLE_INVALID;

    // DONE: 3. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`
    ble_uuid128_t base_uuid_t = {RANDOM_BASE_UUID};
    service->uuid_type = BLE_UUID_TYPE_VENDOR_BEGIN;

    error_code = sd_ble_uuid_vs_add(&base_uuid_t, &service->uuid_type);
    APP_ERROR_CHECK(error_code);

    // DONE: 4. Add service to the BLE stack using `sd_ble_gatts_service_add`
    service->uuid_type = BLE_UUID_TYPE_BLE;
    ble_uuid_t service_uuid = {RANDOM_SERVICE_UUID, service->uuid_type};

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &service->service_handle);
    APP_ERROR_CHECK(error_code);

    return NRF_SUCCESS;
}
