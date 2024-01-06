/**
 * @file    btc_context.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_CONTEXT_H
#define BTC_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// this makes length of 7 with a termination NULL byte
#define BTC_SHORT_NAME_MAX_SIZE 8
/// this makes length of 15 with a termination NULL byte
#define BTC_LONG_NAME_MAX_SIZE 16
#define BECH32_HRP_SIZE 4

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /** The index value for coin-index level according to BIP32. Refer:
   * https://github.com/satoshilabs/slips/blob/master/slip-0044.md for using
   * correct values.
   */
  uint32_t coin_type;
  /** The address version needed for encoding the public address in base58
   * encoding defined in BIP. Refer: (<a
   * href="https://en.bitcoin.it/wiki/Base58Check_encoding">base58 encoding</a>)
   * and (<a href="https://en.bitcoin.it/wiki/List_of_address_prefixes">version
   * byte values</a>)
   */
  uint8_t p2pkh_addr_ver;
  /** The address version needed for encoding the public address in base58
   * encoding defined in BIP13. Refer:
   * https://en.bitcoin.it/wiki/List_of_address_prefixes and
   * https://github.com/bitcoin/bips/blob/master/bip-0013.mediawiki#specification
   */
  uint8_t p2sh_addr_ver;
  /** The HD version bytes used for base58-encoding of xpub of legacy account
   * types. Not all values are needed for every Bitcoin fork as support is
   * specific to some chains. Refer:
   * https://github.com/satoshilabs/slips/blob/master/slip-0132.md for values.
   * NOTE: We do not intend to export private key hence no need for xpriv HD
   * version bytes.
   */
  uint32_t legacy_xpub_ver;
  uint32_t segwit_xpub_ver;
  uint32_t nsegwit_xpub_ver;

  /** The human-readable prefix for Bech32 encoded addresses. Applicable for
   * Segwit and Taproot addresses. Ref:
   * https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki#segwit-address-format
   * & https://github.com/satoshilabs/slips/blob/master/slip-0173.md */
  char bech32_hrp[BECH32_HRP_SIZE];

  /** Largest unit/denomination indicator/symbol. This will be used in UX for
   * displaying fees and amount.
   */
  char lunit_name[BTC_SHORT_NAME_MAX_SIZE];
  /** Common name of the blockchain known to the users. This will be used in UX
   */
  char name[BTC_LONG_NAME_MAX_SIZE];

  /** Helps put an upper limit on the fee. This tweak is helpful to assist user
   * in identifying potentially fraudulent transactions and help save unwanted
   * loss of funds.
   * TODO: Link a reference
   */
  uint64_t max_fee;

  /** Validates if the provided purpose_index is supported by the Bitcoin fork.
   * The validation acts as safety check for receive address, account addition
   * and transaction type for the specific fork of Bitcoin.
   */
  bool (*is_purpose_supported)(uint32_t purpose_index);
} btc_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif
