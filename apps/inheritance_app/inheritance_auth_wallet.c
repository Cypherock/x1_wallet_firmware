/**
 * @file    inheritance_auth_wallet.c
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

#include <stdbool.h>
#include <stdint.h>

#include "bip39.h"
#include "card_fetch_data.h"
#include "inheritance/core.pb.h"
#include "inheritance_api.h"
#include "inheritance_main.h"
#include "inheritance_priv.h"
#include "nfc.h"
#include "reconstruct_wallet_flow.h"
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
 * STATIC VARIABLES
 *****************************************************************************/
static auth_wallet_config_t *auth = NULL;

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
static bool verify_auth_wallet_inputs();

/**
 * @brief Retrieves encrypted data (entropy) from the card based on the wallet
 * ID.
 *
 * This function initializes a secure_data_t structure, fetches encrypted data
 * from the card, and stores the result in the auth structure. It checks if the
 * operation was successful and if the encrypted data size is within the allowed
 * limit.
 *
 * @return true If the entropy was successfully fetched and stored.
 * @return false If there was an error in fetching the encrypted data or if the
 * data size exceeds the limit.
 */
static bool auth_wallet_get_entropy();

/**
 * @brief Generates the public and private key pairs based on the entropy.
 *
 * This function derives a seed from the entropy and generates an Ed25519 key
 * pair (private and public keys). It stores the keys in the auth structure.
 *
 * @return true Always returns true.
 */
static bool auth_wallet_get_pairs();

/**
 * @brief Generates and verifies a digital signature for the wallet
 * authentication.
 *
 * This function creates an unsigned transaction by concatenating the challenge
 * and wallet ID. It then signs the transaction using the private key and
 * verifies the signature using the public key.
 *
 * @return true If the signature was successfully generated and verified.
 * @return false If the signature verification failed.
 */
static bool auth_wallet_get_signature();

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool verify_auth_wallet_inputs() {
  if (NULL == auth->challenge || NULL == auth->wallet_id ||
      auth->challenge_size < CHALLENGE_SIZE_MIN ||
      auth->challenge_size > CHALLENGE_SIZE_MAX) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_QUERY);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }

  return true;
}

static bool auth_wallet_get_entropy() {
  if (!auth->is_seed_based) {
    secure_data_t msgs[1] = {0};
    msgs[0].plain_data_size = WALLET_ID_SIZE;
    memcpy(msgs[0].plain_data, auth->wallet_id, WALLET_ID_SIZE);

    card_error_type_e status =
        card_fetch_encrypt_data(auth->wallet_id, msgs, 1);
    if (status != CARD_OPERATION_SUCCESS ||
        msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
      delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
      return false;
    }
    memcpy((void *)auth->entropy,
           msgs[0].encrypted_data,
           msgs[0].encrypted_data_size);
    auth->entropy_size = msgs[0].encrypted_data_size;
  } else {
    uint8_t seed[64] = {0};
    if (!reconstruct_seed_without_passphrase(
            auth->wallet_id, seed, inheritance_send_error)) {
      memzero(seed, sizeof(seed));
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_QUERY);
      delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
      return false;
    }
    memcpy((void *)auth->entropy, seed, 64);
    auth->entropy_size = 64;
    memzero(seed, sizeof(seed));
  }
  delay_scr_init(ui_text_inheritance_wallet_authenticating, DELAY_SHORT);
  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_CARD_TAPPED);

  return true;
}

static bool auth_wallet_get_pairs() {
  mnemonic_to_seed((char *)auth->entropy, "", auth->private_key, NULL);
  ed25519_publickey(auth->private_key, auth->public_key);
  // Clear seed as soon as it is not needed
  memzero((void *const)auth->entropy, sizeof(auth->entropy));
  return true;
}

static bool auth_wallet_get_signature() {
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
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }

  return true;
}

static bool send_result() {
  inheritance_result_t result = INHERITANCE_RESULT_INIT_ZERO;
  result.which_response = INHERITANCE_RESULT_AUTH_WALLET_TAG;
  result.auth_wallet.which_response =
      INHERITANCE_AUTH_WALLET_RESPONSE_RESULT_TAG;
  memcpy(result.auth_wallet.result.signature,
         auth->signature,
         sizeof(ed25519_signature));

  if (auth->is_setup) {
    memcpy(result.auth_wallet.result.public_key,
           auth->public_key,
           sizeof(ed25519_public_key));
  }

  inheritance_send_result(&result);
  return true;
}
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_auth_wallet(inheritance_query_t *query) {
  auth = (auth_wallet_config_t *)cy_malloc(sizeof(auth_wallet_config_t));
  memzero(auth, sizeof(auth_wallet_config_t));

  memcpy(
      auth->wallet_id, query->auth_wallet.initiate.wallet_id, WALLET_ID_SIZE);
  auth->challenge_size = query->auth_wallet.initiate.challenge.size;
  memcpy(auth->challenge,
         query->auth_wallet.initiate.challenge.bytes,
         auth->challenge_size);
  auth->is_setup = query->auth_wallet.initiate.is_public_key;
  auth->is_seed_based = query->auth_wallet.initiate.is_seed_based;

  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_INIT);
  if (verify_auth_wallet_inputs() && auth_wallet_get_entropy() &&
      auth_wallet_get_pairs() && auth_wallet_get_signature() && send_result()) {
    delay_scr_init(ui_text_inheritance_wallet_auth_success, DELAY_TIME);
  }
}
