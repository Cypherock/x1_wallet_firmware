/**
 * @file    inheritance_wallet_auth.c
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
#include "inheritance_wallet_auth.h"

#include "bip39.h"
#include "inheritance_main.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

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

/**
 * @brief Verifies the integrity and validity of the wallet authentication
 * inputs.
 *
 * This static function checks if the challenge, wallet ID, and challenge size
 * are non-zero and within the expected range. It ensures the authentication
 * inputs are valid.
 *
 * @return true Always returns true if all assertions pass.
 */
static bool verify_wallet_auth_inputs(wallet_auth_t *auth);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool verify_wallet_auth_inputs(wallet_auth_t *auth) {
  ASSERT(auth->challenge != 0);
  ASSERT(auth->wallet_id != 0);
  ASSERT(auth->challenge_size != 0);
  ASSERT(CHALLENGE_SIZE_MIN <= auth->challenge_size &&
         auth->challenge_size <= CHALLENGE_SIZE_MAX);

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool wallet_auth_get_entropy(wallet_auth_t *auth) {
  SecureData msgs[1] = {0};
  msgs[0].plain_data_size = WALLET_ID_SIZE;
  memcpy(msgs[0].plain_data, auth->wallet_id, WALLET_ID_SIZE);

  card_error_type_e status = card_fetch_encrypt_data(auth->wallet_id, msgs, 1);

  delay_scr_init(ui_text_inheritance_wallet_authenticating, DELAY_SHORT);

  if (status != CARD_OPERATION_SUCCESS ||
      msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
    return false;
  }
  set_app_flow_status(INHERITANCE_WALLET_AUTH_STATUS_CARD_TAPPED);

  memcpy((void *)auth->entropy,
         msgs[0].encrypted_data,
         msgs[0].encrypted_data_size);
  auth->entropy_size = msgs[0].encrypted_data_size;

  return true;
}

bool wallet_auth_get_pairs(wallet_auth_t *auth) {
  mnemonic_to_seed((char *)auth->entropy, "", auth->private_key, NULL);
  ed25519_publickey(auth->private_key, auth->public_key);

  return true;
}

bool wallet_auth_get_signature(wallet_auth_t *auth) {
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
    // Set App flow status
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }

  return true;
}

void inheritance_wallet_login(inheritance_query_t *query) {
  auth = (wallet_auth_t *)malloc(sizeof(wallet_auth_t));
  memzero(auth, sizeof(wallet_auth_t));

  memcpy(
      auth->wallet_id, query->wallet_auth.initiate.wallet_id, WALLET_ID_SIZE);
  auth->challenge_size = query->wallet_auth.initiate.challenge.size;
  memcpy(auth->challenge,
         query->wallet_auth.initiate.challenge.bytes,
         auth->challenge_size);
  auth->is_setup = query->wallet_auth.initiate.is_publickey;

  set_app_flow_status(INHERITANCE_WALLET_AUTH_STATUS_INIT);
  if (!verify_wallet_auth_inputs(auth) || !wallet_auth_get_entropy(auth) ||
      !wallet_auth_get_pairs(auth) || !wallet_auth_get_signature(auth)) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    return;
  }
  // Set App flow status
  delay_scr_init(ui_text_inheritance_wallet_auth_success, DELAY_TIME);

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
