/**
 * @file    evm_txn_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for EVM txn helper functions
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

#include "curves.h"
#include "eth_app.h"
#include "evm_priv.h"
#include "flash_config.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"
#include "utils.h"

TEST_GROUP(evm_txn_test);

bool handle_initiate_query(const evm_query_t *query);
bool fetch_valid_transaction(evm_query_t *query);
bool get_user_verification();
extern evm_txn_context_t *txn_context;

static const uint8_t core_msg[] = {10, 2, 8, 1};
uint8_t buffer[1024] = {0};
pb_ostream_t ostream;

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(evm_txn_test) {
  g_evm_app = get_eth_app_desc()->app_config;
  ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(evm_txn_test) {
  g_evm_app = NULL;
}

TEST(evm_txn_test, evm_txn_eth_transfer) {
  evm_query_t query = {
      .which_request = 2,
      .sign_txn = {.which_request = 1,
                   .initiate = {
                       .chain_id = 1,
                       .derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .address_format = EVM_DEFAULT,
                       .transaction_size = 44,
                   }}};
  evm_query_t query1 = {.which_request = 2,
                        .sign_txn = {.which_request = 2,
                                     .txn_data = {.has_chunk_payload = true,
                                                  .chunk_payload = {
                                                      .chunk =
                                                          {
                                                              .size = 44,
                                                          },
                                                      .remaining_size = 0,
                                                      .chunk_index = 0,
                                                      .total_chunks = 1,
                                                  }}}};
  hex_string_to_byte_array("eb1685050775d80082627094b3c152026d3722cb4acf2fb853f"
                           "e107dd96bbb5e872386f26fc1000080018080",
                           88,
                           query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  memcpy(&txn_context->init_info,
         &query.sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  TEST_ASSERT_TRUE(fetch_valid_transaction(&query));
  TEST_ASSERT_TRUE(get_user_verification());
}
