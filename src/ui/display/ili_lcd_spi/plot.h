/**
 * @brief Plot Display functionality.
 * @ingroup display
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _PLOT_H_
#define _PLOT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "display.h"

typedef struct _trace_ctx_ {
    screen_ctx_t* scr_ctx;
    uint16_t gfxline;        // Graphics line within the scroll area
    uint16_t scroll_start;   // Screen line that scroll window starts
    bool scroll_needed;
} trace_ctx_t;

extern void plot_append_tracepoint(trace_ctx_t* plot_ctx, uint16_t v, rgb16_t rgb);

extern void plot_close(trace_ctx_t* plot_ctx);

extern trace_ctx_t* plot_new();


#ifdef __cplusplus
}
#endif
#endif // _PLOT_H_
