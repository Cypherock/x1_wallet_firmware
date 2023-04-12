/**
 * @file    coin_specific_data.c
 * @author  Cypherock X1 Team
 * @brief   Coin specific data.
 *          Contains functions for managing coin specific data.
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
#include "coin_specific_data.h"

void fill_flash_tlv(uint8_t *array,
                    uint16_t *starting_index,
                    uint8_t tag,
                    uint16_t length,
                    const uint8_t *data) {
  array[(*starting_index)++] = tag;
  array[(*starting_index)++] = length;
  array[(*starting_index)++] = (length >> 8);

  memcpy(array + *starting_index, data, length);
  *starting_index = *starting_index + length;
}

static uint16_t find_latest_coin_data(
    const Coin_Specific_Data_Struct *coin_specific_data_key,
    uint16_t *coin_data_length,
    uint32_t *coin_data_addr) {
  uint16_t offset = 0;
  uint8_t type_length[6] = {
      0};    // Will contain the type of the coin and the size of the data
  while (offset < FLASH_COIN_SPECIFIC_PAGE_COUNT * FLASH_PAGE_SIZE) {
    read_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
             (uint32_t *)type_length,
             sizeof(type_length));
    if (U32_READ_LE_ARRAY(type_length) == 0xFFFFFFFF) {
      break;
    }
    offset += sizeof(type_length);

    Coin_Type f_coin_type = U32_READ_LE_ARRAY(type_length);
    uint16_t data_length = U16_READ_LE_ARRAY(type_length + 4);

    if (f_coin_type == coin_specific_data_key->coin_type) {
      uint8_t temp[3] = {0};
      read_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
               (uint32_t *)temp,
               sizeof(temp));
      offset += sizeof(temp);

      Coin_Specific_Data_Tag f_wallet_id_tag = temp[0];

      if (f_wallet_id_tag != TAG_CSD_WALLET_ID)
        break;

      uint16_t f_wallet_id_length = U16_READ_LE_ARRAY(temp + 1);
      uint8_t f_wallet_id[f_wallet_id_length];

      read_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
               (uint32_t *)f_wallet_id,
               f_wallet_id_length);
      offset += f_wallet_id_length;
      if (memcmp(f_wallet_id,
                 coin_specific_data_key->wallet_id,
                 f_wallet_id_length) == 0) {
        read_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
                 (uint32_t *)temp,
                 sizeof(temp));
        offset += sizeof(temp);
        Coin_Specific_Data_Tag f_coin_data_tag = temp[0];

        if (f_coin_data_tag != TAG_CSD_COIN_DATA)
          break;

        *coin_data_length = U16_READ_LE_ARRAY(temp + 1);
        *coin_data_addr = FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset;
        offset += *coin_data_length;
      } else {
        offset += data_length - f_wallet_id_length - 3;
      }
    } else {
      offset += data_length;
    }
    offset = GET_NEXT_MULTIPLE_OF_8(offset);
  }
  return offset;
}

static uint16_t prepare_coin_specific_data_tlv(
    const Coin_Specific_Data_Struct *coin_specific_data,
    const uint16_t coin_data_size,
    uint8_t *tlv,
    uint16_t data_length) {
  uint16_t index = 0;

  memcpy(tlv,
         (uint8_t *)&coin_specific_data->coin_type,
         sizeof(coin_specific_data->coin_type));
  index += 4;
  tlv[index++] = data_length;
  tlv[index++] = (data_length >> 8);

  fill_flash_tlv(tlv,
                 &index,
                 TAG_CSD_WALLET_ID,
                 WALLET_ID_SIZE,
                 coin_specific_data->wallet_id);
  fill_flash_tlv(tlv,
                 &index,
                 TAG_CSD_COIN_DATA,
                 coin_data_size,
                 coin_specific_data->coin_data);

  // These variables are just used to get the latest offset of the coin data in
  // the flash.
  uint16_t coin_data_len = 0;
  uint32_t coin_data_addr = 0;
  return find_latest_coin_data(
      coin_specific_data, &coin_data_len, &coin_data_addr);
}

static void purge_coin_specific_data() {
  // Store all the unique data length and address in an array
  Coin_Type coin_type_arr[MAX_UNIQUE_COIN_COUNT] = {COIN_TYPE_NEAR};
  struct meta_data_t {
    Coin_Specific_Data_Struct data_struct;
    uint16_t data_length;
    uint32_t data_addr;
  } meta_data_arr[MAX_UNIQUE_COIN_COUNT * MAX_WALLETS_ALLOWED] = {0};

  for (size_t coin_type_index = 0; coin_type_index < MAX_UNIQUE_COIN_COUNT;
       coin_type_index++) {
    for (size_t wallet_id_index = 0; wallet_id_index < MAX_WALLETS_ALLOWED;
         wallet_id_index++) {
      size_t current_index =
          coin_type_index * MAX_WALLETS_ALLOWED + wallet_id_index;

      meta_data_arr[current_index].data_length = 0;
      meta_data_arr[current_index].data_addr = 0;

      meta_data_arr[current_index].data_struct.coin_type =
          coin_type_arr[coin_type_index];
      memcpy(meta_data_arr[current_index].data_struct.wallet_id,
             get_wallet_id(wallet_id_index),
             WALLET_ID_SIZE);
      find_latest_coin_data(&meta_data_arr[current_index].data_struct,
                            &meta_data_arr[current_index].data_length,
                            &meta_data_arr[current_index].data_addr);

      if (meta_data_arr[current_index].data_length > 0 &&
          FLASH_COIN_SPECIFIC_BASE_ADDRESS <
              meta_data_arr[current_index].data_addr &&
          meta_data_arr[current_index].data_addr +
                  meta_data_arr[current_index].data_length <
              FLASH_END) {
        // Allocate a proper size array to store the data
        meta_data_arr[current_index].data_struct.coin_data =
            (uint8_t *)malloc(meta_data_arr[current_index].data_length);
        ASSERT(meta_data_arr[current_index].data_struct.coin_data != NULL);
        read_cmd(meta_data_arr[current_index].data_addr,
                 (uint32_t *)meta_data_arr[current_index].data_struct.coin_data,
                 meta_data_arr[current_index].data_length);
      }
    }
  }

  // Erase everything from flash
  erase_flash_coin_specific_data();

  // Write the data back to flash
  for (size_t i = 0; i < MAX_UNIQUE_COIN_COUNT * MAX_WALLETS_ALLOWED; i++) {
    if (meta_data_arr[i].data_length > 0 &&
        FLASH_COIN_SPECIFIC_BASE_ADDRESS < meta_data_arr[i].data_addr &&
        meta_data_arr[i].data_addr + meta_data_arr[i].data_length < FLASH_END) {
      uint16_t data_length =
          (3 + WALLET_ID_SIZE) + (3 + meta_data_arr[i].data_length);
      uint16_t tlv_size = 6 + data_length;
      tlv_size = GET_NEXT_MULTIPLE_OF_8(tlv_size);
      uint8_t tlv[tlv_size];
      memzero(tlv, sizeof(tlv));

      uint16_t offset =
          prepare_coin_specific_data_tlv(&meta_data_arr[i].data_struct,
                                         meta_data_arr[i].data_length,
                                         tlv,
                                         data_length);
      write_cmd(
          FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset, (uint32_t *)tlv, tlv_size);

      free(meta_data_arr[i].data_struct.coin_data);
    }
  }
}

static int store_coin_data(const uint8_t *tlv_data,
                           uint16_t tlv_data_size,
                           uint16_t offset) {
  if ((offset + tlv_data_size) <=
      (FLASH_COIN_SPECIFIC_PAGE_COUNT * FLASH_PAGE_SIZE)) {
    write_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
              (uint32_t *)tlv_data,
              tlv_data_size);
  } else {
    purge_coin_specific_data();
    uint16_t coin_data_len = 0;
    uint32_t coin_data_addr = 0;
    Coin_Specific_Data_Struct dummy = {0};
    offset = find_latest_coin_data(&dummy, &coin_data_len, &coin_data_addr);
    if ((offset + tlv_data_size) <=
        (FLASH_COIN_SPECIFIC_PAGE_COUNT * FLASH_PAGE_SIZE)) {
      write_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS + offset,
                (uint32_t *)tlv_data,
                tlv_data_size);
    } else {
      return CSD_STATUS_NOT_ENOUGH_SPACE;
    }
  }
  return CSD_STATUS_OK;
}

// Exposed Functions

void erase_flash_coin_specific_data() {
  erase_cmd(FLASH_COIN_SPECIFIC_BASE_ADDRESS,
            FLASH_COIN_SPECIFIC_PAGE_COUNT * FLASH_PAGE_SIZE);
}

int get_coin_data(Coin_Specific_Data_Struct *coin_specific_data,
                  const uint16_t coin_data_max_len,
                  uint16_t *coin_data_len) {
  uint32_t coin_data_addr = 0;
  find_latest_coin_data(coin_specific_data, coin_data_len, &coin_data_addr);

  if (coin_data_addr == 0 || coin_data_max_len < *coin_data_len)
    return CSD_STATUS_DATA_NOT_FOUND;
  read_cmd(coin_data_addr,
           (uint32_t *)coin_specific_data->coin_data,
           *coin_data_len);

  return CSD_STATUS_OK;
}

int set_coin_data(const Coin_Specific_Data_Struct *coin_specific_data,
                  const uint16_t coin_data_size) {
  uint16_t data_length = (3 + WALLET_ID_SIZE) + (3 + coin_data_size);
  uint16_t tlv_size = 6 + data_length;
  tlv_size = GET_NEXT_MULTIPLE_OF_8(tlv_size);
  uint8_t tlv[tlv_size];
  memzero(tlv, sizeof(tlv));

  uint16_t offset = prepare_coin_specific_data_tlv(
      coin_specific_data, coin_data_size, tlv, data_length);
  return store_coin_data(tlv, sizeof(tlv), offset);
}
