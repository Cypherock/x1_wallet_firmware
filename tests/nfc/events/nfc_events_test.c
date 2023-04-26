/**
 * @file    nfc_events_test.c
 * @author  Cypherock X1 Team
 * @brief   NFC Events module tests
 *          Tests the event getter and setter operation used by NFC Module and
 *os
 *
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
#include "nfc_events.h"

#include <stdbool.h>

#include "memzero.h"
#include "nfc_events_priv.h"
#include "unity_fixture.h"

#if USE_SIMULATOR == 0
#endif /* USE_SIMULATOR == 0 */

TEST_GROUP(nfc_events_test);

TEST_SETUP(nfc_events_test) {
  return;
}

TEST_TEAR_DOWN(nfc_events_test) {
  nfc_reset_event();
  return;
}

TEST(nfc_events_test, set_card_detect_event) {
  nfc_event_t nfc_event = {0};
  nfc_set_card_detect_event();

  TEST_ASSERT_TRUE(nfc_get_event(&nfc_event));
  TEST_ASSERT_TRUE(nfc_event.event_occured);
  TEST_ASSERT_EQUAL(NFC_EVENT_CARD_DETECT, nfc_event.event_type);
}

TEST(nfc_events_test, set_card_removed_event) {
  nfc_event_t nfc_event = {0};
  nfc_set_card_removed_event();

  TEST_ASSERT_TRUE(nfc_get_event(&nfc_event));
  TEST_ASSERT_TRUE(nfc_event.event_occured);
  TEST_ASSERT_EQUAL(NFC_EVENT_CARD_REMOVED, nfc_event.event_type);
}