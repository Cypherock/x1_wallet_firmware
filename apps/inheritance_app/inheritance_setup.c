/**
 * @file    inheritance_setup.c
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
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
#include "card_internal.h"
#include "card_utils.h"
#include "constant_texts.h"
#include "create_new_wallet_flow.h"
#include "inheritance_main.h"
#include "pow.h"
#include "pow_utilities.h"
#include "ui_common.h"
#include "ui_core_confirm.h"
#include "ui_input_text.h"
#include "ui_screens.h"
#include "ui_state_machine.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum { PIN_OK = 0, PIN_REJECT, TIMED_OUT = 18 } pin_state_e;
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
extern char *ALPHA_NUMERIC;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void convert_plaindata_to_msg(inheritance_plain_data_t *plain_data,
                                     secure_data_t *msgs,
                                     size_t msg_count) {
  for (size_t i = 0; i < msg_count; i++) {
    msgs[i].plain_data[0] = plain_data[i].is_private ? 1 : 0;
    msgs[i].plain_data_size += 1;

    memcpy(msgs[i].plain_data + 1,
           plain_data[i].message.bytes,
           plain_data[i].message.size);
    msgs[i].plain_data_size += plain_data[i].message.size;
  }
}

static card_error_type_e inheritance_verify_pin(const uint8_t *wallet_id,
                                                uint8_t *password_double_hash) {
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;
  card_operation_data_t card_data = {0};

  const char wallet_name[NAME_SIZE] = "";
#if USE_SIMULATOR == 0
  // Confirm NULL use for rejection_cb
  ASSERT(get_wallet_name_by_id(wallet_id, (uint8_t *)wallet_name, NULL));
#endif

  uint8_t wallet_index = 0xFF;
  ASSERT(SUCCESS == get_index_by_name(wallet_name, &wallet_index));
  instruction_scr_init(ui_text_place_card_below, ui_text_tap_1_2_cards);

  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;
  while (1) {
    card_data.nfc_data.acceptable_cards = ACCEPTABLE_CARDS_ALL;
#if USE_SIMULATOR == 0
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    result = card_initialize_applet(&card_data);
#endif

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status =
          nfc_verify_challenge((const uint8_t *)wallet_name,
                               get_proof_of_work_nonce(),
                               password_double_hash);

      if (card_data.nfc_data.status == SW_NO_ERROR ||
          card_data.nfc_data.status == SW_WARNING_STATE_UNCHANGED) {
        update_wallet_locked_flash(wallet_name, false);
        buzzer_start(BUZZER_DURATION);
        break;
      } else {
        card_handle_errors(&card_data);
        if (POW_SW_CHALLENGE_FAILED == card_data.nfc_data.status) {
          uint16_t bits = pow_count_set_bits(
              get_wallet_by_index(wallet_index)->challenge.target);
          LOG_CRITICAL("ex-pow-tg n: %d", bits);
          log_hex_array(
              "ex-pow-rn: ",
              get_wallet_by_index(wallet_index)->challenge.random_number,
              POW_RAND_NUMBER_SIZE);
        }
      }
    }

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
      const char *error_msg = card_data.error_message;
      if (CARD_OPERATION_SUCCESS == indicate_card_error(error_msg)) {
        // Re-render the instruction screen
        instruction_scr_init(ui_text_place_card_below, ui_text_tap_1_2_cards);
        continue;
      }
    }

    /**
     * @brief For errors which lead to challenge failure or incorrect pin, we
     * have to refetch the challenge which is performed subsequently in the same
     * card tap session by the caller from user's perspective, so only for the
     * conditon of `CARD_OPERATION_LOCKED_WALLET` we don't sound the buzzer as
     * the card tap session has not completed.
     */
    if (CARD_OPERATION_LOCKED_WALLET != card_data.error_type) {
      buzzer_start(BUZZER_DURATION);
    }
    // CONFIRM use of WALLET var
    memcpy(wallet.wallet_name, wallet_name, NAME_SIZE);
    result = handle_wallet_errors(&card_data, &wallet);
    if (CARD_OPERATION_SUCCESS != result) {
      break;
    }

    // If control reached here, it is an unrecoverable error, so break
    result = card_data.error_type;
    break;
  }

  nfc_deselect_card();
  return result;
}
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_setup(inheritance_query_t *query,
                       secure_data_t *msgs,
                       inheritance_result_t *response) {
  uint32_t msg_count = query->setup.plain_data_count;
  if (SESSION_MSG_MAX < msg_count) {
    // ADD error
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  convert_plaindata_to_msg(query->setup.plain_data, msgs, msg_count);

  if (msg_count > 0) {
    // verify msgs if_private
    if (!core_confirmation(ui_text_inheritance_verify_message,
                           inheritance_send_error)) {
      return;
    }

    for (int i = 0; i < query->setup.plain_data_count; i++) {
      // show priv msgs on device
      if (query->setup.plain_data[i].is_private) {
        if (!core_scroll_page(NULL,
                              (char *)query->setup.plain_data[i].message.bytes,
                              inheritance_send_error)) {
          return false;
        }
      }
      continue;
    }
  }
  // WHILE LOOP:
  while (1) {
    // fetch pin
    input_text_init(ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
    // TODO: add status macros to protobuf
    uint32_t pin_status = get_state_on_input_scr(PIN_OK, PIN_REJECT, TIMED_OUT);
    if (PIN_OK != pin_status) {
      // Operation cancel
      return;
    }
    uint8_t password_double_hash[SHA256_DIGEST_LENGTH];
    sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
               strnlen(flow_level.screen_input.input_text,
                       sizeof(flow_level.screen_input.input_text)),
               password_double_hash);
    sha256_Raw(
        password_double_hash, SHA256_DIGEST_LENGTH, password_double_hash);

    card_error_type_e status =
        inheritance_verify_pin(query->setup.wallet_id, password_double_hash);

    if (CARD_OPERATION_SUCCESS == status) {
      // STORE IF CORRECT PIN
      if (SESSION_MSG_MAX <= msg_count) {
        LOG_CRITICAL("xxec %d", __LINE__);
        return;
      }
      size_t last_index = msg_count;
      msgs[last_index].plain_data[0] = 1;    // PIN is private
      msgs[last_index].plain_data_size += 1;

      memcpy(msgs[last_index].plain_data + 1,
             (uint8_t *)flow_level.screen_input.input_text,
             strnlen(flow_level.screen_input.input_text,
                     sizeof(flow_level.screen_input.input_text)));
      msgs[last_index].plain_data_size +=
          strnlen(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text));
      msg_count++;

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      break;
    } else {
      // DISPLAY WRONG PIN/ Unknown ERROR
      continue;
    }
  }

  if (!session_encrypt_secure_data(query->setup.wallet_id, msgs, msg_count)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  uint8_t packet[SESSION_PACKET_SIZE] = {0};
  size_t packet_size = 0;

  if (!session_encrypt_packet(msgs, msg_count, packet, &packet_size)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  response->which_response = INHERITANCE_RESULT_SETUP_TAG;
  response->setup.has_encrypted_data = true;
  memcpy(response->setup.encrypted_data.packet.bytes, packet, packet_size);
  response->setup.encrypted_data.packet.size = packet_size;

#if USE_SIMULATOR == 1
  printf("Inheritance Setup Result: <Encrpyted Data>\n");
  for (int i = 0; i < response->setup.encrypted_data.packet.size; i++) {
    printf("%02x", response->setup.encrypted_data.packet.bytes[i]);
    fflush(stdout);
  }
  printf("\nEnd\n");
#endif
}