/**
 * @file    wallet_sign.c
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "inheritance_app.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_auth_helpers.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static wallet_auth_t *auth = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool verify_wallet_auth_inputs() {
  ASSERT(auth->challenge != NULL);
  ASSERT(auth->wallet_id != NULL);
  ASSERT(auth->challenge_size != NULL);
  ASSERT(CHALLENGE_SIZE_MIN <= auth->challenge_size &&
         auth->challenge_size <= CHALLENGE_SIZE_MAX);

  return true;
}

static bool wallet_auth_get_entropy() {
  SecureMsg msgs[1] = {0};
  msgs[0].plain_data_size = WALLET_ID_SIZE;
  memcpy(msgs[0].plain_data, auth->wallet_id, WALLET_ID_SIZE);

  card_error_type_e status = card_fetch_encrypt_data(auth->wallet_id, msgs, 1);

  if (status != CARD_OPERATION_SUCCESS ||
      msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
    return false;
  }

  memcpy(auth->entropy, msgs[0].encrypted_data, msgs[0].encrypted_data_size);
  auth->entropy_size = msgs[0].encrypted_data_size;

  return true;
}

static bool wallet_auth_get_pairs() {
  mnemonic_to_seed(auth->entropy, "", auth->private_key, NULL);
  ed25519_publickey(auth->private_key, auth->public_key);

  memcpy(auth->public_key, auth->public_key, sizeof(ed25519_public_key));

  return true;
}

static bool wallet_auth_get_signature() {
  const size_t unsigned_txn_size = auth->challenge_size + WALLET_ID_SIZE;
  uint8_t unsigned_txn[unsigned_txn_size];

  memcpy(unsigned_txn, auth->challenge, auth->challenge_size);
  memcpy(unsigned_txn + auth->challenge_size, auth->wallet_id, WALLET_ID_SIZE);

  ed25519_sign(unsigned_txn,
               unsigned_txn_size,
               auth->private_key,
               auth->public_key,
               auth->signature);

  int valid = ed25519_sign_open(
      unsigned_txn, unsigned_txn_size, auth->public_key, auth->signature);

  if (0 != valid) {
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void wallet_login(inheritance_query_t *query) {
  auth = (wallet_auth_t *)malloc(sizeof(wallet_auth_t));
  memzero(auth, sizeof(wallet_auth_t));

  memcpy(
      auth->wallet_id, query->wallet_auth.initiate.wallet_id, WALLET_ID_SIZE);
  auth->challenge_size = query->wallet_auth.initiate.challenge.size;
  memcpy(auth->challenge,
         query->wallet_auth.initiate.challenge.bytes,
         auth->challenge_size);
  auth->is_setup = query->wallet_auth.initiate.is_publickey;

  if (!verify_wallet_auth_inputs() || !wallet_auth_get_entropy() ||
      !wallet_auth_get_pairs() || !wallet_auth_get_signature()) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    return;
  }

  inheritance_result_t result = INHERITANCE_RESULT_INIT_ZERO;
  result.which_response = INHERITANCE_RESULT_WALLET_AUTH_TAG;
  result.wallet_auth.which_response =
      INHERITANCE_WALLET_AUTH_RESPONSE_RESULT_TAG;
  // send public key if Setup
  if (auth->is_setup) {
    memcpy(result.wallet_auth.result.public_key,
           auth->public_key,
           sizeof(ed25519_public_key));
  }
  memcpy(result.wallet_auth.result.signature,
         auth->signature,
         sizeof(ed25519_signature));
  inheritance_send_result(&result);

  if (NULL != auth) {
    free(auth);
    auth = NULL;
  }
}
