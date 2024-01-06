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
   sizeof(uint8_t))

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

static void deserialize_fs(Flash_Struct *flash_struct, uint8_t *tlv);
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
    deserialize_fs(&flash_ram_instance, serialized_flash_instance);
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
  tlv[4] = index - 6;
  tlv[5] = (index - 6) >> 8;

  return index;
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
  uint16_t index = 0;
  while (index < len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);

    switch (tag) {
      case TAG_FLASH_WALLET_CHALLENGE_TARGET: {
        memcpy(flash_pow->target, tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_RANDOM_NUMBER: {
        memcpy(flash_pow->random_number, tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_NONCE: {
        memcpy(flash_pow->nonce, tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_CARD_LOCKED: {
        memcpy(&(flash_pow->card_locked), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE_TIME_LOCKED: {
        memcpy(&(flash_pow->time_to_unlock_in_secs), tlv + index + 2, size);
        break;
      }

      default: {
        break;
      }
    }
    index += (size + 2);
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
  uint16_t index = 0;

  while (index < len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);

    switch (tag) {
      case TAG_FLASH_WALLET_STATE: {
        memcpy(&(flash_wallet->state), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CARD_STATE: {
        memcpy(&(flash_wallet->cards_states), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_INFO: {
        memcpy(&(flash_wallet->wallet_info), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_NAME: {
        memcpy(&(flash_wallet->wallet_name), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_LOCKED: {
        memcpy(&(flash_wallet->is_wallet_locked), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_CHALLENGE: {
        deserialize_fs_pow(&(flash_wallet->challenge), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_ID: {
        memcpy(flash_wallet->wallet_id, tlv + index + 2, size);
        break;
      }

      default: {
        break;
      }
    }
    index += (size + 2);
  }
}

/**
 * @brief Function to extract values of Flash_Struct from TLV.
 *
 * @param flash_struct Pointer to Flash_Struct instance to store the values
 * @param tlv TLV byte array
 */
static void deserialize_fs(Flash_Struct *flash_struct, uint8_t *tlv) {
  uint16_t index = 4;    // First 4 bytes are the TAG_FLASH_STRUCT
  uint16_t len = tlv[index] + (tlv[index + 1] << 8) + 6;

  index += 2;

  while (index < len) {
    Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);

    switch (tag) {
      case TAG_FLASH_FAMILY_ID: {
        memcpy(flash_struct->family_id, tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_COUNT: {
        memcpy(&(flash_struct->wallet_count), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_WALLET_LIST: {
        uint16_t offset = index + 2;
        for (uint8_t wallet_index = 0; wallet_index < MAX_WALLETS_ALLOWED;
             wallet_index++) {
          if (tlv[offset++] == TAG_FLASH_WALLET) {
            uint16_t wallet_len = tlv[offset] + (tlv[offset + 1] << 8);
            deserialize_fs_wallet(&(flash_struct->wallets[wallet_index]),
                                  tlv + offset + 2,
                                  wallet_len);
            offset += wallet_len + 2;
          }
        }
        // TODO: ASSERT(offset == index + size + 2);
        break;
      }

      case TAG_FLASH_DISPLAY_ROTATION: {
        memcpy(&(flash_struct->displayRotation), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_TOGGLE_PASSPHRASE: {
        memcpy(&(flash_struct->enable_passphrase), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_TOGGLE_LOGS: {
        memcpy(&(flash_struct->enable_log), tlv + index + 2, size);
        break;
      }

      case TAG_FLASH_ONBOARDING_STEP: {
        memcpy(&(flash_struct->onboarding_step), tlv + index + 2, size);
        break;
      }

      default: {
        break;
      }
    }
    index += (size + 2);
  }
}
