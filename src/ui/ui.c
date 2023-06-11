/**
 * User Interface - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "system_defs.h"
#include "ui.h"

#include "cmt.h"
#include "core1_main.h"
#include "display.h"
#include "board.h"
#include "multicore.h"
#include "util.h"
#include "ui_disp.h"

#include "hardware/rtc.h"

#include <stdlib.h>
#include <string.h>

#define _UI_STATUS_PULSE_PERIOD 7001

// Internal, non message handler, function declarations

// Message handler functions...
static void _handle_be_initialized(cmt_msg_t* msg);
static void _handle_window_output(cmt_msg_t* msg);

// Idle functions...
static void _ui_idle_function_1();

static cmt_msg_t _msg_ui_initialized;

static const msg_handler_entry_t _be_initialized_handler_entry = { MSG_BE_INITIALIZED, _handle_be_initialized };
static const msg_handler_entry_t _force_to_code_window_entry = { MSG_DISPLAY_MESSAGE, _handle_window_output };

/**
 * @brief List of handler entries.
 * @ingroup ui
 *
 * For performance, put these in the order that we expect to receive the most (most -> least).
 *
 */
static const msg_handler_entry_t* _handler_entries[] = {
    &_force_to_code_window_entry,
    &_be_initialized_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _ui_idle_functions[] = {
    (idle_fn)_ui_idle_function_1,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t ui_msg_loop_cntx = {
    UI_CORE_NUM, // UI runs on Core 1
    _handler_entries,
    _ui_idle_functions,
};

// ============================================
// Idle functions
// ============================================

static void _ui_idle_function_1() {
    // Something to do when there are no messages to process.
    //uint32_t now = now_ms();
}


// ============================================
// Message handler functions
// ============================================

static void _handle_be_initialized(cmt_msg_t* msg) {
    // The Backend has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
    //
}

/**
 * @brief Handles MSG_CODE_TEXT and MSG_DISPLAY_MESSAGE by
 *        writing text into the code section of the display and terminal.
 * @ingroup ui
 *
 * In both cases, the data contains a string to display in the scrolling (code)
 * section of the UI. In the case of MSG_CODE_TEXT it is one or more spaces and
 * then a character. In the case of MSG_DISPLAY_MESSAGE it is a message that
 * the backend or an interrupt handler (non-UI) process wants to be displayed
 * (for example, status, warning, etc).
 *
 * @param msg The data contains a string that needs to be freed once handled.
 */
static void _handle_window_output(cmt_msg_t* msg) {
    char* str = msg->data.str;
    disp_prints(str, Paint);
    free(str);
}


// ============================================
// Internal functions
// ============================================

void _ui_gpio_irq_handler(uint gpio, uint32_t events) {
    switch (gpio) {
        case IRQ_rotary_SW:
            //re_pbsw_irq_handler(gpio, events);
            break;
        case IRQ_rotary_TURN:
            //re_turn_irq_handler(gpio, events);
            break;
    }
}


// ============================================
// Public functions
// ============================================

void start_ui(void) {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;
    start_core1(); // The Core-1 main starts the UI
}

void ui_module_init() {
    ui_disp_build();
    // Let the Backend know that we are initialized
    _msg_ui_initialized.id = MSG_UI_INITIALIZED;
    postBEMsgBlocking(&_msg_ui_initialized);
}
