/**
 * @file    card_basic.c
 * @author  Cypherock X1 Team
 * @brief   Card internal operations
 * Exports all card APIs that enable card initialization and error handling
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "apdu.h"
#include "app_error.h"
#include "buzzer.h"
#include "card_operations.h"
#include "events.h"
#include "nfc.h"
#include "nfc_events.h"
#include "ui_instruction.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define HANDLE_P0_EVENTS(p0_event)                                             \
  do {                                                                         \
    if (true == p0_event.inactivity_evt) {                                     \
      return CARD_OPERATION_P0_TIMEOUT_OCCURED;                                \
    } else if (true == p0_event.abort_evt) {                                   \
      return CARD_OPERATION_P0_ABORT_OCCURED;                                  \
    }                                                                          \
  } while (0)

#define NFC_SET_ERROR_MSG(card_data, msg)                                      \
  (card_data->error_message =                                                  \
       (card_data)->error_message ? (card_data)->error_message : (msg))
#define NFC_GET_ERROR_MSG(card_data)                                           \
  ((card_data)->error_message ? (card_data)->error_message : "(none)")

#define NFC_RETURN_SUCCESS(card_data)                                          \
  return card_data->error_type = CARD_OPERATION_SUCCESS
#define NFC_RETURN_ERROR_TYPE(card_data, error)                                \
  return card_data->error_type = error
#define NFC_RETURN_ERROR_WITH_MSG(card_data, error, msg)                       \
  NFC_SET_ERROR_MSG(card_data, msg);                                           \
  NFC_RETURN_ERROR_TYPE(card_data, error)
#define NFC_RETURN_ABORT_ERROR(card_data, msg)                                 \
  NFC_RETURN_ERROR_WITH_MSG(card_data, CARD_OPERATION_ABORT_OPERATION, msg)
#define NFC_RETURN_RETAP_ERROR(card_data, msg)                                 \
  NFC_RETURN_ERROR_WITH_MSG(                                                   \
      card_data, CARD_OPERATION_RETAP_BY_USER_REQUIRED, msg)
#define NFC_RETURN_P0_EVENT(card_data, evt)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Loads the session key onto the card if pairing is required.
 *
 * This function checks if pairing is required and loads the session key onto
 * card_data. If pairing is not required, the function returns without modifying
 * the card data structure.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the session key
 * loading process.
 */
static card_error_type_e load_session_key_if_pairing_required(
    card_operation_data_t *card_data);

/**
 * @brief Selects an applet and updates the tapped card information.
 *
 * @param nfc_data Pointer to the NFC connection data structure.
 */
static void select_applet_and_update_tapped_card(NFC_connection_data *nfc_data);

/**
 * @brief Handles the NFC errors encountered during a card operation.
 *
 * This function handles the NFC errors encountered during a card operation. The
 * function performs the following actions:
 * 1. Determines the type of error encountered based on the NFC data status.
 * 2. Returns the appropriate card error type so that the card operation can be
 * properly handled.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the NFC
 * interaction.
 */
static card_error_type_e handle_nfc_errors(card_operation_data_t *card_data);

/**
 * @brief Wait for card tap, then select card
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the NFC
 * interaction.
 */
card_error_type_e handle_wait_for_card_selection(
    card_operation_data_t *card_data);
/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static card_error_type_e load_session_key_if_pairing_required(
    card_operation_data_t *card_data) {
  if (false == card_data->nfc_data.pairing_required)
    NFC_RETURN_SUCCESS(card_data);

  card_data->nfc_data.keystore_index =
      is_paired(card_data->nfc_data.card_key_id);

  if (-1 == card_data->nfc_data.keystore_index) {
    NFC_RETURN_ERROR_TYPE(card_data, CARD_OPERATION_PAIRING_REQUIRED);
  }

  const uint8_t *pairing_key =
      get_keystore_pairing_key(card_data->nfc_data.keystore_index);
  init_session_keys(pairing_key, pairing_key + 32, NULL);
  NFC_RETURN_SUCCESS(card_data);
}

static void select_applet_and_update_tapped_card(
    NFC_connection_data *nfc_data) {
  uint8_t temp = 0;

  temp = nfc_data->acceptable_cards;
  nfc_data->status = nfc_select_applet(nfc_data->family_id,
                                       &nfc_data->acceptable_cards,
                                       NULL,
                                       nfc_data->card_key_id,
                                       &nfc_data->recovery_mode);

  /* The tapped_card information should be persistent, as it is used at later
   * stage in the flow For example, in the second half of card-verification,
   * the card tapped in first half becomes the acceptable card for that half.
   * It is the knowledge of previous round of card tapping, hence it's update
   * should happen only here or at the fresh start of a card tap flow. */
  nfc_data->tapped_card = (temp ^ nfc_data->acceptable_cards);
}

static card_error_type_e handle_nfc_errors(card_operation_data_t *card_data) {
  card_data->nfc_data.tapped_card = 0;

  if ((card_data->nfc_data.status == NFC_CARD_ABSENT) ||
      (nfc_diagnose_card_presence() != 0)) {
    instruction_scr_change_text(ui_text_card_removed_fast, true);
    if ((!--card_data->nfc_data.card_absent_retries)) {
      card_data->nfc_data.status = NFC_CARD_ABSENT;
      NFC_RETURN_RETAP_ERROR(card_data, ui_text_card_freq_discon_fault);
    }
  } else if (!(--card_data->nfc_data.retries)) {
    NFC_RETURN_ABORT_ERROR(card_data, ui_text_unknown_error_contact_support);
  } else if ((card_data->nfc_data.status & NFC_ERROR_BASE) == NFC_ERROR_BASE) {
    instruction_scr_change_text(ui_text_card_align_with_device_screen, true);
    nfc_deselect_card();
  }

  // probably comm failure; guess is NFC teardown; reconnect with applet
  // silently retry to connect; can't connect here, cards_state is important
  NFC_RETURN_SUCCESS(card_data);
}

card_error_type_e handle_wait_for_card_selection(
    card_operation_data_t *card_data) {
  evt_status_t evt_status = {0};

  while (1) {
    nfc_en_select_card_task();

    evt_status = get_events(EVENT_CONFIG_NFC, MAX_INACTIVITY_TIMEOUT);

    HANDLE_P0_EVENTS(evt_status.p0_event);

    if (true == evt_status.nfc_event.event_occured) {
      if (STM_SUCCESS == nfc_wait_for_card(DEFAULT_NFC_TG_INIT_TIME))
        NFC_RETURN_SUCCESS(card_data);
    }
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_initialize_applet(card_operation_data_t *card_data) {
  card_data->error_type = CARD_OPERATION_DEFAULT_INVALID;
  card_data->nfc_data.recovery_mode = 0;
  if (!card_data->nfc_data.card_absent_retries) {
    card_data->nfc_data.card_absent_retries = 100;
  }

  while (1) {
    if (CARD_OPERATION_SUCCESS != handle_wait_for_card_selection(card_data)) {
      return card_data->error_type;
    }

    instruction_scr_change_text(ui_text_card_detected, true);

    select_applet_and_update_tapped_card(&(card_data->nfc_data));

    /* Card is in recovery mode. This is a critical situation. Instruct user to
     * safely recover/export wallets to different set of cards in limited
     * attempts. NOTE: Errors such as invalid card & invalid family id have
     * higher priority than this.
     */
    if (card_data->nfc_data.recovery_mode == 1) {
      NFC_RETURN_ABORT_ERROR(card_data, ui_critical_card_health_migrate_data);
    }

    LOG_ERROR("Applet selection err (0x%04X)\n", card_data->nfc_data.status);
    switch (card_data->nfc_data.status) {
      case SW_NO_ERROR:
        return load_session_key_if_pairing_required(card_data);
        break;
      case SW_CONDITIONS_NOT_SATISFIED:
        NFC_RETURN_RETAP_ERROR(card_data, ui_text_wrong_card_sequence);
        break;
      case SW_FILE_INVALID:
        NFC_RETURN_RETAP_ERROR(card_data, ui_text_family_id_mismatch);
        break;
      case SW_FILE_NOT_FOUND:
        NFC_RETURN_ABORT_ERROR(card_data,
                               ui_text_corrupted_card_contact_support);
        break;
      case SW_INCOMPATIBLE_APPLET:
        NFC_RETURN_ABORT_ERROR(card_data, ui_text_incompatible_card_version);
        break;
      default:
        card_data->nfc_data.acceptable_cards |= card_data->nfc_data.tapped_card;
        if (CARD_OPERATION_SUCCESS != handle_nfc_errors(card_data)) {
          return card_data->error_type;
        }
        break;
    }
  }
}

card_error_type_e card_handle_errors(card_operation_data_t *card_data) {
  LOG_ERROR("nfc error occured (0x%04X)\n", card_data->nfc_data.status);
  switch (card_data->nfc_data.status) {
    case SW_NO_ERROR:
      NFC_RETURN_SUCCESS(card_data);
    case SW_SECURITY_CONDITIONS_NOT_SATISFIED:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_security_conditions_not_met);
      break;
    case SW_NOT_PAIRED:
      invalidate_keystore();
      NFC_RETURN_ERROR_TYPE(card_data, CARD_OPERATION_PAIRING_REQUIRED);
    case SW_CONDITIONS_NOT_SATISFIED:
      break;
    case SW_WRONG_DATA:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_invalid_apdu_length);
      break;
    case SW_FILE_FULL:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_is_full);
      break;
    case SW_RECORD_NOT_FOUND:
      card_data->nfc_data.active_cmd_type = WALLET_DOES_NOT_EXISTS_ON_CARD;
      NFC_RETURN_ABORT_ERROR(card_data,
                             ui_text_wallet_doesnt_exists_on_this_card);
      break;
    case SW_TRANSACTION_EXCEPTION:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_transaction_exception);
      break;
    case SW_NULL_POINTER_EXCEPTION:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_null_pointer_exception);
      break;
    case SW_OUT_OF_BOUNDARY:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_out_of_boundary_exception);
      break;
    case SW_INVALID_INS:
      NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_error_contact_support);
      break;
    case SW_INS_BLOCKED:
      NFC_RETURN_ABORT_ERROR(card_data, ui_critical_card_health_migrate_data);
      break;
    default:
      switch (card_data->nfc_data.status & 0xFF00) {
        case POW_SW_WALLET_LOCKED:
          NFC_RETURN_ERROR_TYPE(card_data, CARD_OPERATION_LOCKED_WALLET);
          break;

        case SW_CORRECT_LENGTH_00:
          NFC_RETURN_ERROR_TYPE(card_data,
                                CARD_OPERATION_INCORRECT_PIN_ENTERED);
          break;

        case SW_CRYPTO_EXCEPTION:
          NFC_RETURN_ABORT_ERROR(card_data, ui_text_card_crypto_exception);
          break;

        default:
          return handle_nfc_errors(card_data);
          break;
      }
  }

  // Shouldn't reach here
  NFC_RETURN_ERROR_TYPE(card_data, CARD_OPERATION_DEFAULT_INVALID);
}