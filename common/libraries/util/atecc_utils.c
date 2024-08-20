/**
 * @file    atecc_utils.c
 * @author  Cypherock X1 Team
 * @brief   Definition of the ATECC signature helper functions used for
 *          signing and verifying the messages using the ATECC608A chip.
 *
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

#include "atecc_utils.h"

#include "board.h"
#include "flash_api.h"
#include "sha2.h"
#if USE_SIMULATOR == 0
#include "stm32l4xx_it.h"
#endif

static ATCA_STATUS helper_config_to_sign_internal(
    ATCADeviceType device_type,
    struct atca_sign_internal_in_out *param,
    const uint8_t *config) {
  const uint8_t *value = NULL;
  uint16_t slot_locked = 0;

  if (param == NULL || config == NULL || param->temp_key == NULL) {
    return ATCA_BAD_PARAM;
  }

  // SlotConfig[TempKeyFlags.keyId]
  value = &config[20 + param->temp_key->key_id * 2];
  param->slot_config = (uint16_t)value[0] | ((uint16_t)value[1] << 8);

  // KeyConfig[TempKeyFlags.keyId]
  value = &config[96 + param->temp_key->key_id * 2];
  param->key_config = (uint16_t)value[0] | ((uint16_t)value[1] << 8);

  if (device_type == ATECC108A && param->temp_key->key_id < 8) {
    value = &config[52 + param->temp_key->key_id * 2];
    param->use_flag = value[0];
    param->update_count = value[0];
  } else {
    param->use_flag = 0x00;
    param->update_count = 0x00;
  }

  // SlotLocked:TempKeyFlags.keyId
  slot_locked = (uint16_t)config[88] | ((uint16_t)config[89] << 8);
  param->is_slot_locked =
      (slot_locked & (1 << param->temp_key->key_id)) ? false : true;

  return ATCA_SUCCESS;
}

void helper_get_gendig_hash(atecc_slot_define_t slot,
                            uint8_t *data,
                            uint8_t *digest,
                            uint8_t *postfix,
                            atecc_data_t *atecc_value) {
  if (digest == NULL || data == NULL || postfix == NULL) {
    return;
  }

  uint8_t tempkey_init[96] = {0};
  uint8_t atecc_serial[9];
  atecc_value->status = atcab_read_serial_number(atecc_serial);
  memcpy(tempkey_init, data, 32);
  postfix[0] = tempkey_init[32] = 0x15;
  postfix[1] = tempkey_init[33] = 0x02;
  postfix[2] = tempkey_init[34] = slot;
  postfix[3] = tempkey_init[35] = 0x00;
  postfix[4] = tempkey_init[36] = atecc_serial[8];
  postfix[5] = tempkey_init[37] = atecc_serial[0];
  postfix[6] = tempkey_init[38] = atecc_serial[1];

  sha256_Raw(tempkey_init, 96, digest);
}

ATCA_STATUS helper_sign_internal_msg(struct atca_sign_internal_in_out *param,
                                     uint8_t mode,
                                     uint8_t priv_key_id,
                                     uint8_t data_key_id,
                                     atecc_data_t *atecc_value) {
  uint8_t msg[55];
  uint8_t cfg[128] = {0}, sn[9] = {0};
  atca_temp_key_t temp_key = {0};
  param->key_id = priv_key_id;
  temp_key.key_id = data_key_id;
  temp_key.gen_dig_data = 1;
  temp_key.valid = 1;
  temp_key.source_flag = 1;

  atecc_value->status = atcab_read_config_zone(cfg);
  memcpy(temp_key.value, param->message, 32);
  param->temp_key = &temp_key;
  helper_config_to_sign_internal(ATECC608A, param, cfg);
  atecc_value->status = atcab_read_serial_number(sn);

  if (param == NULL || param->temp_key == NULL) {
    return ATCA_BAD_PARAM;
  }

  memset(msg, 0, sizeof(msg));
  memcpy(&msg[0], param->temp_key->value, 32);
  msg[32] = ATCA_SIGN;                        // Sign OpCode
  msg[33] = param->mode;                      // Sign Mode
  msg[34] = (uint8_t)(param->key_id >> 0);    // Sign KeyID
  msg[35] = (uint8_t)(param->key_id >> 8);
  msg[36] =
      (uint8_t)(param->slot_config >> 0);    // SlotConfig[TempKeyFlags.keyId]
  msg[37] = (uint8_t)(param->slot_config >> 8);
  msg[38] =
      (uint8_t)(param->key_config >> 0);    // KeyConfig[TempKeyFlags.keyId]
  msg[39] = (uint8_t)(param->key_config >> 8);

  // TempKeyFlags (b0-3: keyId, b4: sourceFlag, b5: GenDigData, b6: GenKeyData,
  // b7: NoMacFlag)
  msg[40] |= ((param->temp_key->key_id & 0x0F) << 0);
  msg[40] |= ((param->temp_key->source_flag & 0x01) << 4);
  msg[40] |= ((param->temp_key->gen_dig_data & 0x01) << 5);
  msg[40] |= ((param->temp_key->gen_key_data & 0x01) << 6);
  msg[40] |= ((param->temp_key->no_mac_flag & 0x01) << 7);
  msg[41] = 0x00;
  msg[42] = 0x00;

  // Serial Number
  msg[43] = sn[8];
  memcpy(&msg[48], &sn[0], 2);
  if (param->mode & SIGN_MODE_INCLUDE_SN) {
    memcpy(&msg[44], &sn[4], 4);
    memcpy(&msg[50], &sn[2], 2);
  }

  // The bit within the SlotLocked field corresponding to the last key used in
  // the TempKey computation is in the LSB
  msg[52] = param->is_slot_locked ? 0x00 : 0x01;

  // If the slot contains a public key corresponding to a supported curve, and
  // if PubInfo indicates this key must be validated before being used by
  // Verify, and if the validity bits have a value of 0x05, then the PubKey
  // Valid byte will be 0x01.In all other cases, it will be 0.
  msg[53] = param->for_invalidate ? 0x01 : 0x00;

  msg[54] = 0x00;

  if (param->message) {
    memcpy(param->message, msg, sizeof(msg));
  }
  if (param->verify_other_data) {
    memcpy(&param->verify_other_data[0], &msg[33], 10);
    memcpy(&param->verify_other_data[10], &msg[44], 4);
    memcpy(&param->verify_other_data[14], &msg[50], 5);
  }
  if (param->digest) {
    return atcac_sw_sha2_256(msg, sizeof(msg), param->digest);
  } else {
    return ATCA_SUCCESS;
  }
}

auth_data_t atecc_sign(uint8_t *hash) {
  uint8_t challenge_no[32];
  uint8_t io_protection_key[32] = {0};
  uint8_t nonce[32] = {0};
  auth_data_t auth_challenge_packet = {0};
  uint8_t tempkey_hash[DEVICE_SERIAL_SIZE + POSTFIX2_SIZE] = {0};
  uint8_t final_hash[32] = {0};
  get_io_protection_key(io_protection_key);

  memset(challenge_no, 0, sizeof(challenge_no));
  for (int i = 0; i < 32; ++i)
    challenge_no[i] = challenge_no[i] ^ hash[i];

  atca_sign_internal_in_out_t sign_internal_param = {0};

  atecc_data.retries = DEFAULT_ATECC_RETRIES;
  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    OTG_FS_IRQHandler();

    if (atecc_data.status != ATCA_SUCCESS)
      LOG_CRITICAL("AERR CH: %04x, count:%d",
                   atecc_data.status,
                   DEFAULT_ATECC_RETRIES - atecc_data.retries);

    if ((atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface)) !=
        ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_write_enc(
        slot_5_challenge, 0, challenge_no, io_protection_key, slot_6_io_key);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_nonce(nonce);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_gendig(ATCA_ZONE_DATA, slot_5_challenge, NULL, 0);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_sign_internal(
        slot_2_auth_key, false, false, auth_challenge_packet.signature);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    helper_get_gendig_hash(slot_5_challenge,
                           challenge_no,
                           tempkey_hash,
                           auth_challenge_packet.postfix1,
                           &atecc_data);

    sign_internal_param.message = tempkey_hash;
    sign_internal_param.digest = final_hash;

    helper_sign_internal_msg(&sign_internal_param,
                             SIGN_MODE_INTERNAL,
                             slot_2_auth_key,
                             slot_5_challenge,
                             &atecc_data);

    memset(challenge_no, 0, sizeof(challenge_no));
    atecc_data.status = atcab_write_enc(
        slot_5_challenge, 0, challenge_no, io_protection_key, slot_6_io_key);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    {
      uint8_t result = ecdsa_verify_digest(&nist256p1,
                                           get_auth_public_key(),
                                           auth_challenge_packet.signature,
                                           sign_internal_param.digest);
      if (atecc_data.status != ATCA_SUCCESS || result != 0) {
        LOG_ERROR("err xxx33 fault %d verify %d", atecc_data.status, result);
      }
    }

  } while (--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  memcpy(auth_challenge_packet.postfix2,
         &tempkey_hash[32],
         POSTFIX2_SIZE);    // postfix 2 (12bytes)

  return auth_challenge_packet;
}