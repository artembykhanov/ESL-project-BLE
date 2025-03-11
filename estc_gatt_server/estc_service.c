#include "estc_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

volatile static uint16_t char_value_1 = INITIAL_VALUE_1;
volatile static uint32_t char_value_2 = INITIAL_VALUE_2;

static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service);

ret_code_t estc_ble_service_init(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;
    // Dome: 4. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`

    ble_uuid128_t base_uuid_t = {RANDOM_BASE_UUID};
    service->uuid_type = BLE_UUID_TYPE_VENDOR_BEGIN;

    error_code = sd_ble_uuid_vs_add(&base_uuid_t, &service->uuid_type);
    APP_ERROR_CHECK(error_code);

    // Done: 5. Add service to the BLE stack using `sd_ble_gatts_service_add`
    service->uuid_type = BLE_UUID_TYPE_BLE;
    ble_uuid_t service_uuid = {RANDOM_SERVICE_UUID, service->uuid_type};

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &service->service_handle);
    APP_ERROR_CHECK(error_code);

    return estc_ble_add_characteristics(service);
}

static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;

    /* CHARACTERISTIC 1 */
    
    // DONE: 6.1. Add custom characteristic UUID using `sd_ble_uuid_vs_add`, same as in step 4
    ble_uuid_t characteristic_uuid_1 = {RANDOM_CHARACTERISTIC_UUID_1, BLE_UUID_TYPE_BLE};
    const char characteristic_1_desc[] = CHARACTERISTIC_1_DESC;

    // Done: 6.5. Configure Characteristic metadata (enable read and write)
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));

    char_md.p_char_user_desc = (uint8_t *)characteristic_1_desc;
    char_md.char_user_desc_size = strlen(characteristic_1_desc);
    char_md.char_user_desc_max_size = strlen(characteristic_1_desc);
    char_md.char_props.read = CHARACTERISTIC_1_PROPERTIES_READ;
    char_md.char_props.write = CHARACTERISTIC_1_PROPERTIES_WRITE;

    // Configures attribute metadata. For now we only specify that the attribute will be stored in the softdevice
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc = BLE_GATTS_VLOC_USER;
    // Done: 6.6. Set read/write security levels to our attribute metadata using `BLE_GAP_CONN_SEC_MODE_SET_OPEN`
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    // Done: 6.2. Configure the characteristic value attribute (set the UUID and metadata)
    attr_char_value.p_uuid = &characteristic_uuid_1;
    attr_char_value.p_attr_md = &attr_md;

    // Done: 6.7. Set characteristic length in number of bytes in attr_char_value structure
    attr_char_value.init_len = sizeof(uint16_t);
    attr_char_value.max_len = sizeof(uint16_t);
    attr_char_value.p_value = (uint8_t *)&char_value_1;

    // Done: 6.4. Add new characteristic to the service using `sd_ble_gatts_characteristic_add`
    error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, &service->characteristic_handles_1);
    APP_ERROR_CHECK(error_code);

    /* CHARACTERISTIC 2 */
    ble_uuid_t characteristic_uuid_2 = {RANDOM_CHARACTERISTIC_UUID_2, BLE_UUID_TYPE_BLE};
    const char characteristic_2_desc[] = CHARACTERISTIC_2_DESC;

    memset(&char_md, 0, sizeof(char_md));

    char_md.p_char_user_desc = (uint8_t *)characteristic_2_desc;
    char_md.char_user_desc_size = strlen(characteristic_2_desc);
    char_md.char_user_desc_max_size = strlen(characteristic_2_desc);
    char_md.char_props.read = CHARACTERISTIC_2_PROPERTIES_READ;
    char_md.char_props.write = CHARACTERISTIC_2_PROPERTIES_WRITE;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &characteristic_uuid_2;
    attr_char_value.p_attr_md = &attr_md;

    attr_char_value.init_len = sizeof(uint32_t);
    attr_char_value.max_len = sizeof(uint32_t);
    attr_char_value.p_value = (uint8_t *)&char_value_2;

    error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, &service->characteristic_handles_2);
    APP_ERROR_CHECK(error_code);
    
    /* Characteristic 2 */
    // ble_uuid_t characteristic_uuid_2 = {RANDOM_CHARACTERISTIC_UUID_2, BLE_UUID_TYPE_BLE};

    // 2-я характеристика через функцию characteristic_add
    // ble_add_char_params_t char_props;
    // memset(&char_props, 0, sizeof(char_props));
    // char_props.uuid = RANDOM_CHARACTERISTIC_UUID_2;
    // char_props.uuid_type = BLE_UUID_TYPE_BLE;
    // char_props.max_len = sizeof(uint32_t);
    // char_props.init_len = sizeof(uint32_t);
    // char_props.char_props.read = 1;
    // char_props.char_props.write = 1;
    // char_props.p_init_value = (uint8_t *)&char_value_2;

    // error_code = characteristic_add(service->service_handle, &char_props, &service->characteristic_handles_2);
    // APP_ERROR_CHECK(error_code);

    return NRF_SUCCESS;
}