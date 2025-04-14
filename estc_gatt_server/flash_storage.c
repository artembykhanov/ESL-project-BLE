#include "flash_storage.h"
#include "app_error.h"
#include "nrf_log.h"
#include "estc_service.h"
#include "pwm_control.h"
#include <stdint.h>
#include "nrf_bootloader_info.h"
#include "nrf_dfu_types.h"


#define FLASH_BOOTLOADER_START_ADDR (0xE0000) //BOOTLOADER_ADDRESS
#define FLASH_PAGE_SIZE (0x1000) //CODE_PAGE_SIZE
#define FLASH_PAGE_START (FLASH_BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE) 
#define FLASH_PAGE_END (FLASH_PAGE_START + FLASH_PAGE_SIZE)
#define FLASH_EMPTY_VALUE 0xFFFFFFFF
#define FLASH_WORD_SIZE (sizeof(uint32_t))

// Macros for working with bits in the gs_rgb_data variable
#define RGB_STATE_MASK 0x000000FF   // Mask for RGB state (lowest byte)
#define RED_VALUE_MASK 0x0000FF00   // Mask for R component (2nd byte)
#define GREEN_VALUE_MASK 0x00FF0000 // Mask for G component (3rd byte)
#define BLUE_VALUE_MASK 0xFF000000  // Mask for B component (highest byte)

#define GET_RGB_STATE(val) ((uint8_t)((val) & RGB_STATE_MASK))
#define GET_RED_VALUE(val) ((uint8_t)(((val) & RED_VALUE_MASK) >> 8))
#define GET_GREEN_VALUE(val) ((uint8_t)(((val) & GREEN_VALUE_MASK) >> 16))
#define GET_BLUE_VALUE(val) ((uint8_t)(((val) & BLUE_VALUE_MASK) >> 24))

// Macro for setting all values at once
#define SET_RGB_VALUES(val, state, r, g, b) \
    (((val) & ~(RGB_STATE_MASK | RED_VALUE_MASK | GREEN_VALUE_MASK | BLUE_VALUE_MASK)) | \
    ((state) & RGB_STATE_MASK) | \
    (((uint32_t)(r) << 8) & RED_VALUE_MASK) | \
    (((uint32_t)(g) << 16) & GREEN_VALUE_MASK) | \
    (((uint32_t)(b) << 24) & BLUE_VALUE_MASK))

// Macros for backward compatibility
#define SET_RGB_STATE(val, state) (((val) & ~RGB_STATE_MASK) | ((state) & RGB_STATE_MASK))
#define SET_RED_VALUE(val, r) (((val) & ~RED_VALUE_MASK) | (((uint32_t)(r) << 8) & RED_VALUE_MASK))
#define SET_GREEN_VALUE(val, g) (((val) & ~GREEN_VALUE_MASK) | (((uint32_t)(g) << 16) & GREEN_VALUE_MASK))
#define SET_BLUE_VALUE(val, b) (((val) & ~BLUE_VALUE_MASK) | (((uint32_t)(b) << 24) & BLUE_VALUE_MASK))

// Structure for storing flash context
typedef struct {
    bool erase_needed;
    uint32_t *current_address;
} flash_context_t;

static uint32_t gs_rgb_data = 0;

static flash_context_t flash_context = {
    .erase_needed = false,
    .current_address = NULL
};

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
    .start_addr = FLASH_PAGE_START,
    .end_addr = FLASH_PAGE_END - 1,
};

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

/**
 * @brief Search for the address of the last written block
 * @param result Pointer to save the found address
 * @return true if a block is found, false if the page is empty or full
 */
static bool flash_find_last_address(uint32_t **result)
{
    uint32_t *addr = (uint32_t *)FLASH_PAGE_START;
    uint32_t *last_valid_addr = NULL;
    uint32_t *page_end = (uint32_t *)FLASH_PAGE_END;
    uint32_t data;

    while (addr < page_end)
    {
        // Read data at the current address
        int ret = nrf_fstorage_read(&fstorage, (uint32_t)addr, &data, FLASH_WORD_SIZE);
        if (ret != NRF_SUCCESS)
        {
            NRF_LOG_INFO("FLASH STORAGE: Error reading data at address 0x%x", (uint32_t)addr);
            return false;
        }

        // If an empty cell is found, the previous one was the last
        if (data == FLASH_EMPTY_VALUE)
        {
            if (last_valid_addr != NULL)
            {
                *result = last_valid_addr;
                return true;
            }
            else
            {
                // The page is empty
                flash_context.current_address = addr;
                return false;
            }
        }

        last_valid_addr = addr;
        addr += 1;  // Go to the next word
    }

    // If we get here, the page is full
    flash_context.erase_needed = true;
    return false;
}

/**
 * @brief Initialize flash storage
 */
void flash_storage_init(void)
{
    int ret = nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_INFO("FLASH STORAGE: Initialized at address 0x%x, size: 0x%x", 
                 FLASH_PAGE_START, FLASH_PAGE_SIZE);

    uint32_t *last_addr = NULL;
    bool found = flash_find_last_address(&last_addr);

    if (!found)
    {
        if (flash_context.erase_needed)
        {
            NRF_LOG_INFO("FLASH STORAGE: Page is full, erasing");
            ret = nrf_fstorage_erase(&fstorage, FLASH_PAGE_START, 1, NULL);
            APP_ERROR_CHECK(ret);
            flash_context.erase_needed = false;
            flash_context.current_address = (uint32_t *)FLASH_PAGE_START;
            gs_rgb_data = 0;
        }
        else
        {
            NRF_LOG_INFO("FLASH STORAGE: No data found, initializing to defaults");
            gs_rgb_data = 0;
            flash_context.current_address = (uint32_t *)FLASH_PAGE_START;
        }
    }
    else
    {
        // Read the last saved value
        ret = nrf_fstorage_read(&fstorage, (uint32_t)last_addr, &gs_rgb_data, FLASH_WORD_SIZE);
        APP_ERROR_CHECK(ret);
        
        // Set the current address to the next cell
        flash_context.current_address = last_addr + 1;
        
        // Check if there is space for the next write
        if ((uint32_t)flash_context.current_address >= FLASH_PAGE_END)
        {
            flash_context.erase_needed = true;
        }
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

/**
 * @brief Update the RGB state (on/off)
 * @param new_state The new state value
 */
void flash_storage_update_state(uint8_t new_state)
{
    int rc;
    
    // Get the current state
    uint8_t current_state = GET_RGB_STATE(gs_rgb_data);
    
    // Check if there are changes
    if (current_state == new_state)
    {
        NRF_LOG_INFO("FLASH STORAGE: No changes in RGB state, skipping flash update.");
        return;
    }
    
    // Current RGB values
    uint8_t current_r = GET_RED_VALUE(gs_rgb_data);
    uint8_t current_g = GET_GREEN_VALUE(gs_rgb_data);
    uint8_t current_b = GET_BLUE_VALUE(gs_rgb_data);
    
    // Update data in memory
    gs_rgb_data = SET_RGB_VALUES(gs_rgb_data, new_state, current_r, current_g, current_b);
    
    // If you need to erase the page or we have reached the end of the page
    if (flash_context.erase_needed || 
        (uint32_t)flash_context.current_address >= FLASH_PAGE_END)
    {
        NRF_LOG_INFO("FLASH STORAGE: Need to erase page before writing");
        rc = nrf_fstorage_erase(&fstorage, FLASH_PAGE_START, 1, NULL);
        APP_ERROR_CHECK(rc);
        
        flash_context.erase_needed = false;
        flash_context.current_address = (uint32_t *)FLASH_PAGE_START;
    }
    
    // Write new data
    NRF_LOG_INFO("FLASH STORAGE: Writing RGB state 0x%x to address 0x%x", 
                 new_state, (uint32_t)flash_context.current_address);
                 
    rc = nrf_fstorage_write(&fstorage, 
                           (uint32_t)flash_context.current_address, 
                           &gs_rgb_data, 
                           FLASH_WORD_SIZE, 
                           NULL);
    APP_ERROR_CHECK(rc);
    
    // Increase the address for the next write
    flash_context.current_address += 1;
}

/**
 * @brief Update the RGB color values
 * @param r The new red value
 * @param g The new green value
 * @param b The new blue value
 */
void flash_storage_update_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    int rc;
    
    // Get the current values
    uint8_t current_state = GET_RGB_STATE(gs_rgb_data);
    uint8_t current_r = GET_RED_VALUE(gs_rgb_data);
    uint8_t current_g = GET_GREEN_VALUE(gs_rgb_data);
    uint8_t current_b = GET_BLUE_VALUE(gs_rgb_data);
    
    // Check if there are changes
    if (current_r == r && current_g == g && current_b == b)
    {
        NRF_LOG_INFO("FLASH STORAGE: No changes in RGB values, skipping flash update.");
        return;
    }
    
    // Update data in memory
    gs_rgb_data = SET_RGB_VALUES(gs_rgb_data, current_state, r, g, b);
    
    // If you need to erase the page or we have reached the end of the page
    if (flash_context.erase_needed || 
        (uint32_t)flash_context.current_address >= FLASH_PAGE_END)
    {
        NRF_LOG_INFO("FLASH STORAGE: Need to erase page before writing");
        rc = nrf_fstorage_erase(&fstorage, FLASH_PAGE_START, 1, NULL);
        APP_ERROR_CHECK(rc);
        
        flash_context.erase_needed = false;
        flash_context.current_address = (uint32_t *)FLASH_PAGE_START;
    }
    
    // Write new data
    NRF_LOG_INFO("FLASH STORAGE: Writing RGB values (%u,%u,%u) to address 0x%x", 
                 r, g, b, (uint32_t)flash_context.current_address);
                 
    rc = nrf_fstorage_write(&fstorage, 
                           (uint32_t)flash_context.current_address, 
                           &gs_rgb_data, 
                           FLASH_WORD_SIZE, 
                           NULL);
    APP_ERROR_CHECK(rc);
    
    // Increase the address for the next write
    flash_context.current_address += 1;
}