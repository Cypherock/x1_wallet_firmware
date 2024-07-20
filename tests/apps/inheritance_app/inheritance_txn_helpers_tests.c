/**
 * @file    inheritance_txn_helper_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bitcoin transaction helper functions
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

#include "inheritance_api.h"

TEST_GROUP(inheritance_txn_helper_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(inheritance_txn_helper_test) {
  // g_inheritance_app = get_inheritance_app_desc()->app_config;
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(inheritance_txn_helper_test) {
  // g_inheritance_app = NULL;
}

// TEST(inheritance_txn_helper_test, inheritance_txn_helper_verify_input_p2pk) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
//   uint8_t raw_txn[300] = {0};
//   hex_string_to_byte_array(
//       "010000000100000000000000000000000000000000000000000000000000000000000000"
//       "00ffffffff0704ffff001d0134ffffffff0100f2052a0100000043410411db93e1dcdb8a"
//       "016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0eaddfb84ccf9744464"
//       "f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac00000000",
//       268,
//       raw_txn);
//   // only fill necessary values
//   inheritance_sign_txn_input_t input = {.prev_output_index = 0,
//                                 .value = 5000000000,
//                                 .script_pub_key = {
//                                     .size = 67,
//                                 }};
//   hex_string_to_byte_array(
//       "0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9",
//       64,
//       input.prev_txn_hash);
//   // revere order of txn-id:
//   cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));

//   hex_string_to_byte_array(
//       "410411db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0"
//       "eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac",
//       134,
//       input.script_pub_key.bytes);

//   int status = inheritance_verify_input_test(&input, raw_txn, 134);

//   TEST_ASSERT_EQUAL_INT(0, status);
// }