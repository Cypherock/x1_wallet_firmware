/**
 * @file    near_helper_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for NEAR helper functions
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

#include "near.h"
#include "near_context.h"
#include "near_helpers.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(near_helper_test);

TEST_SETUP(near_helper_test) {
  return;
}

TEST_TEAR_DOWN(near_helper_test) {
  return;
}

TEST(near_helper_test, near_helper_send_decoder_transfer_action) {
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "400000006165396130393365363930376538366439373730663339623063386265316463"
      "366663333861316236383930326664396432376164353638316130383439613100ae9a09"
      "3e6907e86d9770f39b0c8be1dc6fc38a1b68902fd9d27ad5681a0849a102a40d43345400"
      "004000000061653961303933653639303765383664393737306633396230633862653164"
      "633666633338613162363839303266643964323761643536383161303834396131eba9ab"
      "a955a871062ff51a1544d924aeb721f00324cab7979da9e5da258a459d0100000003400f"
      "5de8db500daeff4d120000000000",
      460,
      raw_txn);

  near_unsigned_txn utxn = {0};
  TEST_ASSERT_TRUE(near_parse_transaction(raw_txn, 230, &utxn));

  // Verify that the txn contains an action of type 'TRANSFER'
  TEST_ASSERT_EQUAL_UINT8(NEAR_ACTION_TRANSFER, utxn.actions_type);

  // Verify receiver address
  const char expected_receiver[] =
      "ae9a093e6907e86d9770f39b0c8be1dc6fc38a1b68902fd9d27ad5681a0849a1";
  TEST_ASSERT_EQUAL_STRING_LEN(
      expected_receiver, (char *)utxn.receiver, strlen(expected_receiver));

  // Verify transfer amount
  const char expected_amount[] = "Verify amount\n22.129003433752526485\nNEAR";
  char amount_decimal_string[100] = "";
  get_amount_string(utxn.action.transfer.amount,
                    amount_decimal_string,
                    sizeof(amount_decimal_string));

  TEST_ASSERT_EQUAL_STRING(expected_amount, amount_decimal_string);
}

TEST(near_helper_test,
     near_helper_send_decoder_transfer_action_to_explicit_account) {
  uint8_t raw_txn[300] = {0};
  hex_string_to_byte_array(
      "120000006379706865726f636b686f646c2e6e65617200ae9a093e6907e86d9770f39b0c"
      "8be1dc6fc38a1b68902fd9d27ad5681a0849a1819aaeab815900000e0000006379706865"
      "726f636b2e6e6561726c9db75a59d0c3ad6b57db90865045b41a98690e3a7fe61cdfda87"
      "413cb5d19b010000000300788799cb4b5c6c310a000000000000",
      260,
      raw_txn);

  near_unsigned_txn utxn = {0};
  TEST_ASSERT_TRUE(near_parse_transaction(raw_txn, 130, &utxn));

  // Verify that the txn contains an action of type 'TRANSFER'
  TEST_ASSERT_EQUAL_UINT8(NEAR_ACTION_TRANSFER, utxn.actions_type);

  // Verify receiver address
  const char expected_receiver[] = "cypherock.near";
  TEST_ASSERT_EQUAL_STRING_LEN(
      expected_receiver, (char *)utxn.receiver, strlen(expected_receiver));

  // Verify transfer amount
  const char expected_amount[] = "Verify amount\n0.0481353634875\nNEAR";
  char amount_decimal_string[100] = "";
  get_amount_string(utxn.action.transfer.amount,
                    amount_decimal_string,
                    sizeof(amount_decimal_string));

  TEST_ASSERT_EQUAL_STRING(expected_amount, amount_decimal_string);
}

TEST(near_helper_test,
     near_helper_send_decoder_function_call_explicit_account) {
  uint8_t raw_txn[350] = {0};
  hex_string_to_byte_array(
      "400000006165396130393365363930376538366439373730663339623063386265316463"
      "366663333861316236383930326664396432376164353638316130383439613100ae9a09"
      "3e6907e86d9770f39b0c8be1dc6fc38a1b68902fd9d27ad5681a0849a102a40d43345400"
      "00040000006e656172dcc303d157e62d1b4cba98a91a0826efebd14ebec5effc58bfe9c4"
      "1a3ed9cb9701000000020e0000006372656174655f6163636f756e74700000007b226e65"
      "775f6163636f756e745f6964223a22686f646c5f746573745f313233342e6e656172222c"
      "226e65775f7075626c69635f6b6579223a22656432353531393a436b61417178585a4653"
      "75783459427376716b693571696662354e6678787569414e6278595476746e356657227d"
      "00c06e31d9100100000080f64ae1c7022d15000000000000",
      624,
      raw_txn);

  near_unsigned_txn utxn = {0};
  TEST_ASSERT_TRUE(near_parse_transaction(raw_txn, 312, &utxn));

  // Verify that the txn contains an action of type 'FUNCTION_CALL'
  TEST_ASSERT_EQUAL_UINT8(NEAR_ACTION_FUNCTION_CALL, utxn.actions_type);

  // Verify that the method in the function call is add account
  const char expected_method[] = "create_account";
  TEST_ASSERT_EQUAL_STRING_LEN(expected_method,
                               (char *)utxn.action.fn_call.method_name,
                               utxn.action.fn_call.method_name_length);

  // Verify the account owner
  const char expected_account_owner[] =
      "ae9a093e6907e86d9770f39b0c8be1dc6fc38a1b68902fd9d27ad5681a0849a1";
  TEST_ASSERT_EQUAL_STRING_LEN(
      expected_account_owner, (char *)utxn.signer, utxn.signer_id_length);

  // Verify new account ID
  const char expected_account_name[] = "hodl_test_1234.near";
  char account[NEAR_ACC_ID_MAX_LEN + 1] = {0};
  near_get_new_account_id_from_fn_args((const char *)utxn.action.fn_call.args,
                                       utxn.action.fn_call.args_length,
                                       account);
  TEST_ASSERT_EQUAL_STRING(expected_account_name, account);

  // Verify amount string
  const char expected_amount[] = "Verify amount\n0.1\nNEAR";
  char amount_decimal_string[100] = "";
  get_amount_string(utxn.action.fn_call.deposit,
                    amount_decimal_string,
                    sizeof(amount_decimal_string));
  TEST_ASSERT_EQUAL_STRING(expected_amount, amount_decimal_string);
}

TEST(near_helper_test, near_helper_sign_txn) {
  uint8_t raw_txn[350] = {0};
  hex_string_to_byte_array(
      "400000006165396130393365363930376538366439373730663339623063386265316463"
      "366663333861316236383930326664396432376164353638316130383439613100ae9a09"
      "3e6907e86d9770f39b0c8be1dc6fc38a1b68902fd9d27ad5681a0849a102a40d43345400"
      "00040000006e656172dcc303d157e62d1b4cba98a91a0826efebd14ebec5effc58bfe9c4"
      "1a3ed9cb9701000000020e0000006372656174655f6163636f756e74700000007b226e65"
      "775f6163636f756e745f6964223a22686f646c5f746573745f313233342e6e656172222c"
      "226e65775f7075626c69635f6b6579223a22656432353531393a436b61417178585a4653"
      "75783459427376716b693571696662354e6678787569414e6278595476746e356657227d"
      "00c06e31d9100100000080f64ae1c7022d15000000000000",
      624,
      raw_txn);

  // Replicate logic from function sign_txn in near_txn.c, but with a hardcoded
  // random secret key/ public key pair
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(raw_txn, 312, digest);

  // Fix secret key and ensure public key generation is correct
  ed25519_secret_key secret_key = {0};
  hex_string_to_byte_array(
      "6D597A1814BB385FE4D69D4D1B2CB3DEE14EC92617D8E1C3476679DDF32CBDB0",
      64,
      secret_key);

  ed25519_public_key public_key = {0};
  ed25519_publickey(secret_key, public_key);

  ed25519_public_key expected_public_key = {0};
  hex_string_to_byte_array(
      "A98F01C375A0114D7AFC16122FE4ED0D297AC490779209FD7174B984F02760E7",
      64,
      expected_public_key);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_public_key, public_key, sizeof(expected_public_key));

  // Ensure signature is correct
  ed25519_signature signature = {0};
  ed25519_sign(digest, sizeof(digest), secret_key, public_key, signature);

  ed25519_signature expected_signature = {0};
  hex_string_to_byte_array(
      "E61DC60F6D148FB32F64C58CEEB2E9D7152AB8EEE28243810C27E8810BC5859E93DA8F5F"
      "420637161C3A864691DDF3B11456062E1293A551A1A7E4BA8224630F",
      128,
      expected_signature);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, signature, sizeof(expected_signature));
}
