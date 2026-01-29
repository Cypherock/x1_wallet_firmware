/**
 * @file    canton_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the canton app
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CANTON_CONTEXT_H
#define CANTON_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include "canton/sign_txn.pb.h"
#include "sha2.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define CANTON_NAME "CANTON"
#define CANTON_LUNIT "CC"

#define CANTON_IMPLICIT_ACCOUNT_DEPTH 5

#define CANTON_PURPOSE_INDEX 0x8000002C    // 44'
#define CANTON_COIN_INDEX 0x80001a6f       // 6767'
#define CANTON_ACCOUNT_INDEX 0x80000000    // 0'
#define CANTON_CHANGE_INDEX 0x80000000     // 0'

#define CANTON_PUB_KEY_SIZE 32
#define CANTON_HASH_PURPOSE_SIZE 4
#define CANTON_HASH_PREFIX_SIZE 2
#define CANTON_HASH_SIZE CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH
#define CANTON_FINGERPRINT_SIZE CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH
#define CANTON_PARTY_HINT_SIZE 5
#define CANTON_FINGERPRINT_STR_SIZE (((CANTON_FINGERPRINT_SIZE) * 2) + 1)
#define CANTON_PARTY_HINT_STR_SIZE ((CANTON_PARTY_HINT_SIZE * 2) + 1)
#define CANTON_PARTY_ID_SEPARATOR_SIZE 3 /*for ::*/
#define CANTON_PARTY_ID_SIZE                                                   \
  (CANTON_PARTY_HINT_STR_SIZE + CANTON_PARTY_ID_SEPARATOR_SIZE +               \
   CANTON_FINGERPRINT_STR_SIZE - 2) /*for null byte*/
#define CANTON_PARTY_ID_STR_SIZE_MAX 256
#define CANTON_INPUT_CONTRACT_HASH_SIZE 8 + SHA256_DIGEST_LENGTH
#define ENCODED_TXN_LENGTH                                                     \
  ((4 + 1 + SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH))
#define CANTON_MAX_TOPOLOGY_TXNS_COUNT 5
// TODO: add references to the purpose
#define CANTON_TOPOLOGY_TXN_HASH_PURPOSE 11
#define CANTON_MULTI_TOPOLOGY_TXNS_COMBINED_HASH_PURPOSE 55

#define TAP_TXN_TYPE_TEXT "Tap"
#define TRANSFER_TXN_TYPE_TEXT "Transfer"
#define WITHDRAW_TXN_TYPE_TEXT "Cancel"
#define ACCEPT_TXN_TYPE_TEXT "Accept"
#define REJECT_TXN_TYPE_TEXT "Reject"
#define PREAPPROVAL_TXN_TYPE_TEXT "Enable Transfer Pre-approval"
#define MERGE_DELEGATION_PROPOSAL_TXN_TYPE_TEXT "Enable UTXO Management"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
} canton_config_t;

typedef struct {
  int32_t node_id;
  uint8_t hash[SHA256_DIGEST_LENGTH];
} canton_txn_node_hash_t;

typedef enum {
  CANTON_TXN_TYPE_UNRECOGNIZED = -1,
  CANTON_TXN_TYPE_TAP = 0,
  CANTON_TXN_TYPE_TRANSFER = 1,
  CANTON_TXN_TYPE_WITHDRAW = 2,
  CANTON_TXN_TYPE_ACCEPT = 3,
  CANTON_TXN_TYPE_REJECT = 4,
  CANTON_TXN_TYPE_PREAPPROVAL = 5,
  CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL = 6,
} canton_transaction_type_t;

typedef struct {
  char id[256];
  char admin[CANTON_PARTY_ID_STR_SIZE_MAX];
} canton_instrument_t;

typedef struct {
  canton_transaction_type_t txn_type;
  char sender_party_id[CANTON_PARTY_ID_STR_SIZE_MAX];
  char receiver_party_id[CANTON_PARTY_ID_STR_SIZE_MAX];
  char amount[30];
  uint64_t start_time;
  uint64_t expiry_time;
  char owner_party_id[CANTON_PARTY_ID_STR_SIZE_MAX];
  canton_instrument_t instrument;
  // TODO: add more fields if/when required like memo, fee, etc.
} canton_txn_display_info_t;

typedef struct {
  uint8_t hash[CANTON_INPUT_CONTRACT_HASH_SIZE];
} canton_txn_input_contract_hash_t;

typedef struct {
  canton_sign_txn_transaction_metadata_t txn_meta;
  canton_sign_txn_canton_metadata_t canton_meta;
  canton_node_seed_t *txn_node_seeds;
  canton_txn_node_hash_t *txn_node_hashes;
  canton_txn_input_contract_hash_t *input_contract_hashes;
  canton_txn_display_info_t txn_display_info;
} canton_unsigned_txn;

typedef struct {
  uint8_t hash[CANTON_HASH_SIZE];
} canton_party_txn;

typedef struct {
  canton_party_txn party_txns[CANTON_MAX_TOPOLOGY_TXNS_COUNT];
  bool has_party_id;
  char party_id[CANTON_PARTY_ID_SIZE];
  bool has_public_key;
  uint8_t public_key[CANTON_PUB_KEY_SIZE];
} canton_unsigned_topology_txn;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif
