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
#include "evm_helpers.h"
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
uint8_t buffer[2048] = {0};
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
  // raw Txn:
  // https://etherscan.io/getRawTx?tx=0x729f88ee19917fe916c58a838b63713fe26729239bb380debabbe2ef0c64e8c1
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

#ifdef EVM_TXN_MANUAL_TEST
  TEST_ASSERT_TRUE(get_user_verification());
#endif
}

TEST(evm_txn_test, evm_txn_usdt_transfer) {
  evm_query_t query = {
      .which_request = 2,
      .sign_txn = {.which_request = 1,
                   .initiate = {
                       .chain_id = 1,
                       .derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .address_format = EVM_DEFAULT,
                       .transaction_size = 109,
                   }}};
  evm_query_t query1 = {.which_request = 2,
                        .sign_txn = {.which_request = 2,
                                     .txn_data = {.has_chunk_payload = true,
                                                  .chunk_payload = {
                                                      .chunk =
                                                          {
                                                              .size = 109,
                                                          },
                                                      .remaining_size = 0,
                                                      .chunk_index = 0,
                                                      .total_chunks = 1,
                                                  }}}};
  // raw Txn:
  // https://etherscan.io/getRawTx?tx=0xb8a6f4b82037dfc9aefa22d4cca2f695ada1d5d568673d001cf2d7dab27bea25
  hex_string_to_byte_array(
      "f86b81a585043be57f958301725d94dac17f958d2ee523a2206206994597c13d831ec700"
      "b844a9059cbb0000000000000000000000001754b2d4414468d38bddea24b83cdb1a9b8c"
      "4355000000000000000000000000000000000000000000000000000000320effa6000180"
      "80",
      218,
      query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  memcpy(&txn_context->init_info,
         &query.sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  TEST_ASSERT_TRUE(fetch_valid_transaction(&query));

#ifdef EVM_TXN_MANUAL_TEST
  TEST_ASSERT_TRUE(get_user_verification());
#endif
}

TEST(evm_txn_test, evm_txn_haka_transfer) {
  evm_query_t query = {
      .which_request = 2,
      .sign_txn = {.which_request = 1,
                   .initiate = {
                       .chain_id = 1,
                       .derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .address_format = EVM_DEFAULT,
                       .transaction_size = 110,
                   }}};
  evm_query_t query1 = {.which_request = 2,
                        .sign_txn = {.which_request = 2,
                                     .txn_data = {.has_chunk_payload = true,
                                                  .chunk_payload = {
                                                      .chunk =
                                                          {
                                                              .size = 110,
                                                          },
                                                      .remaining_size = 0,
                                                      .chunk_index = 0,
                                                      .total_chunks = 1,
                                                  }}}};
  // raw Txn:
  // https://etherscan.io/getRawTx?tx=0xf0002e26888aea20a1112ff7abcb74de9cc6c1067c0a38b8bd5302586dcba140
  hex_string_to_byte_array(
      "f86c831f570d850bbcb5521582fc9294d85ad783cc94bd04196a13dc042a3054a9b52210"
      "00b844a9059cbb000000000000000000000000e086a9f63c184a90ac3a9dd62782b08272"
      "4b45b1000000000000000000000000000000000000000000001c6063d221ab258a800001"
      "8080",
      220,
      query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  memcpy(&txn_context->init_info,
         &query.sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  TEST_ASSERT_TRUE(fetch_valid_transaction(&query));

#ifdef EVM_TXN_MANUAL_TEST
  TEST_ASSERT_TRUE(get_user_verification());
#endif
}

TEST(evm_txn_test, evm_txn_blind_signing) {
  evm_query_t query = {
      .which_request = 2,
      .sign_txn = {.which_request = 1,
                   .initiate = {
                       .chain_id = 1,
                       .derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .address_format = EVM_DEFAULT,
                       .transaction_size = 1489,
                   }}};
  evm_query_t query1 = {.which_request = 2,
                        .sign_txn = {.which_request = 2,
                                     .txn_data = {.has_chunk_payload = true,
                                                  .chunk_payload = {
                                                      .chunk =
                                                          {
                                                              .size = 1489,
                                                          },
                                                      .remaining_size = 0,
                                                      .chunk_index = 0,
                                                      .total_chunks = 1,
                                                  }}}};
  // raw Txn:
  // https://etherscan.io/getRawTx?tx=0x906cdfccc6374f6417af58b57b2887301c202d39cbc6d53c630cf3b499fc44bc
  hex_string_to_byte_array(
      "f905ce830e261c8502b176b8b8831e848094a9d1e08c7793af67e9d92fe308d5697fb81d"
      "3e4380b905a4ca350aa60000000000000000000000000000000000000000000000000000"
      "000000000040000000000000000000000000000000000000000000000000000000000003"
      "d090000000000000000000000000000000000000000000000000000000000000000e0000"
      "00000000000000000000a0246c9032bc3a600820415ae600c6388619a14d000000000000"
      "000000000000fd2b2cd6356af9ac539a32def78c825341524cda00000000000000000000"
      "00000000000000000000000000014e4359a7f4433400000000000000000000000000dac1"
      "7f958d2ee523a2206206994597c13d831ec7000000000000000000000000002c780123c4"
      "5ebb4e5697a84bde870fef1b6a9b00000000000000000000000000000000000000000000"
      "0000000000000203e686000000000000000000000000dac17f958d2ee523a22062069945"
      "97c13d831ec7000000000000000000000000fdd1aa08da011f1c0f049770f86324d2ff71"
      "13450000000000000000000000000000000000000000000000000000000002ed7c5b0000"
      "00000000000000000000dac17f958d2ee523a2206206994597c13d831ec7000000000000"
      "00000000000057116aad440de463b5f772bef16c19b5c9346c3e00000000000000000000"
      "00000000000000000000000000000000000007e1807d000000000000000000000000dac1"
      "7f958d2ee523a2206206994597c13d831ec700000000000000000000000067f75b045420"
      "f946844212a47c51523adf64451300000000000000000000000000000000000000000000"
      "0000000000000b52a9fd000000000000000000000000dac17f958d2ee523a22062069945"
      "97c13d831ec7000000000000000000000000f3e6b30dc2a5ebc27334addd3b6ee4b4b55a"
      "24ec0000000000000000000000000000000000000000000000000000000013c876c30000"
      "00000000000000000000dac17f958d2ee523a2206206994597c13d831ec7000000000000"
      "000000000000aa16ffae31dafaa639a0a99e59e5e50fc93cef4800000000000000000000"
      "0000000000000000000000000000000000003b278f37000000000000000000000000dac1"
      "7f958d2ee523a2206206994597c13d831ec7000000000000000000000000aea6291cb48d"
      "9675b120f74e2f0b0de01db47a0000000000000000000000000000000000000000000000"
      "0000000000bc97add5e600000000000000000000000095ad61b0a150d79219dcf64e1e6c"
      "c01f0b64c4ce00000000000000000000000037bab29ec945191f1d6cbea89a7f10414876"
      "a9d6000000000000000000000000000000000000000000001bd6924a7a73d57624000000"
      "0000000000000000000095ad61b0a150d79219dcf64e1e6cc01f0b64c4ce000000000000"
      "000000000000150ebc67bb51c439a851c80f47eae0b4b057774900000000000000000000"
      "0000000000000000000000005254fb6f5f0fd22d8400000000000000000000000000961c"
      "8c0b1aad0c0b10a51fef6a867e3091bcef1700000000000000000000000081153f0889ab"
      "398c4acb42cb58b565a5392bba9500000000000000000000000000000000000000000000"
      "00896381378fa22fbc00000000000000000000000000961c8c0b1aad0c0b10a51fef6a86"
      "7e3091bcef17000000000000000000000000c5d4ec9300be6da89a3db305c415c7cd3cbb"
      "7e8e0000000000000000000000000000000000000000000001f8eabe252e1a0da0000000"
      "000000000000000000007c84e62859d0715eb77d1b1c4154ecd6abb21bec000000000000"
      "0000000000000a84d9d308f3b7d63cd54c16d600b3526aabd1df00000000000000000000"
      "000000000000000000000000001e4b8dbfe83b90a8000000000000000000000000007420"
      "b4b9a0110cdc71fb720908340c03f9bc03ec0000000000000000000000007f39c294312b"
      "e3134e03154ab41d9b61cba0345100000000000000000000000000000000000000000000"
      "010d1db684112aeb2c00018080",
      2978,
      query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  memcpy(&txn_context->init_info,
         &query.sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  TEST_ASSERT_TRUE(fetch_valid_transaction(&query));

#ifdef EVM_TXN_MANUAL_TEST
  TEST_ASSERT_TRUE(get_user_verification());
#endif
}

TEST(evm_txn_test, evm_txn_token_deposit) {
  evm_query_t query = {
      .which_request = 2,
      .sign_txn = {.which_request = 1,
                   .initiate = {
                       .chain_id = 1,
                       .derivation_path_count = EVM_DRV_BIP44_DEPTH,
                       .derivation_path = {ETHEREUM_PURPOSE_INDEX,
                                           ETHEREUM_COIN_INDEX,
                                           EVM_DRV_ACCOUNT,
                                           0,
                                           0},
                       .wallet_id = {},
                       .address_format = EVM_DEFAULT,
                       .transaction_size = 49,
                   }}};
  evm_query_t query1 = {.which_request = 2,
                        .sign_txn = {.which_request = 2,
                                     .txn_data = {.has_chunk_payload = true,
                                                  .chunk_payload = {
                                                      .chunk =
                                                          {
                                                              .size = 49,
                                                          },
                                                      .remaining_size = 0,
                                                      .chunk_index = 0,
                                                      .total_chunks = 1,
                                                  }}}};
  // dummy raw Txn
  hex_string_to_byte_array("f08084014a86108301e6089482af49447d8a07e3bd95bd0d56f"
                           "35241523fbab188025bf6196bd1000084d0e30db0018080",
                           98,
                           query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  memcpy(&txn_context->init_info,
         &query.sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  TEST_ASSERT_TRUE(fetch_valid_transaction(&query));

#ifdef EVM_TXN_MANUAL_TEST
  TEST_ASSERT_TRUE(get_user_verification());
#endif
}

// large transaction test
// https://etherscan.io/getRawTx?tx=0x2d6a7b0f6adeff38423d4c62cd8b6ccb708ddad85da5d3d06756ad4d8a04a6a2
