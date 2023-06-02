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
 * @return ISO7816 Returns SW_NO_ERROR if the signature generation was
 * successful, or an error code if it failed.
 */
static ISO7816 get_card_auth_signature(uint8_t *sign_data,
                                       uint16_t sign_data_size,
                                       uint8_t *signature);

/**
 * @brief Initializes a smart card and signs data.
 *
 * @param card_data Pointer to the smart card operation data.
 * @param sign_data Pointer to the data to be signed.
 */
static void handle_card_init_and_sign_data_operation(
    card_operation_data_t *card_data,
    card_sign_data_config_t *sign_data);

/**
 * @brief Handles the response of a smart card sign operation.
 * Based on the error code, handles user action required and sets the corret UI
 * screen.
 *
 * @param card_data Pointer to the smart card operation data.
 * @param sign_data Pointer to the data to be signed.
 * @return card_error_type_e Returns a card error code indicating the success or
 * failure of the operation.
 */
static card_error_type_e handle_sign_data_operation_response(
    card_operation_data_t *card_data,
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
static ISO7816 get_card_auth_signature(uint8_t *sign_data,
                                       uint16_t sign_data_size,
                                       uint8_t *signature) {
  ASSERT(ECDSA_SIGNATURE_SIZE >= sign_data_size && NULL != sign_data &&
         NULL != signature);

  uint8_t sign_data_inout[ECDSA_SIGNATURE_SIZE] = {0};
  uint16_t sign_data_size_inout = sign_data_size;
  ISO7816 status = DEFAULT_UINT32_IN_FLASH_ENUM;
  memcpy(sign_data_inout, sign_data, sign_data_size);
  status = nfc_ecdsa(sign_data_inout, &sign_data_size_inout);

  if (SW_NO_ERROR == status) {
    memcpy(signature, sign_data_inout, ECDSA_SIGNATURE_SIZE);
  }

  return status;
}

static void handle_card_init_and_sign_data_operation(
    card_operation_data_t *card_data,
    card_sign_data_config_t *sign_data) {
  card_data->nfc_data = init_nfc_connection_data(sign_data->family_id,
                                                 sign_data->acceptable_cards);
  card_initialize_applet(card_data);

  if (CARD_OPERATION_SUCCESS == card_data->error_type) {
    if (DEFAULT_UINT32_IN_FLASH == *(uint32_t *)(sign_data->family_id)) {
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

  if (CARD_OPERATION_CARD_REMOVED != card_data->error_type)
    buzzer_start(BUZZER_DURATION);
}

static card_error_type_e handle_sign_data_operation_response(
    card_operation_data_t *card_data,
    card_sign_data_config_t *sign_data) {
  card_error_type_e temp_error = CARD_OPERATION_DEFAULT_INVALID;

  switch (card_data->error_type) {
    case CARD_OPERATION_SUCCESS:
      if (false == sign_data->skip_card_removal) {
        temp_error = wait_for_card_removal();

        if (CARD_OPERATION_SUCCESS != temp_error) {
          return temp_error;
        }
      }
      break;

    case CARD_OPERATION_RETAP_BY_USER_REQUIRED:
      temp_error = display_error_message(card_data->error_message);

      if (CARD_OPERATION_SUCCESS != temp_error) {
        return temp_error;
      }

      instruction_scr_init(sign_data->message, sign_data->heading);
      break;

    case CARD_OPERATION_ABORT_OPERATION:
      display_error_message(card_data->error_message);
      break;

    default:
      break;
  }

  return card_data->error_type;
}
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_sign_auth_data(card_sign_data_config_t *sign_data) {
  ASSERT(NULL != sign_data && NULL != sign_data->data);

  card_operation_data_t card_data = {0};
  instruction_scr_init(sign_data->message, sign_data->heading);

  while (1) {
    handle_card_init_and_sign_data_operation(&card_data, sign_data);

    card_data.error_type =
        handle_sign_data_operation_response(&card_data, sign_data);

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
      continue;
    } else {
      break;
    }
  }

  nfc_deselect_card();
  return card_data.error_type;
}