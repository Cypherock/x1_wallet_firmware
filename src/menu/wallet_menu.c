/**
 * @file    wallet_menu.c
 * @author  Cypherock X1 Team
 * @brief   Populate and handle old wallet menu options
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
#include "wallet_menu.h"

#include "constant_texts.h"
#include "core_error_priv.h"
#include "delete_wallet_flow.h"
#include "menu_priv.h"
#include "sync_wallets_flow.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "verify_wallet_flow.h"
#include "view_seed_flow.h"
#include "wallet_list.h"
#include "wallet_unlock_flow.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  VIEW_SEED = 1,
  DELETE_WALLET,

} create_wallet_options_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This is the initializer callback for the wallet menu.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param data_ptr Data pointer here represents the copy of @ref Flash_Wallet
 * object of selected wallet
 */
static void wallet_menu_initialize(engine_ctx_t *ctx, const void *data_ptr);

/**
 * @brief This is the UI event handler for the wallet menu.
 * @details The function decodes the UI event and calls the wallet flow
 * based on selection. It deletes the wallet menu step if the back button
 * is pressed on the menu
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param ui_event The ui event object which triggered the callback
 * @param data_ptr Data pointer here represents the copy of @ref Flash_Wallet
 * object of selected wallet
 */
static void wallet_menu_handler(engine_ctx_t *ctx,
                                ui_event_t ui_event,
                                const void *data_ptr);

/**
 * @brief This p0 event callback function handles clearing p0 events occured
 * while engine is waiting for other events.
 *
 * @details After main menu initalization, we don't expect p0 events as no
 * operation or flow has been started yet.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param p0_evt The p0 event object which triggered the callback
 * @param data_ptr Data pointer here represents the copy of @ref Flash_Wallet
 * object of selected wallet
 */
static void ignore_p0_handler(engine_ctx_t *ctx,
                              p0_evt_t p0_evt,
                              const void *data_ptr);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const Flash_Wallet *wallet_ptr = NULL;

static const flow_step_t wallet_step = {.step_init_cb = wallet_menu_initialize,
                                        .p0_cb = ignore_p0_handler,
                                        .ui_cb = wallet_menu_handler,
                                        .usb_cb = NULL,
                                        .nfc_cb = NULL,
                                        .evt_cfg_ptr = &device_nav_evt_config,
                                        .flow_data_ptr = NULL};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void ignore_p0_handler(engine_ctx_t *ctx,
                              p0_evt_t p0_evt,
                              const void *data_ptr) {
  ignore_p0_event();
}

static void wallet_menu_initialize(engine_ctx_t *ctx, const void *data_ptr) {
  if (1 == wallet_ptr->is_wallet_locked) {
    ui_scrollable_page(NULL,
                       ui_text_wallet_lock_continue_to_unlock,
                       MENU_SCROLL_HORIZONTAL,
                       true);
  } else if (0x0f != wallet_ptr->cards_states) {
    ui_scrollable_page(NULL,
                       ui_text_wallet_partial_continue_to_delete,
                       MENU_SCROLL_HORIZONTAL,
                       true);
  } else {
    switch (wallet_ptr->state) {
      case VALID_WALLET:
        menu_init((const char **)ui_text_options_old_wallet,
                  NUMBER_OF_OPTIONS_OLD_WALLET,
                  (const char *)wallet_ptr->wallet_name,
                  true);
        return;
        break;

      case VALID_WALLET_WITHOUT_DEVICE_SHARE:
        ui_scrollable_page(
            NULL,
            ui_text_wallet_out_of_sync_continue_to_sync_with_x1cards,
            MENU_SCROLL_HORIZONTAL,
            true);
        break;

      case INVALID_WALLET:
        ui_scrollable_page(NULL,
                           ui_text_creation_failed_delete_wallet,
                           MENU_SCROLL_HORIZONTAL,
                           true);
        break;

      case UNVERIFIED_VALID_WALLET:
        ui_scrollable_page(NULL,
                           ui_text_wallet_not_verified_continue_to_verify,
                           MENU_SCROLL_HORIZONTAL,
                           true);
        break;

      default:
        break;
    }
  }
}

static void wallet_menu_handler(engine_ctx_t *ctx,
                                ui_event_t ui_event,
                                const void *data_ptr) {
  if (UI_EVENT_LIST_CHOICE == ui_event.event_type) {
    switch (ui_event.list_selection) {
      case VIEW_SEED: {
        view_seed_flow(wallet_ptr->wallet_id);
        break;
      }
      case DELETE_WALLET: {
        delete_wallet_flow(wallet_ptr);
        break;
      }
      default: {
        break;
      }
    }
  } else if (UI_EVENT_CONFIRM == ui_event.event_type) {
    if (1 == wallet_ptr->is_wallet_locked) {
      wallet_unlock_flow(wallet_ptr);
    } else if (0x0f != wallet_ptr->cards_states) {
      delete_wallet_flow(wallet_ptr);
    } else {
      switch (wallet_ptr->state) {
        case VALID_WALLET_WITHOUT_DEVICE_SHARE:
          (void)sync_wallets_flow(wallet_ptr->wallet_id);
          break;

        case INVALID_WALLET:
          delete_wallet_flow(wallet_ptr);
          break;

        case UNVERIFIED_VALID_WALLET:
          verify_wallet_flow(wallet_ptr);

          // If post verification, the wallet state was updated to INVALID,
          // proceed to delete that wallet
          if (INVALID_WALLET == wallet_ptr->state) {
            message_scr_init(ui_text_wallet_verification_failed_in_creation);
            if (0 != get_state_on_confirm_scr(0, 1, 2)) {
              break;
            }

            delete_wallet_flow(wallet_ptr);
          }
          break;

        default:
          break;
      }
    }
  } else {
    // UI_EVENT_LIST_REJECTION handled below already
  }

  /* Return to the previous menu irrespective if UI_EVENT_REJECTION was
   * detected, or a wallet flow was executed */
  engine_delete_current_flow_step(ctx);

  wallet_ptr = NULL;
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
const flow_step_t *wallet_menu_get_step(const Flash_Wallet *selected_wallet) {
  ASSERT(NULL != selected_wallet);

  wallet_ptr = selected_wallet;
  return &wallet_step;
}
