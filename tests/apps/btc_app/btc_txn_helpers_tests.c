/**
 * @file    btc_txn_helper_tests.c
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

#include "btc_app.h"
#include "btc_helpers.h"
#include "btc_priv.h"
#include "btc_txn_helpers.h"
#include "unity_fixture.h"
#include "utils.h"

uint32_t get_transaction_weight(const btc_txn_context_t *txn_ctx);

TEST_GROUP(btc_txn_helper_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(btc_txn_helper_test) {
  g_app = get_btc_app();
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(btc_txn_helper_test) {
  g_app = NULL;
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pk) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "010000000100000000000000000000000000000000000000000000000000000000000000"
      "00ffffffff0704ffff001d0134ffffffff0100f2052a0100000043410411db93e1dcdb8a"
      "016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0eaddfb84ccf9744464"
      "f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac00000000",
      268,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {.prev_output_index = 0,
                           .value = 5000000000,
                           .script_pub_key = {
                               .size = 67,
                           }};
  hex_string_to_byte_array(
      "0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // 0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array(
      "410411db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0"
      "eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac",
      134,
      input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(0, btc_verify_input_utxo(raw_txn, 134, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pk_fail) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "010000000100000000000000000000000000000000000000000000000000000000000000"
      "00ffffffff0704ffff001d0134ffffffff0100f2052a0100000043410411db93e1dcdb8a"
      "016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0eaddfb84ccf9744464"
      "f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac00000000",
      268,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {
      .prev_output_index = 1,    // incorrect index; correct is '0'
      .value = 5000000000,
      .script_pub_key = {
          .size = 67,
      }};
  hex_string_to_byte_array(
      "0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // 0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array(
      "410411db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0"
      "eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac",
      134,
      input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(1, btc_verify_input_utxo(raw_txn, 134, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pkh) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/16fbc39570ac5f16c103a39da1920ab3b77ad4f21f3c6d415c745bd6a37097e1?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "01000000014da2d059c1c6eb1c66884643f3bfa917cdb182273bf9dd2361db0c1c6bc706"
      "61000000008b483045022100f2522df4a0d2193ee53ad95b698bf502e5874d340b4bb4f8"
      "0720015f2ce87296022061d33d02d6f4a3b131a18328c3b1249e53fa115735c2597e29b0"
      "738a1d5f3f8801410445bd85326dabc1772b4b319e0dc924ef93caf2360a033941e427f0"
      "397b265f3c46e805be40904034880e781ab758a9e67518c624393e2ac14339faa45fafc5"
      "deffffffff0100e1f505000000001976a91412ab8dc588ca9d5787dde7eb29569da63c3a"
      "238c88ac00000000",
      448,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {.prev_output_index = 0,
                           .value = 100000000,
                           .script_pub_key = {
                               .size = 25,
                           }};
  hex_string_to_byte_array(
      "eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array("76a91412ab8dc588ca9d5787dde7eb29569da63c3a238c88ac",
                           50,
                           input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(0, btc_verify_input_utxo(raw_txn, 224, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pkh_fail) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/16fbc39570ac5f16c103a39da1920ab3b77ad4f21f3c6d415c745bd6a37097e1?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "01000000014da2d059c1c6eb1c66884643f3bfa917cdb182273bf9dd2361db0c1c6bc706"
      "61000000008b483045022100f2522df4a0d2193ee53ad95b698bf502e5874d340b4bb4f8"
      "0720015f2ce87296022061d33d02d6f4a3b131a18328c3b1249e53fa115735c2597e29b0"
      "738a1d5f3f8801410445bd85326dabc1772b4b319e0dc924ef93caf2360a033941e427f0"
      "397b265f3c46e805be40904034880e781ab758a9e67518c624393e2ac14339faa45fafc5"
      "deffffffff0100e1f505000000001976a91412ab8dc588ca9d5787dde7eb29569da63c3a"
      "238c88ac00000000",
      448,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {
      .prev_output_index = 0,
      .value = 1000000000,    // invalid value; more by a factor of 10
      .script_pub_key = {
          .size = 25,
      }};
  hex_string_to_byte_array(
      "eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // eb0e2029310edade8e2a034aea4f0c4a1e243fe2dce67d05f95fddb7ac11bfbe
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array("76a91412ab8dc588ca9d5787dde7eb29569da63c3a238c88ac",
                           50,
                           input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(3, btc_verify_input_utxo(raw_txn, 224, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2wpkh) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b03?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/fcb26cf6235d591b89494398b51746610917fd49c376994fa0ce24fcc383eac0?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[2000] = {0};
  hex_string_to_byte_array(
      "0100000006d572e075a155d3fa334230a691cf085d463215aead3c417f08f931ccbbe6a1"
      "321b0000006b483045022100cdeb3add0650fc8b8351512d6df17fbfc602ec484d8c5c6a"
      "7800d3cd5e71fac60220481616b00629c8c7a81a10397db2942ce21676eb5fd4be20bc44"
      "31754c657f6001210217630c3bfab894e6e7322ae6d104d9b9aff58ae8ce4c30f45e9903"
      "51ba3668a4ffffffff9b4139cb7a02366cd228514a89b46cb55fc8183b03c3f9b8fe583e"
      "fe93f28958420000006b483045022100a19519b0d4af6a5f50301c36d620c7c2b5fea705"
      "7874a184c03db88196933fd4022070e559e34fe80e17a6d9e4b400d183c22af094684004"
      "1220a0965487964b46f301210334f414ce378f5f24a128dbf34a1be8d8a1b863e665f782"
      "99a52f06991be8406dffffffff4b98c9a61ad1b2070ebbcfc2d616824ff3259c5c0d09b8"
      "1f38975446f12e57cb3a0000006b483045022100e9b10baf0226b7394474142b87edcd26"
      "fef74f6fcb6b82833559e95481a51f5802204db8a3f7962f1e731b815f01807ef16b6d48"
      "bd73a40e6534d8e4abc1d72a2554012103e7dcb93f93afdf17b60e00b21f8a283e8140f5"
      "3619970daf7d3b5934230c420dfffffffff63acaa7893f29aa069dbbd6c74bdb9f597d70"
      "3e0f9a99d52b5930edd568ff54000000006b483045022100a3b9578999c1fb5d07c047c9"
      "6569476a73cc8684b1b6f1461cdeaa81de58e631022049cb3e73977605aad472e470a995"
      "9ab913f2b101c268b8cddadd4bca16db4bf10121033011c8839eb82d1c37596bee611cf7"
      "0564d21fb38ce0f4535c4d1d5e53dad958ffffffffd5c0e501e2045ba6fed26aba945e0d"
      "6333d2ca555a60e4c1c920f3e27030bc17400000006b483045022100bbf7e176a402c223"
      "32ab21c3aa0a5ca05bb9efe0d2a3f3797d5174873d4fc5e8022061aba78025eb56e7a8e1"
      "663c68bb63ff0d0d7db0b6fc5064e021994a59cf8c0c012103ca9827e71289cfc3581340"
      "ae413ae7860af88829b05e30083cee496cb9cdff39ffffffff64cca9bfba56c7f2dfb497"
      "53a7f90fe8e2b2121f3d1cd9221a1032425095ed3a8f0000006b483045022100b275537b"
      "a5e33b3511925e68df84d62487640dd813cf179cf65abd2920f4c29f022072523568c14f"
      "50881bd527d352847a343d65f5620a0df6e6683c8a793ff77a240121030ab067dab80cd5"
      "89f30e36d45902e933abc270304829b4e7323ba51695f8d331ffffffff01f0270f000000"
      "0000160014854fe623a8a6a4c76779b57c3895ed2e0962647400000000",
      1858,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {.prev_output_index = 0,
                           .value = 993264,
                           .script_pub_key = {
                               .size = 22,
                           }};
  hex_string_to_byte_array(
      "21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b03",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // 21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b03
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array("0014854fe623a8a6a4c76779b57c3895ed2e09626474",
                           44,
                           input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(0, btc_verify_input_utxo(raw_txn, 929, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2wpkh_fail) {
  /* Test data source: rawTxn -
   * https://blockchain.info/rawtx/21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b03?format=hex
   * txnElements -
   * https://blockchain.info/rawtx/fcb26cf6235d591b89494398b51746610917fd49c376994fa0ce24fcc383eac0?format=json
   * Code reference - https://www.blockchain.com/explorer/api/blockchain_api
   */
  uint8_t raw_txn[2000] = {0};
  hex_string_to_byte_array(
      "0100000006d572e075a155d3fa334230a691cf085d463215aead3c417f08f931ccbbe6a1"
      "321b0000006b483045022100cdeb3add0650fc8b8351512d6df17fbfc602ec484d8c5c6a"
      "7800d3cd5e71fac60220481616b00629c8c7a81a10397db2942ce21676eb5fd4be20bc44"
      "31754c657f6001210217630c3bfab894e6e7322ae6d104d9b9aff58ae8ce4c30f45e9903"
      "51ba3668a4ffffffff9b4139cb7a02366cd228514a89b46cb55fc8183b03c3f9b8fe583e"
      "fe93f28958420000006b483045022100a19519b0d4af6a5f50301c36d620c7c2b5fea705"
      "7874a184c03db88196933fd4022070e559e34fe80e17a6d9e4b400d183c22af094684004"
      "1220a0965487964b46f301210334f414ce378f5f24a128dbf34a1be8d8a1b863e665f782"
      "99a52f06991be8406dffffffff4b98c9a61ad1b2070ebbcfc2d616824ff3259c5c0d09b8"
      "1f38975446f12e57cb3a0000006b483045022100e9b10baf0226b7394474142b87edcd26"
      "fef74f6fcb6b82833559e95481a51f5802204db8a3f7962f1e731b815f01807ef16b6d48"
      "bd73a40e6534d8e4abc1d72a2554012103e7dcb93f93afdf17b60e00b21f8a283e8140f5"
      "3619970daf7d3b5934230c420dfffffffff63acaa7893f29aa069dbbd6c74bdb9f597d70"
      "3e0f9a99d52b5930edd568ff54000000006b483045022100a3b9578999c1fb5d07c047c9"
      "6569476a73cc8684b1b6f1461cdeaa81de58e631022049cb3e73977605aad472e470a995"
      "9ab913f2b101c268b8cddadd4bca16db4bf10121033011c8839eb82d1c37596bee611cf7"
      "0564d21fb38ce0f4535c4d1d5e53dad958ffffffffd5c0e501e2045ba6fed26aba945e0d"
      "6333d2ca555a60e4c1c920f3e27030bc17400000006b483045022100bbf7e176a402c223"
      "32ab21c3aa0a5ca05bb9efe0d2a3f3797d5174873d4fc5e8022061aba78025eb56e7a8e1"
      "663c68bb63ff0d0d7db0b6fc5064e021994a59cf8c0c012103ca9827e71289cfc3581340"
      "ae413ae7860af88829b05e30083cee496cb9cdff39ffffffff64cca9bfba56c7f2dfb497"
      "53a7f90fe8e2b2121f3d1cd9221a1032425095ed3a8f0000006b483045022100b275537b"
      "a5e33b3511925e68df84d62487640dd813cf179cf65abd2920f4c29f022072523568c14f"
      "50881bd527d352847a343d65f5620a0df6e6683c8a793ff77a240121030ab067dab80cd5"
      "89f30e36d45902e933abc270304829b4e7323ba51695f8d331ffffffff01f0270f000000"
      "0000160014854fe623a8a6a4c76779b57c3895ed2e0962647400000000",
      1858,
      raw_txn);
  // only fill necessary values
  btc_txn_input_t input = {.prev_output_index = 0,
                           .value = 993264,
                           .script_pub_key = {
                               .size = 22,
                           }};
  hex_string_to_byte_array(
      // invalid txn hash test. valid txn hash/id:
      // 21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b03
      "21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b04",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  // 21706dfac590a74e7d083ad60e790c3a1775a4818afd7aa3ddf1a3d76dc16b04
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));
  hex_string_to_byte_array("0014854fe623a8a6a4c76779b57c3895ed2e09626474",
                           44,
                           input.script_pub_key.bytes);

  TEST_ASSERT_EQUAL_INT(2, btc_verify_input_utxo(raw_txn, 929, &input));
}

TEST(btc_txn_helper_test, btc_txn_helper_transaction_weight_legacy1) {
  /* Test data source:
   * https://blockchain.info/rawtx/4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b?format=json
   */
  btc_txn_context_t txn_ctx = {
      .metadata =
          {
              .version = 1,
              .output_count = 1,
              .input_count = 1,
              .locktime = 0,
              .sighash = 1,
          },
      .inputs = NULL,
      .outputs = NULL,
  };
  txn_ctx.inputs = (btc_txn_input_t *)malloc(sizeof(btc_txn_input_t));
  txn_ctx.outputs =
      (btc_sign_txn_output_t *)malloc(sizeof(btc_sign_txn_output_t));

  btc_txn_input_t *input = &txn_ctx.inputs[0];
  btc_sign_txn_output_t *output = &txn_ctx.outputs[0];

  input->script_pub_key.size = 77;
  input->value = 5000000000;
  input->sequence = 4294967295;
  input->prev_output_index = 4294967295;
  output->value = 5000000000;
  output->script_pub_key.size = 67;
  hex_string_to_byte_array(
      "04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e6365"
      "6c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
      "62616e6b73",
      154,
      input->script_pub_key.bytes);
  hex_string_to_byte_array(
      "4104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6"
      "bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac",
      134,
      output->script_pub_key.bytes);

  TEST_ASSERT_EQUAL_UINT(816, get_transaction_weight(&txn_ctx));

  free(txn_ctx.inputs);
  free(txn_ctx.outputs);
}

TEST(btc_txn_helper_test, btc_txn_helper_transaction_weight_legacy2) {
  /* Test data source:
   * https://blockchain.info/rawtx/6f7cf9580f1c2dfb3c4d5d043cdbb128c640e3f20161245aa7372e9666168516?format=json
   */
  btc_txn_context_t txn_ctx = {
      .metadata =
          {
              .version = 1,
              .output_count = 1,
              .input_count = 2,
              .locktime = 0,
              .sighash = 1,
          },
      .inputs = NULL,
      .outputs = NULL,
  };
  txn_ctx.inputs = (btc_txn_input_t *)malloc(2 * sizeof(btc_txn_input_t));
  txn_ctx.outputs =
      (btc_sign_txn_output_t *)malloc(sizeof(btc_sign_txn_output_t));

  btc_txn_input_t *input = &txn_ctx.inputs[0];
  btc_sign_txn_output_t *output = &txn_ctx.outputs[0];

  input->script_pub_key.size = 74;
  input->value = 5000000000;
  input->sequence = 4294967295;
  input->prev_output_index = 0;
  hex_string_to_byte_array("493046022100e26d9ff76a07d68369e5782be3f8532d25ecc8a"
                           "dd58ee256da6c550b52e8006b022100b4431f5a9a4dcb51cbdc"
                           "aae935218c0ae4cfc8aa903fe4e5bac4c208290b7d5d01",
                           148,
                           input->script_pub_key.bytes);

  input = &txn_ctx.inputs[1];
  input->script_pub_key.size = 74;
  input->value = 5000000000;
  input->sequence = 4294967295;
  input->prev_output_index = 0;
  hex_string_to_byte_array("493046022100a2ab7cdc5b67aca032899ea1b262f6e8181060f"
                           "5a34ee667a82dac9c7b7db4c3022100911bc945c4b435df8227"
                           "466433e56899fbb65833e4853683ecaa12ee840d16bf01",
                           148,
                           input->script_pub_key.bytes);

  output->value = 10000000000;
  output->script_pub_key.size = 25;
  hex_string_to_byte_array("76a91412ab8dc588ca9d5787dde7eb29569da63c3a238c88ac",
                           50,
                           output->script_pub_key.bytes);

  TEST_ASSERT_EQUAL_UINT(1096, get_transaction_weight(&txn_ctx));

  free(txn_ctx.inputs);
  free(txn_ctx.outputs);
}

TEST(btc_txn_helper_test, btc_txn_helper_transaction_weight_segwit1) {
  /* Test data source:
   * https://blockchain.info/rawtx/484856dd051827cb459375b77033c8f2d5bed6971edb7ddcc4ae95c97307f5d3?format=json
   */
  btc_txn_context_t txn_ctx = {
      .metadata =
          {
              .version = 1,
              .output_count = 1,
              .input_count = 1,
              .locktime = 0,
              .sighash = 1,
          },
      .inputs = NULL,
      .outputs = NULL,
  };
  txn_ctx.inputs = (btc_txn_input_t *)malloc(sizeof(btc_txn_input_t));
  txn_ctx.outputs =
      (btc_sign_txn_output_t *)malloc(sizeof(btc_sign_txn_output_t));

  btc_txn_input_t *input = &txn_ctx.inputs[0];
  btc_sign_txn_output_t *output = &txn_ctx.outputs[0];

  input->script_pub_key.size = 34;
  input->value = 4517;
  input->sequence = 4294967295;
  input->prev_output_index = 0;
  hex_string_to_byte_array(
      "00208b0c8df2e80eda0f4eeb6d6cfb9c1f481f49c4a201585c079be76eac7d5326b8",
      68,
      input->script_pub_key.bytes);

  output->value = 2500;
  output->script_pub_key.size = 34;
  hex_string_to_byte_array(
      "5120e2960e4a0b7162b4c016c35a1285ab94d6a87dcadf8cffb192f8c756c62d93e2",
      68,
      output->script_pub_key.bytes);

  TEST_ASSERT_EQUAL_UINT(688, get_transaction_weight(&txn_ctx));

  free(txn_ctx.inputs);
  free(txn_ctx.outputs);
}

TEST(btc_txn_helper_test, btc_txn_helper_transaction_weight_segwit2) {
  /* Test data source:
   * https://blockchain.info/rawtx/f1457142e99b3ce45a80565237732d8f4ffd251fb70d8283fd7bd6159154e530?format=json
   */
  btc_txn_context_t txn_ctx = {
      .metadata =
          {
              .version = 1,
              .output_count = 1,
              .input_count = 1,
              .locktime = 0,
              .sighash = 1,
          },
      .inputs = NULL,
      .outputs = NULL,
  };
  txn_ctx.inputs = (btc_txn_input_t *)malloc(sizeof(btc_txn_input_t));
  txn_ctx.outputs =
      (btc_sign_txn_output_t *)malloc(sizeof(btc_sign_txn_output_t));

  btc_txn_input_t *input = &txn_ctx.inputs[0];
  btc_sign_txn_output_t *output = &txn_ctx.outputs[0];

  input->script_pub_key.size = 22;
  input->value = 382055;
  input->sequence = 0;
  input->prev_output_index = 6;
  hex_string_to_byte_array("00149031c2a9996e57eb787967e794358e823595b666",
                           44,
                           input->script_pub_key.bytes);

  output->value = 380699;
  output->script_pub_key.size = 25;
  hex_string_to_byte_array("76a914d2192be350c2e4b16d4905d0c356080c331e34de88ac",
                           50,
                           output->script_pub_key.bytes);

  TEST_ASSERT_EQUAL_UINT(449, get_transaction_weight(&txn_ctx));

  free(txn_ctx.inputs);
  free(txn_ctx.outputs);
}

TEST(btc_txn_helper_test, btc_txn_helper_transaction_weight_mixed) {
}

TEST(btc_txn_helper_test, btc_txn_helper_format_value_0sat) {
  uint64_t value_in_sat = 0;
  char msg[100] = "";
  const char expected_msg[] = "0 BTC";
  format_value(value_in_sat, msg, sizeof(msg));
  TEST_ASSERT_EQUAL_STRING(expected_msg, msg);
}

TEST(btc_txn_helper_test, btc_txn_helper_format_value_1sat) {
  uint64_t value_in_sat = 1;
  char msg[100] = "";
  const char expected_msg[] = "0.00000001 BTC";
  format_value(value_in_sat, msg, sizeof(msg));
  TEST_ASSERT_EQUAL_STRING(expected_msg, msg);
}

TEST(btc_txn_helper_test, btc_txn_helper_format_value_1001sat) {
  uint64_t value_in_sat = 1001;
  char msg[100] = "";
  const char expected_msg[] = "0.00001001 BTC";
  format_value(value_in_sat, msg, sizeof(msg));
  TEST_ASSERT_EQUAL_STRING(expected_msg, msg);
}

TEST(btc_txn_helper_test, btc_txn_helper_format_value_1btc) {
  uint64_t value_in_sat = SATOSHI_PER_BTC;
  char msg[100] = "";
  const char expected_msg[] = "1 BTC";
  format_value(value_in_sat, msg, sizeof(msg));
  TEST_ASSERT_EQUAL_STRING(expected_msg, msg);
}
