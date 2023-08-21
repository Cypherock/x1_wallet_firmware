/**
 * @file    near_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the NEAR app
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

#include "near_helpers.h"

#include "constant_texts.h"
#include "near_context.h"
#include "utils.h"

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
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool near_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (NEAR_IMPLICIT_ACCOUNT_DEPTH != levels) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/397'/0'/0'/i'
  status = (NEAR_PURPOSE_INDEX == purpose && NEAR_COIN_INDEX == coin &&
            NEAR_ACCOUNT_INDEX == account && NEAR_CHANGE_INDEX == change &&
            is_hardened(address));

  return status;
}

void near_get_new_account_id_from_fn_args(const char *args,
                                          uint32_t args_len,
                                          char *account_id) {
  // length of '{"new_account_id":"'
  const int start = 19;

  // length of '","new_public_key":"ed25519:..."}'
  const int end = args_len - 74;

  memcpy(account_id, args + start, end - start);
  account_id[end - start] = '\0';

  return;
}

void get_amount_string(const uint8_t *amount,
                       char *string,
                       size_t size_of_string) {
  char amount_string[40] = "";
  char amount_decimal_string[40] = "";
  byte_array_to_hex_string(
      amount, NEAR_DEPOSIT_SIZE_BYTES, amount_string, sizeof(amount_string));

  convert_byte_array_to_decimal_string(NEAR_DEPOSIT_SIZE_BYTES * 2,
                                       NEAR_DECIMAL,
                                       amount_string,
                                       amount_decimal_string,
                                       sizeof(amount_decimal_string));

  snprintf(string,
           size_of_string,
           UI_TEXT_VERIFY_AMOUNT,
           amount_decimal_string,
           near_app.lunit_name);
  return;
}
