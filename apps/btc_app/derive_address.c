/**
 * @file    derive_address.c
 * @author  Cypherock X1 Team
 * @brief   generate address from public key for BTC.
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

#include "bip32.h"
#include "btc/core.pb.h"
#include "curves.h"
#include "pb_encode.h"
#include "ui_delay.h"
#include "usb_api.h"
#include "utils.h"

// definition for the btc_get_wallet_public_key.
// This function is defined in the btc_app.c file.
void btc_get_wallet_public_key(btc_query_t *query) {
  // initialize the seed string
  uint8_t seed[64] = {0};
  // initialize the seed buffer
  hex_string_to_byte_array(
      "5fa0a0071c620574977a64c44e73438fba3368c772081cdda5fd5e23cc07566241351d55"
      "2fa87555c1bc468e21a9a4229b709999cc62569f33686a2d907f6a32",
      128,
      seed);

  HDNode node;

  hdnode_from_seed(seed, sizeof(seed), SECP256K1_NAME, &node);

  // check if query.get_public_key.initiate.derivation_path_count is equal to 5.
  // TODO: Need more validation on the derivation path.
  if (query->get_public_key.initiate.derivation_path_count != 5) {
    // if not equal to 5, then return an error.

    // set the error code to 1.
    delay_scr_init("Invalid derivation path", DELAY_TIME);
    return;
  }
  common_derivation_path_t *derivation_path =
      &query->get_public_key.initiate.derivation_path;
  // derive the address from the public key
  // TODO: Add check to see if the path is hardened.
  // Need to use private key to derive hardened path.
  hdnode_private_ckd(&node, derivation_path[0].index);
  hdnode_private_ckd(&node, derivation_path[1].index);
  hdnode_private_ckd(&node, derivation_path[2].index);

  uint32_t version;
  uint8_t address_version;
  // final computed address will be stored in this variable.
  char address[43];
  btc_result_t address_result;

  get_version(derivation_path[0].index,
              derivation_path[1].index,
              &address_version,
              &version);

  hdnode_fill_public_key(&node);

  // TODO: Add check to see if the path is non-hardened.
  // Will be using the public key to derive rest of the non-hardened path.
  hdnode_public_ckd(&node, derivation_path[3].index);
  hdnode_fill_public_key(&node);
  hdnode_public_ckd(&node, derivation_path[4].index);
  hdnode_fill_public_key(&node);

  memzero(address, sizeof(address));
  // check if the bitcoin address is segwit or not.
  uint8_t segwit;
  if (derivation_path[0].index == NATIVE_SEGWIT)
    segwit = 1;
  else
    segwit = 0;

  if (segwit)
    get_segwit_address(node.public_key,
                       sizeof(node.public_key),
                       derivation_path[1].index,
                       address);
  else
    hdnode_get_address(&node, address_version, address, 35);

  // return the result to the host.
  address_result.which_response = BTC_RESULT_GET_PUBLIC_KEY_TAG;
  address_result.get_public_key.which_response =
      BTC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG;
  memcpy(address_result.get_public_key.result.public_key,
         address,
         sizeof(address));

  uint8_t buffer[1024] = {0};
  pb_size_t max_buffer_len = sizeof(buffer);
  /* Create a stream that will write to our buffer. */
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  /* Now we are ready to encode the message! */
  bool status = pb_encode(&stream, BTC_RESULT_FIELDS, &address_result);
  usb_send_msg(buffer, stream.bytes_written);
}
