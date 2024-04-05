/**
 * @file    utils_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for utility functions
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

#include "lv_symbol_def.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(utils_tests);

TEST_SETUP(utils_tests) {
  return;
}

TEST_TEAR_DOWN(utils_tests) {
  return;
}

TEST(utils_tests, der_to_sig_1) {
  uint8_t der_encoded_signature[300] = {0};
  uint8_t expected_signature[64] = {0};

  // DER signature reference: https://asecuritysite.com/digitalcert/sigs5
  hex_string_to_byte_array("3046022100e4e87c417196c6e5cd63f93e94929ccda6d04fc0a"
                           "7446922baf3070e854ec4f4022100a1ecd098008329de9bc93f"
                           "b2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
                           144,
                           der_encoded_signature);

  hex_string_to_byte_array(
      "e4e87c417196c6e5cd63f93e94929ccda6d04fc0a7446922baf3070e854ec4f4a1ecd098"
      "008329de9bc93fb2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
      128,
      expected_signature);

  uint8_t signature[64] = {0};
  der_to_sig(der_encoded_signature, signature);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, signature, sizeof(expected_signature));
}

TEST(utils_tests, der_to_sig_2) {
  uint8_t der_encoded_signature[300] = {0};
  uint8_t expected_signature[64] = {0};

  hex_string_to_byte_array(
      "3044022100b259cf6021d099b43efe4b58c939dfb8be33c5dce38a783ec37a144f08bf00"
      "ce021f513db69c8c3763b58e5f1c8925f11e16206fb15829abe684b9a765721165b8",
      140,
      der_encoded_signature);

  hex_string_to_byte_array(
      "b259cf6021d099b43efe4b58c939dfb8be33c5dce38a783ec37a144f08bf00ce00513db6"
      "9c8c3763b58e5f1c8925f11e16206fb15829abe684b9a765721165b8",
      128,
      expected_signature);

  uint8_t signature[64] = {0};
  der_to_sig(der_encoded_signature, signature);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, signature, sizeof(expected_signature));
}

TEST(utils_tests, escape_string_non_print_utf) {
  // message: "Pound symbol (£)"
  char utf_8_string[] = "Pound symbol (\xc2\xa3)";
  const char expected_string[] = "Pound symbol (\\xc2\\xa3)";
  char actual_string[300] = "";

  uint8_t result = string_to_escaped_string(
      utf_8_string, actual_string, sizeof(actual_string));
  TEST_ASSERT_EQUAL_UINT8(2, result);
  TEST_ASSERT_EQUAL_STRING(expected_string, actual_string);
}

TEST(utils_tests, escape_string_symbol) {
  // message: "Backspace symbol ()"
  char utf_8_string[] = "Backspace symbol (" MY_BACKSPACE ")";
  const char expected_string[] = "Backspace symbol (" MY_BACKSPACE ")";
  char actual_string[300] = "";

  uint8_t result = string_to_escaped_string(
      utf_8_string, actual_string, sizeof(actual_string));
  TEST_ASSERT_EQUAL_UINT8(0, result);
  TEST_ASSERT_EQUAL_STRING(expected_string, actual_string);
}

TEST(utils_tests, escape_string_ascii) {
  // message:
  // "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789
  // !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~"
  char utf_8_string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\nabcdefghijklmnopqrstuvwx"
                        "yz\n0123456789\r !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~";
  const char expected_string[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\nabcdefghijklmnopqrstuvwxyz\n0123456789\r "
      "!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~";
  char actual_string[300] = "";

  uint8_t result = string_to_escaped_string(
      utf_8_string, actual_string, sizeof(actual_string));
  TEST_ASSERT_EQUAL_UINT8(0, result);
  TEST_ASSERT_EQUAL_STRING(expected_string, actual_string);
}

// Mix of valid & invalid utf-8 encodings. Should be escaped as U+0
TEST(utils_tests, escape_string_invalid_non_print_utf) {
  // message: "<Mix of valid & invalid utf-8 encodings>"
  char utf_8_string[] =
      "a"                   // 'Valid ASCII'
      "\xc3\xb1"            // 'Valid 2 Octet Sequence'
      "\xc3\x28"            // 'Invalid 2 Octet Sequence'
      "\xa0\xa1"            // 'Invalid Sequence Identifier'
      "\xe2\x82\xa1"        // 'Valid 3 Octet Sequence'
      "\xe2\x28\xa1"        // 'Invalid 3 Octet Sequence (in 2nd Octet)'
      "\xe2\x82\x28"        // 'Invalid 3 Octet Sequence (in 3rd Octet)'
      "\xf0\x90\x8c\xbc"    // 'Valid 4 Octet Sequence'
      "\xf0\x28\x8c\xbc"    // 'Invalid 4 Octet Sequence (in 2nd Octet)'
      "\xf0\x90\x28\xbc"    // 'Invalid 4 Octet Sequence (in 3rd Octet)'
      "\xf0\x90\x8c\x28"    // 'Invalid 4 Octet Sequence (in 4th Octet)'
      ;
  const char expected_string[] =
      "a"                       // 'Valid ASCII'
      "\\xc3\\xb1"              // 'Valid 2 Octet Sequence'
      "\\x00("                  // 'Invalid 2 Octet Sequence'
      "\\x00\\x00"              // 'Invalid Sequence Identifier'
      "\\xe2\\x82\\xa1"         // 'Valid 3 Octet Sequence'
      "\\x00(\\x00"             // 'Invalid 3 Octet Sequence (in 2nd Octet)'
      "\\x00\\x00("             // 'Invalid 3 Octet Sequence (in 3rd Octet)'
      "\\xf0\\x90\\x8c\\xbc"    // 'Valid 4 Octet Sequence'
      "\\x00(\\x00\\x00"        // 'Invalid 4 Octet Sequence (in 2nd Octet)'
      "\\x00\\x00(\\x00"        // 'Invalid 4 Octet Sequence (in 3rd Octet)'
      "\\x00\\x00\\x00("        // 'Invalid 4 Octet Sequence (in 4th Octet)'
      ;
  char actual_string[300] = "";

  uint8_t result = string_to_escaped_string(
      utf_8_string, actual_string, sizeof(actual_string));
  TEST_ASSERT_EQUAL_UINT8(4, result);
  TEST_ASSERT_EQUAL_STRING(expected_string, actual_string);
}

// short result buffer
TEST(utils_tests, escape_string_short_out_buff) {
  // message: "Pound symbol (£)"
  char utf_8_string[150] = "";
  const char expected_string[] = "Pound symbol (\\xc2\\xa3)";
  char actual_string[24] = "";

  hex_string_to_byte_array(
      "506f756e642073796d626f6c2028c2a329", 34, (uint8_t *)utf_8_string);

  uint8_t result = string_to_escaped_string(
      utf_8_string, actual_string, sizeof(actual_string));
  TEST_ASSERT_EQUAL_UINT8(5, result);
}

// invalid arguments
TEST(utils_tests, escape_string_invalid_args) {
  char utf_8_string[150] = "";
  uint8_t result = 0;

  result = string_to_escaped_string(NULL, utf_8_string, 8);
  TEST_ASSERT_EQUAL_UINT8(1, result);
  result = string_to_escaped_string(utf_8_string, NULL, 8);
  TEST_ASSERT_EQUAL_UINT8(1, result);
  result = string_to_escaped_string(utf_8_string, utf_8_string, 0);
  TEST_ASSERT_EQUAL_UINT8(1, result);
  result = string_to_escaped_string(NULL, NULL, 0);
  TEST_ASSERT_EQUAL_UINT8(1, result);
  result = string_to_escaped_string(utf_8_string, utf_8_string, 8);
  TEST_ASSERT_EQUAL_UINT8(1, result);
}
