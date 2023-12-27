/**
 * @file    nfc_event_getter.c
 * @author  Cypherock X1 Team
 * @brief   NFC Events test for integration with os event getter
 *          Tests the event getter and setter operation used by NFC Module and
 * with os event getter
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

#include <stdbool.h>

#include "adafruit_pn532.h"
#include "board.h"
#include "buzzer.h"
#include "events.h"
#include "memzero.h"
#include "nfc.h"
#include "nfc_events.h"
#include "ui_instruction.h"
#include "unity_fixture.h"
#if USE_SIMULATOR == 0
#ifdef NFC_EVENT_CARD_DETECT_MANUAL_TEST

TEST_GROUP(nfc_events_manual_test);

TEST_SETUP(nfc_events_manual_test) {
  instruction_scr_init("Dummy", "NFC EVENT TEST");
  lv_task_handler();
  return;
}

TEST_TEAR_DOWN(nfc_events_manual_test) {
  instruction_scr_destructor();
  lv_task_handler();
  nfc_reset_event();
  return;
}

TEST(nfc_events_manual_test, detect_and_remove_card) {
  // Initialize screen to indicate tap card
  instruction_scr_change_text("Tap Card", true);
  // Enable select card task
  nfc_en_select_card_task();

  /**
   * Wait for an event occurance, Card tap is awaited at this point
   */
  evt_status_t evt_status = get_events(EVENT_CONFIG_NFC, 50000);
  TEST_ASSERT_TRUE(evt_status.nfc_event.event_occured);
  TEST_ASSERT_EQUAL(NFC_EVENT_CARD_DETECT, evt_status.nfc_event.event_type);

  // Indicate nfc select card event occured
  nfc_select_card();
  instruction_scr_change_text("Card Detected", true);
  buzzer_start(500);
  BSP_DelayMs(500);

  instruction_scr_change_text("Remove card", true);

  uint32_t err = nfc_en_wait_for_card_removal_task();
  TEST_ASSERT_EQUAL(PN532_DIAGNOSE_CARD_DETECTED_RESP, err);
  memzero(&evt_status.nfc_event, sizeof(evt_status.nfc_event));

  /**
   * Wait for an event occurance, Card removal is awaited at this point
   */
  evt_status_t evt_status = get_events(EVENT_CONFIG_NFC, 50000);
  TEST_ASSERT_TRUE(evt_status.nfc_event.event_occured);
  TEST_ASSERT_EQUAL(NFC_EVENT_CARD_REMOVED, evt_status.nfc_event.event_type);

  instruction_scr_change_text("Card Removed", true);
  buzzer_start(500);
  BSP_DelayMs(500);
}

#endif
#endif