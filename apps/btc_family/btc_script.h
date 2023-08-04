/**
 * @file    btc_script.h
 * @author  Cypherock X1 Team
 * @brief   Bitcoin scriptPubKey utility apis.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_SCRIPT_H
#define BTC_SCRIPT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/** Types of scriptPubKey known. Refere bitcoin-core:
 * https://github.com/bitcoin/bitcoin/blob/d23fda05842ba4539b225bbab01b94df0060f697/src/script/standard.h#L51
 */
typedef enum {
  SCRIPT_TYPE_NONSTANDARD = 0x00,
  SCRIPT_TYPE_P2PK = 0x01,
  SCRIPT_TYPE_P2PKH = 0x02,
  SCRIPT_TYPE_P2MS = 0x03,
  SCRIPT_TYPE_P2SH = 0x04,
  SCRIPT_TYPE_NULL_DATA = 0x05,
  SCRIPT_TYPE_P2WPKH = 0x06,
  SCRIPT_TYPE_P2WSH = 0x07,
  SCRIPT_TYPE_P2TR = 0x08,
  SCRIPT_TYPE_UNKNOWN_SEGWIT = 0xFF    // a valid but undefined segwit script
} btc_script_type_e;

/** Script opcodes. Refer opcode list: https://en.bitcoin.it/wiki/Script#Opcodes
 */
enum opcodetype {
  // push value
  OP_0 = 0x00,
  OP_FALSE = OP_0,
  OP_PUSHDATA1 = 0x4c,
  OP_PUSHDATA2 = 0x4d,
  OP_PUSHDATA4 = 0x4e,
  OP_1NEGATE = 0x4f,
  OP_RESERVED = 0x50,
  OP_1 = 0x51,
  OP_TRUE = OP_1,
  OP_2 = 0x52,
  OP_3 = 0x53,
  OP_4 = 0x54,
  OP_5 = 0x55,
  OP_6 = 0x56,
  OP_7 = 0x57,
  OP_8 = 0x58,
  OP_9 = 0x59,
  OP_10 = 0x5a,
  OP_11 = 0x5b,
  OP_12 = 0x5c,
  OP_13 = 0x5d,
  OP_14 = 0x5e,
  OP_15 = 0x5f,
  OP_16 = 0x60,

  // control
  OP_NOP = 0x61,
  OP_VER = 0x62,
  OP_IF = 0x63,
  OP_NOTIF = 0x64,
  OP_VERIF = 0x65,
  OP_VERNOTIF = 0x66,
  OP_ELSE = 0x67,
  OP_ENDIF = 0x68,
  OP_VERIFY = 0x69,
  // Refer the flow control list: https://en.bitcoin.it/wiki/Script#Flow_control
  OP_RETURN = 0x6a,

  // stack ops
  OP_TOALTSTACK = 0x6b,
  OP_FROMALTSTACK = 0x6c,
  OP_2DROP = 0x6d,
  OP_2DUP = 0x6e,
  OP_3DUP = 0x6f,
  OP_2OVER = 0x70,
  OP_2ROT = 0x71,
  OP_2SWAP = 0x72,
  OP_IFDUP = 0x73,
  OP_DEPTH = 0x74,
  OP_DROP = 0x75,
  OP_DUP = 0x76,
  OP_NIP = 0x77,
  OP_OVER = 0x78,
  OP_PICK = 0x79,
  OP_ROLL = 0x7a,
  OP_ROT = 0x7b,
  OP_SWAP = 0x7c,
  OP_TUCK = 0x7d,

  // splice ops
  OP_CAT = 0x7e,
  OP_SUBSTR = 0x7f,
  OP_LEFT = 0x80,
  OP_RIGHT = 0x81,
  OP_SIZE = 0x82,

  // bit logic
  OP_INVERT = 0x83,
  OP_AND = 0x84,
  OP_OR = 0x85,
  OP_XOR = 0x86,
  OP_EQUAL = 0x87,
  OP_EQUALVERIFY = 0x88,
  OP_RESERVED1 = 0x89,
  OP_RESERVED2 = 0x8a,

  // numeric
  OP_1ADD = 0x8b,
  OP_1SUB = 0x8c,
  OP_2MUL = 0x8d,
  OP_2DIV = 0x8e,
  OP_NEGATE = 0x8f,
  OP_ABS = 0x90,
  OP_NOT = 0x91,
  OP_0NOTEQUAL = 0x92,

  OP_ADD = 0x93,
  OP_SUB = 0x94,
  OP_MUL = 0x95,
  OP_DIV = 0x96,
  OP_MOD = 0x97,
  OP_LSHIFT = 0x98,
  OP_RSHIFT = 0x99,

  OP_BOOLAND = 0x9a,
  OP_BOOLOR = 0x9b,
  OP_NUMEQUAL = 0x9c,
  OP_NUMEQUALVERIFY = 0x9d,
  OP_NUMNOTEQUAL = 0x9e,
  OP_LESSTHAN = 0x9f,
  OP_GREATERTHAN = 0xa0,
  OP_LESSTHANOREQUAL = 0xa1,
  OP_GREATERTHANOREQUAL = 0xa2,
  OP_MIN = 0xa3,
  OP_MAX = 0xa4,

  OP_WITHIN = 0xa5,

  // crypto
  OP_RIPEMD160 = 0xa6,
  OP_SHA1 = 0xa7,
  OP_SHA256 = 0xa8,
  OP_HASH160 = 0xa9,
  OP_HASH256 = 0xaa,
  OP_CODESEPARATOR = 0xab,
  OP_CHECKSIG = 0xac,
  OP_CHECKSIGVERIFY = 0xad,
  OP_CHECKMULTISIG = 0xae,
  OP_CHECKMULTISIGVERIFY = 0xaf,

  // expansion
  OP_NOP1 = 0xb0,
  OP_CHECKLOCKTIMEVERIFY = 0xb1,
  OP_NOP2 = OP_CHECKLOCKTIMEVERIFY,
  OP_CHECKSEQUENCEVERIFY = 0xb2,
  OP_NOP3 = OP_CHECKSEQUENCEVERIFY,
  OP_NOP4 = 0xb3,
  OP_NOP5 = 0xb4,
  OP_NOP6 = 0xb5,
  OP_NOP7 = 0xb6,
  OP_NOP8 = 0xb7,
  OP_NOP9 = 0xb8,
  OP_NOP10 = 0xb9,

  // Opcode added by BIP 342 (Tapscript)
  OP_CHECKSIGADD = 0xba,

  OP_INVALIDOPCODE = 0xff,
};

// Refer:https://en.bitcoin.it/wiki/Script#Standard_Transaction_to_Bitcoin_address_.28pay-to-pubkey-hash.29
static inline bool btc_is_p2pkh(const uint8_t *script, size_t script_len) {
  return 25 == script_len && OP_DUP == script[0] && OP_HASH160 == script[1] &&
         0x14 == script[2] && OP_EQUALVERIFY == script[23] &&
         OP_CHECKSIG == script[24];
}

// Refer:
// https://github.com/bitcoin/bips/blob/master/bip-0016.mediawiki#specification
static inline bool btc_is_p2sh(const uint8_t *script, size_t script_len) {
  return 23 == script_len && OP_HASH160 == script[0] && 0x14 == script[1] &&
         OP_EQUAL == script[22];
}

// Refer:
// https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki#address-derivation,
// https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki#p2wpkh
static inline bool btc_is_p2wpkh(const uint8_t *script, size_t script_len) {
  return 22 == script_len && OP_0 == script[0] && 0x14 == script[1];
}

// Refer:
// https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki#user-content-P2WSH
static inline bool btc_is_p2wsh(const uint8_t *script, size_t script_len) {
  return 43 == script_len && OP_0 == script[0] && 0x20 == script[1];
}

// Refer:
// https://github.com/bitcoin/bips/blob/master/bip-0086.mediawiki#address-derivation
static inline bool btc_is_p2tr(const uint8_t *script, size_t script_len) {
  return 34 == script_len && OP_1 == script[0] && 0x20 == script[1];
}

// Refer:
// https://github.com/bitcoin/bips/blob/master/bip-0011.mediawiki#specification
static inline bool btc_is_p2ms(const uint8_t *script, size_t script_len) {
  return 1 <= script_len && OP_CHECKMULTISIG == script[script_len - 1];
  // TODO: stricter checks for a valid Multi-Sig script
}

// Refer:
// https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs
static inline bool btc_is_opreturn(const uint8_t *script, size_t script_len) {
  return script_len > 0 && script_len <= 83 && OP_RETURN == script[0];
}

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Detects if the provided script is of the P2PK type.
 * Refer: https://en.bitcoin.it/wiki/Script#Obsolete_pay-to-pubkey_transaction
 *
 * @param script Reference to the scriptPubKey
 * @param script_len Length of the provided scriptPubKey
 *
 * @return bool Indicating if the script is p2pk
 * @retval true The provided script is of Pay-to-Public-Key
 * @retval false If the provided script is not Pay-to-Public-Key
 */
bool btc_is_p2pk(const uint8_t *script, size_t script_len);

/**
 * @brief Detects the kind of provided script according to btc_script_type_e
 *
 * @param script Reference to the scriptPubKey
 * @param script_len Length of the provided scriptPubKey
 *
 * @return btc_script_type_e Indicating the type of script identified
 *
 * @see is_p2pk(), is_p2pkh(), is_p2sh(), is_p2ms(), is_p2wpkh(), is_p2wsh(),
 * is_p2tr(), is_opreturn()
 */
btc_script_type_e btc_get_script_type(const uint8_t *script, size_t script_len);

/**
 * @brief Returns the null-terminated encoded address string in the scriptPubKey
 *
 * @param script
 * @param script_len
 * @param addr
 * @param out_len
 *
 * @return int Indicating length of the output address or an error status
 * @retval +ve The length of output address string
 * @retval -1 If script type does not contains any address to encode
 * @retval -2 If script type is nonstandard or non-parsable
 * @retval -3 If any encoding error was faced or the provided output buffer is
 * shorter than required
 * @retval 0 & remaining -ve values have no meaning and never returned
 */
int btc_get_script_pub_address(const uint8_t *script,
                               size_t script_len,
                               char *addr,
                               int out_len);

/**
 * @brief Validates the change address for a Bitcoin (and its forks)
 * transaction.
 * @details It checks for existence of the change address and if it is a valid
 * address. The change address is extracted from the script (only P2PKH & P2WPKH
 * supported) of the last output transaction.
 *
 * @param [in] script     - Pointer to the scriptPubKey data bytes
 * @param [in] script_len - Length of the provided script
 * @param [in] public_key - Pointer to the compressed public key
 *
 * @return bool - Indicates whether the change address is valid or not.
 * @retval true     If the change address is valid.
 * @retval false    If the change address is invalid or script type is
 * unsupported.
 *
 * @see btc_sign_unsigned_txn(), BITCOIN, unsigned_txn, txn_metadata, HDNode
 * @since v1.0.0
 *
 * @note The BTC change address is always a segwit address. So the metadata is
 * updated to segwit's purpose index ignoring what is passed in the metadata.
 */
bool btc_check_script_address(const uint8_t *script,
                              size_t script_len,
                              const uint8_t *public_key);

/**
 * @brief Prepares the script sig from provided signature and public key
 * @details The function encodes the script sig according to Bitcoin
 * specification as defined in the BIP-??. Refer:
 *
 * @param sig The non-DER encoded signature data bytes
 * @param pub_key The compressed public key data bytes
 * @param script_sig The output script sig buffer
 *
 * @return uint8_t Indicating the length of the scriptSig
 * @retval 0 If any failure faced during generation of scriptSig
 * @retval >0 Length of the scriptSig
 */
uint8_t btc_sig_to_script_sig(const uint8_t *sig,
                              const uint8_t *pub_key,
                              uint8_t *script_sig);

#endif
