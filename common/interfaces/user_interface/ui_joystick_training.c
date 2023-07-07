/**
 * @file    ui_joystick_training.c
 * @author  Cypherock X1 Team
 * @brief   UI component to train user for joystick.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "ui_joystick_training.h"

#include "ui_events_priv.h"
#ifdef DEV_BUILD
#include "dev_utils.h"
#endif

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Create lvgl object and setup callbacks for joystick training
 *
 */
static void joystick_train_create();

/**
 * @brief Clear internal variables
 *
 */
static void joystick_train_destructor();

/**
 * @brief Event handler callback for the joystick training screen
 *
 * @param obj The pointer to LVGL object
 * @param event LVGL event that triggered the callback
 */
static void joystick_event_handler(lv_obj_t *obj, const lv_event_t event);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const char *scr_message = NULL;
static lv_obj_t *lvgl_object = NULL;
static joystick_actions_e action = JOYSTICK_ACTION_CENTER;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void joystick_train_destructor() {
  scr_message = NULL;
  lvgl_object = NULL;
  return;
}

static void joystick_event_handler(lv_obj_t *obj, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY: {
      if (JOYSTICK_ACTION_CENTER != action &&
          lv_indev_get_key(ui_get_indev()) == action) {
        ui_set_confirm_event();
      }
      break;
    }
    case LV_EVENT_CLICKED: {
      if (JOYSTICK_ACTION_CENTER == action) {
        ui_set_confirm_event();
      }
      break;
    }
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      joystick_train_destructor();
      break;
    }
  }
}

static void joystick_train_create() {
  ASSERT(scr_message != NULL);

  lvgl_object = lv_label_create(lv_scr_act(), NULL);
  ui_paragraph(lvgl_object, scr_message, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lvgl_object, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_group_add_obj(ui_get_group(), lvgl_object);
  lv_obj_set_event_cb(lvgl_object, joystick_event_handler);
}
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void joystick_train_init(const char *message, joystick_actions_e act) {
  ASSERT(message != NULL);

  lv_obj_clean(lv_scr_act());
  action = act;
  scr_message = message;

#ifdef DEV_BUILD
  ekp_enqueue(act, DEFAULT_DELAY);
#endif
  joystick_train_create();
}
