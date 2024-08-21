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
#include "nem.h"
#include "nfc.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define CHALLENGE_SIZE_MAX 32
#define CHALLENGE_SIZE_MIN 16
#define ENTROPY_SIZE_LIMIT 100

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum {
  AUTH_WALLET_OK = 0,
  AUTH_WALLET_TYPE_INVALID,      // TODO: In inheriance app - Show error message
                                 // on ui that which assert is wrong
  AUTH_WALLET_INPUTS_INVALID,    // TODO: In inheritance app - Add in wallet id
                                 // comparison, show error message on ui
} auth_wallet_error_type_e;

#pragma pack(push, 1)
typedef struct {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t challenge[CHALLENGE_SIZE_MAX];
  size_t challenge_size;

  const uint8_t entropy[ENTROPY_SIZE_LIMIT];
  uint8_t entropy_size;
  ed25519_secret_key private_key;

  ed25519_signature signature;
  ed25519_public_key public_key;

  bool is_setup;
  auth_wallet_error_type_e status;
} auth_wallet_config_t;
#pragma pack(pop)

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
      auth->challenge_size ||
      (CHALLENGE_SIZE_MIN <= auth->challenge_size &&
       auth->challenge_size <= CHALLENGE_SIZE_MAX)) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_QUERY);
    return false;
  }

  return true;
}

static bool auth_wallet_get_entropy() {
  secure_data_t msgs[1] = {0};
  msgs[0].plain_data_size = WALLET_ID_SIZE;
  memcpy(msgs[0].plain_data, auth->wallet_id, WALLET_ID_SIZE);

  card_error_type_e status = card_fetch_encrypt_data(auth->wallet_id, msgs, 1);

  delay_scr_init(ui_text_inheritance_wallet_authenticating, DELAY_SHORT);

  if (status != CARD_OPERATION_SUCCESS ||
      msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
    return false;
  }
  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_CARD_TAPPED);

  memcpy((void *)auth->entropy,
         msgs[0].encrypted_data,
         msgs[0].encrypted_data_size);
  auth->entropy_size = msgs[0].encrypted_data_size;

  return true;
}

static bool auth_wallet_get_pairs() {
  mnemonic_to_seed((char *)auth->entropy, "", auth->private_key, NULL);
  ed25519_publickey(auth->private_key, auth->public_key);

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
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_wallet_login(inheritance_query_t *query) {
  auth = (auth_wallet_config_t *)cy_malloc(sizeof(auth_wallet_config_t));
  memzero(auth, sizeof(auth_wallet_config_t));

  memcpy(
      auth->wallet_id, query->auth_wallet.initiate.wallet_id, WALLET_ID_SIZE);
  auth->challenge_size = query->auth_wallet.initiate.challenge.size;
  memcpy(auth->challenge,
         query->auth_wallet.initiate.challenge.bytes,
         auth->challenge_size);
  auth->is_setup = query->auth_wallet.initiate.is_public_key;

  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_INIT);
  if (!verify_auth_wallet_inputs() || !auth_wallet_get_entropy() ||
      !auth_wallet_get_pairs() || !auth_wallet_get_signature()) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
    delay_scr_init(ui_text_inheritance_wallet_auth_fail, DELAY_TIME);
    return;
  }
  delay_scr_init(ui_text_inheritance_wallet_auth_success, DELAY_TIME);

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
}
