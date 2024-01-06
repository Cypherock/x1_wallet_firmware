/**
 * @file    events_tests.c
 * @author  Cypherock X1 Team
 * @brief
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
#include <string.h>

#include "events.h"
#include "ui_events_priv.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"

static const uint8_t core_msg[] = {10, 2, 8, 1};

TEST_GROUP(event_getter_test);

TEST_SETUP(event_getter_test) {
  LOG_SWV("Setting up test...\n");
}

TEST_TEAR_DOWN(event_getter_test) {
  LOG_SWV("\n\n\n");
  usb_clear_event();
  p0_ctx_destroy();
  p0_reset_evt();
  ui_reset_event();
}

TEST(event_getter_test, no_event) {
#if USE_SIMULATOR == 1
  p0_set_inactivity_evt(true);
#endif /* USE_SIMULATOR == 1 */

  evt_status_t evt_status = get_events(0, 5000);
  TEST_ASSERT_TRUE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_FALSE(evt_status.usb_event.flag);
  TEST_ASSERT_TRUE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_FALSE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, p0_event) {
  // explicitly initialize p0; this is to enable aborts so that we can raise
  // an abort event from here itself
  p0_ctx_init(1000);
  // trigger a p0 event
  p0_set_abort_evt(true);
  ui_set_confirm_event();

  evt_status_t evt_status = get_events(0, 5000);
  TEST_ASSERT_TRUE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_FALSE(evt_status.usb_event.flag);
  TEST_ASSERT_FALSE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_TRUE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, ui_event) {
  // trigger an ui event
  ui_set_confirm_event();
  // trigger an usb event
  usb_set_event(4, core_msg, 0, NULL);

  evt_status_t evt_status = get_events(EVENT_CONFIG_UI, 5000);
  TEST_ASSERT_FALSE(evt_status.p0_event.flag);
  TEST_ASSERT_TRUE(evt_status.ui_event.event_occured);
  TEST_ASSERT_FALSE(evt_status.usb_event.flag);
  TEST_ASSERT_FALSE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_FALSE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, usb_event) {
  // trigger an ui event
  ui_set_confirm_event();
  // trigger an usb event
  usb_set_event(4, core_msg, 0, NULL);

  evt_status_t evt_status = get_events(EVENT_CONFIG_USB, 5000);
  TEST_ASSERT_FALSE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_TRUE(evt_status.usb_event.flag);
  TEST_ASSERT_FALSE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_FALSE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, listening_all_events) {
  // explicitly initialize p0; this is to enable aborts so that we can raise
  // an abort event from here itself
  p0_ctx_init(1000);
  // trigger a p0 event
  p0_set_abort_evt(true);
  p0_set_inactivity_evt(true);
  // trigger an ui event
  ui_set_confirm_event();
  // trigger an usb event
  usb_set_event(4, core_msg, 0, NULL);

  evt_status_t evt_status = get_events(0x7, 5000);
  TEST_ASSERT_TRUE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_FALSE(evt_status.usb_event.flag);
  TEST_ASSERT_TRUE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_TRUE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, listening_all_available_one) {
  // trigger an usb event
  usb_set_event(4, core_msg, 0, NULL);

  evt_status_t evt_status = get_events(0x7, 5000);
  TEST_ASSERT_FALSE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_TRUE(evt_status.usb_event.flag);
  TEST_ASSERT_FALSE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_FALSE(evt_status.p0_event.abort_evt);
}

TEST(event_getter_test, disabled_events) {
  // trigger an usb event
  usb_set_event(4, core_msg, 0, NULL);
#if USE_SIMULATOR == 1
  p0_set_inactivity_evt(true);
#endif /* USE_SIMULATOR == 1 */
  evt_status_t evt_status = get_events(0x0, 5000);
  TEST_ASSERT_TRUE(evt_status.p0_event.flag);
  TEST_ASSERT_FALSE(evt_status.ui_event.event_occured);
  TEST_ASSERT_FALSE(evt_status.usb_event.flag);
  TEST_ASSERT_TRUE(evt_status.p0_event.inactivity_evt);
  TEST_ASSERT_FALSE(evt_status.p0_event.abort_evt);
}
