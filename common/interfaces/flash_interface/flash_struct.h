/**
 * @file    flash_struct.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef FLASH_STRUCT_H
#define FLASH_STRUCT_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "flash_config.h"
#include "memzero.h"
#include "wallet.h"

/// enum display orientation
typedef enum displayRotate_options {
  LEFT_HAND_VIEW = 0xAA,
  RIGHT_HAND_VIEW = 0x55
} display_rotation;

/// enum for log config
typedef enum log_config {
  LOGGING_DISABLED,
  LOGGING_ENABLED,
} log_config;

/// enum to signify the wallet state
typedef enum wallet_state {
  INVALID_WALLET = 1,
  UNVERIFIED_VALID_WALLET = 14,
  VALID_WALLET = 7,
  VALID_WALLET_WITHOUT_DEVICE_SHARE = 15,
} wallet_state;

/// enum to signify the passphrase config state
typedef enum passphrase_config {
  PASSPHRASE_DISABLED,
  PASSPHRASE_ENABLED,
} passphrase_config;

/// Different save modes when writing to instance of different flash structs
typedef enum flash_save_mode {
  FLASH_SAVE_LATER,    ///< Signal to save later
  FLASH_SAVE_NOW,      ///< Signal to save now
} flash_save_mode;

/**
 * @brief Store flash data for proof of work.
 * @details The data needed to prove the device has done the work and unlock the
 * card is stored here.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
// TODO: Refactor flash storage, reduce erase cycles
typedef struct Flash_Pow {
  uint8_t target[SHA256_SIZE];
  uint8_t random_number[POW_RAND_NUMBER_SIZE];
  uint8_t nonce[POW_NONCE_SIZE];
  uint8_t card_locked;    // if 3rd card is locked then card_locked = 4
  uint32_t time_to_unlock_in_secs;
} Flash_Pow;
#pragma pack(pop)

/**
 * @brief Struct for wallet data to stored in flash.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct Flash_Wallet {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t wallet_name[NAME_SIZE];
  uint8_t wallet_info;
  uint8_t
      wallet_share_with_mac_and_nonce[BLOCK_SIZE + PADDED_NONCE_SIZE +
                                      WALLET_MAC_SIZE];    // does not include
                                                           // MAC and nonce

  uint8_t state;    // if DEFAULT_VALUE_IN_FLASH then not valid. If equal to
                    // VALID_WALLET then valid
  uint8_t cards_states;    // ith bit from right ((cards_states>>i)&1) bit tells
                           // whether card (i+1) has the share or not.
                           // Attempt state is also recorded on the left nibble,
                           // recorded before card write operation is attempted
                           // and cleared when successful.
  uint8_t is_wallet_locked;    // 1 if wallet if locked
  Flash_Pow challenge;
} Flash_Wallet;
#pragma pack(pop)

/**
 * @brief Struct for storing meta data about device in flash.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note Need to manually update FLASH_STRUCT_TLV_SIZE in case any change is
 * done in Flash_struct
 */
#pragma pack(push, 1)
// TODO: Refactor flash storage, reduce erase cycles
typedef struct Flash_Struct {
  uint8_t family_id[FAMILY_ID_SIZE];
  uint32_t wallet_count;    // number of valid wallets
  Flash_Wallet wallets[MAX_WALLETS_ALLOWED];
  uint8_t displayRotation;
  uint8_t enable_passphrase;
  uint8_t enable_log;
  uint8_t onboarding_step;
} Flash_Struct;
#pragma pack(pop)

/**
 * @brief Erase all data on flash.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void flash_erase();

/**
 * @brief This API clears the user data - for example: pairing data, wallets,
 * settings, but preserves some data onto the flash memory
 * For now, the only thing preserved is the onboarding_step
 *
 */
void flash_clear_user_data(void);

#endif
