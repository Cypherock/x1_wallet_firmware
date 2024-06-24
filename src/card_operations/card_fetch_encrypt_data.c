/**
 * @file    card_fetch_encrypt_data.c
 * @author  Cypherock X1 Team
 * @brief   Wallet unlock flow controller
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
#include "card_internal.h"
#include "card_utils.h"
#include "nfc.h"
#include "pow_utilities.h"
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

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_fetch_encrypt_data(uint8_t *wallet_id,
                                          SessionMsg *msgs,
                                          size_t msg_num) {
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;
  card_operation_data_t card_data = {0};

  const char wallet_name[NAME_SIZE] = "";
  rejection_cb *reject_cb;
  ASSERT(get_wallet_data_by_id(
      wallet_id, (const uint8_t *)wallet_name, reject_cb));

  uint8_t flash_wallet_index = 0xFF;
  ASSERT(SUCCESS == get_index_by_name((const char *)wallet_name,
                                      &flash_wallet_index));    // check issue

  instruction_scr_init(ui_text_place_card_below, ui_text_tap_1_2_cards);

  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;
  while (1) {
    card_data.nfc_data.acceptable_cards = ACCEPTABLE_CARDS_ALL;
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    card_data.nfc_data.tapped_card = 0;

    result = card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      for (int i = 0; i < msg_num; i++) {
        int i = 1;
        memset(msgs[i].msg_enc, 0, sizeof(msgs[i].msg_enc));
        msgs[i].msg_enc_size = 0;
        print_msg(msgs[i]);

        card_data.nfc_data.status = nfc_encrypt_data(wallet_name,
                                                     msgs[i].msg_dec,
                                                     msgs[i].msg_dec_size,
                                                     msgs[i].msg_enc,
                                                     &msgs[i].msg_enc_size);
        print_msg(msgs[i]);
      }

      if (card_data.nfc_data.status == SW_NO_ERROR ||
          card_data.nfc_data.status == SW_WARNING_STATE_UNCHANGED) {
        buzzer_start(BUZZER_DURATION);
        break;
      } else {
        card_handle_errors(&card_data);
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

// TODO: Remove after testing

void print_msg(SessionMsg msg) {
  char hex[200];
  byte_array_to_hex_string(
      msg.msg_dec, msg.msg_dec_size, hex, msg.msg_dec_size * 2 + 1);
  printf("MSG Msg_Dec: %s\n", msg.msg_dec);
  printf("MSG Msg_Dec_Size: %d\n", msg.msg_dec_size);
  byte_array_to_hex_string(
      msg.msg_enc, msg.msg_enc_size, hex, msg.msg_enc_size * 2 + 1);
  printf("MSG Msg_Enc: %s\n", msg.msg_enc);
  printf("MSG Msg_Enc_Size: %d\n", msg.msg_enc_size);
  printf("MSG Is_Private: %s\n", msg.is_private ? "true" : "false");
}

bool debug = true;
// TODO cleanup: delete after testing
char *print_arr(char *name, uint8_t *bytearray, size_t size) {
  char bytearray_hex[size * 2 + 1];
  if (debug == true) {
    uint8ToHexString(bytearray, size, bytearray_hex);
    printf("\n%s[%d bytes]: %s\n",
           name,
           (strlen(bytearray_hex) / 2),
           bytearray_hex);
  }
  return bytearray_hex;
}