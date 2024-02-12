/**
 * @file    sec_flash.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
/*
 * protected_code.h
 *
 *  Created on: 19-Jun-2021
 *      Author: amana
 */

#ifndef SECURETASKS_PROTECTED_CODE_H_
#define SECURETASKS_PROTECTED_CODE_H_

#include "flash_config.h"
#include "wallet.h"

/// Possible device auth states
/// @todo Use secure true/false value
typedef enum device_auth_state {
  DEVICE_NOT_AUTHENTICATED = 1,    ///< Signifies device is not authenticated
  DEVICE_AUTHENTICATED = 0,        ///< Signifies device is authenticated
} device_auth_state;

/**
 * @brief Updates Bootloader boot state to UPGRADE_INITIATED
 * @details
 * To initiate firmware update, boot state needs to be updated to
 * UPGRADE_INITIATED, Upon reset bootloader will detect the UPGRADE_INITIATED
 * state and wait for desktop app to start the DFU package transmission. The
 * function also sets RTC backup register 0 to value 2 to indicate boot state
 * was changed by firmware and not by fault.
 *
 * @since v1.0.0
 */
extern void FW_enter_DFU();

/**
 * @brief returns device hardware version
 * @details Device hardware version is used for hal configuration compatibility
 *
 * @since v1.0.0
 */
extern uint32_t FW_get_hardware_version();

/**
 * @brief returns bootloader version
 *
 * @since v1.0.0
 */
extern uint32_t FW_get_bootloader_version();

/**
 * @brief Struct for mapping device's share of seed for wallet
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Wallet_share {
  uint8_t wallet_id[WALLET_ID_SIZE];    ///< Wallet ID derived from seed
  uint8_t
      wallet_share[BLOCK_SIZE];    ///< Device's (5th) share derived from seed
  uint8_t wallet_nonce[PADDED_NONCE_SIZE];    ///< Wallet's nonce including IV
                                              ///< and version data
} Wallet_Share_Data;
#pragma pack(pop)

/**
 * @brief Keystore struct for storing shared secrets for NFC communication
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct card_keystore {
  uint8_t used;               ///< Flag to indicate if keystore is used
  uint8_t key_id[4];          ///< Key ID to indentify keystore entry
  uint8_t pairing_key[64];    ///< Shared secret for encrypted NFC communication
} Card_Keystore;
#pragma pack(pop)

/**
 * @brief Root struct to hold reference to modifiable data in firewall region
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Sec_Flash_Data {
  Wallet_Share_Data wallet_share_data[MAX_WALLETS_ALLOWED];
  Card_Keystore keystore[MAX_KEYSTORE_ENTRY];
} Sec_Flash_Struct;
#pragma pack(pop)

/**
 * @brief Struct of permanent keys received or derived from external sources
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Perm_Ext_Keys {
  uint8_t
      device_auth_public_key[ECDSA_PUB_KEY_SIZE];    ///< Public key component
                                                     ///< of device-auth
  uint8_t self_key_id[FS_KEYSTORE_KEYID_LEN];    ///< Key id of self key (first
                                                 ///< 4-bytes of sha256 on
                                                 ///< private key)
  uint8_t self_key_path[FS_KEYSTORE_KEYPATH_LEN];    ///< Key path of self key
  uint8_t priv_key[FS_KEYSTORE_PRIVKEY_LEN];    ///< Private key of device for
                                                ///< card pairing
  uint8_t card_root_xpub[FS_KEYSTORE_XPUB_LEN];    ///< Card root xpub used for
                                                   ///< verification of card
                                                   ///< signatures
} Perm_Ext_Keys_Struct;
#pragma pack(pop)

/**
 * @brief Struct for holding keys permanently stored in firewall
 * @details The set of keys stored in the firewall region with one-time write.
 * So the keys cannot be overwritten by the application.
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Perm_Key_Data {
  uint8_t io_protection_key[IO_KEY_SIZE]; /** IO protection key used to
                                           * establish encrypted communication
                                           * b/w MCU and ATECC608A */
  Perm_Ext_Keys_Struct
      ext_keys; /** Keys & related-info derived on external sources */
} Perm_Key_Data_Struct;
#pragma pack(pop)

/**
 * @brief Root Struct to hold the reference to the permanent data in firewall
 * @details This is a one-time write data hence it cannot be changed once
 * written.
 *
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Flash_Perm_Struct {
  upgradeInfo_t bootData;              ///< Firmware upgrade data
  Perm_Key_Data_Struct permKeyData;    ///< One-time writable keys
  uint32_t bootCount;    ///< Number of Boot cycles of the main application
} Flash_Perm_Struct;
#pragma pack(pop)

/**
 * @brief Used to erase application data from firewall
 * @details Used for factory reset operation
 *
 * @since v1.0.0
 */
void sec_flash_erase();

#endif /* SECURETASKS_PROTECTED_CODE_H_ */
