#include "estc_service.h"

#include "app_error.h"
#include "nrf_log.h"
#include "pwm_control.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

#define CHARACTERISTIC_RGB_STATE_DESC "WRITE/READ/NOTIFY: RGB state characteristic 1 byte"
#define CHARACTERISTIC_RGB_VALUE_DESC "WRITE/READ/NOTIFY: RGB value characteristic 3 bytes"

static uint8_t rgb_state_init_value = 0;
static uint8_t rgb_value_init_values[3] = {0, 0, 0};

static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service);
static ret_code_t estc_add_characteristic(ble_estc_service_t *service, ble_gatts_char_handles_t *handles,
                                          uint16_t uuid, const char *desc, size_t size, uint8_t *p_init_value);

void estc_characteristic_init_values(uint8_t rgb_state, uint8_t r, uint8_t g, uint8_t b)
{
    rgb_state_init_value = rgb_state;
    rgb_value_init_values[0] = r;
    rgb_value_init_values[1] = g;
    rgb_value_init_values[2] = b;

    NRF_LOG_INFO("BLE init values set - RGB state: %d, RGB values: (%d, %d, %d)",
                 rgb_state, r, g, b);
}

ret_code_t estc_ble_service_init(ble_estc_service_t *service, const ble_lbs_init_t *lbs_init)
{
    ret_code_t error_code = NRF_SUCCESS;

    service->rgb_state_write_handler = lbs_init->rgb_state_write_handler;
    service->rgb_value_write_handler = lbs_init->rgb_value_write_handler;

    ble_uuid128_t base_uuid_t = {RANDOM_BASE_UUID};

    error_code = sd_ble_uuid_vs_add(&base_uuid_t, &service->uuid_type);
    APP_ERROR_CHECK(error_code);

    ble_uuid_t service_uuid = {RANDOM_SERVICE_UUID, service->uuid_type};

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &service->service_handle);
    APP_ERROR_CHECK(error_code);

    return estc_ble_add_characteristics(service);
}

static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;

    error_code = estc_add_characteristic(service, &service->rgb_state_characteristic_handles,
                                         RANDOM_CHARACTERISTIC_UUID_RGB_STATE,
                                         CHARACTERISTIC_RGB_STATE_DESC,
                                         CHARACTERISTIC_RGB_STATE_SIZE,
                                         &rgb_state_init_value);
    APP_ERROR_CHECK(error_code);

    error_code = estc_add_characteristic(service, &service->rgb_value_characteristic_handles,
                                         RANDOM_CHARACTERISTIC_UUID_RGB_VALUE,
                                         CHARACTERISTIC_RGB_VALUE_DESC,
                                         CHARACTERISTIC_RGB_VALUE_SIZE,
                                         rgb_value_init_values);

    APP_ERROR_CHECK(error_code);

    return NRF_SUCCESS;
}

static ret_code_t estc_add_characteristic(ble_estc_service_t *service, ble_gatts_char_handles_t *handles,
                                          uint16_t uuid, const char *desc, size_t size, uint8_t *p_init_value)
{
    ret_code_t error_code = NRF_SUCCESS;

    ble_add_char_params_t char_props;
    memset(&char_props, 0, sizeof(char_props));

    char_props.uuid = uuid;
    char_props.uuid_type = service->uuid_type;
    char_props.max_len = size;
    char_props.init_len = size;
    char_props.p_init_value = p_init_value;

    char_props.char_props.write = 1;
    char_props.char_props.read = 1;
    char_props.read_access = SEC_OPEN;
    char_props.write_access = SEC_OPEN;

    char_props.char_props.notify = 1;
    char_props.cccd_write_access = SEC_OPEN;

    ble_add_char_user_desc_t user_desc;
    memset(&user_desc, 0, sizeof(user_desc));

    user_desc.p_char_user_desc = (uint8_t *)desc;
    user_desc.size = strlen(desc);
    user_desc.max_size = strlen(desc);
    user_desc.char_props.read = 1;
    user_desc.read_access = SEC_OPEN;

    char_props.p_user_descr = &user_desc;

    error_code = characteristic_add(service->service_handle, &char_props, handles);
    APP_ERROR_CHECK(error_code);

    return error_code;
}

static void on_write(ble_estc_service_t *p_service, ble_evt_t const *p_ble_evt)
{
    ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_service->rgb_state_characteristic_handles.value_handle)
    {
        NRF_LOG_INFO("ESTC SERVICE: RGB STATE characteristic write event received");
        uint8_t led_value = p_evt_write->data[0];
        p_service->rgb_state_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_service, led_value);
    }
    else if (p_evt_write->handle == p_service->rgb_value_characteristic_handles.value_handle)
    {
        NRF_LOG_INFO("ESTC SERVICE: RGB VALUE characteristic write event received");
        uint8_t r = p_evt_write->data[0];
        uint8_t g = p_evt_write->data[1];
        uint8_t b = p_evt_write->data[2];
        p_service->rgb_value_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_service, r, g, b);
    }
}

void ble_lbs_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context)
{
    ble_estc_service_t *p_service = (ble_estc_service_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GATTS_EVT_WRITE:
        NRF_LOG_INFO("ESTC SERVICE: WRITE event received");
        on_write(p_service, p_ble_evt);
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        NRF_LOG_INFO("ESTC SERVICE: RW AUTHORIZE REQUEST event received");
        break;

    case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        NRF_LOG_INFO("ESTC SERVICE: HVN TX COMPLETE event received");
        break;

    default:
        // No implementation needed.
        break;
    }
}