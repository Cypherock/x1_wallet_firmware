/**
 * @file    evm_eip1559_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for EVM EIP1559
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

TEST_GROUP(evm_eip1559_tests);

extern evm_txn_context_t *txn_context;

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(evm_eip1559_tests) {
  g_evm_app = get_eth_app_desc()->app_config;
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(evm_eip1559_tests) {
}

TEST(evm_eip1559_tests, evm_version_1_acl1) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "01f87b018064830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480f85bf8"
      "5994b0ee076d7779a6ce152283f009f4c32b5f887567f842a00000000000000000000000"
      "000000000000000000000000000000000000000000a00000000000000000000000000000"
      "000000000000000000000000000000000001",
      252,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 126, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

TEST(evm_eip1559_tests, evm_version_1_acl2) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "01f6018064830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480d7d694b0"
      "ee076d7779a6ce152283f009f4c32b5f887567c0",
      112,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 56, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

TEST(evm_eip1559_tests, evm_version_1_acl3) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "01df018064830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480c0",
      66,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 33, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

/**
 * Test input generated from:
 * ethers.Transaction.from({
    chainId: 1,
    to: '0xb0ee076d7779a6ce152283f009f4c32b5f88756c',
    gasLimit: 100000,
    maxFeePerGas: 100,
    maxPriorityFeePerGas: 10,
    value: 100,
    nonce: 0,
    accessList: [
      {
        "address": "0xb0ee076d7779a6ce152283f009f4c32b5f887567",
        "storageKeys": [
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000001"
        ]
      }
    ]
  }).unsignedSerialized
*/
TEST(evm_eip1559_tests, evm_version_2_acl1) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "02f87c01800a64830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480f85b"
      "f85994b0ee076d7779a6ce152283f009f4c32b5f887567f842a000000000000000000000"
      "00000000000000000000000000000000000000000000a000000000000000000000000000"
      "00000000000000000000000000000000000001",
      254,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 127, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

/**
 * Test input generated from:
 * ethers.Transaction.from({
    chainId: 1,
    to: '0xb0ee076d7779a6ce152283f009f4c32b5f88756c',
    gasLimit: 100000,
    maxFeePerGas: 100,
    maxPriorityFeePerGas: 10,
    value: 100,
    nonce: 0,
    accessList: [
      {
        "address": "0xb0ee076d7779a6ce152283f009f4c32b5f887567",
        "storageKeys": []
      }
    ]
  }).unsignedSerialized
*/
TEST(evm_eip1559_tests, evm_version_2_acl2) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "02f701800a64830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480d7d694"
      "b0ee076d7779a6ce152283f009f4c32b5f887567c0",
      114,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 57, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

/**
 * Test input generated from:
 *  ethers.Transaction.from({
    chainId: 1,
    to: '0xb0ee076d7779a6ce152283f009f4c32b5f88756c',
    gasLimit: 100000,
    maxFeePerGas: 100,
    maxPriorityFeePerGas: 10,
    value: 100,
    nonce: 0,
    accessList: []
  }).unsignedSerialized
*/
TEST(evm_eip1559_tests, evm_version_2_acl3) {
  uint8_t buffer[500] = {0};
  hex_string_to_byte_array(
      "02e001800a64830186a094b0ee076d7779a6ce152283f009f4c32b5f88756c6480c0",
      68,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 34, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}

/**
 * Transaction reference:
 * https://etherscan.io/tx/0x04848b915bc1705654178ad89be4f37847362c0aae461d90dfb3f748ec752e3f
 * Test input generated from:
 * ethers.Transaction.from('0x02f905730135847735940085037c4609008305503d943fc91a3afd70395cd496c647d5a6cc9d4b2b7fad80b905053593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000018f09984b4700000000000000000000000000000000000000000000000000000000000000040a08060400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000800000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000034000000000000000000000000000000000000000000000000000000000000003c000000000000000000000000000000000000000000000000000000000000001600000000000000000000000007463c17e3d91b17d9cd0b1e9fdfa18d4ddc3cf0e000000000000000000000000ffffffffffffffffffffffffffffffffffffffff00000000000000000000000000000000000000000000000000000000664eeead00000000000000000000000000000000000000000000000000000000000000000000000000000000000000003fc91a3afd70395cd496c647d5a6cc9d4b2b7fad00000000000000000000000000000000000000000000000000000000662768b500000000000000000000000000000000000000000000000000000000000000e000000000000000000000000000000000000000000000000000000000000000417187fb68fb6c386eacd96b45e0ba028cace8c461e70cd0808550888aef6486972e7c57b6515472efaa0c6935f96275a6a9b2acb157691bfb03e5f0781e35aa881c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000012000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000062708e1e3657f47423c4feb00000000000000000000000000000000000000000000000000479904b7d8673000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000030000000000000000000000007463c17e3d91b17d9cd0b1e9fdfa18d4ddc3cf0e000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc200000000000000000000000085d19fb57ca7da715695fcf347ca2169144523a7000000000000000000000000000000000000000000000000000000000000006000000000000000000000000085d19fb57ca7da715695fcf347ca2169144523a700000000000000000000000037a8f295612602f2774d331e562be9e61b83a3270000000000000000000000000000000000000000000000000000000000000019000000000000000000000000000000000000000000000000000000000000006000000000000000000000000085d19fb57ca7da715695fcf347ca2169144523a700000000000000000000000092db1dc7e7d6ca52eaf095447cb3e7b1b26a6db400000000000000000000000000000000000000000000000000476b32257738da0bc001a0e65b900a59df9d0deb548d575e0d127ad430afd333ee680cc7ab4208ddb2600fa0676640cfa1b6122953422c7591fcf3b0354e3b848a7f5e397487481deddbb616').unsignedSerialized
 */
TEST(evm_eip1559_tests, evm_version_2_blind_signing) {
  uint8_t buffer[1400] = {0};
  hex_string_to_byte_array(
      "02f905300135847735940085037c4609008305503d943fc91a3afd70395cd496c647d5a6"
      "cc9d4b2b7fad80b905053593564c00000000000000000000000000000000000000000000"
      "000000000000000000600000000000000000000000000000000000000000000000000000"
      "0000000000a00000000000000000000000000000000000000000000000000000018f0998"
      "4b4700000000000000000000000000000000000000000000000000000000000000040a08"
      "060400000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000400000000000000000000"
      "000000000000000000000000000000000000000000800000000000000000000000000000"
      "000000000000000000000000000000000200000000000000000000000000000000000000"
      "000000000000000000000000034000000000000000000000000000000000000000000000"
      "000000000000000003c00000000000000000000000000000000000000000000000000000"
      "0000000001600000000000000000000000007463c17e3d91b17d9cd0b1e9fdfa18d4ddc3"
      "cf0e000000000000000000000000ffffffffffffffffffffffffffffffffffffffff0000"
      "0000000000000000000000000000000000000000000000000000664eeead000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "00003fc91a3afd70395cd496c647d5a6cc9d4b2b7fad0000000000000000000000000000"
      "0000000000000000000000000000662768b5000000000000000000000000000000000000"
      "00000000000000000000000000e000000000000000000000000000000000000000000000"
      "000000000000000000417187fb68fb6c386eacd96b45e0ba028cace8c461e70cd0808550"
      "888aef6486972e7c57b6515472efaa0c6935f96275a6a9b2acb157691bfb03e5f0781e35"
      "aa881c000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000120000000000000"
      "000000000000000000000000000000000000000000000000000200000000000000000000"
      "00000000000000000000062708e1e3657f47423c4feb0000000000000000000000000000"
      "0000000000000000000000479904b7d86730000000000000000000000000000000000000"
      "00000000000000000000000000a000000000000000000000000000000000000000000000"
      "000000000000000000010000000000000000000000000000000000000000000000000000"
      "0000000000030000000000000000000000007463c17e3d91b17d9cd0b1e9fdfa18d4ddc3"
      "cf0e000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc20000"
      "0000000000000000000085d19fb57ca7da715695fcf347ca2169144523a7000000000000"
      "000000000000000000000000000000000000000000000000006000000000000000000000"
      "000085d19fb57ca7da715695fcf347ca2169144523a700000000000000000000000037a8"
      "f295612602f2774d331e562be9e61b83a327000000000000000000000000000000000000"
      "000000000000000000000000001900000000000000000000000000000000000000000000"
      "0000000000000000006000000000000000000000000085d19fb57ca7da715695fcf347ca"
      "2169144523a700000000000000000000000092db1dc7e7d6ca52eaf095447cb3e7b1b26a"
      "6db400000000000000000000000000000000000000000000000000476b32257738da0bc"
      "0",
      2664,
      buffer);
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  TEST_ASSERT_TRUE(evm_decode_unsigned_txn(buffer, 1332, txn_context));
  TEST_ASSERT_TRUE(evm_validate_unsigned_txn(txn_context));
}
