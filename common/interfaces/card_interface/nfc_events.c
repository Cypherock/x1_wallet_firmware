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

#include "app_error.h"
#include "memzero.h"
#include "nfc_events_priv.h"
#include "string.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define DEFAULT_CARD_REMOVAL_RETRY_COUNT 5

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  NFC_STATE_OFF = 0,
  NFC_STATE_SET_SELECT_CARD_CMD,
  NFC_STATE_WAIT_SELECT_CARD_RESP,
  NFC_STATE_CARD_DETECTED,
  NFC_STATE_WAIT_FOR_CARD_REMOVAL,
  NFC_STATE_CARD_REMOVED
} nfc_task_states_t;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static nfc_task_states_t nfc_state;
static nfc_event_t nfc_event;
static uint8_t card_removal_retry_counter = 0;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief   Checks if card has been removed and returns status
 * @arg     retry_count:  No. of consecutive retries for card not detected
 *
 * @return  true if card is not detected for consecutive retries exceeding
 * retry_count
 */
static bool check_card_removed_status(uint8_t retry_count);

/**
 * @brief   Handles response from read NFC A target init api
 *          If card is selected, card select event is set, if an error is
 * received, other than NFC_RESP_NOT_READY, state is updated to
 * NFC_STATE_SET_SELECT_CARD_CMD
 */
static void nfc_handle_card_select_resp(void);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_card_removed_status(uint8_t retry_count) {
  uint32_t err = adafruit_diagnose_card_presence();
  if (err != PN532_DIAGNOSE_CARD_DETECTED_RESP) {
    card_removal_retry_counter++;
  } else {
    card_removal_retry_counter = 0;
  }

  if (card_removal_retry_counter > retry_count) {
    return true;
  }
  return false;
}

static void nfc_handle_card_select_resp(void) {
  nfc_a_tag_info nfc_tag_info;
  uint32_t card_select_status = pn532_read_nfca_target_init_resp(&nfc_tag_info);
  if (card_select_status == STM_SUCCESS) {
    nfc_set_card_detect_event();
  } else if (card_select_status != NFC_RESP_NOT_READY) {
    nfc_state = NFC_STATE_SET_SELECT_CARD_CMD;
  }
}

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

void nfc_reset_event(void) {
  memzero(&nfc_event, sizeof(nfc_event_t));
}

void nfc_set_card_detect_event(void) {
  nfc_event.event_occured = true;
  nfc_event.event_type = NFC_EVENT_CARD_DETECT;
  nfc_state = NFC_STATE_CARD_DETECTED;
}

void nfc_set_card_removed_event(void) {
  nfc_event.event_occured = true;
  nfc_event.event_type = NFC_EVENT_CARD_REMOVED;
  nfc_state = NFC_STATE_CARD_REMOVED;
}

void nfc_en_select_card_task(void) {
  nfc_state = NFC_STATE_SET_SELECT_CARD_CMD;

  // Deselect card before selection, to avoid unexpected issues.
  // Without deselection failure experienced on second time detection of the
  // same card.
  nfc_deselect_card();
}

uint32_t nfc_en_wait_for_card_removal_task(void) {
  uint32_t card_presence_state = nfc_diagnose_card_presence();
  if (card_presence_state == PN532_DIAGNOSE_CARD_DETECTED_RESP) {
    nfc_state = NFC_STATE_WAIT_FOR_CARD_REMOVAL;
    card_removal_retry_counter = 0;
  }
  return card_presence_state;
}

void nfc_task_handler(void) {
  switch (nfc_state) {
    case NFC_STATE_SET_SELECT_CARD_CMD: {
      if (pn532_set_nfca_target_init_command() == STM_SUCCESS) {
        nfc_state = NFC_STATE_WAIT_SELECT_CARD_RESP;
      }
    } break;

    case NFC_STATE_WAIT_SELECT_CARD_RESP: {
      nfc_handle_card_select_resp();
    } break;

    case NFC_STATE_CARD_DETECTED: {
      // Should never reach here.
    } break;

    case NFC_STATE_WAIT_FOR_CARD_REMOVAL: {
      if (check_card_removed_status(DEFAULT_CARD_REMOVAL_RETRY_COUNT)) {
        nfc_set_card_removed_event();
      }
    } break;

    case NFC_STATE_CARD_REMOVED: {
      // Should never reach here.
    } break;

    default: {
      LOG_ERROR("xxx37: %d", nfc_state);
    } break;
  }
}

void nfc_ctx_destroy(void) {
  nfc_deselect_card();
  nfc_state = NFC_STATE_OFF;
  card_removal_retry_counter = 0;
}