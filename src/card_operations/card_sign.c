/**
 * @file    card_sign.c
 * @author  Cypherock X1 Team
 * @brief   Source file for card sign data operation
 *
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
#include "card_sign.h"

#include "buzzer.h"
#include "card_internal.h"
#include "card_utils.h"
#include "constant_texts.h"
#include "events.h"
#include "nfc.h"
#include "string.h"
#include "ui_instruction.h"
#include "ui_message.h"

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
 * @brief Generates a card signature from card's authentication private key.
 *
 * @param sign_data Pointer to the data to be signed with max 64 bytes.
 * @param sign_data_size Size of the data to be signed(max 64 bytes).
 * @param signature Pointer to the buffer where the signature will be stored.
 * @return card_error_status_word_e Returns SW_NO_ERROR if the signature
 * generation was successful, or an error code if it failed.
 */
static card_error_status_word_e get_card_auth_signature(uint8_t *sign_data,
                                                        uint16_t sign_data_size,
                                                        uint8_t *signature);

/**
 * @brief Initializes a smart card and signs data.
 *
 * @param card_data Pointer to the smart card operation data.
 * @param sign_data Pointer to the data to be signed.
 */
static void handle_card_sign_data_operation(card_operation_data_t *card_data,
                                            card_sign_data_config_t *sign_data);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static card_error_status_word_e get_card_auth_signature(uint8_t *sign_data,
                                                        uint16_t sign_data_size,
                                                        uint8_t *signature) {
  ASSERT(ECDSA_SIGNATURE_SIZE >= sign_data_size && NULL != sign_data &&
         NULL != signature);

  uint8_t sign_data_inout[ECDSA_SIGNATURE_SIZE] = {0};
  uint16_t sign_data_size_inout = 0;
  card_error_status_word_e status = DEFAULT_UINT32_IN_FLASH_ENUM;

  /**
   * @brief This retry attempt has been added to avoid exceptions occuring due
   * to incorrect signature length received from card. In a rare case the X1
   * Card would send a incorrect signature with as the size would be less than
   * expected of 64-bytes. When such a case is encountered, we do a retry
   * attempt to
   */
  do {
    sign_data_size_inout = sign_data_size;
    memcpy(sign_data_inout, sign_data, sign_data_size);
    status = nfc_ecdsa(sign_data_inout, &sign_data_size_inout);
  } while (CARD_SIGNATURE_INCORRECT_LEN == status);

  if (SW_NO_ERROR == status) {
    memcpy(signature, sign_data_inout, ECDSA_SIGNATURE_SIZE);
  }

  return status;
}

static void handle_card_sign_data_operation(
    card_operation_data_t *card_data,
    card_sign_data_config_t *sign_data) {
  /* TODO: Keep retries and card_removed_retries when intializing nfc_data */
  card_data->nfc_data = init_nfc_connection_data(sign_data->family_id,
                                                 sign_data->acceptable_cards);
  card_initialize_applet(card_data);

  if (CARD_OPERATION_SUCCESS == card_data->error_type) {
    if (DEFAULT_UINT32_IN_FLASH == U32_READ_BE_ARRAY(sign_data->family_id)) {
      memcpy(
          sign_data->family_id, card_data->nfc_data.family_id, FAMILY_ID_SIZE);
    }

    if (CARD_SIGN_SERIAL == sign_data->sign_type) {
      get_card_serial(&(card_data->nfc_data), sign_data->data);
      sign_data->data_size = CARD_ID_SIZE;
    }

    card_data->nfc_data.status = get_card_auth_signature(
        sign_data->data, sign_data->data_size, sign_data->signature);

    card_handle_errors(card_data);
  }

  if (CARD_OPERATION_CARD_REMOVED != card_data->error_type) {
    buzzer_start(BUZZER_DURATION);
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_sign_auth_data(card_sign_data_config_t *sign_data) {
  ASSERT(NULL != sign_data && NULL != sign_data->data);

  card_operation_data_t card_data = {0};

  while (1) {
    handle_card_sign_data_operation(&card_data, sign_data);

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type) {
      continue;
    } else {
      break;
    }
  }

  sign_data->acceptable_cards = card_data.nfc_data.acceptable_cards;
  sign_data->status = card_data.nfc_data.status;
  nfc_deselect_card();
  return card_data.error_type;
}