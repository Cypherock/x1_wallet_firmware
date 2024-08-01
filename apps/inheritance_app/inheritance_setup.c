/**
 * @file    inheritance_helpers.c
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "inheritance_main.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

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
 * STATIC VARIABLES
 *****************************************************************************/

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

void inheritance_setup(inheritance_query_t *query,
                       inheritance_result_t *response) {
  uint32_t msg_count = query->setup.plain_data_count;
  if (SESSION_MSG_MAX < msg_count) {
    // ADD error
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  SecureData *msgs = (SecureData *)malloc(sizeof(SecureData) * msg_count);
  memzero(msgs, sizeof(msgs));
  if (NULL == msgs) {
    // ADD error
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  convert_plaindata_to_msg(query->setup.plain_data, msgs, msg_count);

  if (!session_encrypt_secure_data(query->setup.wallet_id, msgs, msg_count)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  // TODO: remove after testing
  set_session();

  uint8_t packet[SESSION_PACKET_SIZE] = {0};
  size_t packet_size = 0;

  if (!session_encrypt_packet(msgs, msg_count, packet, &packet_size)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    return;
  }

  response->which_response = INHERITANCE_RESULT_SETUP_TAG;
  response->setup.has_encrypted_data = true;
  memcpy(response->setup.encrypted_data.packet.bytes, packet, packet_size);
  response->setup.encrypted_data.packet.size = packet_size;

#if USE_SIMULATOR == 1
  printf("Inheritance Setup Result: <Encrpyted Data>\n");
  for (int i = 0; i < response->setup.encrypted_data.packet.size; i++) {
    printf("%02x", response->setup.encrypted_data.packet.bytes[i]);
    fflush(stdout);
  }
  printf("\nEnd");
#endif

  inheritance_send_result(&response);
  delay_scr_init(ui_text_check_cysync, DELAY_TIME);

  free(msgs);
}

void convert_plaindata_to_msg(inheritance_plain_data_t *plain_data,
                              SecureData *msgs,
                              size_t msg_count) {
  for (uint8_t i = 0; i < msg_count; i++) {
    msgs[i].plain_data[0] = plain_data[i].is_private ? 1 : 0;
    msgs[i].plain_data_size += 1;

    memcpy(msgs[i].plain_data + 1,
           plain_data[i].message.bytes,
           plain_data[i].message.size);
    msgs[i].plain_data_size += plain_data[i].message.size;
  }
}