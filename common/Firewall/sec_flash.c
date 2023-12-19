/**
 * @file    sec_flash.c
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
/*
 * sec_flash.c
 *
 *  Created on: Jul 9, 2021
 *      Author: amana
 */

#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "buzzer.h"
#include "coin_utils.h"
#include "sec_flash_priv.h"
#include "utils.h"

#define SEC_FLASH_STRUCT_TLV_SIZE                                              \
  (6 + 3 + (MAX_WALLETS_ALLOWED * (12 + sizeof(Wallet_Share_Data))) + 3 +      \
   (MAX_KEYSTORE_ENTRY * ((4 * 3) + sizeof(Card_Keystore))))

#define FLASH_WRITE_PERM_STRUCTURE_SIZE sizeof(Flash_Perm_Struct) / 4
typedef enum Sec_Flash_tlv_tags {
  TAG_SEC_FLASH_STRUCT = 0x5555AAAA,

  TAG_SEC_FLASH_KEYSTORE_LIST = 0x09,
  TAG_SEC_FLASH_WALLET_SHARE_STRUCT_LIST = 0x10,
  TAG_SEC_FLASH_WALLET_SHARE_STRUCT = 0x11,
  TAG_SEC_FLASH_WALLET_ID = 0x12,
  TAG_SEC_FLASH_WALLET_SHARE = 0x13,
  TAG_SEC_FLASH_WALLET_NONCE = 0x14,

  TAG_SEC_FLASH_KEYSTORE = 0x30,
  TAG_SEC_FLASH_KEYSTORE_USED = 0x31,
  TAG_SEC_FLASH_KEYSTORE_KEYID = 0x32,
  TAG_SEC_FLASH_KEYSTORE_PAIRING_KEY = 0x33,
} Sec_Flash_tlv_tags;

typedef enum Flash_Perm_tlv_tags {
  TAG_PERM_STRUCT = 0x7B7B7B7B,
  TAG_PERM_FLASH_EXT_DATA_IOKEY = 0x01,
  TAG_PERM_FLASH_DEVICE_AUTH_PUBK = 0x02,
  TAG_PERM_FLASH_DEVICE_KEY_ID = 0x03,
  TAG_PERM_FLASH_DEVICE_KEY_PATH = 0x04,
  TAG_PERM_FLASH_DEVICE_NFC_PRIV = 0x05,
  TAG_PERM_FLASH_CARD_ROOT_XPUB = 0x06,
} Flash_Perm_tlv_tags;

Sec_Flash_Struct sec_flash_instance;
Flash_Perm_Struct flash_perm_instance;

bool is_flash_perm_instance_loaded = false;
bool is_sec_flash_ram_instance_loaded = false;

static void fill_flash_tlv(uint8_t *array,
                           uint16_t *starting_index,
                           uint8_t tag,
                           uint16_t length,
                           const uint8_t *data);
static uint16_t serialize_sec_fs(const Sec_Flash_Struct *sec_fs, uint8_t *tlv);
static void deserialize_sec_fs(Sec_Flash_Struct *sec_fs, const uint8_t *tlv);
static void deserialize_perm_fs_key_data(Flash_Perm_Struct *perm_fs,
                                         const uint8_t *tlv,
                                         uint32_t size);

/**
 * @brief   Function used to call firewall callgate
 * @details
 * Firewall callgate is fixed at 0x08009004, call for any firewall task can only
 * be made with the callgate. Firewall is a secure region with some restrictions
 * for calls and data read/write. All read/write for firewall pages needs to be
 * performed via callgate tasks.
 *
 * @param [in]      task task which needs to be performed (check task list in
 * sec_flash_priv.h)
 * @param [in,out]  data data buffer for read or write
 * @param [in]      size size of data to read or write
 * @param [in]      Address address for read/write or 32 bit data
 *
 * @retval uint32_t SEC_TRUE if the task is success
 * @retval uint32_t SEC_FALSE or error code if task failed
 *
 * @see     Check AN4730. TODO:Add firewall documentation
 * @since v1.0.0
 */
static uint32_t firewall_func(const uint32_t task,
                              const uint8_t *data,
                              const uint32_t size,
                              const uint32_t Address) {
#if USE_SIMULATOR == 0
  uint32_t retVal;

  uint32_t (*sec_func)(uint32_t, uint8_t *, uint32_t, uint32_t) = (uint32_t(*)(
      uint32_t, uint8_t *, uint32_t, uint32_t))(FIREWALL_CODE_SEGMENT_ADDR + 1);

  __disable_irq();
  retVal = sec_func(task, (uint8_t *)data, size, Address);
  __enable_irq();
  return retVal;
#else
  return 0;
#endif
}

void FW_enter_DFU() {
  firewall_func(SEC_TASK_UPDATE_BOOT_STATE, NULL, 0, UPGRADE_INITIATED);
#if USE_SIMULATOR == 0
  WRITE_REG(RTC->BKP0R, 0x02);
#endif
}

uint32_t FW_get_hardware_version() {
  return firewall_func(SEC_TASK_GET_HARDWARE_VERSION, NULL, 0, 0);
}

uint32_t FW_get_bootloader_version() {
  return firewall_func(SEC_TASK_GET_BOOTLOADER_VERSION, NULL, 0, 0);
}

/**
 * @brief   Function used to write permanent data to firewall Page 1 (FW P1)
 * @details
 * FW P1 is firewall page which is protected by firewall from page erase and
 * overwrite. Using this page emulates OTP of size 2KB. Data once written on
 * this page cannot be overwritten or deleted due to restrictions on hardware
 * and firewall. Direct write to this page is restricted as the page is part of
 * firewall NV data region.
 *
 * @param [in] writeAddr    Address on FW P1 for write
 * @param [in] data         data to be written
 * @param [in] size         size of data to be written
 *
 * @retval uin32_t SEC_TRUE for success
 * @retval uin32_t SEC_FALSE for failure
 *
 * @since v1.0.0
 */
static uint32_t FW_write_oneTime(const uint32_t writeAddr,
                                 const uint8_t *data,
                                 const uint32_t size) {
  return firewall_func(SEC_TASK_WRITE_NV_DATA, data, size, writeAddr);
}

/**
 * @brief   Used to read data from firewall permanent storage FW P1
 * @details
 * FW P1 is firewall page which is protected by firewall from page erase and
 * overwrite. Using this page emulates OTP of size 2KB. Added advantage is that
 * the page exists in firewall region, which restricts data to be read directly
 * with address. To read the data on a particular address in firewall region,
 * callgate needs to be accessed by application only.
 *
 * @param [in]      readAddr    address to read data from
 * @param [in,out]  data        buffer address to read permanent data into
 * @param [in]      size        size of data to read
 *
 * @retval uin32_t SEC_TRUE for success
 * @retval uin32_t SEC_FALSE for failure
 *
 * @since v1.0.0
 */
static uint32_t FW_read_NVData(const uint32_t readAddr,
                               uint8_t *data,
                               const uint32_t size) {
  return firewall_func(SEC_TASK_READ_NV_DATA, data, size, readAddr);
}

/**
 * @brief   Used to receive bootloader data on boot marker page
 * @details
 * Boot marker page stores serialized instance of `upgradeInfo_t` and bootcount
 * for every successful boot. This function only fetches the `upgardeInfo_t`
 * instance
 *
 * @param [in,out] bootData pointer to `upgradeInfo_t` instance to read boot
 * data into
 *
 * @retval SEC_TRUE for successful read
 * @retval SEC_FALSE for fault
 *
 * @since v1.0.0
 */
static uint32_t FW_GetBootData(upgradeInfo_t *bootData) {
  return firewall_func(
      SEC_TASK_READ_BOOT_FLAGS, (uint8_t *)bootData, UPGRADE_INFO_WORD_SIZE, 0);
}

/**
 * @brief   Used to receive boot count for a device
 * @details Boot count is updated by bootloader after firmware hash is verified
 *
 * @return  uint32_t latest boot count
 *
 * @since v1.0.0
 */
static uint32_t FW_GetBootCount() {
  return firewall_func(SEC_TASK_GET_BOOT_COUNT, NULL, 0, 0);
}

uint32_t FW_update_auth_state(const uint32_t auth_state) {
  return firewall_func(SEC_TASK_UPDATE_AUTH_STATE, NULL, 0, auth_state);
}

/**
 * @brief   Function used to write permanent data to firewall Page 2 (FW P2)
 * @details
 * FW P2 is firewall page in firewall region which allows read/write. This page
 * is used to store store sensitive data which should not be directly
 * accessible. Data once written is protected from overwrite but the page can be
 * erase.
 *
 * @param [in] dest_addr    Address on FW P2 for write
 * @param [in] data         data to be written
 * @param [in] size         size of data to be written
 *
 * @retval uin32_t SEC_TRUE for success
 * @retval uin32_t SEC_FALSE for failure
 *
 * @since v1.0.0
 */
static uint32_t FW_write_flash_data(const uint32_t dest_addr,
                                    const uint8_t *data,
                                    const uint32_t size) {
  return firewall_func(SEC_TASK_WRITE_APPLICATION_DATA, data, size, dest_addr);
}

/**
 * @brief   Used to read data from firewall permanent storage FW P1
 * @details
 * FW P1 is firewall page which is protected by firewall from page erase and
 * overwrite. Using this page emulates OTP of size 2KB. Added advantage is that
 * the page exists in firewall region, which restricts data to be read directly
 * with address. To read the data on a particular address in firewall region,
 * callgate needs to be accessed by application only.
 *
 * @param [in]      source_addr     address to read data from
 * @param [in,out]  data            buffer address to read permanent data into
 * @param [in]      size            size of data to read
 *
 * @retval uin32_t SEC_TRUE for success
 * @retval uin32_t SEC_FALSE for failure
 *
 * @since v1.0.0
 */
static uint32_t FW_read_flash_data(const uint32_t source_addr,
                                   uint8_t *data,
                                   const uint32_t size) {
  return firewall_func(SEC_TASK_READ_APPLICATION_DATA, data, size, source_addr);
}

/**
 * @brief   Used to erase firewall page 2 (FW P2)
 * @details
 *
 * @param [in] dest_addr page address to delete
 *
 * @retval uin32_t SEC_TRUE for success
 * @retval uin32_t SEC_FALSE for failure
 *
 * @since v1.0.0
 */
static uint32_t FW_delete_flash_data(const uint32_t dest_addr) {
  return firewall_func(SEC_TASK_DELETE_APPLICATION_DATA, NULL, 0, dest_addr);
}

/**
 * @brief   Used to load tlv data from FW P2 data into ram instance
 * `sec_flash_instance`
 * @details This function must be called before read/write to the
 * `sec_flash_instance`
 *
 * @since v1.0.0
 */
static void sec_flash_struct_load() {
  uint8_t sec_fs_tlv_header[6] = {0};
  FW_read_flash_data(
      FIREWALL_APPLICATION_DATA_START_ADDR, sec_fs_tlv_header, 6);

  uint32_t serialized_flash_struct_tag =
      sec_fs_tlv_header[0] + (sec_fs_tlv_header[1] << 8) +
      (sec_fs_tlv_header[2] << 16) + (sec_fs_tlv_header[3] << 24);
  uint16_t serialized_flash_size =
      sec_fs_tlv_header[4] + (sec_fs_tlv_header[5] << 8);

  if (serialized_flash_struct_tag == TAG_SEC_FLASH_STRUCT &&
      serialized_flash_size <= FLASH_DATA_SIZE_LIMIT) {
    // 6 is added to include the TAG_FLASH_STRUCT and length of the serialized
    // structure
    uint8_t *serialized_flash_instance =
        (uint8_t *)malloc(serialized_flash_size + 6);
    ASSERT(serialized_flash_instance != NULL);
    FW_read_flash_data(FIREWALL_APPLICATION_DATA_START_ADDR,
                       serialized_flash_instance,
                       serialized_flash_size + 6);
    deserialize_sec_fs(&sec_flash_instance, serialized_flash_instance);
    memzero(serialized_flash_instance, sizeof(sec_flash_instance));
    free(serialized_flash_instance);
    serialized_flash_instance = NULL;
  } else {
    FW_delete_flash_data(FIREWALL_APPLICATION_DATA_START_ADDR);
    memzero(&sec_flash_instance, sizeof(sec_flash_instance));
  }
}

/**
 * @brief   Used for updating data to FW P2
 *
 * @param [in]  src     address for write data buffer
 * @param [in]  size    size of data to be written
 *
 * @since v1.0.0
 */
static void sec_flash_write(uint8_t *src, uint16_t size) {
  FW_delete_flash_data(FIREWALL_APPLICATION_DATA_START_ADDR);
  FW_write_flash_data(FIREWALL_APPLICATION_DATA_START_ADDR, src, size);
}

void sec_flash_struct_save() {
  uint8_t *serialized_flash_instance =
      (uint8_t *)malloc(SEC_FLASH_STRUCT_TLV_SIZE);
  ASSERT(serialized_flash_instance != NULL);
  serialize_sec_fs(&sec_flash_instance, serialized_flash_instance);
  sec_flash_write(serialized_flash_instance, SEC_FLASH_STRUCT_TLV_SIZE);
  memzero(serialized_flash_instance, SEC_FLASH_STRUCT_TLV_SIZE);
  free(serialized_flash_instance);
  serialized_flash_instance = NULL;
}

const Sec_Flash_Struct *get_sec_flash_ram_instance() {
  if (!is_sec_flash_ram_instance_loaded) {
    sec_flash_struct_load();
    is_sec_flash_ram_instance_loaded = true;
  }
  return &sec_flash_instance;
}

/**
 * @brief   should only be called for factory reset and syncing X1 Cards
 *
 * @since   v1.0.0
 */
void sec_flash_erase() {
  FW_delete_flash_data(FIREWALL_APPLICATION_DATA_START_ADDR);
  is_sec_flash_ram_instance_loaded = false;
}

/**
 * @brief   Load firewall read only data to `flash_perm_instance`
 *
 * @since v1.0.0
 */
static void flash_perm_struct_load() {
  ASSERT((&flash_perm_instance) != NULL);
  uint8_t fw_raw_data[FLASH_PAGE_SIZE];
  FW_read_NVData(FIREWALL_NVDATA_APP_KEYS_ADDR, fw_raw_data, FLASH_PAGE_SIZE);
  deserialize_perm_fs_key_data(
      &flash_perm_instance, fw_raw_data, FLASH_PAGE_SIZE);
  FW_GetBootData(&(flash_perm_instance.bootData));
  flash_perm_instance.bootCount = FW_GetBootCount();
}

static void flash_perm_key_save(uint8_t tag,
                                uint16_t size,
                                const uint8_t *data) {
  uint16_t length = 0, fs_offset = 0;
  uint8_t tlv[8 + 3 + size + 8];
  uint8_t fw_raw_data[FLASH_PAGE_SIZE] = {0};

  memset(tlv, DEFAULT_VALUE_IN_FLASH, sizeof(tlv));
  FW_read_NVData(FIREWALL_NVDATA_APP_KEYS_ADDR, fw_raw_data, FLASH_PAGE_SIZE);
  for (int index = 0; index < FLASH_PAGE_SIZE; index += 8) {
    if ((*((uint32_t *)(fw_raw_data + index)) != DEFAULT_UINT32_IN_FLASH) ||
        (*((uint32_t *)(fw_raw_data + index + 4)) != DEFAULT_UINT32_IN_FLASH))
      fs_offset = index + 8;
  }
  ASSERT(fs_offset < FLASH_PAGE_SIZE);

  if (fs_offset == 0) {
    memset(tlv, TAG_PERM_STRUCT & 0xFF, 2 * sizeof(uint32_t));
    length = 8;
  }
  fill_flash_tlv(tlv, &length, tag, size, data);
  if (length % 8 != 0)
    length += (8 - length % 8);
  FW_write_oneTime(FIREWALL_NVDATA_APP_KEYS_ADDR + fs_offset, tlv, length);
}

void flash_perm_struct_save_IOProtectKey() {
  ASSERT((&flash_perm_instance) != NULL);
  flash_perm_key_save(TAG_PERM_FLASH_EXT_DATA_IOKEY,
                      IO_KEY_SIZE,
                      flash_perm_instance.permKeyData.io_protection_key);
}

void flash_perm_struct_save_ext_keys() {
  ASSERT((&flash_perm_instance) != NULL);
  flash_perm_key_save(
      TAG_PERM_FLASH_DEVICE_AUTH_PUBK,
      ECDSA_PUB_KEY_SIZE,
      flash_perm_instance.permKeyData.ext_keys.device_auth_public_key);
  flash_perm_key_save(TAG_PERM_FLASH_DEVICE_KEY_ID,
                      FS_KEYSTORE_KEYID_LEN,
                      flash_perm_instance.permKeyData.ext_keys.self_key_id);
  flash_perm_key_save(TAG_PERM_FLASH_DEVICE_KEY_PATH,
                      FS_KEYSTORE_KEYPATH_LEN,
                      flash_perm_instance.permKeyData.ext_keys.self_key_path);
  flash_perm_key_save(TAG_PERM_FLASH_DEVICE_NFC_PRIV,
                      FS_KEYSTORE_PRIVKEY_LEN,
                      flash_perm_instance.permKeyData.ext_keys.priv_key);
  flash_perm_key_save(TAG_PERM_FLASH_CARD_ROOT_XPUB,
                      FS_KEYSTORE_XPUB_LEN,
                      flash_perm_instance.permKeyData.ext_keys.card_root_xpub);
}

/**
 * @brief   returns pointer to `flash_perm_instance` with data updated with
 * firewall
 *
 * @return  const Flash_Perm_Struct*
 *
 * @since v1.0.0
 */
const Flash_Perm_Struct *get_flash_perm_instance() {
  ASSERT((&flash_perm_instance) != NULL);

  if (!is_flash_perm_instance_loaded) {
    flash_perm_struct_load();
    is_flash_perm_instance_loaded = true;
  }
  return &flash_perm_instance;
}

/**
 * @brief Fills the TLV for the passed tag and passed flash object.
 * flash_obj is type casted to an appropriate struct type before accessing its
 * members.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param tag Sec_Flash_tlv_tags tag for TLV
 * @param length Size of the value
 * @param data Byte data array that will be filled in the TLV array
 *
 * @since v1.0.0
 */
static void fill_flash_tlv(uint8_t *array,
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

/**
 * @brief Function to fill the TLV with the properties of Wallet_Share_Data
 * struct.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param tag tag to be used for Wallet_Share_Data
 * @param sec_fs Pointer to the Flash_Struct instance ot access the list of
 * Card_Keystore structs
 *
 * @note TODO:remove tag param
 * @since v1.0.0
 */
static void serialize_sec_fs_wallet(uint8_t *array,
                                    uint16_t *starting_index,
                                    const Sec_Flash_tlv_tags tag,
                                    const Sec_Flash_Struct *sec_fs) {
  array[(*starting_index)++] = tag;

  uint16_t wallet_list_len_index = (*starting_index);
  (*starting_index) += 2;

  for (uint8_t wallet_index = 0; wallet_index < MAX_WALLETS_ALLOWED;
       wallet_index++) {
    array[(*starting_index)++] = TAG_SEC_FLASH_WALLET_SHARE_STRUCT;

    uint16_t len_index = (*starting_index);
    (*starting_index) += 2;

    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_WALLET_ID,
                   WALLET_ID_SIZE,
                   sec_fs->wallet_share_data[wallet_index].wallet_id);
    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_WALLET_SHARE,
                   BLOCK_SIZE,
                   sec_fs->wallet_share_data[wallet_index].wallet_share);
    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_WALLET_NONCE,
                   PADDED_NONCE_SIZE,
                   sec_fs->wallet_share_data[wallet_index].wallet_nonce);

    array[len_index] = (*starting_index) - len_index - 2;
    array[len_index + 1] = ((*starting_index) - len_index - 2) >> 8;
  }

  array[wallet_list_len_index] =
      ((*starting_index) - wallet_list_len_index - 2);
  array[wallet_list_len_index + 1] =
      ((*starting_index) - wallet_list_len_index - 2) >> 8;
}

/**
 * @brief Function to fill the TLV with the properties of Card_Keystore struct.
 *
 * @param array TLV array
 * @param starting_index Pointer to the index of the current location in tlv
 * @param sec_fs Pointer to the Flash_Struct instance ot access the list of
 * Card_Keystore structs
 *
 * @since v1.0.0
 */
static void serialise_sec_fs_keystore(uint8_t *array,
                                      uint16_t *starting_index,
                                      const Sec_Flash_Struct *sec_fs) {
  array[(*starting_index)++] = TAG_SEC_FLASH_KEYSTORE_LIST;

  const Card_Keystore *keystore = NULL;
  uint16_t keystore_list_len_index = (*starting_index);
  (*starting_index) += 2;

  for (uint8_t keystore_index = 0; keystore_index < MAX_KEYSTORE_ENTRY;
       keystore_index++) {
    array[(*starting_index)++] = TAG_SEC_FLASH_KEYSTORE;

    uint16_t len_index = (*starting_index);
    (*starting_index) += 2;
    keystore = &(sec_fs->keystore[keystore_index]);

    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_KEYSTORE_USED,
                   sizeof(keystore->used),
                   &(keystore->used));
    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_KEYSTORE_KEYID,
                   sizeof(keystore->key_id),
                   keystore->key_id);
    fill_flash_tlv(array,
                   starting_index,
                   TAG_SEC_FLASH_KEYSTORE_PAIRING_KEY,
                   sizeof(keystore->pairing_key),
                   keystore->pairing_key);

    array[len_index] = (*starting_index) - len_index - 2;
    array[len_index + 1] = ((*starting_index) - len_index - 2) >> 8;
  }

  array[keystore_list_len_index] =
      ((*starting_index) - keystore_list_len_index - 2);
  array[keystore_list_len_index + 1] =
      ((*starting_index) - keystore_list_len_index - 2) >> 8;
}

/**
 * @brief Create a tlv from the passed sec_fs object.
 *
 * @param flash_struct Flash Struct pointer
 * @param tlv TLV array
 * @return uint32_t Size of the TLV
 *
 * @since v1.0.0
 */
static uint16_t serialize_sec_fs(const Sec_Flash_Struct *sec_fs, uint8_t *tlv) {
  tlv[0] = (uint8_t)(TAG_SEC_FLASH_STRUCT);
  tlv[1] = (uint8_t)(TAG_SEC_FLASH_STRUCT >> 8);
  tlv[2] = (uint8_t)(TAG_SEC_FLASH_STRUCT >> 16);
  tlv[3] = (uint8_t)(TAG_SEC_FLASH_STRUCT >> 24);

  uint16_t index = 6;

  serialize_sec_fs_wallet(
      tlv, &index, TAG_SEC_FLASH_WALLET_SHARE_STRUCT_LIST, sec_fs);
  serialise_sec_fs_keystore(tlv, &index, sec_fs);

  tlv[4] = index - 6;
  tlv[5] = (index - 6) >> 8;

  return index;
}

/**
 * @brief Helper function to extract values of Card_Keystore from TLV.
 *
 * @param flash_keystore Pointer to Card_Keystore instance to store the values
 * @param tlv TLV byte array
 * @param len Length of the stored Card_Keystore in TLV (including all the tags
 * and intermediate lengths).
 *
 * @since v1.0.0
 */
static void deserialize_sec_fs_keystore(Card_Keystore *flash_keystore,
                                        const uint8_t *tlv,
                                        const uint16_t len) {
  uint16_t index = 0;

  while (index < len) {
    Sec_Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);

    switch (tag) {
      case TAG_SEC_FLASH_KEYSTORE_USED: {
        memcpy(&(flash_keystore->used), tlv + index + 2, size);
        break;
      }

      case TAG_SEC_FLASH_KEYSTORE_KEYID: {
        memcpy(flash_keystore->key_id, tlv + index + 2, size);
        break;
      }

      case TAG_SEC_FLASH_KEYSTORE_PAIRING_KEY: {
        memcpy(flash_keystore->pairing_key, tlv + index + 2, size);
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
 *
 * @since v1.0.0
 */
static void deserialize_sec_fs_wallet(Wallet_Share_Data *wallet_share_data,
                                      const uint8_t *tlv,
                                      uint16_t len) {
  uint16_t index = 0;

  while (index < len) {
    Sec_Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);

    switch (tag) {
      case TAG_SEC_FLASH_WALLET_SHARE: {
        memcpy(wallet_share_data->wallet_share, tlv + index + 2, size);
        break;
      }

      case TAG_SEC_FLASH_WALLET_NONCE: {
        memcpy(wallet_share_data->wallet_nonce, tlv + index + 2, size);
        break;
      }

      case TAG_SEC_FLASH_WALLET_ID: {
        memcpy(wallet_share_data->wallet_id, tlv + index + 2, size);
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
 *
 * @since v1.0.0
 */
static void deserialize_sec_fs(Sec_Flash_Struct *sec_fs, const uint8_t *tlv) {
  uint16_t index = 4;    // First 4 bytes are the TAG_FLASH_STRUCT
  uint16_t len = tlv[index] + (tlv[index + 1] << 8);

  index += 2;

  while (index < len) {
    Sec_Flash_tlv_tags tag = tlv[index++];
    uint16_t size = tlv[index] + (tlv[index + 1] << 8);
    switch (tag) {
      case TAG_SEC_FLASH_WALLET_SHARE_STRUCT_LIST: {
        uint16_t offset = index + 2;
        for (uint8_t wallet_index = 0; wallet_index < MAX_WALLETS_ALLOWED;
             wallet_index++) {
          if (tlv[offset++] == TAG_SEC_FLASH_WALLET_SHARE_STRUCT) {
            uint16_t wallet_len = tlv[offset] + (tlv[offset + 1] << 8);
            deserialize_sec_fs_wallet(
                &(sec_fs->wallet_share_data[wallet_index]),
                tlv + offset + 2,
                wallet_len);
            offset += wallet_len + 2;
          }
        }
        // TODO: ASSERT(offset == index + size + 2);
        break;
      }

      case TAG_SEC_FLASH_KEYSTORE_LIST: {
        uint16_t offset = index + 2;
        for (uint8_t keystore_index = 0; keystore_index < MAX_KEYSTORE_ENTRY;
             keystore_index++) {
          if (tlv[offset++] == TAG_SEC_FLASH_KEYSTORE) {
            uint16_t keystore_len = tlv[offset] + (tlv[offset + 1] << 8);
            deserialize_sec_fs_keystore(&(sec_fs->keystore[keystore_index]),
                                        tlv + offset + 2,
                                        keystore_len);
            offset += keystore_len + 2;
          }
        }
        // TODO: ASSERT(offset == index + size + 2);
        break;
      }

      default: {
        break;
      }
    }
    index += (size + 2);
  }
}

static void deserialize_perm_fs_key_data(Flash_Perm_Struct *perm_fs,
                                         const uint8_t *tlv,
                                         uint32_t size) {
  int64_t index = 0, tag, length;
  if (U32_READ_LE_ARRAY(tlv) == DEFAULT_UINT32_IN_FLASH &&
      U32_READ_LE_ARRAY(tlv + 4) == DEFAULT_UINT32_IN_FLASH) {
    // This is an older version of serialisation (without TLV); copy as it is
    // and de-serialise remaining data
    memcpy((uint8_t *)&perm_fs->permKeyData,
           tlv + 2 * sizeof(uint32_t),
           sizeof(Perm_Key_Data_Struct));
    index = (2 * sizeof(uint32_t) + sizeof(Perm_Key_Data_Struct));
    if (index % 8 != 0)
      index += (8 - index % 8);
  } else if (U32_READ_LE_ARRAY(tlv) == TAG_PERM_STRUCT &&
             U32_READ_LE_ARRAY(tlv + 4) == TAG_PERM_STRUCT) {
    index += 8;
  } else {
    LOG_CRITICAL("xxx10");
    return;
  }
  while (index < size) {
    tag = tlv[index++];
    length = U16_READ_LE_ARRAY(tlv + index);
    index += 2;
    switch (tag) {
      case TAG_PERM_FLASH_EXT_DATA_IOKEY:
        s_memcpy(perm_fs->permKeyData.io_protection_key,
                 tlv,
                 size,
                 CY_MIN(length, IO_KEY_SIZE),
                 &index);
        break;
      case TAG_PERM_FLASH_DEVICE_AUTH_PUBK:
        s_memcpy(perm_fs->permKeyData.ext_keys.device_auth_public_key,
                 tlv,
                 size,
                 CY_MIN(length, ECDSA_PUB_KEY_SIZE),
                 &index);
        break;
      case TAG_PERM_FLASH_DEVICE_KEY_ID:
        s_memcpy(perm_fs->permKeyData.ext_keys.self_key_id,
                 tlv,
                 size,
                 CY_MIN(length, FS_KEYSTORE_KEYID_LEN),
                 &index);
        break;
      case TAG_PERM_FLASH_DEVICE_KEY_PATH:
        s_memcpy(perm_fs->permKeyData.ext_keys.self_key_path,
                 tlv,
                 size,
                 CY_MIN(length, FS_KEYSTORE_KEYPATH_LEN),
                 &index);
        break;
      case TAG_PERM_FLASH_DEVICE_NFC_PRIV:
        s_memcpy(perm_fs->permKeyData.ext_keys.priv_key,
                 tlv,
                 size,
                 CY_MIN(length, FS_KEYSTORE_PRIVKEY_LEN),
                 &index);
        break;
      case TAG_PERM_FLASH_CARD_ROOT_XPUB:
        s_memcpy(perm_fs->permKeyData.ext_keys.card_root_xpub,
                 tlv,
                 size,
                 CY_MIN(length, FS_KEYSTORE_XPUB_LEN),
                 &index);
        break;
      default:
        return;
    }
    if (index % 8 != 0)
      index += (8 - index % 8);
  }
}