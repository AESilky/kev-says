/**
 * Board Initialization and General Functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * This sets up the Pico for use for KevSays.
 * It:
 * 1. Configures the GPIO Pins for the proper IN/OUT, pull-ups, etc.
 * 2. Calls the init routines for Config, UI (Display, Touch, rotary)
 *
 * It provides some utility methods to:
 * 1. Turn the On-Board LED ON/OFF
 * 2. Flash the On-Board LED a number of times
 * 3. Turn the buzzer ON/OFF
 * 4. Beep the buzzer a number of times
 *
 */
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "pico/bootrom.h"

#include "system_defs.h"

#include "board.h"
#include "debug.h"
#include "display.h"
#include "multicore.h"
#include "util.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
static uint8_t _options_value = 0;

// Internal function declarations

/**
 * @brief Initialize the board
 *
 * This sets up the GPIO for the proper direction (IN/OUT), pull-ups, etc.
 * This calls the init for each of the devices/subsystems.
 * If all is okay, it returns 0, else non-zero.
 *
 * Although each subsystem could (some might argue should) configure thier own Pico
 * pins, having everything here makes the overall system easier to understand
 * and helps assure that there are no conflicts.
*/
int board_init() {

    stdio_init_all();

    sleep_ms(50); // Ok to `sleep` as msg system not started

    // LED initialization
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // SPI 0 initialization for the touch and SD card. Use SPI at 8MHz.
    spi_init(SPI_TSD_DEVICE, 8000 * 1000);
    gpio_set_function(SPI_TSD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TSD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TSD_MISO, GPIO_FUNC_SPI);
    // SPI 1 initialization for the display. Use SPI at 18MHz.
    spi_init(SPI_DISPLAY_DEVICE, 18000 * 1000);
    gpio_set_function(SPI_DISPLAY_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISPLAY_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISPLAY_MISO, GPIO_FUNC_SPI);
    // Chip selects for the SPI paripherals
    gpio_set_function(SPI_CS_DISPLAY,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_DC_DISPLAY,   GPIO_FUNC_SIO);  // Data/Command
    gpio_set_function(SPI_CS_SDCARD,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_CS_TOUCH,   GPIO_FUNC_SIO);
    // Chip selects are active-low, so we'll initialize them to a driven-high state
    gpio_set_dir(SPI_CS_DISPLAY, GPIO_OUT);
    gpio_set_dir(SPI_DC_DISPLAY, GPIO_OUT);
    gpio_set_dir(SPI_CS_SDCARD, GPIO_OUT);
    gpio_set_dir(SPI_CS_TOUCH, GPIO_OUT);
    // Signal drive strengths
    gpio_set_drive_strength(SPI_TSD_SCK, GPIO_DRIVE_STRENGTH_4MA);      // Multiple devices connected
    gpio_set_drive_strength(SPI_TSD_MOSI, GPIO_DRIVE_STRENGTH_4MA);     // Multiple devices connected
    gpio_set_drive_strength(SPI_DISPLAY_SCK, GPIO_DRIVE_STRENGTH_2MA);  // SPI Display is a single device
    gpio_set_drive_strength(SPI_DISPLAY_MOSI, GPIO_DRIVE_STRENGTH_2MA); // SPI Display is a single device
    gpio_set_drive_strength(SPI_CS_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // CS goes to a single device
    gpio_set_drive_strength(SPI_DC_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // DC goes to a single device
    gpio_set_drive_strength(SPI_CS_SDCARD, GPIO_DRIVE_STRENGTH_2MA);    // CS goes to a single device
    gpio_set_drive_strength(SPI_CS_TOUCH, GPIO_DRIVE_STRENGTH_2MA);     // CS goes to a single device
    // Initial output state
    gpio_put(SPI_CS_DISPLAY, SPI_CS_DISABLE);
    gpio_put(SPI_DC_DISPLAY, DISPLAY_DC_DATA);
    gpio_put(SPI_CS_SDCARD, SPI_CS_DISABLE);
    gpio_put(SPI_CS_TOUCH, SPI_CS_DISABLE);

    // NOT USING I2C AT THIS TIME.
    //
    // I2C Initialisation.
    // i2c_init(I2C_PORT, 400*1000);
    // // I2C is "open drain", pull ups to keep signal high when no data is being sent
    // gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);

    // GPIO Outputs (other than chip-selects)
    gpio_set_function(DISPLAY_RESET_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_RESET_OUT, GPIO_OUT);
    gpio_set_drive_strength(DISPLAY_RESET_OUT, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_ON);           // Hold reset on until rest of board is initialized
    gpio_set_function(DISPLAY_BACKLIGHT_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_BACKLIGHT_OUT, GPIO_OUT);
    gpio_set_drive_strength(DISPLAY_BACKLIGHT_OUT, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_OFF);     // No backlight until the display is initialized
    gpio_set_function(TONE_DRIVE,   GPIO_FUNC_SIO);
    gpio_set_dir(TONE_DRIVE, GPIO_OUT);
    gpio_set_drive_strength(TONE_DRIVE, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(TONE_DRIVE, TONE_OFF);

    // GPIO Inputs
    //    Options Switch
    gpio_init(OPTIONS_1_IN);
    gpio_set_dir(OPTIONS_1_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_1_IN);
    gpio_init(OPTIONS_2_IN);
    gpio_set_dir(OPTIONS_2_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_2_IN);
    gpio_init(OPTIONS_3_IN);
    gpio_set_dir(OPTIONS_3_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_3_IN);
    //    rotary Encoder A & B
    gpio_init(ROTARY_A_IN);
    gpio_set_dir(ROTARY_A_IN, GPIO_IN);
    gpio_pull_up(ROTARY_A_IN);
    gpio_init(ROTARY_B_IN);
    gpio_set_dir(ROTARY_B_IN, GPIO_IN);
    gpio_pull_up(ROTARY_B_IN);
    //    rotary Ecoder Push-button Switch
    gpio_init(ROTARY_PB_SW_IN);
    gpio_set_dir(ROTARY_PB_SW_IN, GPIO_IN);
    gpio_pull_up(ROTARY_PB_SW_IN);


    // Check the rotary switch to see if it's pressed.
    //  If yes, set 'debug_enabled'
    if (gpio_get(ROTARY_PB_SW_IN) == ROTARY_PB_SW_PUSHED) {
        debug_set(true);
    }
    // Read and cache the option switch value
    options_read();

    // Initialize hardware AD converter, enable onboard temperature sensor and
    //  select its channel.
    adc_init();
    // adc_set_temp_sensor_enabled(true);
    // adc_select_input(4); // Inputs 0-3 are GPIO pins, 4 is the built-in temp sensor
    adc_gpio_init(ADC_GP);
    adc_select_input(ADC_NUM);

    // Initialize the display
    display_reset_on(false);
    sleep_ms(100); // Ok to `sleep` as msg system not started
    disp_module_init();
    display_backlight_on(true);

    // Initialize the multicore subsystem
    multicore_module_init();

    return(true);
}

void boot_to_bootsel() {
    reset_usb_boot(0, 0);
}

static void _tone_sound_pattern_cont(void *user_data) {
    tone_on(false);
}
void tone_sound_pattern(int ms) {
    tone_on(true);
    if (!cmt_message_loop_0_running()) {
        sleep_ms(ms);
        _tone_sound_pattern_cont(NULL);
    }
    else {
        cmt_sleep_ms(ms, _tone_sound_pattern_cont, NULL);
    }
}

void tone_on(bool on) {
    gpio_put(TONE_DRIVE, (on ? TONE_ON : TONE_OFF));
}

void _tone_on_off_cont(void* data) {
    int32_t *pattern = (int32_t*)data;
    tone_on_off(pattern);
}
void tone_on_off(const int32_t *pattern) {
    while (*pattern) {
        tone_sound_pattern(*pattern++);
        int off_time = *pattern++;
        if (off_time == 0) {
            return;
        }
        if (!cmt_message_loop_0_running()) {
            sleep_ms(off_time);
        }
        else {
            cmt_sleep_ms(off_time, _tone_on_off_cont, (void*)pattern);
        }
    }
}

void display_backlight_on(bool on) {
    if (on) {
        gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_ON);
    }
    else {
        gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_OFF);
    }
}

void display_reset_on(bool on) {
    if (on) {
        gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_ON);
    }
    else {
        gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_OFF);
    }
}

static void _led_flash_cont(void* user_data) {
    led_on(false);
}
void led_flash(int ms) {
    led_on(true);
    if (!cmt_message_loop_0_running()) {
        sleep_ms(ms);
        _led_flash_cont(NULL);
    }
    else {
        cmt_sleep_ms(ms, _led_flash_cont, NULL);
    }
}

void led_on(bool on) {
    gpio_put(LED_PIN, on);
}

void _led_on_off_cont(void* user_data) {
    int32_t* pattern = (int32_t*)user_data;
    led_on_off(pattern);
}
void led_on_off(const int32_t *pattern) {
    while (*pattern) {
        led_flash(*pattern++);
        int off_time = *pattern++;
        if (off_time == 0) {
            return;
        }
        if (!cmt_message_loop_0_running()) {
            sleep_ms(off_time);
        }
        else {
            cmt_sleep_ms(off_time, _led_on_off_cont, (void*)pattern);
        }
    }
}

uint32_t now_ms() {
    return (us_to_ms(time_us_64()));
}

uint64_t now_us() {
    return (time_us_64());
}

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float onboard_temp_c() {

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return (tempC);
}

float onboard_temp_f() {
    return (onboard_temp_c() * 9 / 5 + 32);
}

uint8_t options_read(void) {
    uint8_t opt_value = 0x00;
    uint8_t opt_bit = gpio_get(OPTIONS_3_IN);
    opt_value |= opt_bit;
    opt_value <<= 1;
    opt_bit = gpio_get(OPTIONS_2_IN);
    opt_value |= opt_bit;
    opt_value <<= 1;
    opt_bit = gpio_get(OPTIONS_1_IN);
    opt_value |= opt_bit;
    opt_value ^= 0x07;  // Invert the final value (the switches are tied to GND)
    _options_value = opt_value;

    return (opt_value);
}

bool option_value(uint opt) {
    if (_options_value & opt) {
        return true;
    }
    return false;
}

void debug_printf(const char* format, ...) {
    if (debug_enabled()) {
        char buf[512];
        int index = 0;
        index += snprintf(&buf[index], sizeof(buf) - index, " DEBUG: ");
        va_list xArgs;
        va_start(xArgs, format);
        index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
        printf(buf);
        va_end(xArgs);
    }
}

void error_printf(const char* format, ...) {
    char buf[512];
    int index = 0;
    index += snprintf(&buf[index], sizeof(buf) - index, "\033[91m ERROR: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf("%s\033[0m", buf);
}

void info_printf(const char* format, ...) {
    char buf[512];
    int index = 0;
    index += snprintf(&buf[index], sizeof(buf) - index, " INFO: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf(buf);
}

void warn_printf(const char* format, ...) {
    char buf[512];
    int index = 0;
    index += snprintf(&buf[index], sizeof(buf) - index, " WARN: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf(buf);
}

