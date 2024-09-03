/**
 * @author  Cypherock X1 Team
 * @brief   Card encryption data implementation
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

#include "buzzer.h"
#include "card_fetch_data.h"
#include "card_fetch_wallet_list.h"
#include "card_internal.h"
#include "card_utils.h"
#include "nfc.h"
#include "pow_utilities.h"
#include "ui_delay.h"
#include "ui_instruction.h"

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

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
card_error_type_e card_get_wallet_list(
    card_operation_data_t card_data,
    const card_fetch_wallet_list_config_t *configuration,
    card_fetch_wallet_list_response_t *response) {
  if (NULL == configuration || NULL == response ||
      NULL == response->wallet_list) {
    return CARD_OPERATION_DEFAULT_INVALID;
  }
  if (CARD_OPERATION_SUCCESS == card_data.error_type) {
    card_data.nfc_data.status = nfc_list_all_wallet(response->wallet_list);

    if (card_data.nfc_data.status == SW_NO_ERROR ||
        card_data.nfc_data.status == SW_RECORD_NOT_FOUND) {
      if (card_data.nfc_data.status == SW_RECORD_NOT_FOUND ||
          configuration->operation.buzzer_on_success) {
        buzzer_start(BUZZER_DURATION);
      }
      if (!configuration->operation.skip_card_removal) {
        wait_for_card_removal();
      }
      // break;
    } else {
      card_handle_errors(&card_data);
    }
  }

  response->card_info.pairing_error = card_data.nfc_data.pairing_error;
  response->card_info.tapped_card = card_data.nfc_data.tapped_card;
  response->card_info.recovery_mode = card_data.nfc_data.recovery_mode;
  response->card_info.status = card_data.nfc_data.status;
  return card_data.error_type;
}
/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_fetch_encrypt_data(const uint8_t *wallet_id,
                                          secure_data_t *msgs,
                                          size_t msg_count) {
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;
  card_operation_data_t card_data = {0};

  char wallet_name[NAME_SIZE] = "";

  instruction_scr_init(ui_text_place_card_below, ui_text_tap_1_2_cards);

  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  uint8_t plain_data_buffer[PLAIN_DATA_BUFFER_SIZE];
  uint8_t encrypted_data_buffer[ENCRYPTED_DATA_BUFFER_SIZE];
  uint16_t encrypted_data_buffer_size = 0;
  uint16_t plain_data_buffer_size = 0;
  size_t index = 0;

  while (1) {
    card_data.nfc_data.acceptable_cards = ACCEPTABLE_CARDS_ALL;
#if USE_SIMULATOR == 0
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    result = card_initialize_applet(&card_data);
#endif
    // Fetch wallet list from card
    wallet_list_t wallets_in_card = {0};
    card_fetch_wallet_list_config_t configuration = {
        .operation = {.acceptable_cards = ACCEPTABLE_CARDS_ALL,
                      .skip_card_removal = true,
                      .expected_family_id = card_data.nfc_data.family_id,
                      .buzzer_on_success = false},
        .frontend = {.heading = ui_text_tap_1_2_cards,
                     .msg = ui_text_place_card_below}};

    card_fetch_wallet_list_response_t response = {
        .wallet_list = &wallets_in_card, .card_info = {0}};
#if USE_SIMULATOR == 0
    card_error_type_e status =
        card_get_wallet_list(card_data, &configuration, &response);
#endif
    // If the tapped card is not paired, it is a terminal case in the flow
    if (true == response.card_info.pairing_error) {
      return CARD_OPERATION_DEFAULT_INVALID;
    }

    // At this stage, either there is no core error message set, or it is set
    // but we want to overwrite the error message using user facing messages in
    // this flow
    uint32_t card_fault_status = 0;
    if (1 == response.card_info.recovery_mode) {
      card_fault_status = NFC_NULL_PTR_ERROR;
    } else if (CARD_OPERATION_SUCCESS != status) {
      card_fault_status = response.card_info.status;
    }

    for (uint8_t i = 0; i < wallets_in_card.count; i++) {
      if (memcmp(wallet_id, wallets_in_card.wallet[i].id, WALLET_ID_SIZE) ==
          0) {
        memcpy(wallet_name,
               (const char *)wallets_in_card.wallet[i].name,
               NAME_SIZE);
        break;
      }
    }
    if (0 == strlen(wallet_name)) {
      delay_scr_init(ui_text_wallet_doesnt_exists_on_this_card, DELAY_TIME);
      return CARD_OPERATION_DEFAULT_INVALID;
    }

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      for (int i = 0; i < msg_count; i++) {
        memzero(plain_data_buffer, PLAIN_DATA_BUFFER_SIZE);
        memzero(encrypted_data_buffer, ENCRYPTED_DATA_BUFFER_SIZE);

        index = 0;
        while (index < msgs[i].plain_data_size) {
          plain_data_buffer_size =
              (msgs[i].plain_data_size - index) < PLAIN_DATA_BUFFER_SIZE
                  ? (msgs[i].plain_data_size - index)
                  : PLAIN_DATA_BUFFER_SIZE;
          memcpy(plain_data_buffer,
                 msgs[i].plain_data + index,
                 plain_data_buffer_size);

#if USE_SIMULATOR == 0
          card_data.nfc_data.status =
              nfc_encrypt_data((const uint8_t *)wallet_name,
                               plain_data_buffer,
                               plain_data_buffer_size,
                               encrypted_data_buffer,
                               &encrypted_data_buffer_size);
#else
          card_data.nfc_data.status = SW_NO_ERROR;
          result = CARD_OPERATION_SUCCESS;
#endif
          if (card_data.nfc_data.status == SW_NO_ERROR) {
            msgs[i].encrypted_data[msgs[i].encrypted_data_size] =
                encrypted_data_buffer_size;
            msgs[i].encrypted_data_size += 1;

            memcpy(msgs[i].encrypted_data + msgs[i].encrypted_data_size,
                   encrypted_data_buffer,
                   encrypted_data_buffer_size);
            msgs[i].encrypted_data_size += encrypted_data_buffer_size;
          } else {
            card_handle_errors(&card_data);
          }
          index += plain_data_buffer_size;
        }
      }
    }

    if (card_data.nfc_data.status == SW_NO_ERROR) {
      buzzer_start(BUZZER_DURATION);
      break;
    }

    card_handle_errors(&card_data);

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
      const char *error_msg = card_data.error_message;
      if (CARD_OPERATION_SUCCESS == indicate_card_error(error_msg)) {
        // Re-render the instruction screen
        instruction_scr_init(ui_text_place_card_below, ui_text_tap_1_2_cards);
        continue;
      }
    }

    result = handle_wallet_errors(&card_data, &wallet);
    if (CARD_OPERATION_SUCCESS != result) {
      break;
    }

    // If control reached here, it is an unrecoverable error, so break
    result = card_data.error_type;
    break;
  }
#if USE_SIMULATOR == 0
  nfc_deselect_card();
#endif
  return result;
}
