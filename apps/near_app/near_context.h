/**
 * @file    near_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the NEAR app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_CONTEXT_H
#define NEAR_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
// NEAR implicit account derivation path: m/44'/397'/0'/0'/i'

// Rationale to keep it m/44'/397'/0'/0'/i':
// NEAR implicit accounts can be any public/private keypair
// https://docs.near.org/integrator/implicit-accounts

// NEAR official CLI uses m/44'/397'/0'/0'/1' as the first account
// https://docs.near.org/tools/near-cli

// Ledger uses m/44'/397'/0'/0'/0'
// https://github.com/near/near-wallet/issues/1537

// There are some wallets which even use m/44'/9000'/0'/0/0
// https://support.atomicwallet.io/article/146-list-of-derivation-paths

#define NEAR_IMPLICIT_ACCOUNT_DEPTH 5

#define NEAR_PURPOSE_INDEX (0x8000002C)    // 44'
#define NEAR_COIN_INDEX (0x8000018D)       // 397'
#define NEAR_ACCOUNT_INDEX 0x80000000      // 0'
#define NEAR_CHANGE_INDEX 0x80000000       // 0'

// Ref: https://nomicon.io/Economics/Economic
#define NEAR_DECIMAL (24U)

/// Ref:
/// https://docs.near.org/concepts/basics/transactions/gas#the-cost-of-common-actions
#define NEAR_FEES_DECIMAL (20U)

/// this makes length of 5 with a termination NULL byte
#define NEAR_SHORT_NAME_MAX_SIZE 6
/// this makes length of 5 with a termination NULL byte
#define NEAR_LONG_NAME_MAX_SIZE 6

#define NEAR_COIN_VERSION 0x00000000

#define NEAR_REGISTERED_ACCOUNT_COUNT 4
#define NEAR_TOP_LEVEL_ACCOUNT_MIN_LEN 32
#define NEAR_ACC_ID_MIN_LEN 2
#define NEAR_ACC_ID_MAX_LEN 64
#define NEAR_COIN_DATA_MAX_LEN 300

#define NEAR_NONCE_SIZE_BYTES 8
#define NEAR_ALLOWANCE_SIZE_BYTES 8
#define NEAR_DEPOSIT_SIZE_BYTES 16

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /** Largest unit/denomination indicator/symbol. This will be used in UX for
   * displaying fees and amount.
   */
  const char lunit_name[NEAR_SHORT_NAME_MAX_SIZE];
  /** Common name of the blockchain known to the users. This will be used in UX
   */
  const char name[NEAR_LONG_NAME_MAX_SIZE];
} near_config_t;

// Datatypes and structures for the transaction
// ref:
// https://github.com/near/near-api-js/blob/d4d4cf1ac3182fa998b1e004e6782219325a641b/src/transaction.ts#L139
// ref: https://nomicon.io/DataStructures/DataTypes

typedef enum near_key_type {
  NEAR_CURVE_ED25519 = 0,
  NEAR_CURVE_SECP256K1 = 1,
} near_key_type;

/**
 * @brief Tags for the coin specific data tlv format.
 */
typedef enum near_data_tag {
  TAG_NEAR_DEFAULT_NETWORK = 0x01,
  TAG_NEAR_REGISTERED_ACC = 0x02,
} near_data_tag;

typedef enum near_action {
  NEAR_ACTION_CREATE_ACCOUNT = 0,
  NEAR_ACTION_DEPLOY_CONTRACT,
  NEAR_ACTION_FUNCTION_CALL,
  NEAR_ACTION_TRANSFER,
  NEAR_ACTION_STAKE,
  NEAR_ACTION_ADD_KEY,
  NEAR_ACTION_DELETE_KEY,
  NEAR_ACTION_DELETE_ACCOUNT,
} near_action;

typedef struct near_key {
  uint8_t key_type;
  const uint8_t *key;
} near_key;

/// ref:
/// https://nomicon.io/DataStructures/AccessKey#accesskeypermissionfunctioncall
typedef struct near_fn_access {
  uint8_t allowance[NEAR_ALLOWANCE_SIZE_BYTES];
  char *account_id;
  char **methods;
} near_fn_access;

/// ref: https://nomicon.io/DataStructures/AccessKey
typedef struct near_access_key {
  uint8_t nonce[NEAR_NONCE_SIZE_BYTES];
  uint8_t full_access;
  near_fn_access access;
} near_access_key;

/// ref: https://nomicon.io/RuntimeSpec/Actions#createaccountaction
typedef struct near_create_acc {
  uint8_t action_id;
  char
      *user_id;    // @see NEAR_TOP_LEVEL_ACCOUNT_MIN_LEN and
                   // https://nomicon.io/RuntimeSpec/Actions#createaccountaction
  char public_key[65];
} near_create_acc;

// ref:
// https://docs.near.org/docs/tutorials/create-transactions#low-level----create-a-transaction
/// ref: https://nomicon.io/RuntimeSpec/Actions#transferaction
typedef struct near_transfer {
  uint8_t amount[NEAR_DEPOSIT_SIZE_BYTES];
} near_transfer;

/// ref: https://nomicon.io/RuntimeSpec/Actions#stakeaction
typedef struct near_stake {
  // TODO
} near_stake;

/// ref: https://nomicon.io/RuntimeSpec/Actions#addkeyaction
typedef struct near_add_key {
  // TODO
} near_add_key;

/// ref: https://nomicon.io/RuntimeSpec/Actions#deletekeyaction
typedef struct near_del_key {
  // TODO
} near_del_key;

/// ref: https://nomicon.io/RuntimeSpec/Actions#deleteaccountaction
typedef struct near_del_acc {
  // TODO
} near_del_acc;

/// ref: https://nomicon.io/RuntimeSpec/Actions#functioncallaction
typedef struct near_fn_call {
  uint32_t method_name_length;
  const char *method_name;
  uint32_t args_length;
  const uint8_t *args;
  uint8_t gas[NEAR_NONCE_SIZE_BYTES];
  uint8_t deposit[NEAR_DEPOSIT_SIZE_BYTES];
} near_fn_call;

typedef struct near_unsigned_txn {
  uint32_t receiver_id_length;
  uint32_t signer_id_length;
  const uint8_t *signer;
  const uint8_t *receiver;
  near_key signer_key;
  const uint8_t *blockhash;
  uint8_t nonce[NEAR_NONCE_SIZE_BYTES];
  uint32_t action_count;
  uint8_t actions_type;
  union {
    near_transfer transfer;
    near_fn_call fn_call;
  } action;
} near_unsigned_txn;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern const near_config_t near_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* NEAR_CONTEXT_H */
