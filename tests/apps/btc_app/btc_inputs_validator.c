/**
 * @file    btc_inputs_validator_testss.c
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

#include "btc_inputs_validator.h"

#include <stdint.h>

#include "byte_stream.h"
#include "unity_fixture.h"
#include "utils.h"

static uint8_t *data_ptr = NULL;
static uint64_t data_total_size = 0;
static uint64_t global_offset = 0;
static bool throw_error = false;

static byte_stream_writer_status_e generic_writer(byte_stream_t *stream) {
  global_offset += stream->capacity;
  if (throw_error || global_offset >= data_total_size) {
    return BYTE_STREAM_WRITER_UNKNOWN_ERROR;
  }

  stream->capacity = stream->capacity < (data_total_size - global_offset)
                         ? stream->capacity
                         : (data_total_size - global_offset);

  stream->stream_pointer = data_ptr + global_offset;
  stream->offset = 0;

  return BYTE_STREAM_WRITER_SUCCESS;
}

TEST_GROUP(btc_inputs_validator_tests);

TEST_SETUP(btc_inputs_validator_tests) {
  data_ptr = NULL;
  data_total_size = 0;
  global_offset = 0;
  throw_error = false;
}

TEST_TEAR_DOWN(btc_inputs_validator_tests) {
  data_ptr = NULL;
  data_total_size = 0;
  global_offset = 0;
  throw_error = false;
}

TEST(btc_inputs_validator_tests, test_null_stream) {
  byte_stream_t *stream = NULL;
  btc_sign_txn_input_t input = {0};
  btc_validation_error_e result = btc_validate_inputs(stream, &input);
  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_PARAMS, result);
}

TEST(btc_inputs_validator_tests, test_null_input) {
  byte_stream_t *stream = {0};
  btc_validation_error_e result = btc_validate_inputs(stream, NULL);
  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_PARAMS, result);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2pk) {
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
  btc_sign_txn_input_t input = {.prev_output_index = 0,
                                .value = 5000000000,
                                .script_pub_key = {
                                    .size = 67,
                                }};
  hex_string_to_byte_array(
      "0437cd7f8525ceed2324359c2d0ba26006d92d856a9c20fa0241106ee5a597c9",
      64,
      input.prev_txn_hash);
  // revere order of txn-id:
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));

  hex_string_to_byte_array(
      "410411db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0"
      "eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac",
      134,
      input.script_pub_key.bytes);

  data_ptr = raw_txn;
  data_total_size = 268 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 10,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2pk_fail) {
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
  btc_sign_txn_input_t input = {
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

  data_ptr = raw_txn;
  data_total_size = 268 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 10,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_OUTPUT_VALUE, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2pkh) {
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
  btc_sign_txn_input_t input = {.prev_output_index = 0,
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

  data_ptr = raw_txn;
  data_total_size = 448 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 100,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2pkh_fail) {
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
  btc_sign_txn_input_t input = {
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

  data_ptr = raw_txn;
  data_total_size = 448 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 100,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_OUTPUT_VALUE, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2wpkh) {
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
  btc_sign_txn_input_t input = {.prev_output_index = 0,
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

  data_ptr = raw_txn;
  data_total_size = 1858 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 100,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2wpkh_fail) {
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
  btc_sign_txn_input_t input = {.prev_output_index = 0,
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

  data_ptr = raw_txn;
  data_total_size = 1858 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 500,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_TX_HASH, status);
}

TEST(btc_inputs_validator_tests, btc_txn_helper_verify_input_p2wpkh_in_p2sh) {
  /* Test data source: Bip143
   * https://blockstream.info/testnet/tx/b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2?expand
   */
  uint8_t raw_txn[2000] = {0};
  hex_string_to_byte_array(
      "0200000000010258afb1ece76f01c24f4935f453d210518163cb1d0383eaec331b202ebe"
      "b5e389"
      "0000000017160014a76cad25cb569bb47305513ebedd6011dc419deeffffffff2b3682b3"
      "592588"
      "5001f0e321df28d4ac675a9cbbccef2a69533bea7c5e5ad2c40000000017160014a76cad"
      "25cb56"
      "9bb47305513ebedd6011dc419deeffffffff02941100000000000017a914bd7aabdeeef2"
      "11b1bd"
      "ad7218e14fea6c032101c087f22f00000000000017a914eaf97514c5ac1e41e413502e97"
      "ae42eb"
      "f27ace3a870247304402206e038f4712541d699697ed55efc41219df4f244fc72caa5edd"
      "653837"
      "f6555f6f02201cd8ea15b65fda17992abafaed86e066c3271ac16b9c46c54c2192438843"
      "dd0401"
      "21029f75e1ef6b04e004a308b1f59215a8a3a5b7958bbcf184cc24ba7ab6574448780248"
      "304502"
      "2100d15ce61648edc28b8b5a3531b80a1e8fc3b3eebe7d3fc4ca962cb04afc770dda0220"
      "7c7eaf8"
      "82d7fac45d2752f20e48d2f896715cbc5a3b0f5de3e19fea0da99beac0121029f75e1ef6"
      "b04e004"
      "a308b1f59215a8a3a5b7958bbcf184cc24ba7ab65744487800000000",
      838,
      raw_txn);
  // only fill necessary values
  btc_sign_txn_input_t input[] = {{
                                      .value = 4500,
                                      .prev_output_index = 0x00000000,
                                      .script_pub_key = {.size = 23},
                                  },
                                  {
                                      .value = 12274,
                                      .prev_output_index = 0x00000001,
                                      .script_pub_key = {.size = 23},
                                  }};

  hex_string_to_byte_array(
      "b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2",
      64,
      input[0].prev_txn_hash);
  // Reverse order of txn-id:
  // A244A293475AC245DEE8B3343F51EE2296BAB24FCC3FE0A49628DF605E2295B0
  cy_reverse_byte_array(input[0].prev_txn_hash, sizeof(input[0].prev_txn_hash));

  hex_string_to_byte_array(
      "b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2",
      64,
      input[1].prev_txn_hash);
  // Reverse order of txn-id:
  // A244A293475AC245DEE8B3343F51EE2296BAB24FCC3FE0A49628DF605E2295B0
  cy_reverse_byte_array(input[1].prev_txn_hash, sizeof(input[1].prev_txn_hash));

  hex_string_to_byte_array("a914bd7aabdeeef211b1bdad7218e14fea6c032101c087",
                           46,
                           input[0].script_pub_key.bytes);
  hex_string_to_byte_array("a914eaf97514c5ac1e41e413502e97ae42ebf27ace3a87",
                           46,
                           input[1].script_pub_key.bytes);

  data_ptr = raw_txn;
  data_total_size = 838 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 100,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);

  data_ptr = raw_txn;
  data_total_size = 838 / 2;
  global_offset = 0;

  stream.stream_pointer = raw_txn;
  stream.offset = 0;
  stream.capacity = 100;

  status = btc_validate_inputs(&stream, input + 1);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);
}

TEST(btc_inputs_validator_tests,
     btc_txn_helper_verify_input_p2wpkh_in_p2sh_fail) {
  /* Test data source: Bip143
   * https://blockstream.info/testnet/tx/b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2?expand
   */
  uint8_t raw_txn[2000] = {0};
  hex_string_to_byte_array(
      "0200000000010258afb1ece76f01c24f4935f453d210518163cb1d0383eaec331b202ebe"
      "b5e389"
      "0000000017160014a76cad25cb569bb47305513ebedd6011dc419deeffffffff2b3682b3"
      "592588"
      "5001f0e321df28d4ac675a9cbbccef2a69533bea7c5e5ad2c40000000017160014a76cad"
      "25cb56"
      "9bb47305513ebedd6011dc419deeffffffff02941100000000000017a914bd7aabdeeef2"
      "11b1bd"
      "ad7218e14fea6c032101c087f22f00000000000017a914eaf97514c5ac1e41e413502e97"
      "ae42eb"
      "f27ace3a870247304402206e038f4712541d699697ed55efc41219df4f244fc72caa5edd"
      "653837"
      "f6555f6f02201cd8ea15b65fda17992abafaed86e066c3271ac16b9c46c54c2192438843"
      "dd0401"
      "21029f75e1ef6b04e004a308b1f59215a8a3a5b7958bbcf184cc24ba7ab6574448780248"
      "304502"
      "2100d15ce61648edc28b8b5a3531b80a1e8fc3b3eebe7d3fc4ca962cb04afc770dda0220"
      "7c7eaf8"
      "82d7fac45d2752f20e48d2f896715cbc5a3b0f5de3e19fea0da99beac0121029f75e1ef6"
      "b04e004"
      "a308b1f59215a8a3a5b7958bbcf184cc24ba7ab65744487800000000",
      838,
      raw_txn);
  // only fill necessary values
  btc_sign_txn_input_t input[] = {{
                                      .value = 4500,
                                      .prev_output_index = 0x00000000,
                                      .script_pub_key = {.size = 23},
                                  },
                                  {
                                      .value = 12274,
                                      .prev_output_index = 0x00000001,
                                      .script_pub_key = {.size = 23},
                                  }};

  hex_string_to_byte_array(
      // invalid txn hash test. valid txn hash/id:
      // b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2
      "b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a3",
      64,
      input[0].prev_txn_hash);
  // Reverse order of txn-id:
  // A344A293475AC245DEE8B3343F51EE2296BAB24FCC3FE0A49628DF605E2295B0
  cy_reverse_byte_array(input[0].prev_txn_hash, sizeof(input[0].prev_txn_hash));

  hex_string_to_byte_array(
      // invalid txn hash test. valid txn hash/id:
      // b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a2
      "b095225e60df2896a4e03fcc4fb2ba9622ee513f34b3e8de45c25a4793a244a3",
      64,
      input[1].prev_txn_hash);
  // Reverse order of txn-id:
  // A344A293475AC245DEE8B3343F51EE2296BAB24FCC3FE0A49628DF605E2295B0
  cy_reverse_byte_array(input[1].prev_txn_hash, sizeof(input[1].prev_txn_hash));

  hex_string_to_byte_array("a914bd7aabdeeef211b1bdad7218e14fea6c032101c087",
                           46,
                           input[0].script_pub_key.bytes);
  hex_string_to_byte_array("a914eaf97514c5ac1e41e413502e97ae42ebf27ace3a87",
                           46,
                           input[1].script_pub_key.bytes);

  data_ptr = raw_txn;
  data_total_size = 838 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 500,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_TX_HASH, status);

  data_ptr = raw_txn;
  data_total_size = 838 / 2;
  global_offset = 0;

  stream.stream_pointer = raw_txn;
  stream.offset = 0;
  stream.capacity = 100;

  status = btc_validate_inputs(&stream, input + 1);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_ERR_INVALID_TX_HASH, status);
}

TEST(btc_inputs_validator_tests,
     btc_validate_inputs_for_a_transaction_with_witness_data) {
  /* Test data source: rawTxn -
   * https://bitcoin.stackexchange.com/questions/114547/how-to-calculate-a-bitcoin-transaction-hash-of-a-coinbase-transaction-in-2022
   */
  uint8_t raw_txn[1000] = {0};
  hex_string_to_byte_array(
      "010000000001010000000000000000000000000000000000000000000000000000000000"
      "000000ffffffff640332610b2cfabe6d6ddcb8d8f2a2ddf5191d8191cfa7aa4fd9d85c52"
      "9f2ce8ed4363c1a8942f14810b10000000f09f909f092f4632506f6f6c2f650000000000"
      "000000000000000000000000000000000000000000000000000000000000050054000000"
      "0000000004b1bbbb26000000001976a914c825a1ecf2a6830c4401620c3a16f1995057c2"
      "ab88ac0000000000000000266a24aa21a9edd02fc86dcb2b66db1a5add17b3660e4046f5"
      "0cde03199425f0944c7becb6546a0000000000000000266a2448617468e62698d5bdd575"
      "72ff76305ed48933e8b787a67df4319ade7d798df03c706edf00000000000000002c6a4c"
      "2952534b424c4f434b3a3c622bf845a23d0ee4927d14fe0455d8e30af6e68e7e3620aa1a"
      "f6270044738b012000000000000000000000000000000000000000000000000000000000"
      "00000000730b053f",
      736,
      raw_txn);
  // only fill necessary values
  btc_sign_txn_input_t input = {.prev_output_index = 0,
                                .value = 649837489,
                                .script_pub_key = {
                                    .size = 25,
                                }};
  hex_string_to_byte_array(
      "aa7f5b5f200feac9364ec40b4a5c0cb5c291f846422dec373a71cf30c55ce856",
      64,
      input.prev_txn_hash);
  cy_reverse_byte_array(input.prev_txn_hash, sizeof(input.prev_txn_hash));

  data_ptr = raw_txn;
  data_total_size = 736 / 2;

  byte_stream_t stream = {
      .stream_pointer = raw_txn,
      .writer = generic_writer,
      .offset = 0,
      .capacity = 128,
  };

  btc_validation_error_e status = btc_validate_inputs(&stream, &input);

  TEST_ASSERT_EQUAL(BTC_VALIDATE_SUCCESS, status);
}
