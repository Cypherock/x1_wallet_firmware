/**
 * @file    common_error.c
 * @author  Cypherock X1 Team
 * @brief   Helpers to initialize an instance of common error.
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

#include "common_error.h"

#include "apdu.h"

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

error_common_error_t init_common_error(pb_size_t which_error,
                                       uint32_t error_code) {
  error_common_error_t error = ERROR_COMMON_ERROR_INIT_ZERO;
  error.which_error = which_error;

  // unrestricted initialization; pick and fill any one union member
  // NOTE: this works because size of union is uint32_t
  error.unknown_error = error_code;
  return error;
}

error_card_error_t get_card_error_from_nfc_status(
    card_error_status_word_e error_status_word) {
  switch (error_status_word) {
    case SW_FILE_INVALID:
      return ERROR_CARD_ERROR_SW_FILE_INVALID;
      break;
    case SW_INCOMPATIBLE_APPLET:
      return ERROR_CARD_ERROR_SW_INCOMPATIBLE_APPLET;
      break;
    case SW_FILE_NOT_FOUND:
      return ERROR_CARD_ERROR_SW_FILE_NOT_FOUND;
    case SW_SECURITY_CONDITIONS_NOT_SATISFIED:
      return ERROR_CARD_ERROR_SW_SECURITY_CONDITIONS_NOT_SATISFIED;
      break;
    case SW_NOT_PAIRED:
      return ERROR_CARD_ERROR_SW_NOT_PAIRED;
      break;
    case SW_CONDITIONS_NOT_SATISFIED:
      return ERROR_CARD_ERROR_SW_CONDITIONS_NOT_SATISFIED;
      break;
    case SW_WRONG_DATA:
      return ERROR_CARD_ERROR_SW_WRONG_DATA;
      break;
    case SW_FILE_FULL:
      return ERROR_CARD_ERROR_SW_FILE_FULL;
      break;
    case SW_RECORD_NOT_FOUND:
      return ERROR_CARD_ERROR_SW_RECORD_NOT_FOUND;
      break;
    case SW_TRANSACTION_EXCEPTION:
      return ERROR_CARD_ERROR_SW_TRANSACTION_EXCEPTION;
      break;
    case SW_NULL_POINTER_EXCEPTION:
      return ERROR_CARD_ERROR_SW_NULL_POINTER_EXCEPTION;
      break;
    case SW_OUT_OF_BOUNDARY:
      return ERROR_CARD_ERROR_SW_OUT_OF_BOUNDARY;
      break;
    case SW_INVALID_INS:
      return ERROR_CARD_ERROR_SW_INVALID_INS;
      break;
    case SW_INS_BLOCKED:
      return ERROR_CARD_ERROR_SW_INS_BLOCKED;
      break;
    default:
      switch (error_status_word & 0xFF00) {
        case POW_SW_WALLET_LOCKED:
          return ERROR_CARD_ERROR_POW_SW_WALLET_LOCKED;
          break;

        case SW_CORRECT_LENGTH_00:
          return ERROR_CARD_ERROR_SW_CORRECT_LENGTH_00;
          break;

        case SW_CRYPTO_EXCEPTION:
          return ERROR_CARD_ERROR_SW_CRYPTO_EXCEPTION;
          break;

        default:
          return ERROR_CARD_ERROR_UNKNOWN;
          break;
      }
  }
}