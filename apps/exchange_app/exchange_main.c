/**
 * @file    exchange_main.c
 * @author  Cypherock X1 Team
 * @brief   A common entry point to various Exchange actions supported.
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

#include "exchange_main.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atecc_utils.h"
#include "base58.h"
#include "bip32.h"
#include "board.h"
#include "composable_app_queue.h"
#include "core_session.h"
#include "core_shared_context.h"
#include "curves.h"
#include "exchange/core.pb.h"
#include "exchange_api.h"
#include "exchange_priv.h"
#include "nist256p1.h"
#include "sha2.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the EXCHANGE application of the X1 vault. It is
 * invoked by the X1 vault firmware, as soon as there is a USB request raised
 * for the Exchange app.
 *
 * @param usb_evt The USB event which triggered invocation of the exchange
 * app
 */
void exchange_main(usb_event_t usb_evt, const void *app_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const cy_app_desc_t exchange_app_desc = {.id = 24,
                                                .version =
                                                    {
                                                        .major = 1,
                                                        .minor = 0,
                                                        .patch = 0,
                                                    },
                                                .app = exchange_main,
                                                .app_config = NULL};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void derive_server_public_key(uint8_t *server_verification_pub_key) {
  HDNode node;
  char xpub[XPUB_SIZE] = {'\0'};

  base58_encode_check(get_card_root_xpub(),
                      FS_KEYSTORE_XPUB_LEN,
                      nist256p1_info.hasher_base58,
                      xpub,
                      XPUB_SIZE);

  hdnode_deserialize_public(
      (char *)xpub, 0x0488b21e, NIST256P1_NAME, &node, NULL);
  hdnode_public_ckd(&node, SESSION_KEY_INDEX);

  memcpy(server_verification_pub_key, node.public_key, SESSION_PUB_KEY_SIZE);
}

void exchange_main(usb_event_t usb_evt, const void *app_config) {
  exchange_query_t query = EXCHANGE_QUERY_INIT_DEFAULT;

  if (false == decode_exchange_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  LOG_SWV("%s (%d) - Query:%d\n", __func__, __LINE__, query.which_request);
  switch ((uint8_t)query.which_request) {
    case EXCHANGE_QUERY_INITIATE_FLOW_TAG: {
      exchange_initiate_flow(&query);
      break;
    }
    case EXCHANGE_QUERY_GET_SIGNATURE_TAG: {
      exchange_get_signature(&query);
      break;
    }
    case EXCHANGE_QUERY_STORE_SIGNATURE_TAG: {
      exchange_store_signature(&query);
      break;
    }
    case EXCHANGE_QUERY_CLOSE_FLOW_TAG: {
      exchange_close_flow(&query);
      break;
    }
    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_QUERY);
    } break;
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
const cy_app_desc_t *get_exchange_app_desc() {
  return &exchange_app_desc;
}

bool exchange_app_validate_caq(caq_node_data_t data) {
  bool status = false;
  caq_node_data_t caq_data = caq_peek(&status);
  if (!status) {
    return false;
  }

  if (caq_data.applet_id == data.applet_id &&
      memcmp(caq_data.params, data.params, sizeof(data.params)) == 0) {
    {
      char hex_arr[500] = {0};
      char title[100] = {0};
      snprintf(title, 100, "%ld", data.applet_id);
      byte_array_to_hex_string(data.params, 40, hex_arr, 100);
      LOG_INFO("Match %s [%s]", title, hex_arr);
    }

    caq_pop();
    return true;
  }

  {
    char hex_arr[500] = {0};
    char title[100] = {0};
    snprintf(title, 100, "%ld", data.applet_id);
    byte_array_to_hex_string(data.params, 40, hex_arr, 100);
    LOG_ERROR("CAQ Invalid data received: %s [%s]", title, hex_arr);
  }

  {
    char hex_arr[500] = {0};
    char title[100] = {0};
    snprintf(title, 100, "%ld", caq_data.applet_id);
    byte_array_to_hex_string(caq_data.params, 40, hex_arr, 100);
    LOG_ERROR("CAQ data exptected: %s [%s]", title, hex_arr);
  }

  delay_scr_init("Invalid operation during Swap\n rebooting...", DELAY_TIME);
  BSP_reset();

  return false;
}

bool exchange_validate_stored_signature(char *receiver,
                                        size_t receiver_max_size) {
  uint8_t sig[64] = {0};
  memcpy(sig, shared_context, sizeof(sig));

  core_clear_shared_context();

  uint8_t server_verification_pub_key[SESSION_PUB_KEY_SIZE];
  derive_server_public_key(server_verification_pub_key);

  size_t len = strnlen(receiver, receiver_max_size);
  char recv_addr[len];
  strncpy(recv_addr, receiver, len);

  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};

  sha256_Raw((uint8_t *)recv_addr, len, hash);

  if (ecdsa_verify_digest(&nist256p1, server_verification_pub_key, sig, hash) !=
      0) {
    delay_scr_init("Failed to validate signature\n Do not proceed with Swap",
                   DELAY_TIME);

    return false;
  }

  return true;
}

void exchange_sign_address(char *address, size_t address_max_size) {
  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  size_t len = strnlen(address, address_max_size);
  sha256_Raw((uint8_t *)address, len, hash);

  auth_data_t data = atecc_sign(hash);
  core_clear_shared_context();
  size_t offset = 0;
  memcpy(shared_context, data.signature, sizeof(data.signature));
  offset += sizeof(data.signature);

  memcpy(shared_context + offset, data.postfix1, sizeof(data.postfix1));
  offset += sizeof(data.postfix1);

  memcpy(shared_context + offset, data.postfix2, sizeof(data.postfix2));
  offset += sizeof(data.postfix2);
}
