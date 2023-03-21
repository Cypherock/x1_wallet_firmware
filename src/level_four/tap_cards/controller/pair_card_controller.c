/**
 * @file    pair_card_controller.c
 * @author  Cypherock X1 Team
 * @brief   Pair card controller.
 *          This file contains the pair card controller and helper functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include "base58.h"
#include "bip32.h"
#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "curves.h"
#include "nfc.h"
#include "nist256p1.h"
#include "ui_instruction.h"

static void _tap_card_backend(uint8_t card_number);

void tap_card_pair_card_controller() {
  switch (flow_level.level_four) {
    case PAIR_CARD_TAP_A_CARD_DUMMY:
      flow_level.level_four = PAIR_CARD_RED_FRONTEND;
      break;

    case PAIR_CARD_RED_FRONTEND:
      flow_level.level_four = PAIR_CARD_RED_BACKEND;
      break;

    case PAIR_CARD_RED_BACKEND:
      tap_card_data.desktop_control  = false;
      tap_card_data.lvl4_retry_point = PAIR_CARD_RED_FRONTEND;
      tap_card_data.tapped_card      = 0;
      _tap_card_backend(1);
      break;

    case PAIR_CARD_BLUE_FRONTEND:
      flow_level.level_four = PAIR_CARD_BLUE_BACKEND;
      break;

    case PAIR_CARD_BLUE_BACKEND:
      tap_card_data.lvl4_retry_point = PAIR_CARD_BLUE_FRONTEND;
      tap_card_data.tapped_card      = 0;
      _tap_card_backend(2);
      break;

    case PAIR_CARD_GREEN_FRONTEND:
      flow_level.level_four = PAIR_CARD_GREEN_BACKEND;
      break;

    case PAIR_CARD_GREEN_BACKEND:
      tap_card_data.lvl4_retry_point = PAIR_CARD_GREEN_FRONTEND;
      tap_card_data.tapped_card      = 0;
      _tap_card_backend(3);
      break;

    case PAIR_CARD_YELLOW_FRONTEND:
      flow_level.level_four = PAIR_CARD_YELLOW_BACKEND;
      break;

    case PAIR_CARD_YELLOW_BACKEND:
      tap_card_data.lvl4_retry_point = PAIR_CARD_YELLOW_FRONTEND;
      tap_card_data.tapped_card      = 0;
      _tap_card_backend(4);
      break;

    default:
      reset_flow_level();
      break;
  }
}

static void _tap_card_backend(uint8_t card_number) {
  uint8_t card_pairing_data[128] = {0}, pairing_data_len = 44;
  uint8_t digest[64] = {0}, sig[65] = {0}, session_nonce[32] = {0};
  uint8_t invalid_self_keypath[8] = {
      DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH,
      DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH,
      DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH};

  random_generate(session_nonce, sizeof(session_nonce));
  memcpy(card_pairing_data, get_perm_self_key_id(), 4);
  memcpy(card_pairing_data + 4, session_nonce, sizeof(session_nonce));
  memcpy(card_pairing_data + 36, get_perm_self_key_path(), 8);
  if (memcmp(get_perm_self_key_path(), invalid_self_keypath,
             sizeof(invalid_self_keypath)) == 0) {
    /* Device is not provisioned */
    reset_flow_level();
    instruction_scr_destructor();
    mark_error_screen(ui_text_device_compromised);
    return;
  }

  sha256_Raw(card_pairing_data, pairing_data_len, digest);
  uint8_t status = atecc_nfc_sign_hash(digest, sig);
  if (status)
    LOG_CRITICAL("xxec %d:%d", status, __LINE__);
  pairing_data_len +=
      ecdsa_sig_to_der(sig, card_pairing_data + pairing_data_len);
  tap_card_data.retries = 5;

  while (1) {
    tap_card_data.acceptable_cards = (1 << (card_number - 1));
    tap_card_data.lvl3_retry_point = flow_level.level_three - 1;
    memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    if (nfc_wait_for_card(DEFAULT_NFC_TG_INIT_TIME) != STM_SUCCESS)
      instruction_scr_change_text(ui_text_card_removed_fast, true);
    if (!tap_card_applet_connection())
      return;
    if (is_paired(tap_card_data.card_key_id) > -1) {
      instruction_scr_destructor();
      flow_level.level_four++;
      return;
    }
    tap_card_data.status = nfc_pair(card_pairing_data, &pairing_data_len);

    if (tap_card_data.status == SW_NO_ERROR) {
      buzzer_start(BUZZER_DURATION);
      if (*(uint32_t *)get_family_id() == DEFAULT_UINT32_IN_FLASH)
        set_family_id_flash(tap_card_data.family_id);
      instruction_scr_change_text(ui_text_remove_card_prompt, true);
      handle_pair_card_success(card_number, session_nonce, card_pairing_data);
      nfc_detect_card_removal();
      instruction_scr_destructor();
      break;
    } else if (tap_card_handle_applet_errors()) {
      break;
    }
  }
}

void handle_pair_card_success(uint8_t card_number,
                              uint8_t *session_nonce,
                              uint8_t *card_pairing_data) {
  HDNode guest_card_node;
  SHA512_CTX ctx;
  uint8_t keystore_index, digest[32] = {0}, buffer[64] = {0};
  uint8_t public_key_uncompressed[65] = {0};
  uint32_t index;
  char xpub[112] = {'\0'};

  base58_encode_check(get_card_root_xpub(), FS_KEYSTORE_XPUB_LEN,
                      nist256p1_info.hasher_base58, xpub, 112);
  hdnode_deserialize_public((char *)xpub, 0x0488b21e, NIST256P1_NAME,
                            &guest_card_node, NULL);

  index = read_be(card_pairing_data + 36);
  hdnode_public_ckd(&guest_card_node, index);

  index = read_be(card_pairing_data + 40);
  hdnode_public_ckd(&guest_card_node, index);

  der_to_sig(card_pairing_data + 44, buffer);
  sha256_Raw(card_pairing_data, 44, digest);
  uint8_t status = ecdsa_verify_digest(&nist256p1, guest_card_node.public_key,
                                       buffer, digest);
  if (status) {
    LOG_CRITICAL("xxec %d:%d", status, __LINE__);
    log_hex_array("resp", card_pairing_data, 128);
    log_hex_array("sig", buffer, sizeof(buffer));
    reset_flow_level();
    mark_error_screen(ui_text_cannot_verify_card_contact_support);
    return;
  }
  keystore_index = card_number - 1;
  ecdsa_uncompress_pubkey(&nist256p1, guest_card_node.public_key,
                          public_key_uncompressed);
  status = atecc_nfc_ecdh(&public_key_uncompressed[1], card_pairing_data + 45);
  if (status)
    LOG_CRITICAL("xxec %d:%d", status, __LINE__);
  sha512_Init(&ctx);
  sha512_Update(&ctx, card_pairing_data + 45, 32);
  sha512_Update(&ctx, card_pairing_data + 4, 32);
  sha512_Update(&ctx, session_nonce, 32);
  sha512_Final(&ctx, buffer);
  set_keystore_pairing_key(keystore_index, buffer, sizeof(buffer),
                           FLASH_SAVE_LATER);
  set_keystore_key_id(keystore_index, card_pairing_data, 4, FLASH_SAVE_LATER);
  set_keystore_used_status(keystore_index, 1, FLASH_SAVE_NOW);

#if X1WALLET_MAIN == 1
  flow_level.level_four++;
#elif X1WALLET_INITIAL == 1
  flow_level.level_three++;
#endif
}