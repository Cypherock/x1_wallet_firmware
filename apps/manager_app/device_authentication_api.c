/**
 * @file    device_authentication_api.c
 * @author  Cypherock X1 Team
 * @brief   Source file APIs to support X1 vault device authentication
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
#include "atca_host.h"
#include "board.h"
#include "cryptoauthlib.h"
#include "curves.h"
#include "flash_api.h"
#include "nist256p1.h"
#include "sec_flash.h"
#include "string.h"
#if USE_SIMULATOR == 0
#include "stm32l4xx_it.h"
#endif

#include "atca_basic.h"
#include "device_authentication_api.h"

#define SIGNATURE_SIZE 64
#define POSTFIX1_SIZE 7
#define POSTFIX2_SIZE 23
#define RANDOM_CHALLENGE_SIZE 32

atecc_data_t atecc_data = {0};

#if (FIRMWARE_HASH_CALC == 0)
static const uint8_t firmware_hash[] = {
    0x75, 0x36, 0x92, 0xec, 0x36, 0xad, 0xb4, 0xc7, 0x94, 0xc9, 0x73,
    0x94, 0x5e, 0xb2, 0xa9, 0x9c, 0x16, 0x49, 0x70, 0x3e, 0xa6, 0xf7,
    0x6b, 0xf2, 0x59, 0xab, 0xb4, 0xfb, 0x83, 0x8e, 0x01, 0x3e,
};
#else
static void fw_hash_calculate(uint8_t *m_digest) {
  sha256_Raw((uint8_t *)APPLICATION_ADDRESS_BASE, get_fwSize(), m_digest);
}
#endif

static ATCA_STATUS helper_config_to_sign_internal(
    ATCADeviceType device_type,
    struct atca_sign_internal_in_out *param,
    const uint8_t *config);
static ATCA_STATUS helper_sign_internal_msg(
    struct atca_sign_internal_in_out *param,
    uint8_t mode,
    uint8_t priv_key_id,
    uint8_t data_key_id);
static void helper_get_gendig_hash(atecc_slot_define_t slot,
                                   uint8_t *data,
                                   uint8_t *digest,
                                   uint8_t *postfix);

manager_auth_device_response_t __attribute__((optimize("O0")))
sign_serial_number(void) {
  manager_auth_device_response_t response =
      MANAGER_AUTH_DEVICE_RESPONSE_INIT_ZERO;
  response.which_response = MANAGER_AUTH_DEVICE_RESPONSE_SERIAL_SIGNATURE_TAG;

  uint8_t nonce[32] = {0};
  uint8_t tempkey_hash[DEVICE_SERIAL_SIZE + POSTFIX2_SIZE] = {0};
  uint8_t final_hash[32] = {0};
  atca_sign_internal_in_out_t sign_internal_param = {0};

  atecc_data.retries = DEFAULT_ATECC_RETRIES;
  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    OTG_FS_IRQHandler();

    if (atecc_data.status != ATCA_SUCCESS)
      LOG_CRITICAL("AUTH SN: %04x, count:%d",
                   atecc_data.status,
                   DEFAULT_ATECC_RETRIES - atecc_data.retries);

    if ((atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface)) !=
        ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_nonce(nonce);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_gendig(ATCA_ZONE_DATA, slot_8_serial, NULL, 0);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_sign_internal(
        slot_2_auth_key, false, false, response.serial_signature.signature);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    atecc_data.status = atcab_read_zone(ATCA_ZONE_DATA,
                                        slot_8_serial,
                                        0,
                                        0,
                                        response.serial_signature.serial,
                                        32);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    helper_get_gendig_hash(slot_8_serial,
                           response.serial_signature.serial,
                           tempkey_hash,
                           response.serial_signature.postfix1);

    sign_internal_param.message = tempkey_hash;
    sign_internal_param.digest = final_hash;

    helper_sign_internal_msg(&sign_internal_param,
                             SIGN_MODE_INTERNAL,
                             slot_2_auth_key,
                             slot_8_serial);
    {
      uint8_t result = ecdsa_verify_digest(&nist256p1,
                                           get_auth_public_key(),
                                           response.serial_signature.signature,
                                           sign_internal_param.digest);
      if (atecc_data.status != ATCA_SUCCESS || result != 0) {
        LOG_ERROR("err xxx32 fault %d verify %d", atecc_data.status, result);
        continue;
      }
    }
  } while (--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  memcpy(response.serial_signature.postfix2, &tempkey_hash[32], POSTFIX2_SIZE);

  return response;
}

manager_auth_device_response_t __attribute__((optimize("O0")))
sign_random_challenge(uint8_t *challenge) {
  manager_auth_device_response_t response =
      MANAGER_AUTH_DEVICE_RESPONSE_INIT_ZERO;
  response.which_response =
      MANAGER_AUTH_DEVICE_RESPONSE_CHALLENGE_SIGNATURE_TAG;

  uint8_t nonce[32] = {0};
  uint8_t io_protection_key[32] = {0};
  uint8_t tempkey_hash[DEVICE_SERIAL_SIZE + POSTFIX2_SIZE] = {0};
  uint8_t final_hash[32] = {0};
  atca_sign_internal_in_out_t sign_internal_param = {0};
  get_io_protection_key(io_protection_key);
#if (FIRMWARE_HASH_CALC == 1)
  // generating firmware hash code
  uint8_t firmware_hash[32];
  fw_hash_calculate(firmware_hash);
#endif
  for (int i = 0; i < 32; ++i)
    challenge[i] = challenge[i] ^ firmware_hash[i];

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
        slot_5_challenge, 0, challenge, io_protection_key, slot_6_io_key);
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
        slot_2_auth_key, false, false, response.challenge_signature.signature);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    helper_get_gendig_hash(slot_5_challenge,
                           challenge,
                           tempkey_hash,
                           response.challenge_signature.postfix1);

    sign_internal_param.message = tempkey_hash;
    sign_internal_param.digest = final_hash;

    helper_sign_internal_msg(&sign_internal_param,
                             SIGN_MODE_INTERNAL,
                             slot_2_auth_key,
                             slot_5_challenge);

    // overwrite challenge slot to signature generation on same challenge
    memset(challenge, 0, RANDOM_CHALLENGE_SIZE);
    atecc_data.status = atcab_write_enc(
        slot_5_challenge, 0, challenge, io_protection_key, slot_6_io_key);
    if (atecc_data.status != ATCA_SUCCESS) {
      continue;
    }

    {
      uint8_t result =
          ecdsa_verify_digest(&nist256p1,
                              get_auth_public_key(),
                              response.challenge_signature.signature,
                              sign_internal_param.digest);
      if (atecc_data.status != ATCA_SUCCESS || result != 0)
        LOG_ERROR("err xxx33 fault %d verify %d", atecc_data.status, result);
    }
  } while (--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  memcpy(
      response.challenge_signature.postfix2, &tempkey_hash[32], POSTFIX2_SIZE);

  /* Zeroise IO protection key */
  memset(&io_protection_key[0], 0, sizeof(io_protection_key));
  return response;
}

void helper_get_gendig_hash(atecc_slot_define_t slot,
                            uint8_t *data,
                            uint8_t *digest,
                            uint8_t *postfix) {
  if (digest == NULL || data == NULL || postfix == NULL) {
    return;
  }

  uint8_t tempkey_init[96] = {0};
  uint8_t atecc_serial[9];
  atecc_data.status = atcab_read_serial_number(atecc_serial);
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
                                     uint8_t data_key_id) {
  uint8_t msg[55];
  uint8_t cfg[128] = {0}, sn[9] = {0};
  atca_temp_key_t temp_key = {0};
  param->key_id = priv_key_id;
  temp_key.key_id = data_key_id;
  temp_key.gen_dig_data = 1;
  temp_key.valid = 1;
  temp_key.source_flag = 1;

  atecc_data.status = atcab_read_config_zone(cfg);
  memcpy(temp_key.value, param->message, 32);
  param->temp_key = &temp_key;
  helper_config_to_sign_internal(ATECC608A, param, cfg);
  atecc_data.status = atcab_read_serial_number(sn);

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

  // TempKeyFlags (b0-3: keyId, b4: sourceFlag, b5: GenDigData, b6:
  // GenKeyData, b7: NoMacFlag)
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

ATCA_STATUS helper_config_to_sign_internal(
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