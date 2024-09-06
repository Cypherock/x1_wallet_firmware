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
#include "card_pair.h"
#include "constant_texts.h"
#include "core_error.h"
#include "inheritance/core.pb.h"
#include "inheritance_api.h"
#include "inheritance_priv.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
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
 * @brief Retrieves and stores seed-based entropy for authentication.
 *
 * Generates entropy from the seed based on the wallet ID and stores it in the
 * auth structure.
 *
 * @return true if seed-based entropy is successfully generated and stored,
 * false otherwise.
 */
static bool auth_wallet_get_seed_entropy();

/**
 * @brief Pairs the card with the device.
 *
 * Initiates the card pairing process and updates the app flow status upon
 * success.
 *
 * @return true if the card is successfully paired, false otherwise.
 */
static bool auth_wallet_pair_card();

/**
 * @brief Retrieves and stores wallet-based entropy from the card.
 *
 * Fetches encrypted wallet ID data from the card to generate entropy.
 *
 * @return true if wallet-based entropy is successfully fetched and stored,
 * false otherwise.
 */
static bool auth_wallet_get_wallet_entropy();

/**
 * @brief Retrieves encrypted data (entropy) from the card based on the type
 * requested from the host (seed based and/or wallet_id based)
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
 * @brief Signs the given challenge.
 *
 * The function generates an Ed25519 signature for the provided challenge
 * and verifies it against the public key.
 *
 * @return true if the signature is successfully created and verified, false
 * otherwise.
 */
static bool auth_wallet_sign_challenge(const uint8_t *unsigned_txn,
                                       const size_t unsigned_txn_size,
                                       const ed25519_secret_key private_key,
                                       const ed25519_public_key public_key,
                                       ed25519_signature signature);

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

static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->encrypt.which_request) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

STATIC bool auth_wallet_handle_inititate_query(inheritance_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query,
                           INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG)) {
    return false;
  }
  if (get_wallet_name_by_id(query->auth_wallet.initiate.wallet_id,
                            (uint8_t *)wallet_name,
                            NULL)) {
    snprintf(msg,
             sizeof(msg),
             ui_text_inheritance_wallet_auth_flow_confirmation,
             wallet_name);

  } else {
    snprintf(msg,
             sizeof(msg),
             "%s",
             ui_text_inheritance_wallet_auth_flow_confirmation_generic);
  }

  if (!core_confirmation(msg, inheritance_send_error)) {
    return false;
  }

  // TODO: update flow status here

  return true;
}

static bool verify_auth_wallet_inputs() {
  if (auth->data.challenge_size == 0 ||
      auth->data.challenge_size < CHALLENGE_SIZE_MIN ||
      auth->data.challenge_size > CHALLENGE_SIZE_MAX ||
      (auth->do_wallet_based == false && auth->do_seed_based == false)) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_QUERY);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }
  return true;
}

static bool auth_wallet_get_seed_entropy() {
  if (auth->do_seed_based) {
    uint8_t seed[SIZE_SEED] = {0};
    if (!reconstruct_seed_without_passphrase(
            auth->data.wallet_id, seed, inheritance_send_error)) {
      memzero(seed, sizeof(seed));
      return false;
    }
    memcpy((void *)auth->seed_based_data.entropy, seed, SIZE_SEED);
    auth->seed_based_data.entropy_size = SIZE_SEED;
    auth->seed_based_data.has_data = true;
    memzero(seed, sizeof(seed));
    // seed generation complete
    set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_SEED_BASED_CARD_TAPPED);
  }
  return true;
}

static bool auth_wallet_pair_card() {
  card_error_type_e status = single_card_pair_operation(
      (char *)ui_text_tap_the_card, ui_text_place_card_below);
  if (status != CARD_OPERATION_SUCCESS) {
    return false;
  }
  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_PAIRING_CARD_TAPPED);
  return true;
}

static bool auth_wallet_get_wallet_entropy() {
  if (auth->do_wallet_based) {
    // Pair the card first
    if (!auth_wallet_pair_card()) {
      return false;
    }
    secure_data_t msgs[1] = {0};
    msgs[0].plain_data_size = WALLET_ID_SIZE;
    memcpy(msgs[0].plain_data, auth->data.wallet_id, WALLET_ID_SIZE);
    // fetch encrypted wallet_id
    card_error_type_e status =
        card_fetch_encrypt_data(auth->data.wallet_id, msgs, 1);
    if (status != CARD_OPERATION_SUCCESS ||
        msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
      return false;
    }
    memcpy((void *)auth->wallet_based_data.entropy,
           msgs[0].encrypted_data,
           msgs[0].encrypted_data_size);
    auth->wallet_based_data.entropy_size = msgs[0].encrypted_data_size;
    auth->wallet_based_data.has_data = true;
    // wallet id encryption complete
    set_app_flow_status(
        INHERITANCE_AUTH_WALLET_STATUS_WALLET_BASED_CARD_TAPPED);
  }
  return true;
}

static bool auth_wallet_get_entropy() {
  if (!auth_wallet_get_seed_entropy() || !auth_wallet_get_wallet_entropy()) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }
  delay_scr_init(ui_text_inheritance_wallet_authenticating, DELAY_SHORT);
  return true;
}

static bool auth_wallet_get_pairs() {
  if (auth->seed_based_data.has_data) {
    mnemonic_to_seed((char *)auth->seed_based_data.entropy,
                     "",
                     auth->seed_based_data.private_key,
                     NULL);
    ed25519_publickey(auth->seed_based_data.private_key,
                      auth->seed_based_data.result.public_key);
    // Clear seed as soon as it is not needed
    memzero((void *const)auth->seed_based_data.entropy,
            sizeof(auth->seed_based_data.entropy));
  }
  if (auth->wallet_based_data.has_data) {
    mnemonic_to_seed((char *)auth->wallet_based_data.entropy,
                     "",
                     auth->wallet_based_data.private_key,
                     NULL);
    ed25519_publickey(auth->wallet_based_data.private_key,
                      auth->wallet_based_data.result.public_key);
  }
  return true;
}
static bool auth_wallet_sign_challenge(const uint8_t *unsigned_txn,
                                       const size_t unsigned_txn_size,
                                       const ed25519_secret_key private_key,
                                       const ed25519_public_key public_key,
                                       ed25519_signature signature) {
  ed25519_sign(
      unsigned_txn, unsigned_txn_size, private_key, public_key, signature);

  int valid =
      ed25519_sign_open(unsigned_txn, unsigned_txn_size, public_key, signature);

  if (0 != valid) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return false;
  }
  return true;
}

static bool auth_wallet_get_signature() {
  const size_t unsigned_txn_size = auth->data.challenge_size + WALLET_ID_SIZE;
  uint8_t unsigned_txn[unsigned_txn_size];
  memcpy(unsigned_txn, auth->data.challenge, auth->data.challenge_size);
  memcpy(unsigned_txn + auth->data.challenge_size,
         auth->data.wallet_id,
         WALLET_ID_SIZE);
  if (auth->do_seed_based) {
    if (!auth_wallet_sign_challenge(unsigned_txn,
                                    unsigned_txn_size,
                                    auth->seed_based_data.private_key,
                                    auth->seed_based_data.result.public_key,
                                    auth->seed_based_data.result.signature)) {
      return false;
    }
  }
  if (auth->do_wallet_based) {
    if (!auth_wallet_sign_challenge(unsigned_txn,
                                    unsigned_txn_size,
                                    auth->wallet_based_data.private_key,
                                    auth->wallet_based_data.result.public_key,
                                    auth->wallet_based_data.result.signature)) {
      return false;
    }
  }
  return true;
}

static bool send_result() {
  inheritance_result_t result = INHERITANCE_RESULT_INIT_ZERO;
  result.which_response = INHERITANCE_RESULT_AUTH_WALLET_TAG;
  result.auth_wallet.which_response =
      INHERITANCE_AUTH_WALLET_RESPONSE_RESULT_TAG;
  if (auth->do_seed_based) {
    memcpy(result.auth_wallet.result.seed_based.signature,
           auth->seed_based_data.result.signature,
           sizeof(ed25519_signature));
    result.auth_wallet.result.has_seed_based = true;
    if (auth->with_public_key) {
      memcpy(result.auth_wallet.result.seed_based.public_key,
             auth->seed_based_data.result.public_key,
             sizeof(ed25519_public_key));
      result.auth_wallet.result.seed_based.has_public_key = true;
    }
  }
  if (auth->do_wallet_based) {
    memcpy(result.auth_wallet.result.wallet_based.signature,
           auth->wallet_based_data.result.signature,
           sizeof(ed25519_signature));
    result.auth_wallet.result.has_wallet_based = true;
    if (auth->with_public_key) {
      memcpy(result.auth_wallet.result.wallet_based.public_key,
             auth->wallet_based_data.result.public_key,
             sizeof(ed25519_public_key));
      result.auth_wallet.result.wallet_based.has_public_key = true;
    }
  }
  inheritance_send_result(&result);
  return true;
}
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_auth_wallet(inheritance_query_t *query) {
  auth = (auth_wallet_config_t *)malloc(sizeof(auth_wallet_config_t));
  ASSERT(auth != NULL);
  memzero(auth, sizeof(auth_wallet_config_t));

  memcpy(auth->data.wallet_id,
         query->auth_wallet.initiate.wallet_id,
         WALLET_ID_SIZE);
  auth->data.challenge_size = query->auth_wallet.initiate.challenge.size;
  memcpy(auth->data.challenge,
         query->auth_wallet.initiate.challenge.bytes,
         auth->data.challenge_size);
  auth->with_public_key = query->auth_wallet.initiate.with_public_key;
  auth->do_seed_based = query->auth_wallet.initiate.do_seed_based;
  auth->do_wallet_based = query->auth_wallet.initiate.do_wallet_based;

  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_INIT);
  if (verify_auth_wallet_inputs() &&
      auth_wallet_handle_inititate_query(query) && auth_wallet_get_entropy() &&
      auth_wallet_get_pairs() && auth_wallet_get_signature() && send_result()) {
    delay_scr_init(ui_text_inheritance_wallet_auth_success, DELAY_TIME);
  }

  memzero(auth, sizeof(auth_wallet_config_t));
  free(auth);
  auth = NULL;
}
