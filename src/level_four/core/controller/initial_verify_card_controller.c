/**
 * @file    initial_verify_card_controller.c
 * @author  Cypherock X1 Team
 * @brief   Verify card next controller (initial).
 *          Handles post event (only next events) operations for card
 *verification flow initiated by desktop app.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
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
#include "buzzer.h"
#include "communication.h"
#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "curves.h"
#include "flash_api.h"
#include "nfc.h"
#include "nist256p1.h"
#include "ui_instruction.h"
#if USE_SIMULATOR == 0
#include "stm32l4xx_it.h"
#endif

uint8_t atecc_nfc_sign_hash(const uint8_t *hash, uint8_t *sign) {
  atecc_data.retries = DEFAULT_ATECC_RETRIES;

  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    if (atecc_data.status != ATCA_SUCCESS)
      LOG_CRITICAL("PAIR SG: %04x, count:%d",
                   atecc_data.status,
                   DEFAULT_ATECC_RETRIES - atecc_data.retries);
    atcab_init(atecc_data.cfg_atecc608a_iface);
    atecc_data.status = atcab_sign(slot_3_nfc_pair_key, hash, sign);
  } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  return atecc_data.status;
}

uint8_t atecc_nfc_ecdh(const uint8_t *pub_key, uint8_t *shared_secret) {
  uint8_t io_key[IO_KEY_SIZE];
  atecc_data.retries = DEFAULT_ATECC_RETRIES;

  if (get_io_protection_key(io_key) != SUCCESS_)
    return -1;

  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    if (atecc_data.status != ATCA_SUCCESS)
      LOG_CRITICAL("ECDH: %04x, count:%d",
                   atecc_data.status,
                   DEFAULT_ATECC_RETRIES - atecc_data.retries);
    atcab_init(atecc_data.cfg_atecc608a_iface);
    atecc_data.status =
        atcab_ecdh_ioenc(slot_3_nfc_pair_key, pub_key, shared_secret, io_key);
  } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  return atecc_data.status;
}