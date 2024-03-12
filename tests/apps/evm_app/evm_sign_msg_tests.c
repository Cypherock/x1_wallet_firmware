/**
 * @file    evm_sign_msg_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for EVM sign msg helper funcitons
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
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"
#include "utils.h"

TEST_GROUP(evm_sign_msg_test);

static evm_sign_msg_context_t ctx;

static const uint8_t core_msg[] = {10, 2, 8, 1};

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(evm_sign_msg_test) {
  g_evm_app = get_eth_app_desc()->app_config;
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(evm_sign_msg_test) {
  g_evm_app = NULL;
  cy_free();
  memzero(ctx.msg_data, ctx.init.total_msg_size);
  ctx.msg_data = NULL;
}

TEST(evm_sign_msg_test, evm_sign_msg_test_empty_typed_data_hash) {
  evm_query_t query = {
      .which_request = 3,
      .sign_msg = {
          .which_request = 1,
          .initiate = {.derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .message_type = EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA,
                       .total_msg_size = 121}}};
  uint8_t buffer[1024];
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  uint8_t expected_digest[SHA256_DIGEST_LENGTH] = {
      48, 26,  80,  178, 145, 211, 60,  225, 232, 233, 6,
      78, 63,  106, 108, 81,  217, 2,   236, 34,  137, 43,
      80, 213, 138, 191, 99,  87,  198, 164, 85,  65};

  // Set init query to ctx buffer
  memcpy(&ctx.init,
         &query.sign_msg.initiate,
         sizeof(evm_sign_msg_initiate_request_t));

  // Prepare typed data from msg_data
  char *string =
      "0a3a0a06646f6d61696e1007220c454950373132446f6d61696e322020bcc3f8105eea47"
      "d067386e42e60246e89393cd61c512edd1e87688890fb914123b0a076d65737361676510"
      "07220c454950373132446f6d61696e322020bcc3f8105eea47d067386e42e60246e89393"
      "cd61c512edd1e87688890fb914";
  ctx.msg_data = buffer;
  hex_string_to_byte_array(string, ctx.init.total_msg_size * 2, buffer);
  pb_istream_t istream =
      pb_istream_from_buffer(ctx.msg_data, ctx.init.total_msg_size);
  bool result =
      pb_decode(&istream, EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));

#ifdef EVM_SIGN_TYPED_DATA_DISPLAY_TEST
  // Display the typed data
  ui_display_node *display_node = NULL;
  evm_init_typed_data_display_node(&display_node, &(ctx.typed_data));
  while (NULL != display_node) {
    core_scroll_page(display_node->title, display_node->value, evm_send_error);
    display_node = display_node->next;
  }
#endif
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_TRUE(evm_get_msg_data_digest(&ctx, digest));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_digest, digest, SHA256_DIGEST_LENGTH);

  pb_release(EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));
}

TEST(evm_sign_msg_test, evm_sign_msg_test_domain_only_typed_data_hash) {
  evm_query_t query = {
      .which_request = 3,
      .sign_msg = {
          .which_request = 1,
          .initiate = {.derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .message_type = EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA,
                       .total_msg_size = 355}}};
  uint8_t buffer[1024];
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  uint8_t expected_digest[SHA256_DIGEST_LENGTH] = {
      116, 118, 39,  55,  210, 218, 151, 77,  201, 234, 163,
      106, 199, 100, 63,  130, 38,  147, 175, 116, 232, 98,
      80,  99,  235, 157, 156, 36,  198, 0,   10,  52};

  // Set init query to ctx buffer
  memcpy(&ctx.init,
         &query.sign_msg.initiate,
         sizeof(evm_sign_msg_initiate_request_t));

  // Prepare typed data from msg_data
  char *string =
      "0aa3020a06646f6d61696e10071805220c454950373132446f6d61696e3220d87cd6ef79"
      "d4e2b95e15ce8abf732db51ec771f1ca2edccf22a46c729ac564723a1a0a046e616d6510"
      "0318062206737472696e672a065472657a6f723a220a0776657273696f6e1003180b2206"
      "737472696e672a0b546573742076302e302e303a360a07636861696e4964182022077569"
      "6e743235362a200000000000000000000000000000000000000000000000000000000000"
      "0000013a360a11766572696679696e67436f6e7472616374100518142207616464726573"
      "732a14cccccccccccccccccccccccccccccccccccccccc3a350a0473616c741002182022"
      "07627974657333322a200123456789abcdef0123456789abcdef0123456789abcdef0123"
      "456789abcdef123b0a076d6573736167651007220c454950373132446f6d61696e3220d8"
      "7cd6ef79d4e2b95e15ce8abf732db51ec771f1ca2edccf22a46c729ac56472";
  ctx.msg_data = buffer;
  hex_string_to_byte_array(string, ctx.init.total_msg_size * 2, buffer);
  pb_istream_t istream =
      pb_istream_from_buffer(ctx.msg_data, ctx.init.total_msg_size);
  bool result =
      pb_decode(&istream, EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));

#ifdef EVM_SIGN_TYPED_DATA_DISPLAY_TEST
  // Display the typed data
  ui_display_node *display_node = NULL;
  evm_init_typed_data_display_node(&display_node, &(ctx.typed_data));
  while (NULL != display_node) {
    core_scroll_page(display_node->title, display_node->value, evm_send_error);
    display_node = display_node->next;
  }
#endif
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_TRUE(evm_get_msg_data_digest(&ctx, digest));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_digest, digest, SHA256_DIGEST_LENGTH);

  pb_release(EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));
}

TEST(evm_sign_msg_test, evm_sign_msg_test_typed_data_hash) {
  evm_query_t query = {
      .which_request = 3,
      .sign_msg = {
          .which_request = 1,
          .initiate = {.derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .message_type = EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA,
                       .total_msg_size = 572}}};
  uint8_t buffer[1024];
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  uint8_t expected_digest[SHA256_DIGEST_LENGTH] = {
      190, 96,  154, 238, 52,  63,  179, 196, 178, 142, 29,
      249, 230, 50,  252, 166, 79,  207, 174, 222, 32,  240,
      46,  134, 36,  78,  253, 223, 48,  149, 123, 210};

  // Set init query to ctx buffer
  memcpy(&ctx.init,
         &query.sign_msg.initiate,
         sizeof(evm_sign_msg_initiate_request_t));

  // Prepare typed data from msg_data
  char *string =
      "0ae6010a06646f6d61696e10071804220c454950373132446f6d61696e32208b73c3c69b"
      "b8fe3d512ecc4cf759cc79239f7b179b0ffacaa9a75d522b39400f3a1e0a046e616d6510"
      "03180a2206737472696e672a0a4574686572204d61696c3a180a0776657273696f6e1003"
      "18012206737472696e672a01313a360a07636861696e49641820220775696e743235362a"
      "2000000000000000000000000000000000000000000000000000000000000000013a360a"
      "11766572696679696e67436f6e7472616374100518142207616464726573732a14cccccc"
      "cccccccccccccccccccccccccccccccccc12d0020a076d6573736167651007180322044d"
      "61696c3220a0cedeb2dc280ba39b857546d74f5549c3a1d7bdc2dd96bf881f76108e23da"
      "c23a7a0a0466726f6d100718022206506572736f6e3220b9d8c78acf9b987311de6c7b45"
      "bb6a9c8e1bf361fa7fd3467a2163f994c795003a170a046e616d65100318032206737472"
      "696e672a03436f773a2b0a0677616c6c6574100518142207616464726573732a14cd2a3d"
      "9f938e13cd947ec05abc7fe734df8dd8263a780a02746f100718022206506572736f6e32"
      "20b9d8c78acf9b987311de6c7b45bb6a9c8e1bf361fa7fd3467a2163f994c795003a170a"
      "046e616d65100318032206737472696e672a03426f623a2b0a0677616c6c657410051814"
      "2207616464726573732a14bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb3a230a0863"
      "6f6e74656e74731003180b2206737472696e672a0b48656c6c6f2c20426f6221";
  ctx.msg_data = buffer;
  hex_string_to_byte_array(string, ctx.init.total_msg_size * 2, buffer);
  pb_istream_t istream =
      pb_istream_from_buffer(ctx.msg_data, ctx.init.total_msg_size);
  bool result =
      pb_decode(&istream, EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));

#ifdef EVM_SIGN_TYPED_DATA_DISPLAY_TEST
  // Display the typed data
  ui_display_node *display_node = NULL;
  evm_init_typed_data_display_node(&display_node, &(ctx.typed_data));
  while (NULL != display_node) {
    core_scroll_page(display_node->title, display_node->value, evm_send_error);
    display_node = display_node->next;
  }
#endif

  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_TRUE(evm_get_msg_data_digest(&ctx, digest));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_digest, digest, SHA256_DIGEST_LENGTH);

  pb_release(EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(ctx.typed_data));
}

TEST(evm_sign_msg_test, evm_sign_msg_test_personal_sign_hash) {
  evm_query_t query = {
      .which_request = 3,
      .sign_msg = {
          .which_request = 1,
          .initiate = {.derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .message_type = EVM_SIGN_MSG_TYPE_PERSONAL_SIGN,
                       .total_msg_size = 40}}};
  uint8_t buffer[1024];
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  uint8_t expected_digest[SHA256_DIGEST_LENGTH] = {0};

  hex_string_to_byte_array(
      "F8572657F9D0EA800C2EAF259932A95AC445F747ECC9AE18BBC0F9AEF590164E",
      64,
      expected_digest);

  // Set init query to ctx buffer
  memcpy(&ctx.init,
         &query.sign_msg.initiate,
         sizeof(evm_sign_msg_initiate_request_t));

  // Set the msg_data to ctx
  char *string = "4D7920656D61696C206973206A6F686E40646F652E636F6D202D203136393"
                 "3383938333735353631";
  ctx.msg_data = buffer;
  hex_string_to_byte_array(string, ctx.init.total_msg_size * 2, buffer);
  TEST_ASSERT_TRUE(evm_get_msg_data_digest(&ctx, digest));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_digest, digest, SHA256_DIGEST_LENGTH);
}

TEST(evm_sign_msg_test, evm_sign_msg_test_eth_sign_hash) {
  evm_query_t query = {
      .which_request = 3,
      .sign_msg = {
          .which_request = 1,
          .initiate = {.derivation_path_count = 5,
                       .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0, 0},
                       .wallet_id = {},
                       .message_type = EVM_SIGN_MSG_TYPE_ETH_SIGN,
                       .total_msg_size = 40}}};
  uint8_t buffer[1024];
  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  uint8_t expected_digest[SHA256_DIGEST_LENGTH] = {0};

  hex_string_to_byte_array(
      "1B105102255ECF3522F0D66F05D6E6AAA3F97CA1896E34112FF300AB0FE09B20",
      64,
      expected_digest);

  // Set init query to ctx buffer
  memcpy(&ctx.init,
         &query.sign_msg.initiate,
         sizeof(evm_sign_msg_initiate_request_t));

  // Set the msg_data to ctx
  char *string = "4D7920656D61696C206973206A6F686E40646F652E636F6D202D203136393"
                 "3383938343031363333";
  ctx.msg_data = buffer;
  hex_string_to_byte_array(string, ctx.init.total_msg_size * 2, buffer);
  TEST_ASSERT_TRUE(evm_get_msg_data_digest(&ctx, digest));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_digest, digest, SHA256_DIGEST_LENGTH);
}
