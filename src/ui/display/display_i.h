/**
 * @brief Display functionality.
 * @ingroup display
 *
 * This defines the display/screen functionality in a generic way. The
 * `display_xxx` files contain implementations that are specific to a
 * particular display/screen device.
 *
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _DISPLAY_I_H_
#define _DISPLAY_I_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "display.h"

/**
 * @brief Test if there are screen contexts available on the stack.
 *
 * @return true If screen contexts are available.
 * @return false If no contexts are available.
 */
bool _has_scr_context();

/**
 * @brief Peek the top screen context.
 *
 * @return screen_ctx_t* The top context or NULL if none exist.
 */
screen_ctx_t* _peek_scr_context();

/**
 * @brief Pop the top screen context from the stack.
 *
 * @return screen_ctx_t* The top context or NULL if none exist.
 */
screen_ctx_t* _pop_scr_context();

/**
 * @brief Push a screen context on the stack.
 *
 * @param sc The screen context to push.
 * @return true If the context could be pushed.
 * @return false If the stack is full.
 */
bool _push_scr_context(screen_ctx_t* sc);

#ifdef __cplusplus
}
#endif
#endif // _DISPLAY_I_H_

