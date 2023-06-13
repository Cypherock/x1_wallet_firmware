/**
 * @file    flash_config.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef FLASH_CONFIG_H_
#define FLASH_CONFIG_H_

#define PERM_DATA_MAX_PAGES                                                    \
  (1)    ///< Maximum number of pages for the permanent data area under firewall
#define SHA256_SIZE 32             ///< Size of SHA256 hash
#define POW_RAND_NUMBER_SIZE 32    ///< Size of random number used for PoW
#define POW_NONCE_SIZE 32          ///< Size of nonce used for PoW
#define XPUB_SIZE                                                              \
  113    ///< Size of extended public key. Refer BIP32 doc
         ///< https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
#define FAMILY_ID_SIZE 4    ///< Size of family id of Card
#define MAX_TRANSACTION_SIZE                                                   \
  200    ///< Maximum size of transaction for bunker signature (offline signing)
#define IO_KEY_SIZE                                                            \
  32    ///< Size of IO protection key (used for encrypted comm b/w MCU and
        ///< ATECC608A)
#define ECDSA_PUB_KEY_SIZE 33        ///< Size of ECDSA (compressed) public key
#define MAX_KEYSTORE_ENTRY 4         ///< Maximum number of keystore entries
#define FS_KEYSTORE_KEYID_LEN 4      ///< Length of key id in keystore entry
#define FS_KEYSTORE_KEYPATH_LEN 8    ///< Length of key path in keystore entry
#define FS_KEYSTORE_PRIVKEY_LEN                                                \
  32                               ///< Length of private key in keystore entry
#define FS_KEYSTORE_XPUB_LEN 78    ///< Length of xpub in keystore entry

/// enum to return the status of read and write in flash related operations
typedef enum ret_code {
  SUCCESS_,             ///< Success
  MEMORY_OVERFLOW,      ///< Memory overflow
  INVALID_ARGUMENT,     ///< Invalid argument
  ALREADY_EXISTS,       ///< Already exists (e.g. wallet already exists)
  DOESNT_EXIST,         ///< Doesn't exist (e.g. wallet doesn't exist)
  ILLEGAL,              ///< Illegal operation
  INCONSISTENT_STATE    ///< Not an error by client but some inconsistency in
                        ///< flash - most likely due to some bug by me

} ret_code;

#endif    // FLASH_CONFIG_H_