/**
 * @file    flash_api.c
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
#include "flash_api.h"

#include "flash_struct_priv.h"
#include "pow_utilities.h"
#include "sec_flash_priv.h"

extern Flash_Struct flash_ram_instance;
extern Sec_Flash_Struct sec_flash_instance;
extern Flash_Perm_Struct flash_perm_instance;

extern bool is_flash_ram_instance_loaded;
extern bool is_flash_perm_instance_loaded;
extern bool is_sec_flash_ram_instance_loaded;

/**
 * @brief Return true if wallet[index] is filled
 *
 * A wallet slot is filled if it contains some data.
 * It doesn't matter if the wallet is valid, unverified or
 * in partial state. This function will return true.
 *
 * @param index Index of wallet to be checked
 * @returns bool return true if wallet is filled else return false
 */
static bool _wallet_is_filled(uint8_t index) {
  ASSERT(index < MAX_WALLETS_ALLOWED);

  if (flash_ram_instance.wallets[index].state == UNVERIFIED_VALID_WALLET ||
      flash_ram_instance.wallets[index].state == VALID_WALLET ||
      flash_ram_instance.wallets[index].state == INVALID_WALLET ||
      flash_ram_instance.wallets[index].state ==
          VALID_WALLET_WITHOUT_DEVICE_SHARE) {
    return true;
  }

  return false;
}

bool wallet_is_filled(uint8_t index, wallet_state *state_output) {
  if (MAX_WALLETS_ALLOWED <= index) {
    return false;
  }

  /* Make sure that we always work on the latest RAM instance */
  get_flash_ram_instance();

  wallet_state state = flash_ram_instance.wallets[index].state;

  if (NULL != state_output) {
    *state_output = state;
  }

  if ((UNVERIFIED_VALID_WALLET == state) || (VALID_WALLET == state) ||
      (INVALID_WALLET == state) ||
      (VALID_WALLET_WITHOUT_DEVICE_SHARE == state)) {
    return true;
  }

  return false;
}

bool wallet_is_filled_with_share(uint8_t index) {
  if (MAX_WALLETS_ALLOWED <= index) {
    return false;
  }

  /* Make sure that we always work on the latest RAM instance */
  get_flash_ram_instance();

  wallet_state state = flash_ram_instance.wallets[index].state;
  // Read card states without write attempt state
  uint8_t cards_states = flash_ram_instance.wallets[index].cards_states & 0x0F;

  // If wallet state state is where share is present on device and card state is
  // not zero, then wallet is filled
  if (((UNVERIFIED_VALID_WALLET == state) || (VALID_WALLET == state) ||
       (INVALID_WALLET == state)) &&
      (0x00 != cards_states)) {
    return true;
  }

  return false;
}

/**
 * @brief Save a new wallet on the flash
 *
 * @param fwallet
 * @param index_OUT
 * @return int
 */
int add_wallet_to_flash(const Flash_Wallet *fwallet, uint32_t *index_OUT) {
  ASSERT(fwallet != NULL);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  if (flash_ram_instance.wallet_count == MAX_WALLETS_ALLOWED)
    return MEMORY_OVERFLOW;
  size_t name_len = strnlen((const char *)fwallet->wallet_name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  if (fwallet->state != VALID_WALLET &&
      fwallet->state != UNVERIFIED_VALID_WALLET &&
      fwallet->state != VALID_WALLET_WITHOUT_DEVICE_SHARE)
    return INVALID_ARGUMENT;
  int first_empty_index = -1;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex)) {
      if (first_empty_index == -1)
        first_empty_index = (int)walletIndex;
    } else if (!strcmp((const char *)flash_ram_instance.wallets[walletIndex]
                           .wallet_name,
                       (const char *)fwallet->wallet_name)) {
      return ALREADY_EXISTS;
    }
  }
  *index_OUT = first_empty_index;
  memcpy(
      &flash_ram_instance.wallets[*index_OUT], fwallet, sizeof(Flash_Wallet));
  flash_ram_instance.wallet_count++;
  flash_struct_save();
  return SUCCESS_;
}

int add_wallet_share_to_sec_flash(const Flash_Wallet *fwallet,
                                  uint32_t *index_OUT,
                                  const uint8_t *wallet_share,
                                  const uint8_t *wallet_nonce) {
  get_flash_ram_instance();    // to load
  get_sec_flash_ram_instance();
  if (flash_ram_instance.wallet_count == MAX_WALLETS_ALLOWED)
    return MEMORY_OVERFLOW;
  size_t name_len = strnlen((const char *)fwallet->wallet_name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  if (fwallet->state == VALID_WALLET ||
      fwallet->state == UNVERIFIED_VALID_WALLET)
    return INVALID_ARGUMENT;
  int first_empty_index = -1;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex)) {
      if (first_empty_index == -1)
        first_empty_index = (int)walletIndex;
    } else if (!strcmp((const char *)flash_ram_instance.wallets[walletIndex]
                           .wallet_name,
                       (const char *)fwallet->wallet_name)) {
      return ALREADY_EXISTS;
    }
  }
  *index_OUT = first_empty_index;
  memcpy(sec_flash_instance.wallet_share_data[*index_OUT].wallet_id,
         fwallet->wallet_id,
         WALLET_ID_SIZE);
  memcpy(sec_flash_instance.wallet_share_data[*index_OUT].wallet_share,
         wallet_share,
         BLOCK_SIZE);
  memcpy(sec_flash_instance.wallet_share_data[*index_OUT].wallet_nonce,
         wallet_nonce,
         PADDED_NONCE_SIZE);
  sec_flash_struct_save();
  return SUCCESS_;
}

/**
 * @brief Update the contents of new wallet and reflect the wallet addition in
 * wallet_count of flash
 *
 * @param index
 * @param wallet
 * @return int
 */
int put_wallet_flash(const uint8_t index, const Flash_Wallet *wallet) {
  ASSERT(index < MAX_WALLETS_ALLOWED);
  ASSERT(wallet != NULL);

  get_flash_ram_instance();    // to load
  size_t name_len = strnlen((const char *)wallet->wallet_name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;

  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (walletIndex == index)
      continue;
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!strcmp(
            (const char *)flash_ram_instance.wallets[walletIndex].wallet_name,
            (const char *)wallet->wallet_name))
      return ALREADY_EXISTS;
  }
  if (!_wallet_is_filled(index))    // it was not valid earlier but now is
    flash_ram_instance.wallet_count++;
  memcpy(&flash_ram_instance.wallets[index], wallet, sizeof(Flash_Wallet));
  flash_struct_save();
  return SUCCESS_;
}

int put_wallet_share_sec_flash(const uint8_t index,
                               const uint8_t *wallet_share,
                               const uint8_t *wallet_nonce) {
  get_flash_ram_instance();    // to load
  get_sec_flash_ram_instance();
  if (index >= MAX_WALLETS_ALLOWED)
    return INVALID_ARGUMENT;

  if (!is_wallet_share_not_present(index)) {
    return INVALID_ARGUMENT;
  }

  memcpy(sec_flash_instance.wallet_share_data[index].wallet_id,
         flash_ram_instance.wallets[index].wallet_id,
         WALLET_ID_SIZE);
  memcpy(sec_flash_instance.wallet_share_data[index].wallet_share,
         wallet_share,
         BLOCK_SIZE);
  memcpy(sec_flash_instance.wallet_share_data[index].wallet_nonce,
         wallet_nonce,
         PADDED_NONCE_SIZE);
  sec_flash_struct_save();
  flash_ram_instance.wallets[index].state = VALID_WALLET;
  flash_struct_save();
  return SUCCESS_;
}

/**
 * @brief Delete wallet at the given index from the flash memory
 *
 * @param wallet_index
 * @return int
 */
int delete_wallet_from_flash(const uint8_t wallet_index) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return INVALID_ARGUMENT;
  if (!_wallet_is_filled(
          wallet_index))    // Cannot delete if wallet is not filled
    return INVALID_ARGUMENT;
  flash_ram_instance.wallet_count--;
  flash_ram_instance.wallets[wallet_index].state = DEFAULT_VALUE_IN_FLASH;
  flash_ram_instance.wallets[wallet_index].cards_states = 0;
  memset(flash_ram_instance.wallets[wallet_index].wallet_name, 0, NAME_SIZE);
  flash_struct_save();
  return SUCCESS_;
}

int delete_wallet_share_from_sec_flash(const uint8_t wallet_index) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load
  get_sec_flash_ram_instance();
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return INVALID_ARGUMENT;
  if (!_wallet_is_filled(
          wallet_index))    // Cannot delete if wallet is not filled
    return INVALID_ARGUMENT;
  if (is_wallet_share_not_present(wallet_index))
    return INVALID_ARGUMENT;
  memset(&sec_flash_instance.wallet_share_data[wallet_index],
         0,
         sizeof(Wallet_Share_Data));
  sec_flash_struct_save();
  return SUCCESS_;
}

/**
 * @brief Set the family id flash object
 *
 * @param family_id
 * @return int
 */
int set_family_id_flash(const uint8_t family_id[FAMILY_ID_SIZE]) {
  ASSERT(family_id != NULL);

  get_flash_ram_instance();    // to load
  if (flash_ram_instance.wallet_count != 0 &&
      flash_ram_instance.wallet_count != DEFAULT_UINT32_IN_FLASH)
    return ILLEGAL;
  memcpy(flash_ram_instance.family_id, family_id, FAMILY_ID_SIZE);
  flash_struct_save();
  return SUCCESS_;
}

/**
 * @brief Get the index by name object
 *
 * @param name
 * @param index_OUT
 * @return int
 */
int get_index_by_name(const char *name, uint8_t *index_OUT) {
  ASSERT(name != NULL);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  size_t name_len = strnlen(name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!strcmp(
            (const char *)flash_ram_instance.wallets[walletIndex].wallet_name,
            name)) {
      *index_OUT = walletIndex;
      return SUCCESS_;
    }
  }
  return DOESNT_EXIST;
}

/**
 * @brief Get the index by name object
 *
 * @param wallet_id
 * @param index_OUT
 * @return int
 */
int get_first_matching_index_by_id(const uint8_t wallet_id[WALLET_ID_SIZE],
                                   uint8_t *index_OUT) {
  ASSERT(wallet_id != NULL);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!memcmp(flash_ram_instance.wallets[walletIndex].wallet_id,
                wallet_id,
                WALLET_ID_SIZE)) {
      *index_OUT = walletIndex;
      return SUCCESS_;
    }
  }
  return DOESNT_EXIST;
}

/**
 * @brief Get the ith valid wallet index object
 *
 * @param i
 * @param index_OUT
 * @return int
 */
int get_ith_valid_wallet_index(const uint8_t i, uint8_t *index_OUT) {
  ASSERT(i <= MAX_WALLETS_ALLOWED);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  if (i >= flash_ram_instance.wallet_count)
    return INVALID_ARGUMENT;
  int valids = 0;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (valids == i) {
      *index_OUT = walletIndex;
      return SUCCESS_;
    }
    valids++;
  }
  return INCONSISTENT_STATE;
}

int get_ith_wallet_to_export(const uint8_t i, uint8_t *index_OUT) {
  ASSERT(i <= MAX_WALLETS_ALLOWED);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  if (i >= flash_ram_instance.wallet_count)
    return INVALID_ARGUMENT;
  int valids = 0;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (is_wallet_unverified(walletIndex) || is_wallet_partial(walletIndex) ||
        is_wallet_locked(walletIndex) ||
        is_wallet_share_not_present(walletIndex))
      continue;
    if (valids == i) {
      *index_OUT = walletIndex;
      return SUCCESS_;
    }
    valids++;
  }
  return INCONSISTENT_STATE;
}

int get_ith_wallet_without_share(const uint8_t i, uint8_t *index_OUT) {
  ASSERT(i <= MAX_WALLETS_ALLOWED);
  ASSERT(index_OUT != NULL);

  get_flash_ram_instance();    // to load
  if (i >= flash_ram_instance.wallet_count)
    return INVALID_ARGUMENT;
  int valids = 0;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!is_wallet_share_not_present(walletIndex))
      continue;
    if (valids == i) {
      *index_OUT = walletIndex;
      return SUCCESS_;
    }
    valids++;
  }
  return INCONSISTENT_STATE;
}

/**
 * @brief Get the count of valid wallets present on device
 *
 * @return int
 */
int get_valid_wallet_count() {
  const Flash_Struct *flash_wallet = get_flash_ram_instance();

  uint8_t walletCount = 0;
  uint8_t walletIndex = 0;

  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (flash_wallet->wallets[walletIndex].state == VALID_WALLET &&
        flash_wallet->wallets[walletIndex].cards_states == 0x0f &&
        flash_wallet->wallets[walletIndex].is_wallet_locked == 0)
      walletCount++;
  }
  return walletCount;
}

/**
 * @brief Get the flash wallet by name object
 *
 * @param name
 * @param flash_wallet_OUT
 * @return int
 */
int get_flash_wallet_by_name(const char *name,
                             Flash_Wallet **flash_wallet_OUT) {
  ASSERT(name != NULL);
  ASSERT(flash_wallet_OUT != NULL);

  get_flash_ram_instance();    // to load
  size_t name_len = strnlen(name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!strcmp(
            (const char *)flash_ram_instance.wallets[walletIndex].wallet_name,
            name)) {
      *flash_wallet_OUT = &flash_ram_instance.wallets[walletIndex];
      return SUCCESS_;
    }
  }
  return DOESNT_EXIST;
}

/**
 *
 */
int get_flash_wallet_share_by_name(const char *name, uint8_t *wallet_share) {
  ASSERT(name != NULL);
  ASSERT(wallet_share != NULL);

  get_flash_ram_instance();    // to load
  get_sec_flash_ram_instance();
  size_t name_len = strnlen(name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!strcmp(
            (const char *)flash_ram_instance.wallets[walletIndex].wallet_name,
            name)) {
      if (is_wallet_share_not_present(walletIndex))
        return DOESNT_EXIST;
      for (int i = 0; i < WALLET_ID_SIZE; i++) {
        if (flash_ram_instance.wallets[walletIndex].wallet_id[i] !=
            sec_flash_instance.wallet_share_data[walletIndex].wallet_id[i]) {
          flash_ram_instance.wallets[walletIndex].state =
              VALID_WALLET_WITHOUT_DEVICE_SHARE;
          flash_struct_save();
          return DOESNT_EXIST;
        }
      }
      memcpy(wallet_share,
             sec_flash_instance.wallet_share_data[walletIndex].wallet_share,
             BLOCK_SIZE);
      return SUCCESS_;
    }
  }
  return DOESNT_EXIST;
}

int get_flash_wallet_nonce_by_name(const char *name, uint8_t *wallet_nonce) {
  ASSERT(name != NULL);
  ASSERT(wallet_nonce != NULL);

  get_flash_ram_instance();    // to load
  get_sec_flash_ram_instance();
  size_t name_len = strnlen(name, NAME_SIZE);
  if (name_len == 0 || name_len >= NAME_SIZE)
    return INVALID_ARGUMENT;
  uint8_t walletIndex = 0;
  for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
    if (!_wallet_is_filled(walletIndex))
      continue;
    if (!strcmp(
            (const char *)flash_ram_instance.wallets[walletIndex].wallet_name,
            name)) {
      if (is_wallet_share_not_present(walletIndex))
        return DOESNT_EXIST;
      for (int i = 0; i < WALLET_ID_SIZE; i++) {
        if (flash_ram_instance.wallets[walletIndex].wallet_id[i] !=
            sec_flash_instance.wallet_share_data[walletIndex].wallet_id[i]) {
          flash_ram_instance.wallets[walletIndex].state =
              VALID_WALLET_WITHOUT_DEVICE_SHARE;
          flash_struct_save();
          return DOESNT_EXIST;
        }
      }
      memcpy(wallet_nonce,
             sec_flash_instance.wallet_share_data[walletIndex].wallet_nonce,
             BLOCK_SIZE);
      return SUCCESS_;
    }
  }
  return DOESNT_EXIST;
}

/**
 * @brief Tells if wallet is in partial state
 *
 * @param name
 * @return true
 * @return false
 */
bool is_wallet_partial(const uint8_t wallet_index) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load
  return flash_ram_instance.wallets[wallet_index].cards_states !=
         15;    // 0b1111
}

bool is_wallet_unverified(const uint8_t wallet_index) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load
  return flash_ram_instance.wallets[wallet_index].state ==
         UNVERIFIED_VALID_WALLET;
}

bool is_wallet_share_not_present(const uint8_t wallet_index) {
  get_flash_ram_instance();    // to load
  return flash_ram_instance.wallets[wallet_index].state ==
         VALID_WALLET_WITHOUT_DEVICE_SHARE;
}

/**
 * @brief Tells if the specified wallet is locked
 *
 * @param name
 * @return true
 * @return false
 */
bool is_wallet_locked(const uint8_t wallet_index) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load
  return flash_ram_instance.wallets[wallet_index].is_wallet_locked == 1;
}

// TODO: return codes for illegal and all
int delete_from_kth_card_flash(const uint8_t index, const uint8_t card_number) {
  ASSERT(index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load

  // Reset card write state
  RESET_Ith_BIT(flash_ram_instance.wallets[index].cards_states,
                card_number - 1);
  // Reset card write attempt state
  RESET_Ith_BIT(flash_ram_instance.wallets[index].cards_states,
                card_number - 1 + 4);

  flash_struct_save();
  return SUCCESS_;
}

// TODO: check for illegal arguments and all
bool card_already_deleted_flash(const uint8_t index,
                                const uint8_t card_number) {
  ASSERT(index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();    // to load

  bool wallet_found_on_card = IS_Ith_BIT_SET(
      flash_ram_instance.wallets[index].cards_states, card_number - 1);
  bool write_attempted_on_card = IS_Ith_BIT_SET(
      flash_ram_instance.wallets[index].cards_states, card_number - 1 + 4);

  return !(wallet_found_on_card | write_attempted_on_card);
}

/**
 * @brief
 *
 * @param name
 * @param target
 * @param random_number
 * @return int
 */
int update_challenge_flash(const char *name,
                           const uint8_t target[SHA256_SIZE],
                           const uint8_t random_number[POW_RAND_NUMBER_SIZE]) {
  ASSERT(name != NULL);
  ASSERT(target != NULL);
  ASSERT(random_number != NULL);

  get_flash_ram_instance();

  Flash_Wallet *flash_wallet;
  int ret = get_flash_wallet_by_name(name, &flash_wallet);

  if (ret != SUCCESS_)
    return INVALID_ARGUMENT;

  memcpy(flash_wallet->challenge.target, target, SHA256_SIZE);
  memcpy(flash_wallet->challenge.random_number,
         random_number,
         POW_RAND_NUMBER_SIZE);
  memset(flash_wallet->challenge.nonce, 0, POW_NONCE_SIZE);
  flash_wallet->is_wallet_locked =
      true;    // Assuming that if challenge is updated then wallet must be
               // locked
  pow_get_approx_time_in_secs(target,
                              &flash_wallet->challenge.time_to_unlock_in_secs);

  flash_struct_save();

  return SUCCESS_;
}

int set_wallet_locked(const char *wallet_name, uint8_t encoded_card_number) {
  ASSERT((NULL != wallet_name));

  Flash_Wallet *flash_wallet;
  int status = get_flash_wallet_by_name(wallet_name, &flash_wallet);
  if (SUCCESS != status) {
    return status;
  }

  flash_wallet->is_wallet_locked = true;
  memzero(&(flash_wallet->challenge), sizeof(flash_wallet->challenge));
  flash_wallet->challenge.card_locked = encoded_card_number;
  memset(flash_wallet->challenge.nonce, 0xFF, POW_NONCE_SIZE);
  flash_struct_save();
  return SUCCESS;
}

/**
 * @brief
 *
 * @param name
 * @param target
 * @param random_number
 * @param card_locked
 * @return int
 */
int add_challenge_flash(const char *name,
                        const uint8_t target[SHA256_SIZE],
                        const uint8_t random_number[POW_RAND_NUMBER_SIZE]) {
  ASSERT(name != NULL);
  ASSERT(target != NULL);
  ASSERT(random_number != NULL);

  get_flash_ram_instance();

  Flash_Wallet *flash_wallet;
  int ret = get_flash_wallet_by_name(name, &flash_wallet);

  if (ret != SUCCESS_)
    return INVALID_ARGUMENT;

  if (false == flash_wallet->is_wallet_locked) {
    return ILLEGAL;
  }

  memcpy(flash_wallet->challenge.target, target, SHA256_SIZE);
  memcpy(flash_wallet->challenge.random_number,
         random_number,
         POW_RAND_NUMBER_SIZE);
  memset(flash_wallet->challenge.nonce, 0, POW_NONCE_SIZE);
  pow_get_approx_time_in_secs(target,
                              &flash_wallet->challenge.time_to_unlock_in_secs);

  flash_struct_save();

  return SUCCESS_;
}

int update_wallet_locked_flash(const char *name, const bool is_wallet_locked) {
  ASSERT(name != NULL);

  get_flash_ram_instance();

  Flash_Wallet *flash_wallet;
  int ret = get_flash_wallet_by_name(name, &flash_wallet);

  if (ret != SUCCESS_)
    return INVALID_ARGUMENT;

  if (false == flash_wallet->is_wallet_locked) {
    /**
     * @brief Can only udpate lock status if wallet is already locked, else card
     * locked can be lost. To set wallet to locked for the first time, use @ref
     * set_wallet_locked.
     */
    return INCONSISTENT_STATE;
  }

  flash_wallet->is_wallet_locked = is_wallet_locked;

  /**
   * @brief If wallet is still in locked case, initialize the challenge values
   * similar to @ref set_wallet_locked. If wallet is unlocked, zeroise complete
   * @ref Flash_Pow object.
   */
  if (is_wallet_locked) {
    // Reset previous challenge
    memzero(flash_wallet->challenge.random_number, POW_RAND_NUMBER_SIZE);
    memzero(flash_wallet->challenge.target, SHA256_SIZE);
    flash_wallet->challenge.time_to_unlock_in_secs = 0;

    // Reset nonce to 0xFF as challenge is not fetched
    memset(flash_wallet->challenge.nonce, 0xFF, POW_NONCE_SIZE);
  } else {
    // Wallet unlocked, reset challenge
    memzero(&(flash_wallet->challenge), sizeof(flash_wallet->challenge));
  }

  flash_struct_save();

  return SUCCESS_;
}

int update_time_to_unlock_flash(const char *name,
                                const uint32_t time_to_unlock_in_secs) {
  ASSERT(name != NULL);

  get_flash_ram_instance();

  Flash_Wallet *flash_wallet;
  int ret = get_flash_wallet_by_name(name, &flash_wallet);

  if (ret != SUCCESS_)
    return INVALID_ARGUMENT;

  flash_wallet->challenge.time_to_unlock_in_secs = time_to_unlock_in_secs;

  flash_struct_save();

  return SUCCESS_;
}

int save_nonce_flash(const char *name,
                     const uint8_t nonce[POW_NONCE_SIZE],
                     const uint32_t time_to_unlock_in_secs) {
  ASSERT(name != NULL);
  ASSERT(nonce != NULL);

  get_flash_ram_instance();

  Flash_Wallet *flash_wallet;
  int ret = get_flash_wallet_by_name(name, &flash_wallet);

  if (ret != SUCCESS_)
    return INVALID_ARGUMENT;

  memcpy(flash_wallet->challenge.nonce, nonce, POW_NONCE_SIZE);
  flash_wallet->challenge.time_to_unlock_in_secs = time_to_unlock_in_secs;

  flash_struct_save();

  return SUCCESS_;
}

int set_wallet_state(const uint8_t wallet_index, const wallet_state new_state) {
  ASSERT(wallet_index < MAX_WALLETS_ALLOWED);

  get_flash_ram_instance();
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return INVALID_ARGUMENT;
  if (!_wallet_is_filled(wallet_index))
    return INVALID_ARGUMENT;

  if (new_state == VALID_WALLET || new_state == UNVERIFIED_VALID_WALLET ||
      new_state == INVALID_WALLET) {
    // new_state is valid and can be set
    flash_ram_instance.wallets[wallet_index].state = new_state;
  } else {
    return INVALID_ARGUMENT;
  }

  flash_struct_save();

  return SUCCESS_;
}

int set_display_rotation(const display_rotation _rotation,
                         flash_save_mode save_mode) {
  get_flash_ram_instance();
  flash_ram_instance.displayRotation = _rotation;
  if (save_mode == FLASH_SAVE_NOW)
    flash_struct_save();
  return SUCCESS_;
}

int set_enable_passphrase(const passphrase_config enable_passphrase,
                          flash_save_mode save_mode) {
  get_flash_ram_instance();
  flash_ram_instance.enable_passphrase = enable_passphrase;
  if (save_mode == FLASH_SAVE_NOW)
    flash_struct_save();
  return SUCCESS_;
}

int set_auth_state(const device_auth_state _auth_state) {
  get_flash_perm_instance();
  FW_update_auth_state(_auth_state);

  // force read from firewall
  is_flash_perm_instance_loaded = false;
  get_flash_perm_instance();
  if (flash_perm_instance.bootData.auth_state == _auth_state)
    return 1;
  else
    return 0;
}

int get_io_protection_key(uint8_t *key) {
  ASSERT(key != NULL);

  get_flash_perm_instance();

  if (key == NULL) {
    return INVALID_ARGUMENT;
  }

  memcpy(key,
         &flash_perm_instance.permKeyData.io_protection_key,
         sizeof(flash_perm_instance.permKeyData.io_protection_key));

  return SUCCESS_;
}

int set_io_protection_key(const uint8_t *key) {
  ASSERT(key != NULL);

  get_flash_perm_instance();

  if (key == NULL) {
    return INVALID_ARGUMENT;
  }

  memcpy(&flash_perm_instance.permKeyData.io_protection_key, key, IO_KEY_SIZE);
  flash_perm_struct_save_IOProtectKey();

  /* Zero-ise buffer to ensure that comparison happens on the latest data */
  memzero(&(flash_perm_instance.permKeyData.io_protection_key), IO_KEY_SIZE);

  is_flash_perm_instance_loaded = false;
  get_flash_perm_instance();

  if (0 == memcmp(flash_perm_instance.permKeyData.io_protection_key,
                  key,
                  IO_KEY_SIZE)) {
    return SUCCESS_;
  }

  return INCONSISTENT_STATE;
}

int set_ext_key(const Perm_Ext_Keys_Struct *ext_keys) {
  ASSERT(ext_keys != NULL);

  get_flash_perm_instance();

  if (ext_keys == NULL) {
    return INVALID_ARGUMENT;
  }

  memcpy(&flash_perm_instance.permKeyData.ext_keys,
         ext_keys,
         sizeof(Perm_Ext_Keys_Struct));
  flash_perm_struct_save_ext_keys();

  /* Zero-ise buffer to ensure that comparison happens on the latest data */
  memzero(&(flash_perm_instance.permKeyData.ext_keys),
          sizeof(Perm_Ext_Keys_Struct));

  is_flash_perm_instance_loaded = false;
  get_flash_perm_instance();

  if (0 == memcmp(&flash_perm_instance.permKeyData.ext_keys,
                  ext_keys,
                  sizeof(Perm_Ext_Keys_Struct))) {
    return SUCCESS_;
  }

  return INCONSISTENT_STATE;
}

int get_paired_card_index(const uint8_t *card_key_id) {
  ASSERT(card_key_id != NULL);

  get_sec_flash_ram_instance();
  for (int index = 0; index < MAX_KEYSTORE_ENTRY; index++) {
    if (!sec_flash_instance.keystore[index].used ||
        sec_flash_instance.keystore[index].used == DEFAULT_VALUE_IN_FLASH)
      continue;
    if (memcmp(card_key_id,
               sec_flash_instance.keystore[index].key_id,
               sizeof(sec_flash_instance.keystore[index].key_id)) == 0)
      return index;
  }
  return -1;
}

void invalidate_keystore() {
  get_sec_flash_ram_instance();
  for (int index = 0; index < MAX_KEYSTORE_ENTRY; index++) {
    sec_flash_instance.keystore[index].used = 0;
  }
  sec_flash_struct_save();
}

/***************************************
 *         GETTERS AND SETTERS         *
 ***************************************/

uint32_t get_first_boot_on_update() {
  get_flash_perm_instance();
  return flash_perm_instance.bootData.first_boot_on_update;
}

device_auth_state get_auth_state() {
  get_flash_perm_instance();
  return flash_perm_instance.bootData.auth_state;
}

uint32_t get_fwSize() {
  get_flash_perm_instance();
  return flash_perm_instance.bootData.fwSize;
}

bool is_logging_enabled() {
  get_flash_ram_instance();
  return (flash_ram_instance.enable_log == LOGGING_ENABLED);
}

int set_logging_config(log_config state, flash_save_mode save_mode) {
  get_flash_ram_instance();
  flash_ram_instance.enable_log = state;
  if (save_mode == FLASH_SAVE_NOW)
    flash_struct_save();
  return STM_SUCCESS;
}

void save_onboarding_step(const uint8_t onboarding_step) {
  get_flash_ram_instance();
  flash_ram_instance.onboarding_step = onboarding_step;
  flash_struct_save();
}

uint8_t get_onboarding_step(void) {
  get_flash_ram_instance();
  return flash_ram_instance.onboarding_step;
}

const uint8_t *get_perm_self_key_id() {
  get_flash_perm_instance();
  return flash_perm_instance.permKeyData.ext_keys.self_key_id;
}

const uint8_t *get_auth_public_key() {
  get_flash_perm_instance();
  return flash_perm_instance.permKeyData.ext_keys.device_auth_public_key;
}

uint32_t get_boot_count() {
  get_flash_perm_instance();
  return flash_perm_instance.bootCount;
}

uint32_t get_fwVer() {
  get_flash_perm_instance();
  return flash_perm_instance.bootData.fwVer;
}

const uint8_t *get_perm_self_key_path() {
  get_flash_perm_instance();
  return flash_perm_instance.permKeyData.ext_keys.self_key_path;
}

const uint8_t *get_priv_key() {
  get_flash_perm_instance();
  return flash_perm_instance.permKeyData.ext_keys.priv_key;
}

const uint8_t *get_card_root_xpub() {
  get_flash_perm_instance();
  return flash_perm_instance.permKeyData.ext_keys.card_root_xpub;
}

const uint8_t get_sizeof_card_root_xpub() {
  return sizeof(flash_perm_instance.permKeyData.ext_keys.card_root_xpub);
}

const display_rotation get_display_rotation() {
  get_flash_ram_instance();
  return flash_ram_instance.displayRotation;
}

const passphrase_config get_enable_passphrase() {
  get_flash_ram_instance();
  return flash_ram_instance.enable_passphrase;
}

const wallet_state get_wallet_state(uint8_t wallet_index) {
  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].state;
}

const uint8_t get_wallet_card_state(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return 0;

  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].cards_states;
}

const uint8_t *get_wallet_name(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return NULL;

  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].wallet_name;
}

const uint8_t get_wallet_count() {
  get_flash_ram_instance();
  return flash_ram_instance.wallet_count;
}

const uint8_t *get_wallet_id(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return NULL;

  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].wallet_id;
}

const uint8_t get_wallet_info(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return 0;

  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].wallet_info;
}

const uint8_t get_wallet_locked_status(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return 0;

  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].is_wallet_locked;
}

const uint8_t *get_family_id() {
  get_flash_ram_instance();
  return flash_ram_instance.family_id;
}

const uint8_t get_wallet_card_locked(uint8_t wallet_index) {
  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index].challenge.card_locked;
}

const uint32_t get_wallet_time_to_unlock(uint8_t wallet_index) {
  get_flash_ram_instance();
  return flash_ram_instance.wallets[wallet_index]
      .challenge.time_to_unlock_in_secs;
}

const uint8_t get_keystore_used_status(uint8_t keystore_index) {
  if (keystore_index >= MAX_KEYSTORE_ENTRY)
    return -1;

  get_sec_flash_ram_instance();
  return sec_flash_instance.keystore[keystore_index].used;
}

const uint8_t get_keystore_used_count() {
  uint8_t paired_card_count = 0;
  for (uint8_t i = 0; i < MAX_KEYSTORE_ENTRY; i++) {
    if (get_keystore_used_status(i) == 1) {
      paired_card_count++;
    }
  }
  return paired_card_count;
}

const uint8_t *get_keystore_pairing_key(uint8_t keystore_index) {
  if (keystore_index >= MAX_KEYSTORE_ENTRY)
    return NULL;

  get_sec_flash_ram_instance();
  return sec_flash_instance.keystore[keystore_index].pairing_key;
}

uint8_t set_keystore_pairing_key(uint8_t keystore_index,
                                 const uint8_t *pairing_key,
                                 uint8_t len,
                                 flash_save_mode save_mode) {
  if (keystore_index >= MAX_KEYSTORE_ENTRY || pairing_key == NULL ||
      len != sizeof(sec_flash_instance.keystore[0].pairing_key))
    return INVALID_ARGUMENT;

  get_sec_flash_ram_instance();
  memcpy(sec_flash_instance.keystore[keystore_index].pairing_key,
         pairing_key,
         len);

  if (save_mode == FLASH_SAVE_NOW)
    sec_flash_struct_save();

  return SUCCESS_;
}

const Flash_Wallet *get_wallet_by_index(uint8_t wallet_index) {
  if (wallet_index >= MAX_WALLETS_ALLOWED)
    return NULL;

  get_flash_ram_instance();
  return &flash_ram_instance.wallets[wallet_index];
}

uint8_t set_keystore_key_id(uint8_t keystore_index,
                            const uint8_t *_key_id,
                            uint8_t len,
                            flash_save_mode save_mode) {
  if (keystore_index >= MAX_KEYSTORE_ENTRY || _key_id == NULL ||
      len != sizeof(sec_flash_instance.keystore[0].key_id))
    return INVALID_ARGUMENT;

  get_sec_flash_ram_instance();
  memcpy(sec_flash_instance.keystore[keystore_index].key_id, _key_id, len);

  if (save_mode == FLASH_SAVE_NOW)
    sec_flash_struct_save();

  return SUCCESS_;
}

uint8_t set_keystore_used_status(uint8_t keystore_index,
                                 uint8_t _used,
                                 flash_save_mode save_mode) {
  if (keystore_index >= MAX_KEYSTORE_ENTRY)
    return INVALID_ARGUMENT;

  get_sec_flash_ram_instance();
  sec_flash_instance.keystore[keystore_index].used = _used;

  if (save_mode == FLASH_SAVE_NOW)
    sec_flash_struct_save();

  return SUCCESS_;
}

void flash_delete_all_wallets() {
  get_flash_ram_instance();
  memzero(&flash_ram_instance.wallets,
          MAX_WALLETS_ALLOWED * sizeof(Flash_Wallet));
  flash_ram_instance.wallet_count = 0;
  flash_struct_save();
}
