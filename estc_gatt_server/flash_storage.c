#include "flash_storage.h"
#include "app_error.h"
#include "nrf_log.h"
#include "estc_service.h"
#include "pwm_control.h"

// Data storage address
#define FLASH_ADDR_DATA 0x3e000

// Macros for working with bits in the gs_rgb_data variable
#define RGB_STATE_MASK 0x000000FF   // Mask for RGB state (lowest byte)
#define RED_VALUE_MASK 0x0000FF00   // Mask for R component (2nd byte)
#define GREEN_VALUE_MASK 0x00FF0000 // Mask for G component (3rd byte)
#define BLUE_VALUE_MASK 0xFF000000  // Mask for B component (highest byte)

#define GET_RGB_STATE(val) ((uint8_t)((val) & RGB_STATE_MASK))
#define GET_RED_VALUE(val) ((uint8_t)(((val) & RED_VALUE_MASK) >> 8))
#define GET_GREEN_VALUE(val) ((uint8_t)(((val) & GREEN_VALUE_MASK) >> 16))
#define GET_BLUE_VALUE(val) ((uint8_t)(((val) & BLUE_VALUE_MASK) >> 24))

#define SET_RGB_STATE(val, state) ((val & ~RGB_STATE_MASK) | ((state) & RGB_STATE_MASK))
#define SET_RED_VALUE(val, r) ((val & ~RED_VALUE_MASK) | (((uint32_t)(r) << 8) & RED_VALUE_MASK))
#define SET_GREEN_VALUE(val, g) ((val & ~GREEN_VALUE_MASK) | (((uint32_t)(g) << 16) & GREEN_VALUE_MASK))
#define SET_BLUE_VALUE(val, b) ((val & ~BLUE_VALUE_MASK) | (((uint32_t)(b) << 24) & BLUE_VALUE_MASK))

static uint32_t gs_rgb_data = 0;

// Declaration of fstorage event handler
static void fstorage_evt_handler(nrf_fstorage_evt_t *p_evt);

// Initialization of fstorage instance
NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) = {
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = FLASH_ADDR_DATA,
    .end_addr = 0x3ffff,
};

static uint32_t flash_storage_get_end_addr(void)
{
    uint32_t const bootloader_addr = BOOTLOADER_ADDRESS;
    uint32_t const page_sz = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ? bootloader_addr : (code_sz * page_sz));
}

static void fstorage_evt_handler(nrf_fstorage_evt_t *p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
    case NRF_FSTORAGE_EVT_WRITE_RESULT:
    {
        NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                     p_evt->len, p_evt->addr);
    }
    break;

    case NRF_FSTORAGE_EVT_ERASE_RESULT:
    {
        NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                     p_evt->len, p_evt->addr);
    }
    break;

    default:
        break;
    }
}

void flash_storage_init(void)
{
    int ret = nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL);
    APP_ERROR_CHECK(ret);

    (void)flash_storage_get_end_addr();

    ret = nrf_fstorage_read(&fstorage, FLASH_ADDR_DATA, &gs_rgb_data, sizeof(gs_rgb_data));
    APP_ERROR_CHECK(ret);

    if (0xFFFFFFFF == gs_rgb_data)
    {
        NRF_LOG_INFO("FLASH STORAGE: RGB data is not set in the flash, defaulting to OFF and (0,0,0)");
        gs_rgb_data = 0;
    }

    uint8_t rgb_state = GET_RGB_STATE(gs_rgb_data);
    uint8_t red_value = GET_RED_VALUE(gs_rgb_data);
    uint8_t green_value = GET_GREEN_VALUE(gs_rgb_data);
    uint8_t blue_value = GET_BLUE_VALUE(gs_rgb_data);

    NRF_LOG_INFO("FLASH STORAGE: Recovered RGB state: %u, RGB values: (%u, %u, %u)",
                 rgb_state, red_value, green_value, blue_value);

    estc_characteristic_init_values(rgb_state, red_value, green_value, blue_value);

    pwm_set_rgb_color(red_value, green_value, blue_value);

    if (rgb_state)
    {
        pwm_on_rgb();
        NRF_LOG_INFO("FLASH STORAGE: Enabling the RGB");
    }
    else
    {
        pwm_off_rgb();
        NRF_LOG_INFO("FLASH STORAGE: Disabling the RGB");
    }
}

void flash_storage_update_values(bool update_state, uint32_t new_state,
                                 bool update_rgb, uint8_t r, uint8_t g, uint8_t b)
{
    int rc;

    bool need_update = false;

    if (update_state)
    {
        uint8_t current_state = GET_RGB_STATE(gs_rgb_data);
        need_update = (current_state != new_state);
        if (need_update)
        {
            gs_rgb_data = SET_RGB_STATE(gs_rgb_data, new_state);
        }
    }

    if (update_rgb)
    {
        uint8_t current_r = GET_RED_VALUE(gs_rgb_data);
        uint8_t current_g = GET_GREEN_VALUE(gs_rgb_data);
        uint8_t current_b = GET_BLUE_VALUE(gs_rgb_data);

        if (current_r != r || current_g != g || current_b != b)
        {
            need_update = true;
            gs_rgb_data = SET_RED_VALUE(gs_rgb_data, r);
            gs_rgb_data = SET_GREEN_VALUE(gs_rgb_data, g);
            gs_rgb_data = SET_BLUE_VALUE(gs_rgb_data, b);
        }
    }

    if (!need_update)
    {
        NRF_LOG_INFO("FLASH STORAGE: No changes in RGB data, skipping flash update.");
        return;
    }

    rc = nrf_fstorage_erase(&fstorage, FLASH_ADDR_DATA, 1, NULL);
    APP_ERROR_CHECK(rc);
    NRF_LOG_INFO("FLASH STORAGE: Erased flash page for RGB data.");

    NRF_LOG_INFO("FLASH STORAGE: Writing RGB data \"%x\" to flash.", gs_rgb_data);
    rc = nrf_fstorage_write(&fstorage, FLASH_ADDR_DATA, &gs_rgb_data, sizeof(gs_rgb_data), NULL);
    APP_ERROR_CHECK(rc);
}
