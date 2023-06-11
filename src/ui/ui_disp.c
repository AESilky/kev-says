/**
 * User Interface - On the display, rotary, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"
#include "display.h"
#include "font.h"
#include "util.h"
#include "pico/printf.h"

#include <stdlib.h>
#include <string.h>

// TODO - Have these adjust based on the screen and font sizes

#define UI_DISP_TOP_FIXED_LINES 0
#define UI_DISP_BOTTOM_FIXED_LINES 0

void ui_disp_build(void) {
    disp_text_colors_set(C16_LT_GREEN, C16_BLACK);
    disp_clear(Paint);
    disp_scroll_area_define(UI_DISP_TOP_FIXED_LINES, UI_DISP_BOTTOM_FIXED_LINES);
}

