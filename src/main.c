/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_defs.h"
#include "board.h"
#include "display.h"
#include "ili_lcd_spi.h"
#include "plot.h"

#include <stdio.h>

#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"


#define ADC_VREF 3.3
#define ADC_RANGE (1 << 12)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

int main() {
    bi_decl(bi_program_description("KevSays - Kevin Says... to turn things on/off.")); // for picotool
    bi_decl(bi_1pin_with_name(ADC_GP, "ADC2 input pin"));

     board_init();

    uint adc_raw;
    trace_ctx_t* plot_ctx = plot_new();
    uint16_t width = plot_ctx->scr_ctx->screen_width;
    float f_adc_range = (float)ADC_RANGE;
    float f_adc_plot_factor = (f_adc_range - 1.0) / (float)width;
    while (1) {
        adc_raw = adc_read(); // raw voltage from ADC
        // Convert the raw value to a number between 0 and the screen width
        uint16_t v = (uint16_t)((float)adc_raw / f_adc_plot_factor);
        plot_append_tracepoint(plot_ctx, v, ILI_LT_MAGENTA);
    }

    return 0;
}
