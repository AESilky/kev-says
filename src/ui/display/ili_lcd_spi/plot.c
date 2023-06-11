/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */

#include "plot.h"
#include "board.h"
#include "display.h"
#include "ili_lcd_spi.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include "string.h"

extern void plot_append_tracepoint(trace_ctx_t* trace_ctx, uint16_t v, rgb16_t rgb) {
    rgb16_t* sl_buf = ili_get_line_buf();
    screen_ctx_t* scr_ctx = trace_ctx->scr_ctx;
    uint16_t ss = trace_ctx->scroll_start;
    uint16_t line = trace_ctx->gfxline;
    uint16_t scr_w_limit = scr_ctx->screen_width - 1;
    uint16_t scr_h_limit = scr_ctx->screen_height - 1;
    if (v > scr_w_limit) {
        v = scr_w_limit;
    }
    memset(sl_buf, 0, (scr_w_limit + 1) * sizeof(rgb16_t));
    *(sl_buf + v) = rgb;
    if (trace_ctx->scroll_needed || line > scr_h_limit) {
        if (line > scr_h_limit) {
            line = 0;
        }
        trace_ctx->scroll_needed = true;
        ss++;
        if (ss > scr_h_limit) {
            ss = 0;
        }
        trace_ctx->scroll_start = ss;
        ili_scroll_set_start(ss);
    }
    ili_line_paint(line, sl_buf);
    trace_ctx->gfxline = line + 1;
}

void plot_close(trace_ctx_t* trace_ctx) {
    disp_screen_close();
    free(trace_ctx);
}

trace_ctx_t* plot_new() {
    trace_ctx_t *pctx = malloc(sizeof(trace_ctx_t));
    if (!pctx) {
        error_printf("PLOT - Could not create plot context.\n");
        panic("PLOT - Could not create plot context");
        return (NULL);
    }
    pctx->scr_ctx = disp_screen_new();
    pctx->gfxline = 0;
    pctx->scroll_start = 0;
    pctx->scroll_needed = false;

    return (pctx);
}

