/**
 * @file    ui_events_test.h
 * @author  Cypherock X1 Team
 * @brief   UI Events module tests
 *          Tests the event getter and setter operation used by UI screens and
 *os
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
#include "ui_events_test.h"

#include "memzero.h"

#if USE_SIMULATOR == 0
#endif /* USE_SIMULATOR == 0 */

TEST_GROUP(ui_events_test);

TEST_SETUP(ui_events_test) {
  return;
}

TEST_TEAR_DOWN(ui_events_test) {
  ui_reset_event();
  return;
}

TEST(ui_events_test, set_confirm) {
  ui_event_t ui_event = {0};
  ui_set_confirm_event();

  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_CONFIRM);
}

TEST(ui_events_test, set_cancel) {
  ui_event_t ui_event = {0};
  ui_set_cancel_event();

  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_REJECT);
}

TEST(ui_events_test, set_list) {
  ui_event_t ui_event = {0};
  uint16_t list_selection = 40000;
  ui_set_list_event(list_selection);

  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_LIST_CHOICE);
  TEST_ASSERT(ui_event.list_selection == list_selection);
}

TEST(ui_events_test, set_text_input) {
  ui_event_t ui_event = {0};
  char *text_ptr = "DUMMY";
  ui_set_text_input_event(text_ptr);

  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_TEXT_INPUT);
  TEST_ASSERT(ui_event.text_ptr == text_ptr);
}

TEST(ui_events_test, event_getter) {
  ui_event_t ui_event = {0};
  char *text_ptr = "DUMMY";
  ui_set_text_input_event(text_ptr);

  // Getter call when event occured
  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_TEXT_INPUT);
  TEST_ASSERT(ui_event.text_ptr == text_ptr);

  // Getter call when waiting for event
  memzero(&ui_event, sizeof(ui_event));
  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == false);
  TEST_ASSERT(ui_event.event_occured == false);
  TEST_ASSERT(ui_event.event_type == 0);
  TEST_ASSERT(ui_event.text_ptr == NULL);
}

TEST(ui_events_test, fill_input_test) {
  ui_event_t ui_event = {0};
  char text_ptr[] = "Test String 1";
  char text_input_test[sizeof(text_ptr)];
  ui_fill_text(text_ptr, text_input_test, sizeof(text_ptr));
  ui_set_text_input_event(text_input_test);

  // Getter call when event occured
  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == true);
  TEST_ASSERT(ui_event.event_occured == true);
  TEST_ASSERT(ui_event.event_type == UI_EVENT_TEXT_INPUT);
  TEST_ASSERT(ui_event.text_ptr == text_input_test);
  TEST_ASSERT(memcmp(text_ptr, text_input_test, sizeof(text_ptr)) == 0);
}

TEST(ui_events_test, input_event_null_ptr) {
  ui_event_t ui_event = {0};
  ui_set_text_input_event(NULL);

  // Getter call when event occured
  TEST_ASSERT(ui_get_and_reset_event(&ui_event) == false);
  TEST_ASSERT(ui_event.event_occured == false);
}

TEST(ui_events_test, ui_get_event_null_ptr) {
  TEST_ASSERT(ui_get_and_reset_event(NULL) == false);
}