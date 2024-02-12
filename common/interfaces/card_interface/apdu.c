/**
 * @file    apdu.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
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
#include "apdu.h"

#include "app_error.h"
#include "assert_conf.h"
#include "utils.h"

/**
 * NOTE :
 * Wallet Share is Wallet_Share + Chacha Polly Mac + Nonce
 */
static Card_Data_Health card_data_health = DATA_HEALTH_UNKNOWN;
static uint8_t session_enc_key[32];
static uint8_t session_mac_key[32];
static uint8_t session_iv[16];

/**
 * Fills array in this format :
 * [TAG][Length][Value] where,
 * Tag : Unique Identifier
 * Length : Length of Value. (skipped if length of value = 1)
 * Value : The data to be sent
 * Example for Name - [0xE0][16][Data from wallet->wallet_name]
 */
void fill_tlv(uint8_t *array,
              uint16_t *starting_index,
              const Tag_value tag,
              const uint8_t length,
              const struct Wallet *wallet) {
  ASSERT(array != NULL);
  ASSERT(starting_index != NULL);
  ASSERT(length != 0);
  ASSERT(wallet != NULL);

  array[(*starting_index)++] = tag;
  if (length != 1)
    array[(*starting_index)++] = length;

  switch (tag) {
    case INS_NAME:
      memcpy(array + *starting_index,
             wallet->wallet_name,
             sizeof(wallet->wallet_name));
      *starting_index =
          *starting_index + (sizeof(wallet->wallet_name) / sizeof(uint8_t));
      break;
    case INS_PASSWORD:
      memcpy(array + *starting_index,
             wallet->password_double_hash,
             sizeof(wallet->password_double_hash));
      *starting_index =
          *starting_index +
          (sizeof(wallet->password_double_hash) / sizeof(uint8_t));
      break;
    case INS_xCor:
      array[*starting_index] = wallet->xcor;
      (*starting_index)++;
      break;
    case INS_NO_OF_MNEMONICS:
      array[*starting_index] = wallet->number_of_mnemonics;
      (*starting_index)++;
      break;
    case INS_TOTAL_NO_OF_SHARE:
      array[*starting_index] = wallet->total_number_of_shares;
      (*starting_index)++;
      break;
    case INS_WALLET_SHARE:
      memcpy(array + *starting_index,
             wallet->wallet_share_with_mac_and_nonce,
             sizeof(wallet->wallet_share_with_mac_and_nonce));
      *starting_index =
          *starting_index +
          (sizeof(wallet->wallet_share_with_mac_and_nonce) / sizeof(uint8_t));
      break;
    case INS_STRUCTURE_CHECKSUM:
      memcpy(
          array + *starting_index, wallet->checksum, sizeof(wallet->checksum));
      *starting_index =
          *starting_index + (sizeof(wallet->checksum) / sizeof(uint8_t));
      break;
    case INS_MIN_NO_OF_SHARES:
      array[*starting_index] = wallet->minimum_number_of_shares;
      (*starting_index)++;
      break;
    case INS_WALLET_INFO:
      array[*starting_index] = wallet->wallet_info;
      (*starting_index)++;
      break;
    case INS_KEY:
      memcpy(array + *starting_index, wallet->key, sizeof(wallet->key));
      *starting_index =
          *starting_index + (sizeof(wallet->key) / sizeof(uint8_t));
      break;
    case INS_BENEFICIARY_KEY:
      memcpy(array + *starting_index,
             wallet->beneficiary_key,
             sizeof(wallet->beneficiary_key));
      *starting_index =
          *starting_index + (sizeof(wallet->beneficiary_key) / sizeof(uint8_t));
      break;
    case INS_IV_FOR_BENEFICIARY_KEY:
      memcpy(array + *starting_index,
             wallet->iv_for_beneficiary_key,
             sizeof(wallet->iv_for_beneficiary_key));
      *starting_index =
          *starting_index +
          (sizeof(wallet->iv_for_beneficiary_key) / sizeof(uint8_t));
      break;
    case INS_WALLET_ID:
      memcpy(array + *starting_index,
             wallet->wallet_id,
             sizeof(wallet->wallet_id));
      *starting_index =
          *starting_index + (sizeof(wallet->wallet_id) / sizeof(uint8_t));
      break;
    case INS_ARBITRARY_DATA:
      if (length == 1)
        array[(*starting_index)++] = length;
      memcpy(array + *starting_index,
             wallet->arbitrary_data_share,
             wallet->arbitrary_data_size);
      *starting_index = *starting_index + wallet->arbitrary_data_size;
      break;
    default:
      break;
  }
}

uint16_t create_apdu_pair(const uint8_t *data,
                          const uint16_t length,
                          uint8_t *apdu) {
  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_PAIR;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;
  apdu[OFFSET_LC] = length;

  // data: (device_key_id || device_nonce || device_derivation_path)
  memcpy(apdu + OFFSET_CDATA, data, length * sizeof(uint8_t));

  return length + 5;
}

uint16_t create_apdu_add_wallet(const struct Wallet *wallet, uint8_t apdu[]) {
  ASSERT(apdu != NULL);
  ASSERT(wallet != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_ADD_WALLET;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  fill_tlv(apdu, &index, INS_NAME, NAME_SIZE, wallet);
  fill_tlv(apdu, &index, INS_PASSWORD, BLOCK_SIZE, wallet);
  fill_tlv(apdu, &index, INS_xCor, 1, wallet);
  fill_tlv(apdu, &index, INS_NO_OF_MNEMONICS, 1, wallet);
  fill_tlv(apdu, &index, INS_TOTAL_NO_OF_SHARE, 1, wallet);
  fill_tlv(apdu,
           &index,
           INS_WALLET_SHARE,
           BLOCK_SIZE + PADDED_NONCE_SIZE + WALLET_MAC_SIZE,
           wallet);
  fill_tlv(apdu, &index, INS_STRUCTURE_CHECKSUM, CHECKSUM_SIZE, wallet);
  fill_tlv(apdu, &index, INS_MIN_NO_OF_SHARES, 1, wallet);
  fill_tlv(apdu, &index, INS_WALLET_INFO, 1, wallet);
  fill_tlv(apdu, &index, INS_KEY, KEY_SIZE, wallet);
  fill_tlv(apdu, &index, INS_BENEFICIARY_KEY, BENEFICIARY_KEY_SIZE, wallet);
  fill_tlv(apdu,
           &index,
           INS_IV_FOR_BENEFICIARY_KEY,
           IV_FOR_BENEFICIARY_KEY_SIZE,
           wallet);
  fill_tlv(apdu, &index, INS_WALLET_ID, WALLET_ID_SIZE, wallet);

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint16_t create_apdu_add_arbitrary_data(const struct Wallet *wallet,
                                        uint8_t apdu[]) {
  ASSERT(apdu != NULL);
  ASSERT(wallet != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_ADD_WALLET;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  fill_tlv(apdu, &index, INS_NAME, NAME_SIZE, wallet);
  fill_tlv(apdu, &index, INS_PASSWORD, BLOCK_SIZE, wallet);
  fill_tlv(apdu, &index, INS_xCor, 1, wallet);
  fill_tlv(apdu, &index, INS_TOTAL_NO_OF_SHARE, 1, wallet);
  fill_tlv(apdu, &index, INS_STRUCTURE_CHECKSUM, CHECKSUM_SIZE, wallet);
  fill_tlv(apdu, &index, INS_MIN_NO_OF_SHARES, 1, wallet);
  fill_tlv(apdu, &index, INS_WALLET_INFO, 1, wallet);
  fill_tlv(apdu, &index, INS_BENEFICIARY_KEY, BENEFICIARY_KEY_SIZE, wallet);
  fill_tlv(apdu,
           &index,
           INS_IV_FOR_BENEFICIARY_KEY,
           IV_FOR_BENEFICIARY_KEY_SIZE,
           wallet);
  fill_tlv(apdu, &index, INS_WALLET_ID, WALLET_ID_SIZE, wallet);
  fill_tlv(
      apdu, &index, INS_ARBITRARY_DATA, wallet->arbitrary_data_size, wallet);

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint16_t create_apdu_retrieve_wallet(const struct Wallet *wallet,
                                     uint8_t apdu[]) {
  ASSERT(apdu != NULL);
  ASSERT(wallet != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_RETRIEVE_WALLET;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  fill_tlv(apdu, &index, INS_NAME, NAME_SIZE, wallet);
  fill_tlv(apdu, &index, INS_PASSWORD, BLOCK_SIZE, wallet);

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint16_t create_apdu_delete_wallet(const struct Wallet *wallet,
                                   uint8_t apdu[]) {
  ASSERT(apdu != NULL);
  ASSERT(wallet != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_DELETE_WALLET;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  fill_tlv(apdu, &index, INS_NAME, NAME_SIZE, wallet);
  fill_tlv(apdu, &index, INS_PASSWORD, BLOCK_SIZE, wallet);

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint8_t create_apdu_list_wallet(uint8_t apdu[]) {
  ASSERT(apdu != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_LIST_ALL_WALLET;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;
  apdu[OFFSET_LC] = 0x00;
  apdu[OFFSET_CDATA] = 0x00;

  return 5;
}

uint8_t create_apdu_select_applet(uint8_t apdu[]) {
  ASSERT(apdu != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = INS_SELECT;
  apdu[OFFSET_P1] = 0x04;
  apdu[OFFSET_P2] = 0x00;
  apdu[OFFSET_LC] = 0x05;

  uint8_t ofs = OFFSET_CDATA;

  apdu[ofs++] = 0x01;
  apdu[ofs++] = 0x02;
  apdu[ofs++] = 0x03;
  apdu[ofs++] = 0x04;
  apdu[ofs++] = 0x05;

  return 10;
}

uint16_t create_apdu_ecdsa(const uint8_t data[],
                           const uint16_t length,
                           uint8_t apdu[]) {
  ASSERT(apdu != NULL);
  ASSERT(data != NULL);
  ASSERT(length != 0);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_SIGN_DATA_ECDSA;
  apdu[OFFSET_P1] = 0x00;
  apdu[OFFSET_P2] = 0x00;
  apdu[OFFSET_LC] = length;

  memcpy(apdu + OFFSET_CDATA, data, length * sizeof(uint8_t));

  return length + 5;
}

uint16_t create_apdu_verify_challenge(const uint8_t name[NAME_SIZE],
                                      const uint8_t nonce[POW_NONCE_SIZE],
                                      const uint8_t password[BLOCK_SIZE],
                                      uint8_t apdu[]) {
  ASSERT(name != NULL);
  ASSERT(nonce != NULL);
  ASSERT(password != NULL);
  ASSERT(apdu != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_PROOF_OF_WORK;
  apdu[OFFSET_P1] = P1_POW_VERIFY_CHALLENGE;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  apdu[index++] = INS_NAME;
  apdu[index++] = NAME_SIZE;
  memcpy(apdu + index, name, NAME_SIZE);
  index += NAME_SIZE;

  apdu[index++] = TAG_POW_NONCE;
  apdu[index++] = POW_NONCE_SIZE;
  memcpy(apdu + index, nonce, POW_NONCE_SIZE);
  index += POW_NONCE_SIZE;

  apdu[index++] = INS_PASSWORD;
  apdu[index++] = BLOCK_SIZE;
  memcpy(apdu + index, password, BLOCK_SIZE);
  index += BLOCK_SIZE;

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint16_t create_apdu_get_challenge(const uint8_t name[NAME_SIZE],
                                   uint8_t apdu[]) {
  ASSERT(name != NULL);
  ASSERT(apdu != NULL);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_PROOF_OF_WORK;
  apdu[OFFSET_P1] = P1_POW_GET_CHALLENGE;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  apdu[index++] = INS_NAME;
  apdu[index++] = NAME_SIZE;
  memcpy(apdu + index, name, NAME_SIZE);
  index += NAME_SIZE;

  apdu[OFFSET_LC] = index - 5;

  return index;
}

uint16_t create_apdu_inheritance(const uint8_t name[NAME_SIZE],
                                 const uint8_t *data,
                                 const uint16_t data_size,
                                 uint8_t apdu[],
                                 const uint8_t operation) {
  ASSERT(name != NULL);
  ASSERT(data != NULL);
  ASSERT(apdu != NULL);
  ASSERT(data_size != 0);
  ASSERT(operation != 0);

  apdu[OFFSET_CLA] = CLA_ISO7816;
  apdu[OFFSET_INS] = APDU_INHERITANCE;
  apdu[OFFSET_P1] = operation;
  apdu[OFFSET_P2] = 0x00;

  uint16_t index = 5;

  apdu[index++] = INS_NAME;
  apdu[index++] = NAME_SIZE;
  memcpy(apdu + index, name, NAME_SIZE);
  index += NAME_SIZE;

  apdu[index++] = operation == P1_INHERITANCE_ENCRYPT_DATA
                      ? TAG_INHERITANCE_PLAIN_DATA
                      : TAG_INHERITANCE_ENCRYPTED_DATA;
  apdu[index++] = data_size;
  memcpy(apdu + index, data, data_size);
  index += data_size;

  apdu[OFFSET_LC] = index - 5;

  return index;
}

void extract_apdu_get_challenge(uint8_t target[SHA256_SIZE],
                                uint8_t random_number[POW_RAND_NUMBER_SIZE],
                                const uint8_t apdu[],
                                const uint16_t len) {
  ASSERT(target != NULL);
  ASSERT(random_number != NULL);
  ASSERT(apdu != NULL);
  ASSERT(len != 0);

  uint16_t index = 0;

  while (index < len) {
    switch (apdu[index++]) {
      case TAG_POW_TARGET:
        memcpy(target, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case TAG_POW_RANDOM_NUM:
        memcpy(random_number, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      default:
        break;
    }
  }
}

void extract_from_apdu(struct Wallet *wallet,
                       const uint8_t apdu[],
                       const uint16_t len) {
  ASSERT(wallet != NULL);
  ASSERT(apdu != NULL);
  ASSERT(len != 0);

  uint16_t index = 0;

  while (index < len) {
    switch (apdu[index++]) {
      case INS_NAME:
        memcpy(wallet->wallet_name, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_PASSWORD:
        memcpy(wallet->password_double_hash, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_xCor:
        wallet->xcor = apdu[index];
        index++;
        break;
      case INS_NO_OF_MNEMONICS:
        wallet->number_of_mnemonics = apdu[index];
        index++;
        break;
      case INS_TOTAL_NO_OF_SHARE:
        wallet->total_number_of_shares = apdu[index];
        index++;
        break;
      case INS_WALLET_SHARE:
        memcpy(wallet->wallet_share_with_mac_and_nonce,
               apdu + index + 1,
               apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_STRUCTURE_CHECKSUM:
        memcpy(wallet->checksum, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_MIN_NO_OF_SHARES:
        wallet->minimum_number_of_shares = apdu[index];
        index++;
        break;
      case INS_WALLET_INFO:
        wallet->wallet_info = apdu[index];
        index++;
        break;
      case INS_KEY:
        memcpy(wallet->key, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_BENEFICIARY_KEY:
        memcpy(wallet->beneficiary_key, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_IV_FOR_BENEFICIARY_KEY:
        memcpy(wallet->iv_for_beneficiary_key, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_WALLET_ID:
        memcpy(wallet->wallet_id, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case INS_ARBITRARY_DATA:
        memcpy(wallet->arbitrary_data_share, apdu + index + 1, apdu[index]);
        wallet->arbitrary_data_size = apdu[index];
        index += (apdu[index] + 1);
        break;
      default:
        break;
    }
  }
}

ISO7816 extract_card_detail_from_apdu(const uint8_t apdu[],
                                      const uint8_t len,
                                      uint8_t family_id[],
                                      uint8_t *version,
                                      uint8_t *card_number,
                                      uint8_t *card_key_id,
                                      uint8_t *recovery_mode) {
  ASSERT(apdu != NULL);
  ASSERT(family_id != NULL);
  ASSERT(card_number != NULL);
  ASSERT(len != 0);

  ISO7816 status_word;
  status_word = (apdu[len - 2] * 256);
  status_word += apdu[len - 1];

  uint16_t index = 0;

  while (index < len) {
    switch (apdu[index++]) {
      case TAG_VERSION:
        if (version)
          memcpy(version, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case TAG_FAMILY_ID:
        memcpy(family_id, apdu + index + 1, FAMILY_ID_SIZE);
        index += CARD_ID_SIZE;
        *card_number = apdu[index];
        index++;
        break;
      case TAG_CARD_KEYID:
        if (card_key_id)
          memcpy(card_key_id, apdu + index + 1, 4);
        index += (apdu[index] + 1);
        break;
      case TAG_CARD_IV:
        memcpy(session_iv, apdu + index + 1, apdu[index]);
        index += (apdu[index] + 1);
        break;
      case TAG_RECOVERY_MODE:
        if (recovery_mode)
          *recovery_mode = apdu[++index];
        break;
      default:
        break;
    }
  }

  return status_word;
}

void init_session_keys(const uint8_t enc_key[32],
                       const uint8_t mac_key[32],
                       const uint8_t iv[16]) {
  if (!enc_key || !mac_key)
    return;

  memcpy(session_enc_key, enc_key, 32);
  memcpy(session_mac_key, mac_key, 32);
}

int apdu_encrypt_data(uint8_t *InOut_data, uint16_t *data_len) {
  ASSERT(InOut_data != NULL);
  ASSERT(data_len != NULL);

  uint16_t len =
      *data_len + 1;    // plaintext data length + 1 required padding byte
  uint16_t padding_len = (16 - (len % 16)) % 16;
  uint8_t payload[len + padding_len];
  aes_encrypt_ctx ctx = {0};

  if (aes_encrypt_key256(session_enc_key, &ctx) != EXIT_SUCCESS)
    return NFC_SC_ENC_KEY_ERROR;

  memcpy(payload, InOut_data, len);

  payload[len - 1] = 0x80;
  if (padding_len > 0)
    memset(payload + len, 0, padding_len);
  if (aes_cbc_encrypt(
          payload, InOut_data + 16, sizeof(payload), session_iv, &ctx) !=
      EXIT_SUCCESS)
    return NFC_SC_ENC_ERROR;

  memset(&ctx, 0, sizeof(ctx));
  memzero(session_iv, sizeof(session_iv));

  if (aes_encrypt_key256(session_mac_key, &ctx) != EXIT_SUCCESS)
    return NFC_SC_MAC_KEY_ERROR;
  if (aes_cbc_encrypt(
          InOut_data + 16, payload, sizeof(payload), session_iv, &ctx) !=
      EXIT_SUCCESS)
    return NFC_SC_MAC_ERROR;
  memcpy(InOut_data, payload + sizeof(payload) - 16, 16);
  memcpy(session_iv, payload + sizeof(payload) - 16, 16);

  *data_len = len + padding_len + 16;
  return 0;
}

int apdu_decrypt_data(uint8_t *InOut_data, uint8_t *len) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);

  uint16_t data_len = *len - 16 - 2;
  uint8_t payload[data_len], iv[16] = {0};
  aes_decrypt_ctx dec_ctx = {0};
  aes_encrypt_ctx enc_ctx = {0};

  if (aes_encrypt_key256(session_mac_key, &enc_ctx) != EXIT_SUCCESS)
    return NFC_SC_MAC_KEY_ERROR;
  if (aes_cbc_encrypt(
          InOut_data + 16, payload, sizeof(payload), iv, &enc_ctx) !=
      EXIT_SUCCESS)
    return NFC_SC_MAC_ERROR;
  if (memcmp(payload + data_len - 16, InOut_data, 16) != 0)
    return NFC_SC_MAC_MISMATCH;
  if (aes_decrypt_key256(session_enc_key, &dec_ctx) != EXIT_SUCCESS)
    return NFC_SC_DEC_KEY_ERROR;

  memcpy(iv, session_iv, sizeof(session_iv));
  memcpy(session_iv, InOut_data, sizeof(session_iv));
  memcpy(payload, InOut_data + 16, data_len);

  if (aes_cbc_decrypt(payload, InOut_data, data_len, iv, &dec_ctx) !=
      EXIT_SUCCESS)
    return NFC_SC_DEC_ERROR;
  while (InOut_data[data_len - 1] == 0x00)
    data_len--;
  if (InOut_data[data_len - 1] == 0x80)
    data_len--;
  InOut_data[data_len] = InOut_data[*len - 2];
  InOut_data[data_len + 1] = InOut_data[*len - 1];
  *len = data_len + 2;

  return 0;
}

uint16_t extract_card_data_health(uint8_t apdu[], const uint16_t len) {
  card_data_health = DATA_HEALTH_UNKNOWN;
  ASSERT(apdu != NULL);

  if (len < 5)
    return len;
  if (apdu[len - 5] != TAG_DATA_DISCREPANCY || apdu[len - 4] != 1)
    return len;
  card_data_health =
      apdu[len - 3] == 0xFF ? DATA_HEALTH_CORRUPT : DATA_HEALTH_OK;
  apdu[len - 5] = apdu[len - 2];
  apdu[len - 4] = apdu[len - 1];

  return len - 3;
}

Card_Data_Health get_card_data_health() {
  return card_data_health;
}

void reset_card_data_health() {
  card_data_health = DATA_HEALTH_UNKNOWN;
}

void apdu_extract_wallet_list(wallet_list_t *list,
                              uint8_t *apdu,
                              uint16_t len) {
  if (NULL == list || NULL == apdu || 0 == len) {
    return;
  }

  uint8_t read_offset = 0;
  // First byte of the APDU depicts number of wallets
  list->count = apdu[read_offset++];

  for (uint8_t index = 0; index < list->count; index++) {
    if (read_offset < len && INS_WALLET_INFO == apdu[read_offset++]) {
      list->wallet[index].info = apdu[read_offset++];
    }

    if (read_offset < len && TAG_WALLET_LOCKED == apdu[read_offset++]) {
      list->wallet[index].locked = apdu[read_offset++];
    }

    if (read_offset < len && INS_NAME == apdu[read_offset++]) {
      memcpy(
          list->wallet[index].name, apdu + read_offset + 1, apdu[read_offset]);
      read_offset += (apdu[read_offset] + 1);
    }

    if (read_offset < len && INS_WALLET_ID == apdu[read_offset++]) {
      memcpy(list->wallet[index].id, apdu + read_offset + 1, apdu[read_offset]);
      read_offset += (apdu[read_offset] + 1);
    }
  }

  return;
}
