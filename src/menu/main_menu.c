/**
 * @file    main_menu.c
 * @author  Cypherock X1 Team
 * @brief   Populate and handle main menu options
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
#include "main_menu.h"

#include "constant_texts.h"
#include "core_error_priv.h"
#include "create_wallet_menu.h"
#include "host_interface.h"
#include "manager/get_device_info.pb.h"
#include "menu_priv.h"
#include "settings_menu.h"
#include "status_api.h"
#include "wallet_list.h"
#include "wallet_menu.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
/* Main menu contains the wallet list and the options on the main menu. In case
 * the wallet count is equal to MAX_WALLETS_ALLOWED, then the options will be
 * less as MAIN_MENU_CREATE_NEW_WALLET option will be omitted. */
#define MAIN_MENU_MAX_OPTIONS                                                  \
  (MAX_WALLETS_ALLOWED + NUMBER_OF_OPTIONS_MAIN_MENU)

#define MAIN_MENU_INVALID_WALLET_SELECTION (MAX_WALLETS_ALLOWED + 1)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  bool update_required;
  uint8_t wallet_count;
  uint8_t wallet_selected;
} main_menu_ctx_t;

typedef enum {
  MAIN_MENU_OLD_WALLET,
  MAIN_MENU_CREATE_WALLET,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_NONE,
} main_menu_options_e;
/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Get the boolean flag which depicts whether rendering the main menu is
 * required or not
 *
 * @return true If the update is required
 * @return false If the update is NOT required
 */
static bool main_menu_get_update_req(void);

/**
 * @brief This function determines which menu option was selected from the main
 * menu. This selection cannot be static logic as the number of wallets/ number
 * of options on the main menu are dynamic.
 *
 * @param menu_selectn_idx The selection index from the main menu
 * @return main_menu_options_e The option of type main_menu_options_e detected
 * by the lookup function
 */
static main_menu_options_e main_menu_lookup(uint16_t menu_selectn_idx);

/**
 * @brief This function resets internal values of the main menu module
 *
 */
static void main_menu_reset_context(void);

/**
 * @brief The function retrieves Flash_Wallet pointer based on a selection from
 * main menu wallet list.
 *
 * @param selection The selection parameter represents the user's selection from
 * the main menu wallet list. It should be a value between 1 and 4 (inclusive).
 * @param flash_wallet A double pointer to a Flash_Wallet object. This parameter
 * is used to store the address of the Flash_Wallet object retrieved from the
 * list selection.
 */

static void get_flash_wallet_from_list_selection(uint8_t selection,
                                                 Flash_Wallet **flash_wallet);

/**
 * @brief This p0 event callback function handles clearing p0 events occured
 * while engine is waiting for other events.
 *
 * @details After main menu initalization, we don't expect p0 events as no
 * operation or flow has been started yet.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param p0_evt The p0 event object which triggered the callback
 * @param data_ptr Currently unused pointer set by the engine
 */
static void ignore_p0_handler(engine_ctx_t *ctx,
                              p0_evt_t p0_evt,
                              const void *data_ptr);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static main_menu_ctx_t main_menu_ctx = {
    .update_required = true,
    .wallet_count = 0,
    .wallet_selected = MAIN_MENU_INVALID_WALLET_SELECTION};

static const flow_step_t main_menu_flow = {.step_init_cb = main_menu_initialize,
                                           .p0_cb = ignore_p0_handler,
                                           .ui_cb = main_menu_handler,
                                           .usb_cb = main_menu_host_interface,
                                           .nfc_cb = NULL,
                                           .evt_cfg_ptr = &main_menu_evt_config,
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

static void get_flash_wallet_from_list_selection(uint8_t selection,
                                                 Flash_Wallet **flash_wallet) {
  ASSERT(0 != selection && 4 >= selection && NULL != flash_wallet);

  const char *wallet_list[MAIN_MENU_MAX_OPTIONS] = {0};
  get_wallet_list(&wallet_list[0]);

  ASSERT(SUCCESS ==
         get_flash_wallet_by_name(wallet_list[selection - 1], flash_wallet));
}

static bool main_menu_get_update_req(void) {
  return main_menu_ctx.update_required;
}

static main_menu_options_e main_menu_lookup(uint16_t menu_selectn_idx) {
  main_menu_options_e menu_selected = MAIN_MENU_NONE;

  /* Check if the selected index is greater than the wallet count */
  if ((0 == main_menu_ctx.wallet_count) ||
      (main_menu_ctx.wallet_count < menu_selectn_idx)) {
    /* If there are already MAX_WALLETS_ALLOWED on the device, then
     * MAIN_MENU_CREATE_WALLET is not a valid option */
    if (MAX_WALLETS_ALLOWED == main_menu_ctx.wallet_count) {
      menu_selected = MAIN_MENU_SETTINGS;
    } else {
      /* Normalize the menu selection index by subtracting the wallet count */
      menu_selectn_idx = menu_selectn_idx - main_menu_ctx.wallet_count;

      /* Order of the main menu: WALLET-x -> MAIN_MENU_CREATE_WALLET ->
       * MAIN_MENU_SETTINGS */
      if (1 == menu_selectn_idx) {
        menu_selected = MAIN_MENU_CREATE_WALLET;
      } else if (2 == menu_selectn_idx) {
        menu_selected = MAIN_MENU_SETTINGS;
      }
    }
  } else {
    menu_selected = MAIN_MENU_OLD_WALLET;
    main_menu_ctx.wallet_selected = menu_selectn_idx - 1;
  }

  return menu_selected;
}

static void main_menu_reset_context(void) {
  /* Reset wallet count to 0 and set update required to true */
  main_menu_ctx.update_required = true;
  main_menu_ctx.wallet_count = 0;
  main_menu_ctx.wallet_selected = MAIN_MENU_INVALID_WALLET_SELECTION;
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void main_menu_initialize(engine_ctx_t *ctx, const void *data_ptr) {
  handle_core_errors();

  /* First check if we even require to update the content on the main menu. This
   * check is mainly to handle cases where-in a background USB event was served
   * by the device, in which case this function will be called by the callback
   * but an update actually not required */
  if (false == main_menu_get_update_req()) {
    return;
  }

  /* Set core_status to CORE_DEVICE_IDLE_STATE_IDLE as we are entering back to
   * the main menu */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_IDLE);

  /* Reset flow status back to zero */
  set_core_flow_status(0);
  set_app_flow_status(0);

  /* Clear the context of the options rendered on the main menu */
  main_menu_reset_context();

  /* Create an array of pointers holding the string to display on the menu. */
  const char *menu_option_ptr_array[MAIN_MENU_MAX_OPTIONS] = {0};

  uint8_t menu_idx = 0;

  /* First, iterate through all the wallets and set the pointer to wallet name
   * for the main menu (if it exists) */
  menu_idx += get_wallet_list(&menu_option_ptr_array[0]);

  /* Set the wallet count in the context */
  main_menu_ctx.wallet_count = menu_idx;

  /* MAIN_MENU_CREATE_NEW_WALLET is only required if the total number of wallets
   * do not exceed MAX_WALLETS_ALLOWED */
  if (MAX_WALLETS_ALLOWED != menu_idx) {
    menu_option_ptr_array[menu_idx] =
        ui_text_options_main_menu[MAIN_MENU_CREATE_WALLET_INDEX];
    menu_idx += 1;
  }

  /* MAIN_MENU_SETTINGS will be always required */
  menu_option_ptr_array[menu_idx] =
      ui_text_options_main_menu[MAIN_MENU_SETTINGS_INDEX];
  menu_idx += 1;

  /* Initialize the main menu screen here, it will not be rendered on the screen
   * unless get_event() is called by the caller. */
  menu_init(menu_option_ptr_array, menu_idx, ui_text_heading_main_menu, false);

  /* Since we have just updated the main menu, we do not need to update it
   * again. */
  main_menu_set_update_req(false);
  return;
}

void main_menu_handler(engine_ctx_t *ctx,
                       ui_event_t ui_event,
                       const void *data_ptr) {
  main_menu_options_e menu_selected = MAIN_MENU_NONE;

  /* Only consider if ui_event is of type `UI_EVENT_LIST_CHOICE` */
  if (UI_EVENT_LIST_CHOICE == ui_event.event_type) {
    menu_selected = main_menu_lookup(ui_event.list_selection);

    /* There is a new screen going to be rendered other than the main menu,
     * therefore set update required to true */
    main_menu_set_update_req(true);

    /* Set core_status to CORE_DEVICE_IDLE_STATE_DEVICE as we are probably
     * entering a device initiated flow */
    core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_DEVICE);
  }

  switch (menu_selected) {
    case MAIN_MENU_OLD_WALLET: {
      Flash_Wallet *flash_wallet = NULL;
      get_flash_wallet_from_list_selection(ui_event.list_selection,
                                           &flash_wallet);
      engine_add_next_flow_step(ctx, wallet_menu_get_step(flash_wallet));
      engine_goto_next_flow_step(ctx);
      flash_wallet = NULL;
      break;
    }
    case MAIN_MENU_CREATE_WALLET: {
      engine_add_next_flow_step(ctx, create_wallet_menu_get_step());
      engine_goto_next_flow_step(ctx);
      break;
    }
    case MAIN_MENU_SETTINGS: {
      engine_add_next_flow_step(ctx, settings_menu_get_step());
      engine_goto_next_flow_step(ctx);
      break;
    }
    case MAIN_MENU_NONE:
    default: {
      /* MAIN_MENU_NONE is not expected to be the outcome of a UI event on the
       * main menu, therefore, we don't do anything if this case arise.
       * Therefore fall through of case is intentional. */
      break;
    }
  }

  /* Device is unauthenticated (this can happen if auth failed when triggered by
   * cySync settings) or onboarding incomplete (this is unlikely but keep for
   * completeness), reset the flow as the core will now need to render the
   * appropriate app (onboarding app or restricted app) */
  if (MANAGER_ONBOARDING_STEP_COMPLETE != get_onboarding_step() ||
      DEVICE_NOT_AUTHENTICATED == get_auth_state()) {
    engine_reset_flow(ctx);
  }

  return;
}

void main_menu_set_update_req(bool update_required) {
  main_menu_ctx.update_required = update_required;
  return;
}

const flow_step_t *main_menu_get_step(void) {
  return &main_menu_flow;
}
