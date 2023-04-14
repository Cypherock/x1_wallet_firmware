/**
 * @file    nfc_events.c
 * @author  Cypherock X1 Team
 * @brief   NFC Events module
 *          Provides NFC module setters and getters
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "nfc_events.h"

#include "memzero.h"
#include "string.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define DEFAULT_NFC_TIMEOUT 100

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static nfc_ctx_t nfc_ctx;
static nfc_event_t nfc_event;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool nfc_get_event(nfc_event_t *nfc_event_os_obj) {
  ASSERT(nfc_event_os_obj != NULL);

  if (nfc_event.event_occured == true) {
    memcpy(nfc_event_os_obj, &nfc_event, sizeof(nfc_event_t));
    nfc_reset_event();
    return true;
  }
  return false;
}

void nfc_reset_event() {
  memzero(&nfc_event, sizeof(nfc_event_t));
}

void nfc_enable_card_detect_event() {
  nfc_ctx.card_detect_enabled = true;
}

void nfc_disable_card_detect_event() {
  nfc_ctx.card_detect_enabled = false;
}

void nfc_set_card_detect_event() {
  if (nfc_ctx.card_detect_enabled) {
    nfc_event.event_occured = true;
    nfc_event.event_type = NFC_EVENT_CARD_DETECT;
  }
}

void nfc_tick_inc(uint16_t tick_inc) {
  if (nfc_ctx.nfc_time < DEFAULT_NFC_TIMEOUT)
    nfc_ctx.nfc_time += tick_inc;
}

void nfc_ctx_init() {
  if (nfc_ctx.card_detect_enabled) {
    nfc_ctx.nfc_time = 0;
    nfc_ctx.nfc_field_off = false;
  }
}

void nfc_task_handler() {
  if (nfc_ctx.card_detect_enabled && nfc_ctx.nfc_time >= DEFAULT_NFC_TIMEOUT) {
    nfc_card_presence_detect();
    nfc_ctx.nfc_time = 0;
  }
}

void nfc_ctx_destroy() {
  if (!nfc_ctx.nfc_field_off)
    nfc_deselect_card();
  nfc_ctx.nfc_field_off = true;
}