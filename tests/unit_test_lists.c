/**
 * @file    unit_tests_lists.c
 * @author  Cypherock X1 Team
 * @brief   MMain file to handle execution of all unit tests
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
#include "p0_events_test.h"
#include "ui_events_test.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(event_getter_test) {
  RUN_TEST_CASE(event_getter_test, no_event);
  RUN_TEST_CASE(event_getter_test, p0_event);
  RUN_TEST_CASE(event_getter_test, ui_event);
  RUN_TEST_CASE(event_getter_test, usb_event);
  RUN_TEST_CASE(event_getter_test, listening_all_events);
  RUN_TEST_CASE(event_getter_test, listening_all_available_one);
  RUN_TEST_CASE(event_getter_test, disabled_events);
}

TEST_GROUP_RUNNER(xpub) {
  RUN_TEST_CASE(xpub, derivation_path_tests);
}

TEST_GROUP_RUNNER(p0_events_test) {
  RUN_TEST_CASE(p0_events_test, inactivity_evt);
  RUN_TEST_CASE(p0_events_test, abort_evt);
  RUN_TEST_CASE(p0_events_test, abort_inactivity_race);
  RUN_TEST_CASE(p0_events_test, inactivity_refresh_on_joystick_movement);
}

TEST_GROUP_RUNNER(usb_evt_api_test) {
  RUN_TEST_CASE(usb_evt_api_test, basic);
  RUN_TEST_CASE(usb_evt_api_test, consume_and_free)
  RUN_TEST_CASE(usb_evt_api_test, consume_and_respond)
  RUN_TEST_CASE(usb_evt_api_test, stitch_data_chunks)
  RUN_TEST_CASE(usb_evt_api_test, send_data_chunks)
  RUN_TEST_CASE(usb_evt_api_test, api_interference_1)
  RUN_TEST_CASE(usb_evt_api_test, api_interference_2)
  RUN_TEST_CASE(usb_evt_api_test, api_interference_3)
  RUN_TEST_CASE(usb_evt_api_test, wrong_cmd_1)
  RUN_TEST_CASE(usb_evt_api_test, wrong_cmd_2)
}

TEST_GROUP_RUNNER(ui_events_test) {
  RUN_TEST_CASE(ui_events_test, set_confirm);
  RUN_TEST_CASE(ui_events_test, set_cancel);
  RUN_TEST_CASE(ui_events_test, set_list);
  RUN_TEST_CASE(ui_events_test, set_text_input);
  RUN_TEST_CASE(ui_events_test, event_getter);
  RUN_TEST_CASE(ui_events_test, fill_input_test);
  RUN_TEST_CASE(ui_events_test, input_event_null_ptr);
  RUN_TEST_CASE(ui_events_test, ui_get_event_null_ptr);
}

TEST_GROUP_RUNNER(nfc_events_test) {
  RUN_TEST_CASE(nfc_events_test, set_card_detect_event);
  RUN_TEST_CASE(nfc_events_test, set_card_removed_event);
}

#ifdef NFC_EVENT_CARD_DETECT_MANUAL_TEST
TEST_GROUP_RUNNER(nfc_events_manual_test) {
  RUN_TEST_CASE(nfc_events_manual_test, detect_and_remove_card);
}
#endif

TEST_GROUP_RUNNER(array_lists_tests) {
  RUN_TEST_CASE(array_list_tests, insert_multiple);
  RUN_TEST_CASE(array_list_tests, insert_in_full_array);
  RUN_TEST_CASE(array_list_tests, insert_NULL);
  RUN_TEST_CASE(array_list_tests, delete_single_element);
  RUN_TEST_CASE(array_list_tests, delete_first_element);
  RUN_TEST_CASE(array_list_tests, delete_last_element);
  RUN_TEST_CASE(array_list_tests, delete_middle_element);
  RUN_TEST_CASE(array_list_tests, delete_element_when_list_empty);
}
TEST_GROUP_RUNNER(flow_engine_tests) {
  RUN_TEST_CASE(flow_engine_tests, engine_use_case_test);
}

TEST_GROUP_RUNNER(manager_api_test) {
  RUN_TEST_CASE(manager_api_test, decode_valid_manager_bs);
  RUN_TEST_CASE(manager_api_test, decode_invalid_manager_bs_incorrect_size);
  RUN_TEST_CASE(manager_api_test, encode_valid_manager_result);
  RUN_TEST_CASE(manager_api_test, encode_invalid_size_manager_result);
}

TEST_GROUP_RUNNER(btc_txn_helper_test) {
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pk);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pk_fail);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pkh);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2pkh_fail);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2wpkh);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_verify_utxo_p2wpkh_fail);

  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_transaction_weight_legacy1);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_transaction_weight_legacy2);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_transaction_weight_segwit1);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_transaction_weight_segwit2);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_transaction_weight_mixed);

  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_format_value_0sat);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_format_value_1sat);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_format_value_1001sat);
  RUN_TEST_CASE(btc_txn_helper_test, btc_txn_helper_format_value_1btc);
}