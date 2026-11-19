/**
 * @file    flash_struct.c
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
#include "flash_struct.h"

#include <stdint.h>
#include <string.h>

#include "assert_conf.h"
#include "base58.h"
#include "board.h"
#include "chacha20poly1305.h"
#include "flash_if.h"
#include "flash_struct_priv.h"
#include "logger.h"
#include "memzero.h"
#include "pow_utilities.h"
#include "rfc7539.h"

/**
 * @brief Calculates the size of the TLV for memory allocation.
 *
 * The first 6 bytes consist of 4 bytes of TAG_FLASH_STRUCT + 2 bytes of total
 * structure size. In the calculation 3 is the size of TAG (1 byte) + size of
 * LENGTH (2 bytes) and 15 is the number of times the TAG, LENGTH, VALUE
 * combination occurs in Flash_Wallet. The number of tags of Flash_Pow are
 * included in the 15 number.
 */
#define FLASH_STRUCT_TLV_SIZE                                                  \
  (6 + 3 + FAMILY_ID_SIZE + 3 + sizeof(uint32_t) + 3 +                         \
   (MAX_WALLETS_ALLOWED * ((15 * 3) + sizeof(Flash_Wallet))) + 3 +             \
   sizeof(uint8_t) + 3 + sizeof(uint8_t) + 3 + sizeof(uint8_t) + 3 +           \
   sizeof(uint8_t)) +                                                          \
      3 + sizeof(uint8_t)

/// The size of tlv that will be read and written to flash. Since we read/write
/// in multiples of 4 hence it is essential to make the size divisible by 4.
#define FLASH_STRUCT_TLV_RW_SIZE                                               \
  (FLASH_STRUCT_TLV_SIZE +                                                     \
   (FLASH_STRUCT_TLV_SIZE % 4 == 0 ? 0 : 4 - (FLASH_STRUCT_TLV_SIZE % 4)))

#define FLASH_WRITE_STRUCTURE_SIZE sizeof(Flash_Struct)

/// Tags  for TLV
typedef enum Flash_tlv_tags {
  TAG_FLASH_STRUCT = 0xAAAAAAAA,
  TAG_FLASH_FAMILY_ID = 0x01,
  TAG_FLASH_WALLET_COUNT = 0x02,
  TAG_FLASH_WALLET_LIST = 0x03,
  TAG_UNUSED_1 = 0x04,
  TAG_UNUSED_2 = 0x05,
  TAG_FLASH_DISPLAY_ROTATION = 0x06,
  TAG_FLASH_TOGGLE_PASSPHRASE = 0x07,
  TAG_FLASH_TOGGLE_LOGS = 0x08,
  TAG_FLASH_ONBOARDING_STEP = 0x09,
  TAG_FLASH_TOGGLE_RAW_CALLDATA = 0x10,

  TAG_FLASH_WALLET = 0x20,
  TAG_FLASH_WALLET_STATE = 0x21,
  TAG_FLASH_WALLET_CARD_STATE = 0x22,
  TAG_FLASH_WALLET_INFO = 0x23,
  TAG_FLASH_WALLET_BUNKER_STATE = 0x24,
  TAG_FLASH_WALLET_PARTIAL_SHARE = 0x25,
  TAG_FLASH_WALLET_NAME = 0x26,
  TAG_FLASH_WALLET_LOCKED = 0x27,
  TAG_FLASH_WALLET_CHALLENGE = 0x28,
  TAG_FLASH_WALLET_ID = 0x29,

  TAG_FLASH_WALLET_CHALLENGE_TARGET = 0x40,
  TAG_FLASH_WALLET_CHALLENGE_RANDOM_NUMBER = 0x41,
  TAG_FLASH_WALLET_CHALLENGE_NONCE = 0x42,
  TAG_FLASH_WALLET_CHALLENGE_CARD_LOCKED = 0x43,
  TAG_FLASH_WALLET_CHALLENGE_TIME_LOCKED = 0x44,

} Flash_tlv_tags;

Flash_Struct flash_ram_instance;
bool is_flash_ram_instance_loaded = false;

static void deserialize_fs(Flash_Struct *flash_struct,
                           const uint8_t *tlv,
                           uint32_t tlv_size);
static uint16_t serialize_fs(const Flash_Struct *flash_struct, uint8_t *tlv);

/**
 * @brief Load flash struct instance
 *
 */
static void flash_struct_load() {
  ASSERT((&flash_ram_instance) != NULL);
#if USE_SIMULATOR == 1
  uint32_t serialized_flash_metadata[2];
  read_cmd(FLASH_DATA_ADDRESS, serialized_flash_metadata, 8);
#else
  uint8_t *serialized_flash_metadata = (uint8_t *)FLASH_DATA_ADDRESS;
#endif
  uint32_t serialized_flash_struct_tag = serialized_flash_metadata[0] +
                                         (serialized_flash_metadata[1] << 8) +
                                         (serialized_flash_metadata[2] << 16) +
                                         (serialized_flash_metadata[3] << 24);
  uint16_t serialized_flash_size =
      serialized_flash_metadata[4] + (serialized_flash_metadata[5] << 8);

  if (serialized_flash_struct_tag == TAG_FLASH_STRUCT &&
      serialized_flash_size <= FLASH_DATA_SIZE_LIMIT) {
    // 6 is added to include the TAG_FLASH_STRUCT and length of the serialized
    // structure
    uint16_t serialized_flash_size_tagged = serialized_flash_size + 6;

    uint8_t *serialized_flash_instance =
        (uint8_t *)malloc(serialized_flash_size_tagged);
    ASSERT(serialized_flash_instance != NULL);
    read_cmd(FLASH_DATA_ADDRESS,
             (uint32_t *)serialized_flash_instance,
             serialized_flash_size_tagged);
    deserialize_fs(&flash_ram_instance,
                   serialized_flash_instance,
                   serialized_flash_size_tagged);
    free(serialized_flash_instance);
    serialized_flash_instance = NULL;
  } else {
    LOG_CRITICAL("xxxa");
    erase_cmd(FLASH_DATA_ADDRESS, FLASH_STRUCT_TLV_SIZE);
    memset(&flash_ram_instance,
           DEFAULT_VALUE_IN_FLASH,
           FLASH_WRITE_STRUCTURE_SIZE);
  }

  if (flash_ram_instance.wallet_count == DEFAULT_UINT32_IN_FLASH) {
    flash_ram_instance.wallet_count = 0;
  }

  // a corrupt count is rebuilt from the wallet slots, as callers rely on it
  // being at most MAX_WALLETS_ALLOWED
  if (flash_ram_instance.wallet_count > MAX_WALLETS_ALLOWED) {
    flash_ram_instance.wallet_count = 0;
    for (uint8_t i = 0; i < MAX_WALLETS_ALLOWED; i++) {
      uint8_t state = flash_ram_instance.wallets[i].state;
      if (VALID_WALLET == state || UNVERIFIED_VALID_WALLET == state ||
          INVALID_WALLET == state ||
          VALID_WALLET_WITHOUT_DEVICE_SHARE == state) {
        flash_ram_instance.wallet_count++;
      }
    }
  }
}

/**
 * @brief
 *
 */
void flash_struct_save() {
  ASSERT((&flash_ram_instance) != NULL);
  uint8_t *serialized_flash_instance = (uint8_t *)malloc(FLASH_STRUCT_TLV_SIZE);
  ASSERT(serialized_flash_instance != NULL);
  serialize_fs(&flash_ram_instance, serialized_flash_instance);
  erase_cmd(FLASH_DATA_ADDRESS, FLASH_STRUCT_TLV_SIZE);
  write_cmd(FLASH_DATA_ADDRESS,
            (uint32_t *)serialized_flash_instance,
            FLASH_STRUCT_TLV_SIZE);
  free(serialized_flash_instance);
  serialized_flash_instance = NULL;
}

/**
 * @brief Get the flash ram instance object
 *
 * @return const Flash_Struct*
 */
const Flash_Struct *get_flash_ram_instance() {
  ASSERT((&flash_ram_instance) != NULL);

  if (!is_flash_ram_instance_loaded) {
    flash_struct_load();
    is_flash_ram_instance_loaded = true;
  }
  return &flash_ram_instance;
}

/**
 * @brief
 *
 */
void flash_erase() {
  erase_cmd(FLASH_DATA_ADDRESS, FLASH_STRUCT_TLV_SIZE);
  memset(
      &flash_ram_instance, DEFAULT_VALUE_IN_FLASH, FLASH_WRITE_STRUCTURE_SIZE);

  if (flash_ram_instance.wallet_count == DEFAULT_UINT32_IN_FLASH) {
    flash_ram_instance.wallet_count = 0;
  }
  is_flash_ram_instance_loaded = false;
}

void flash_clear_user_data(void) {
  /* Copy data to be preserved on the flash memory */
  get_flash_ram_instance();
  uint8_t last_onboarding_step = flash_ram_instance.onboarding_step;

  /* Erase the flash page to get a clean slate */
  flash_erase();

  /* Restore data on to the flash */
  save_onboarding_step(last_onboarding_step);
  return;
}

/**
 * @brief Fills the TLV for the passed tag and passed flash object.
 * flash_obj is type casted to an appropriate struct type before accessing its
 * members.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param tag Flash_tlv_tags tag for TLV
 * @param length Size of the value
 * @param data Byte data array that will be filled in the TLV array
 */
static void fill_flash_tlv(uint8_t *array,
                           uint16_t *starting_index,
                           const uint8_t tag,
                           const uint16_t length,
                           const uint8_t *data) {
  array[(*starting_index)++] = tag;
  array[(*starting_index)++] = length;
  array[(*starting_index)++] = (length >> 8);

  memcpy(array + *starting_index, data, length);
  *starting_index = *starting_index + length;
}

/**
 * @brief Function to fill the TLV with the properties of Flash_Pow struct.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param tag Flash_tlv_tags tag for TLV
 * @param length Size of the value
 * @param flash_pow Pointer to the Flash_Pow instance
 */
static void serialize_fs_pow(uint8_t *array,
                             uint16_t *starting_index,
                             const Flash_Pow *flash_pow) {
  array[(*starting_index)++] = TAG_FLASH_WALLET_CHALLENGE;

  uint16_t pow_len_index = (*starting_index);
  (*starting_index) += 2;

  fill_flash_tlv(array,
                 starting_index,
                 TAG_FLASH_WALLET_CHALLENGE_TARGET,
                 SHA256_SIZE,
                 (uint8_t *)flash_pow->target);
  fill_flash_tlv(array,
                 starting_index,
                 TAG_FLASH_WALLET_CHALLENGE_RANDOM_NUMBER,
                 POW_RAND_NUMBER_SIZE,
                 (uint8_t *)flash_pow->random_number);
  fill_flash_tlv(array,
                 starting_index,
                 TAG_FLASH_WALLET_CHALLENGE_NONCE,
                 POW_NONCE_SIZE,
                 (uint8_t *)flash_pow->nonce);
  fill_flash_tlv(array,
                 starting_index,
                 TAG_FLASH_WALLET_CHALLENGE_CARD_LOCKED,
                 sizeof(flash_pow->card_locked),
                 (uint8_t *)(&(flash_pow->card_locked)));
  fill_flash_tlv(array,
                 starting_index,
                 TAG_FLASH_WALLET_CHALLENGE_TIME_LOCKED,
                 sizeof(flash_pow->time_to_unlock_in_secs),
                 (uint8_t *)(&(flash_pow->time_to_unlock_in_secs)));

  array[pow_len_index] = (*starting_index) - pow_len_index - 2;
  array[pow_len_index + 1] = ((*starting_index) - pow_len_index - 2) >> 8;
}

/**
 * @brief Function to fill the TLV with the properties of Flash_Wallet struct.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param tag Flash_tlv_tags tag for TLV
 * @param length Size of the value
 * @param flash_struct Pointer to the Flash_Struct instance ot access the list
 * of Flash_Wallet structs
 */
static void serialize_fs_wallet(uint8_t *array,
                                uint16_t *starting_index,
                                const Flash_Struct *flash_struct) {
  array[(*starting_index)++] = TAG_FLASH_WALLET_LIST;

  const Flash_Wallet *wallet = NULL;
  uint16_t wallet_list_len_index = (*starting_index);
  (*starting_index) += 2;

  for (uint8_t wallet_index = 0; wallet_index < MAX_WALLETS_ALLOWED;
       wallet_index++) {
    array[(*starting_index)++] = TAG_FLASH_WALLET;

    uint16_t len_index = (*starting_index);
    (*starting_index) += 2;
    wallet = &(flash_struct->wallets[wallet_index]);

    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_STATE,
                   sizeof(wallet->state),
                   &(wallet->state));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_CARD_STATE,
                   sizeof(wallet->cards_states),
                   &(wallet->cards_states));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_INFO,
                   sizeof(wallet->wallet_info),
                   &(wallet->wallet_info));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_NAME,
                   NAME_SIZE,
                   (uint8_t *)(&(wallet->wallet_name)));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_LOCKED,
                   sizeof(wallet->is_wallet_locked),
                   (uint8_t *)(&(wallet->is_wallet_locked)));
    serialize_fs_pow(array, starting_index, &(wallet->challenge));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_FLASH_WALLET_ID,
                   WALLET_ID_SIZE,
                   (uint8_t *)(&(wallet->wallet_id)));

    array[len_index] = (*starting_index) - len_index - 2;
    array[len_index + 1] = ((*starting_index) - len_index - 2) >> 8;
  }

  array[wallet_list_len_index] =
      ((*starting_index) - wallet_list_len_index - 2);
  array[wallet_list_len_index + 1] =
      ((*starting_index) - wallet_list_len_index - 2) >> 8;
}

/**
 * @brief Create a tlv from the passed flash_struct object.
 *
 * @param flash_struct Flash Struct pointer
 * @param tlv TLV array
 * @return uint32_t Size of the TLV
 */
static uint16_t serialize_fs(const Flash_Struct *flash_struct, uint8_t *tlv) {
  tlv[0] = (uint8_t)(TAG_FLASH_STRUCT);
  tlv[1] = (uint8_t)(TAG_FLASH_STRUCT >> 8);
  tlv[2] = (uint8_t)(TAG_FLASH_STRUCT >> 16);
  tlv[3] = (uint8_t)(TAG_FLASH_STRUCT >> 24);

  uint16_t index = 6;

  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_FAMILY_ID,
                 FAMILY_ID_SIZE,
                 flash_struct->family_id);
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_WALLET_COUNT,
                 sizeof(flash_struct->wallet_count),
                 (const uint8_t *)(&(flash_struct->wallet_count)));
  serialize_fs_wallet(tlv, &index, flash_struct);
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_DISPLAY_ROTATION,
                 sizeof(flash_struct->displayRotation),
                 &(flash_struct->displayRotation));
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_TOGGLE_PASSPHRASE,
                 sizeof(flash_struct->enable_passphrase),
                 &(flash_struct->enable_passphrase));
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_TOGGLE_LOGS,
                 sizeof(flash_struct->enable_log),
                 &(flash_struct->enable_log));
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_ONBOARDING_STEP,
                 sizeof(flash_struct->onboarding_step),
                 &(flash_struct->onboarding_step));
  fill_flash_tlv(tlv,
                 &index,
                 TAG_FLASH_TOGGLE_RAW_CALLDATA,
                 sizeof(flash_struct->enable_raw_calldata),
                 &(flash_struct->enable_raw_calldata));

  tlv[4] = index - 6;
  tlv[5] = (index - 6) >> 8;

  return index;
}

/**
 * @brief Copies a TLV value into a field if it fits, so a corrupt length read
 * from flash can not overflow the field.
 *
 * @param field Destination field
 * @param field_size Size of the destination field
 * @param value TLV value
 * @param size Length of the TLV value
 */
static void copy_fs_field(void *field,
                          const size_t field_size,
                          const uint8_t *value,
                          const uint16_t size) {
  if (size <= field_size) {
    memcpy(field, value, size);
  }
}

/**
 * @brief Helper function to extract values of Flash_Pow from TLV.
 *
 * @param flash_pow Pointer to Flash_Pow instance to store the values
 * @param tlv TLV byte array
 * @param len Length of the stored Flash_Pow in TLV (including all the tags and
 * intermediate lengths). Note that this is not sizeof(Flash_Pow).
 */
static void deserialize_fs_pow(Flash_Pow *flash_pow,
                               const uint8_t *tlv,
                               const uint16_t len) {
  uint32_t index = 0;

  // each element is a tag, a 2-byte length and the value; parsing stops at an
  // element that runs past the challenge
  while (index + 3 <= len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);
    index += 2;
    if (size > len - index) {
      break;
    }
    const uint8_t *value = tlv + index;

    switch (tag) {
      case TAG_FLASH_WALLET_CHALLENGE_TARGET: {
        copy_fs_field(
            flash_pow->target, sizeof(flash_pow->target), value, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_RANDOM_NUMBER: {
        copy_fs_field(flash_pow->random_number,
                      sizeof(flash_pow->random_number),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_NONCE: {
        copy_fs_field(flash_pow->nonce, sizeof(flash_pow->nonce), value, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_CARD_LOCKED: {
        copy_fs_field(&(flash_pow->card_locked),
                      sizeof(flash_pow->card_locked),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_TIME_LOCKED: {
        copy_fs_field(&(flash_pow->time_to_unlock_in_secs),
                      sizeof(flash_pow->time_to_unlock_in_secs),
                      value,
                      size);
        break;
      }

      default: {
        break;
      }
    }
    index += size;
  }
}

/**
 * @brief Helper function to extract values of Flash_Wallet from TLV.
 *
 * @param flash_wallet Pointer to Flash_Wallet instance to store the values
 * @param tlv TLV byte array
 * @param len Length of the stored Flash_Wallet in TLV (including all the tags
 * and intermediate lengths). Note that this is not sizeof(Flash_Wallet).
 */
static void deserialize_fs_wallet(Flash_Wallet *flash_wallet,
                                  const uint8_t *tlv,
                                  const uint16_t len) {
  uint32_t index = 0;

  // each element is a tag, a 2-byte length and the value; parsing stops at an
  // element that runs past the wallet
  while (index + 3 <= len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);
    index += 2;
    if (size > len - index) {
      break;
    }
    const uint8_t *value = tlv + index;

    switch (tag) {
      case TAG_FLASH_WALLET_STATE: {
        copy_fs_field(
            &(flash_wallet->state), sizeof(flash_wallet->state), value, size);
        break;
      }

      case TAG_FLASH_WALLET_CARD_STATE: {
        copy_fs_field(&(flash_wallet->cards_states),
                      sizeof(flash_wallet->cards_states),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_INFO: {
        copy_fs_field(&(flash_wallet->wallet_info),
                      sizeof(flash_wallet->wallet_info),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_NAME: {
        copy_fs_field(flash_wallet->wallet_name,
                      sizeof(flash_wallet->wallet_name),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_LOCKED: {
        copy_fs_field(&(flash_wallet->is_wallet_locked),
                      sizeof(flash_wallet->is_wallet_locked),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE: {
        deserialize_fs_pow(&(flash_wallet->challenge), value, size);
        break;
      }

      case TAG_FLASH_WALLET_ID: {
        copy_fs_field(flash_wallet->wallet_id,
                      sizeof(flash_wallet->wallet_id),
                      value,
                      size);
        break;
      }

      default: {
        break;
      }
    }
    index += size;
  }

  // names are at most NAME_SIZE - 1 characters; the terminator keeps string
  // functions inside the field even if flash is corrupt
  flash_wallet->wallet_name[NAME_SIZE - 1] = 0;
}

/**
 * @brief Function to extract values of Flash_Struct from TLV.
 * @details Every length is read from flash, so each one is checked against
 * the enclosing element before use. Parsing stops at the first element that
 * does not fit, which leaves the rest of the structure as it was.
 *
 * @param flash_struct Pointer to Flash_Struct instance to store the values
 * @param tlv TLV byte array, starting with the header
 * @param tlv_size Size of the tlv buffer, including the header
 */
static void deserialize_fs(Flash_Struct *flash_struct,
                           const uint8_t *tlv,
                           uint32_t tlv_size) {
  // First 4 bytes are the TAG_FLASH_STRUCT, followed by the length
  uint32_t len = tlv[4] + (tlv[5] << 8) + 6;
  uint32_t index = 6;

  if (len > tlv_size) {
    len = tlv_size;
  }

  while (index + 3 <= len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);
    index += 2;
    if (size > len - index) {
      break;
    }
    const uint8_t *value = tlv + index;
    const uint32_t list_end = index + size;

    switch (tag) {
      case TAG_FLASH_FAMILY_ID: {
        copy_fs_field(flash_struct->family_id,
                      sizeof(flash_struct->family_id),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_COUNT: {
        copy_fs_field(&(flash_struct->wallet_count),
                      sizeof(flash_struct->wallet_count),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_WALLET_LIST: {
        uint32_t offset = index;
        for (uint8_t wallet_index = 0; wallet_index < MAX_WALLETS_ALLOWED;
             wallet_index++) {
          if (offset >= list_end) {
            break;
          }
          if (tlv[offset++] == TAG_FLASH_WALLET) {
            if (offset + 2 > list_end) {
              break;
            }
            uint16_t wallet_len = tlv[offset] + (tlv[offset + 1] << 8);
            offset += 2;
            if (wallet_len > list_end - offset) {
              break;
            }
            deserialize_fs_wallet(&(flash_struct->wallets[wallet_index]),
                                  tlv + offset,
                                  wallet_len);
            offset += wallet_len;
          }
        }
        break;
      }

      case TAG_FLASH_DISPLAY_ROTATION: {
        copy_fs_field(&(flash_struct->displayRotation),
                      sizeof(flash_struct->displayRotation),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_TOGGLE_PASSPHRASE: {
        copy_fs_field(&(flash_struct->enable_passphrase),
                      sizeof(flash_struct->enable_passphrase),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_TOGGLE_LOGS: {
        copy_fs_field(&(flash_struct->enable_log),
                      sizeof(flash_struct->enable_log),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_ONBOARDING_STEP: {
        copy_fs_field(&(flash_struct->onboarding_step),
                      sizeof(flash_struct->onboarding_step),
                      value,
                      size);
        break;
      }

      case TAG_FLASH_TOGGLE_RAW_CALLDATA: {
        copy_fs_field(&(flash_struct->enable_raw_calldata),
                      sizeof(flash_struct->enable_raw_calldata),
                      value,
                      size);
        break;
      }

      default: {
        break;
      }
    }
    index += size;
  }
}
