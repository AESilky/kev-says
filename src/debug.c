/**
 * Debugging flags and utilities.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#include "debug.h"
#include "cmt.h"

volatile uint16_t debugging_flags = 0;
static bool _debug = false;


bool debug_enabled() {
    return _debug;
}

bool debug_set(bool on) {
    bool temp = _debug;
    _debug = on;
    if (on != temp && cmt_message_loops_running()) {
        cmt_msg_t msg = { MSG_DEBUG_CHANGED };
        msg.data.debug = _debug;
        postBothMsgNoWait(&msg);
    }
    return (temp != on);
}

