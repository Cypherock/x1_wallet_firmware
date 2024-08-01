/**
 * @file    inheritance_script_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bitcoin scripts
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
#include "inheritance_main.h"
#include "unity_fixture.h"

TEST_GROUP(inheritance_setup_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(inheritance_setup_test) {
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(inheritance_setup_test) {
}

TEST(inheritance_setup_test, inheritance_setup_test_first) {

  // SETUP

  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;

  query.setup.plain_data_count = 1;

  char message_1[PLAIN_DATA_SIZE] = "An important message.";
  memcpy(query.setup.plain_data[0].message.bytes, message_1, 21);
  query.setup.plain_data[0].message.size = 21;
  query.setup.plain_data[0].is_private = true;

  inheritance_setup(&query);


  // RECOVERY

  inheritance_query_t query_r = INHERITANCE_QUERY_INIT_ZERO;
  query_r.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  uint8_t encrpyted_result[ENCRYPTED_DATA_SIZE/2];
  
  hex_string_to_byte_array("d8f3cf0fe0668494f419c266d8755334d65723f98aec2369f9fb446ed335ad86", 64, encrpyted_result);
      
  query_r.recovery.has_encrypted_data = true;
  query_r.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(query_r.recovery.encrypted_data.packet.bytes, encrpyted_result, 32);
  query_r.recovery.encrypted_data.packet.size = 32;

  inheritance_recovery(&query_r);

  TEST_ASSERT_EQUAL_INT(1, 1);
}