/**
 * @file    ui_joystick_training.h
 * @author  Cypherock X1 Team
 * @brief   UI component to train user for joystick.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_JOYSTICK_TRAINING_H
#define UI_JOYSTICK_TRAINING_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "ui_common.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef enum joystick_actions {
  JS_ACTION_UP = LV_KEY_UP,
  JS_ACTION_DOWN = LV_KEY_DOWN,
  JS_ACTION_LEFT = LV_KEY_LEFT,
  JS_ACTION_RIGHT = LV_KEY_RIGHT,
  JS_ACTION_CENTER = LV_KEY_ENTER,
} joystick_actions_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Initialize and create message UI screen
 * @details The component translates various user actions into UI_EVENT_CONFIRM.
 * The application can provide which action (joystick_actions_t) to map to the
 * UI_EVENT_CONFIRM event.
 *
 * @param message message text
 * @param act indicates which action will raise an UI-event
 *
 * @see
 * @since v1.0.0
 */
void joystick_train_init(const char *message, joystick_actions_t act);

#endif
