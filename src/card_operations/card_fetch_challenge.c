/**
 * @file    card_fetch_challenge.c
 * @author  Cypherock X1 Team
 * @brief   Implements card operation to handle fetching of wallet challenge
 *from X1 card
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
#include "card_fetch_challenge.h"

#include "card_internal.h"
#include "card_utils.h"
#include "constant_texts.h"
#include "flash_api.h"
#include "nfc.h"
#include "pow_utilities.h"
#include "ui_screens.h"

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

card_error_type_e card_fetch_challenge(const uint8_t *wallet_name,
                                       card_operation_frontend_t *frontend) {
  if (NULL == wallet_name) {
    return CARD_OPERATION_DEFAULT_INVALID;
  }

  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;
  uint8_t target[SHA256_SIZE] = {0};
  uint8_t random_number[POW_RAND_NUMBER_SIZE] = {0};

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  uint8_t wallet_index = 0xFF;
  ASSERT(SUCCESS ==
         get_index_by_name((const char *)wallet_name, &wallet_index));

  while (1) {
    card_data.nfc_data.acceptable_cards = get_wallet_card_locked(wallet_index);
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    card_data.nfc_data.tapped_card = 0;

    result = card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status =
          nfc_get_challenge(wallet_name, target, random_number);

      if (SW_NO_ERROR == card_data.nfc_data.status) {
        ASSERT(SUCCESS == add_challenge_flash((const char *)wallet_name,
                                              target,
                                              random_number));
        log_hex_array("pow-rn: ", random_number, POW_RAND_NUMBER_SIZE);
        LOG_CRITICAL("pow-tg n: %d", pow_count_set_bits(target));
        buzzer_start(BUZZER_DURATION);
        break;
      } else if (SW_WARNING_STATE_UNCHANGED == card_data.nfc_data.status) {
        // wallet is in unlocked state on card; update flash wallet data.
        ASSERT(SUCCESS ==
               update_wallet_locked_flash((const char *)wallet_name, false));
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
        instruction_scr_init(frontend->msg, frontend->heading);
        continue;
      }
    }

    // If control reached here, it is an unrecoverable error, so break
    result = card_data.error_type;
    break;
  }

  nfc_deselect_card();
  return result;
}
