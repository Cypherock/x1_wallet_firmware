/**
 * @file    stellar_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the STELLAR app
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STELLAR_CONTEXT_H
#define STELLAR_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Stellar network constants
#define STELLAR_NAME "Stellar"
#define STELLAR_LUNIT "XLM"

// Key and address sizes
#define STELLAR_PUB_KEY_SIZE 33
#define STELLAR_PUBKEY_RAW_SIZE 32
#define STELLAR_PRIVATE_KEY_SIZE 32
#define STELLAR_ADDRESS_LENGTH 57
#define STELLAR_SECRET_KEY_LENGTH 57
#define STELLAR_SIGNATURE_SIZE 64

// Stellar XDR constants
#define STELLAR_ENVELOPE_TYPE_TX 2
#define STELLAR_KEY_TYPE_ED25519 0
#define STELLAR_ASSET_TYPE_NATIVE 0

// Network passphrases
// See https://developers.stellar.org/docs/learn/fundamentals/networks
#define TESTNET_PASSPHRASE "Test SDF Network ; September 2015"
#define MAINNET_PASSPHRASE "Public Global Stellar Network ; September 2015"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// TODO: Populate structure for STELLAR
typedef struct {
} stellar_config_t;

// Stellar Memo types
// See
// https://developers.stellar.org/docs/learn/encyclopedia/transactions-specialized/memos
typedef enum {
  STELLAR_MEMO_NONE = 0,
  STELLAR_MEMO_TEXT = 1,
  STELLAR_MEMO_ID = 2,
  STELLAR_MEMO_HASH = 3,
  STELLAR_MEMO_RETURN = 4
} stellar_memo_type_t;

// Stellar operation types
// See
// https://developers.stellar.org/docs/learn/fundamentals/transactions/list-of-operations
typedef enum {
  STELLAR_OPERATION_CREATE_ACCOUNT = 0,
  STELLAR_OPERATION_PAYMENT = 1
} stellar_operation_type_t;

// Custom generic structure for Stellar operation data
// So far we only support create account and payment operations with native
// asset only
typedef struct {
  stellar_operation_type_t type;
  uint8_t destination[STELLAR_PUBKEY_RAW_SIZE];
  uint64_t amount;
} stellar_operation_data_t;

// Stellar preconditions types
// See
// https://developers.stellar.org/docs/learn/fundamentals/transactions/operations-and-transactions#preconditions
// See
// https://github.com/stellar/js-stellar-base/blob/master/xdr/curr/Stellar-transaction.x#L804
typedef enum {
  STELLAR_PRECOND_NONE = 0,
  STELLAR_PRECOND_TIME = 1,
  STELLAR_PRECOND_V2 = 2
} stellar_preconditions_type_t;

// Stellar time bounds structure
// See
// https://developers.stellar.org/docs/learn/fundamentals/transactions/operations-and-transactions#time-bounds
// See
// https://github.com/stellar/js-stellar-base/blob/master/xdr/curr/Stellar-transaction.x#L759
typedef struct {
  uint64_t min_time;
  uint64_t max_time;
} stellar_time_bounds_t;

// Stellar preconditions structure
// See
// https://developers.stellar.org/docs/learn/fundamentals/transactions/operations
typedef struct {
  stellar_preconditions_type_t type;
  union {
    stellar_time_bounds_t
        time_bounds;    // For PRECOND_TIME, only PRECOND_TIME is supported
    // For PRECOND_V2, we can add more fields in future if needed
  } preconditions;
} stellar_preconditions_t;

// Stellar transaction extension types
// See
// https://github.com/stellar/js-stellar-base/blob/master/xdr/curr/Stellar-transaction.x#L929
typedef enum {
  STELLAR_EXT_TYPE_EMPTY = 0,    // Empty extension, only supported type
  // Other extension types can be added in future if needed
} stellar_ext_type_t;

// Stellar transaction extension structure
// See
// https://github.com/stellar/js-stellar-base/blob/master/xdr/curr/Stellar-transaction.x#L929
typedef struct {
  stellar_ext_type_t
      type;    // Currently only STELLAR_EXT_TYPE_EMPTY is supported
  // Other fields can be added in future if needed
} stellar_transaction_extension_t;

// Stellar transaction structures
// See
// https://github.com/stellar/stellar-xdr/blob/curr/Stellar-transaction.x#L911
typedef struct {
  uint8_t source_account[STELLAR_PUBKEY_RAW_SIZE];
  uint64_t sequence_number;
  uint32_t fee;
  stellar_memo_type_t memo_type;
  union {
    char text[29];       // STELLAR_MEMO_TEXT (max 28 bytes + 1 byte delimiter)
    uint64_t id;         // STELLAR_MEMO_ID
    uint8_t hash[32];    // STELLAR_MEMO_HASH or STELLAR_MEMO_RETURN(32 bytes)
  } memo;
  uint32_t operation_count;
  stellar_operation_data_t operations[1];    // Only one operation supported
  stellar_preconditions_t preconditions;
  stellar_transaction_extension_t ext;
} stellar_transaction_t;

typedef enum {
  STELLAR_NETWORK_MAINNET = 0,
  STELLAR_NETWORK_TESTNET = 1
} stellar_network_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* STELLAR_CONTEXT_H */