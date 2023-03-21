/**
 * @file    near.h
 * @author  Cypherock X1 Team
 * @brief   Header for near coin.
 *          Stores declarations for near coin functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef NEAR_HEADER
#define NEAR_HEADER

#include <stdint.h>
#include "coin_specific_data.h"
#include "coin_utils.h"

#define NEAR_PURPOSE_INDEX 0x8000002C
#define NEAR_COIN_INDEX    0x8000018D
#define NEAR_ACCOUNT_INDEX 0x80000000
#define NEAR_CHANGE_INDEX  0x80000000
#define NEAR_ADDRESS_INDEX 0x80000001

/// Ref: https://nomicon.io/Economics/Economic
#define NEAR_DECIMAL (24U)
/// Ref: https://docs.near.org/concepts/basics/transactions/gas#the-cost-of-common-actions
#define NEAR_FEES_DECIMAL (20U)

#define NEAR_COIN_VERSION 0x00000000

#define NEAR_REGISTERED_ACCOUNT_COUNT  4
#define NEAR_TOP_LEVEL_ACCOUNT_MIN_LEN 32
#define NEAR_ACC_ID_MIN_LEN            2
#define NEAR_ACC_ID_MAX_LEN            64
#define NEAR_COIN_DATA_MAX_LEN         300

#define NEAR_NONCE_SIZE_BYTES     8
#define NEAR_ALLOWANCE_SIZE_BYTES 8
#define NEAR_DEPOSIT_SIZE_BYTES   16

#define NEAR_TOKEN_NAME   "Near"
#define NEAR_TOKEN_SYMBOL "NEAR"

// Datatypes and structures for the transaction
// ref: https://github.com/near/near-api-js/blob/d4d4cf1ac3182fa998b1e004e6782219325a641b/src/transaction.ts#L139
// ref: https://nomicon.io/DataStructures/DataTypes

typedef enum near_key_type {
  NEAR_CURVE_ED25519   = 0,
  NEAR_CURVE_SECP256K1 = 1,
} near_key_type;

/**
 * @brief Tags for the coin specific data tlv format.
 */
typedef enum near_data_tag {
  TAG_NEAR_DEFAULT_NETWORK = 0x01,
  TAG_NEAR_REGISTERED_ACC  = 0x02,
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
  uint8_t *key;
} near_key;

/// ref: https://nomicon.io/DataStructures/AccessKey#accesskeypermissionfunctioncall
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
  char *
      user_id;  // @see NEAR_TOP_LEVEL_ACCOUNT_MIN_LEN and https://nomicon.io/RuntimeSpec/Actions#createaccountaction
  char public_key[65];
} near_create_acc;

// ref: https://docs.near.org/docs/tutorials/create-transactions#low-level----create-a-transaction
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
  char *method_name;
  uint32_t args_length;
  uint8_t *args;
  uint8_t gas[NEAR_NONCE_SIZE_BYTES];
  uint8_t deposit[NEAR_DEPOSIT_SIZE_BYTES];
} near_fn_call;

typedef struct near_unsigned_txn {
  uint32_t receiver_id_length;
  uint32_t signer_id_length;
  uint8_t *signer;
  uint8_t *receiver;
  near_key signer_key;
  uint8_t *blockhash;
  uint8_t nonce[NEAR_NONCE_SIZE_BYTES];
  uint32_t action_count;
  uint8_t actions_type;
  union {
    near_transfer transfer;
    near_fn_call fn_call;
  } action;
} near_unsigned_txn;

/**
 * @brief Convert byte array representation of unsigned transaction to near_unsigend_txn.
 * @details
 *
 * @param [in] byte_array                   Byte array of unsigned transaction.
 * @param [in] byte_array_size              Size of byte array.
 * @param [out] utxn                        Pointer to the near_unsigned_txn instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 Success
 * @retval -1 Failure
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void near_byte_array_to_unsigned_txn(uint8_t *byte_array,
                                     uint16_t byte_array_size,
                                     near_unsigned_txn *utxn);

/**
 * @brief Signed unsigned byte array.
 * @details
 *
 * @param [in] unsigned_txn_byte_array      Byte array of unsigned transaction.
 * @param [in] unsigned_txn_len             length of unsigned transaction byte array.
 * @param [in] transaction_metadata         Pointer to txn_metadata instance.
 * @param [in] mnemonics                    char array of mnemonics.
 * @param [in] passphrase                   char array of passphrase.
 * @param [out] sig                         Byte array of signature to store the result of signing unsigned transaction byte array.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void near_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                  uint64_t unsigned_txn_len,
                                  const txn_metadata *transaction_metadata,
                                  const char *mnemonics,
                                  const char *passphrase,
                                  uint8_t *sig);

/**
 * @brief gets new_account_id from args field of a create_account method call.
 * 
 * @param args                  args field of a create_account method call.
 * @param args_len              length of args field.
 * @param account_id            new_account_id to store the result.
 * @return size_t               length of new_account_id.
 */
size_t near_get_new_account_id_from_fn_args(const char *args,
                                            uint32_t args_len,
                                            char *account_id);

/**
 * @brief serialize a list of account ids to a byte array in tlv format.
 * 
 * @param account_ids   list of account ids.
 * @param count         count of account ids. 
 * @param data          byte array to store the result.
 * @param data_len      length of byte array.
 */
void near_serialize_account_ids(const char **account_ids,
                                const size_t count,
                                uint8_t *data,
                                uint16_t *data_len);

/**
 * @brief  deserialize a byte array in tlv format to a list of account ids.
 * 
 * @param data          byte array to deserialize.
 * @param data_len      length of byte array.
 * @param account_ids   list of account ids to store the result.
 * @param count         count of account ids.
 */
void near_deserialize_account_ids(const uint8_t *data,
                                  const uint16_t data_len,
                                  char **account_ids,
                                  size_t count);

/**
 * @brief get count of account ids stored in the byte array.
 * 
 * @param data          byte array to get the count.
 * @param data_len      length of byte array.
 * @return size_t       count of account ids.
 */
size_t near_get_account_ids_count(const uint8_t *data, const uint16_t data_len);

/**
 * @brief Verifies the derivation path for any inconsistent/unsupported values.
 * The derivation depth is fixed at level 5. So if the depth level < 5, then this function
 * return false indicating invalid derivation path.
 *
 * @param[in] path      The address derivation path to be checked
 * @param[in] levels    The number of levels in the derivation path
 *
 * @return bool     Returns true if the path values are valid. False otherwise.
 */
bool near_verify_derivation_path(const uint32_t *path, uint8_t levels);

/**
 * @brief Returns the decimal value of near asset
 * 
 * @return uint8_t decimal value
 */
uint8_t near_get_decimal();

#endif  //NEAR_HEADER
