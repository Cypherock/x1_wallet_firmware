/**
 * @file    starknet_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for Starknet
 *protocol
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "bignum.h"
#include "ecdsa.h"
#include "mini-gmp-helpers.h"
#include "mini-gmp.h"
#include "rand.h"
#include "reconstruct_wallet_flow.h"
#include "rfc6979.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_helpers.h"
#include "starknet_poseidon.h"
#include "starknet_priv.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"

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
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of starknet_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(const pb_size_t which_response);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of starknet_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const starknet_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * STARKNET_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the starknet_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const starknet_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type starknet_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(starknet_query_t *query);

/**
 * @brief This function executes user verification flow of the unsigned txn
 * received from the host.
 * @details The user verification flow is different for different type of action
 * types identified from the unsigned txn
 * @note This function expected that the unsigned txn is parsed using the helper
 * function as only few action types are supported currently.
 *
 * @return true If the user accepted the transaction display
 * @return false If any user rejection occured or P0 event occured
 */
static bool get_user_verification(void);

/**
 * @brief Calculates ED25519 curve based signature over the digest of the user
 * verified unsigned txn.
 * @details Seed reconstruction takes place within this function
 *
 * @param signature_buffer Reference to buffer where the signature will be
 * populated
 * @return true If the signature was computed successfully
 * @return false If signature could not be computed - maybe due to some error
 * during seed reconstruction phase
 */
static bool sign_txn(uint8_t *signature_buffer);

/**
 * @brief Sends signature of the STARKNET unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * STARKNET_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type starknet_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(starknet_query_t *query, const uint8_t *signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static starknet_txn_context_t *starknet_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void mpz_to_bn(bignum256 *bn, const mpz_t mpz) {
  uint8_t out[32] = {0};
  mpz_to_byte_array(mpz, out, 32);
  bn_read_be(out, bn);
}

void bn_to_mpz(mpz_t mpz, const bignum256 *bn) {
  uint8_t in[32] = {0};
  bn_write_be(bn, in);
  mpz_import(mpz, 32, 1, 1, 1, 0, in);
}

// generate K in a deterministic way, according to RFC6979
// http://tools.ietf.org/html/rfc6979
void generate_k_rfc6979_mpz(mpz_t k, rfc6979_state *state) {
  uint8_t buf[32] = {0};
  generate_rfc6979(buf, state);
  mpz_import(k, 32, 1, 1, 1, 0, buf);
  memzero(buf, sizeof(buf));
}

// generate random K for signing/side-channel noise
static void generate_k_random(bignum256 *k, const bignum256 *prime) {
  do {
    int i = 0;
    for (i = 0; i < 8; i++) {
      k->val[i] = random32() & 0x3FFFFFFF;
    }
    k->val[8] = random32() & 0xFFFF;
    // check that k is in range and not zero.
  } while (bn_is_zero(k) || !bn_is_less(k, prime));
}

void generate_k_random_mpz(mpz_t k, const mpz_t prime) {
  bignum256 prime_bn, k_bn = {0};
  mpz_to_bn(&prime_bn, prime);
  mpz_to_bn(&k_bn, k);
  generate_k_random(&k_bn, &prime_bn);
  bn_to_mpz(k, &k_bn);
}

int starknet_sign_digest(const stark_curve *curve,
                         const uint8_t *priv_key,
                         const uint8_t *digest,
                         uint8_t *sig,
                         uint8_t *pby,
                         int (*is_canonical)(uint8_t by, uint8_t sig[64])) {
  int i = 0;
  stark_point R = {0};
  mpz_t k, z, randk;
  stark_point_init(&R);
  mpz_t *s = &R.y;
  mpz_init(k);
  mpz_init(z);
  mpz_init(randk);

#if USE_RFC6979
  rfc6979_state rng = {0};
  init_rfc6979(priv_key, digest, &rng);
#endif
  mpz_import(z, 32, 1, 1, 0, 0, digest);
  for (i = 0; i < 10000; i++) {
#if USE_RFC6979
    // generate K deterministically
    generate_k_rfc6979_mpz(k, &rng);

    // k >> 4
    mpz_fdiv_q_2exp(k,
                    k,
                    4);    ///< No idea yet why it works; but right shifting k
                           ///< by 4 bits is required

    // if k is too big or too small, we don't like it
    if ((mpz_cmp_ui(k, 0) == 0) || !(mpz_cmp(k, curve->order) < 0)) {
      continue;
    }
#else
    // generate random number k
    generate_k_random_mpz(k, curve->order);
#endif
    // compute k*G
    stark_point_multiply(curve, k, &curve->G, &R);
    mpz_mod(R.x, R.x, curve->order);
    // r = (rx mod n)
    if (!(mpz_cmp(R.x, curve->order) < 0)) {
      mpz_sub(R.x, R.x, curve->order);
    }
    // if r is zero, we retry
    if (mpz_cmp_ui(R.x, 0) == 0) {
      continue;
    }

    // randomize operations to counter side-channel attacks
    generate_k_random_mpz(randk, curve->order);

    // k = k * rand mod n
    mpz_mul(k, k, randk);
    mpz_mod(k, k, curve->order);

    // k = (k * rand)^-1
    mpz_invert(k, k, curve->order);

    mpz_import(*s, 32, 1, 1, 1, 0, priv_key);
    // R.x*priv
    mpz_mul(*s, *s, R.x);
    mpz_mod(*s, *s, curve->order);
    mpz_add(*s, *s, z);    // R.x*priv + z

    // (k*rand)^-1 (R.x*priv + z)
    mpz_mul(*s, *s, k);
    mpz_mod(*s, *s, curve->order);

    // k^-1 (R.x*priv + z)
    mpz_mul(*s, *s, randk);
    mpz_mod(*s, *s, curve->order);

    // if s is zero, we retry
    if ((mpz_cmp_ui(*s, 0) == 0)) {
      continue;
    }

    // if S > order/2 => S = -S
    if ((mpz_cmp(curve->order_half, *s) < 0)) {
      mpz_sub(*s, *s, curve->order);
    }
    // we are done, R.x and s is the result signature
    mpz_to_byte_array(R.x, sig, 32);
    mpz_to_byte_array(*s, sig + 32, 32);

    mpz_clear(k);
    mpz_clear(randk);
    mpz_clear(z);
    mpz_clear(k);

#if USE_RFC6979
    memzero(&rng, sizeof(rng));
#endif
    return 0;
  }

  // Too many retries without a valid signature
  // -> fail with an error
  memzero(&k, sizeof(k));
  memzero(&randk, sizeof(randk));
#if USE_RFC6979
  memzero(&rng, sizeof(rng));
#endif
  return -1;
}

static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  starknet_result_t result = init_starknet_result(STARKNET_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  starknet_send_result(&result);
}

static bool validate_request_data(const starknet_sign_txn_request_t *request) {
  bool status = true;

  if (!starknet_derivation_path_guard(
          request->initiate.derivation_path,
          request->initiate.derivation_path_count)) {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }
  return status;
}

static bool handle_initiate_query(const starknet_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, STARKNET_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             starknet_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SIGN_TXN_PROMPT,
           starknet_app.name,
           wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, starknet_send_error)) {
    return false;
  }

  set_app_flow_status(STARKNET_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&starknet_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(starknet_sign_txn_initiate_request_t));

  send_response(STARKNET_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(starknet_query_t *query) {
  if (!starknet_get_query(query, STARKNET_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query, STARKNET_SIGN_TXN_REQUEST_TXN_TAG)) {
    return false;
  }

  // Get txn type
  starknet_txn_context->which_type = query->sign_txn.txn.which_type;
  switch (starknet_txn_context->which_type) {
    case STARKNET_SIGN_TXN_UNSIGNED_TXN_INVOKE_TXN_TAG: {
      starknet_txn_context->invoke_txn = &query->sign_txn.txn.invoke_txn;
    } break;

    case STARKNET_SIGN_TXN_UNSIGNED_TXN_DEPLOY_TXN_TAG: {
      starknet_txn_context->deploy_txn = &query->sign_txn.txn.deploy_txn;
    } break;

    default: {
      // should not reach here;
      return false;
    }
  }

  if (1) {
    send_response(STARKNET_SIGN_TXN_RESPONSE_UNSIGNED_TXN_ACCEPTED_TAG);
    return true;
  } else {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return false;
}

static bool get_invoke_txn_user_verification() {
  char address[100] = "0x";
  byte_array_to_hex_string(
      starknet_txn_context->invoke_txn->sender_address, 32, &address[2], 64);

  if (!core_scroll_page(ui_text_verify_address, address, starknet_send_error)) {
    return false;
  }
  // TODO:Verify Call Data
}

static bool get_deploy_txn_user_verification() {
  char address[100] = "0x";
  byte_array_to_hex_string(
      starknet_txn_context->deploy_txn->contract_address, 32, &address[2], 64);

  if (!core_scroll_page(ui_text_verify_address, address, starknet_send_error)) {
    return false;
  }
  // TODO:Verify Constructor Call Data
}

static bool get_user_verification(void) {
  bool user_verified = false;

  switch (starknet_txn_context->which_type) {
    case STARKNET_SIGN_TXN_UNSIGNED_TXN_INVOKE_TXN_TAG: {
      user_verified = get_invoke_txn_user_verification();

    } break;

    case STARKNET_SIGN_TXN_UNSIGNED_TXN_DEPLOY_TXN_TAG: {
      user_verified = get_deploy_txn_user_verification();

    } break;
  }

  if (user_verified) {
    set_app_flow_status(STARKNET_SIGN_TXN_STATUS_VERIFY);
  }

  return user_verified;
}

static bool sign_txn(uint8_t *signature_buffer) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(starknet_txn_context->init_info.wallet_id,
                        seed,
                        starknet_send_error)) {
    memzero(seed, sizeof(seed));
    return false;
  }

  set_app_flow_status(STARKNET_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t stark_key[32] = {0};
  if (starknet_derive_key_from_seed(
          stark_key,
          starknet_txn_context->init_info.derivation_path,
          starknet_txn_context->init_info.derivation_path_count,
          stark_key,
          NULL)) {
  } else {
    starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
  }

  // calculate txn hash
  felt_t hash_felt = {0};
  switch (starknet_txn_context->which_type) {
    case STARKNET_SIGN_TXN_UNSIGNED_TXN_INVOKE_TXN_TAG: {
      calculate_txn_hash((void *)starknet_txn_context->invoke_txn,
                         STARKNET_SIGN_TXN_UNSIGNED_TXN_INVOKE_TXN_TAG,
                         hash_felt);
    } break;

    case STARKNET_SIGN_TXN_UNSIGNED_TXN_DEPLOY_TXN_TAG: {
      calculate_txn_hash((void *)starknet_txn_context->deploy_txn,
                         STARKNET_SIGN_TXN_UNSIGNED_TXN_DEPLOY_TXN_TAG,
                         hash_felt);

    } break;
  }
  uint8_t hash[32] = {0};
  felt_t_to_hex(hash_felt, hash);

  // generate signature
  starknet_sign_digest(
      starkCurve, stark_key, hash, signature_buffer, NULL, NULL);

  memzero(seed, sizeof(seed));
  memzero(stark_key, sizeof(stark_key));

  return true;
}

static bool send_signature(starknet_query_t *query, const uint8_t *signature) {
  starknet_result_t result = init_starknet_result(STARKNET_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = STARKNET_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!starknet_get_query(query, STARKNET_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, STARKNET_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature.signature[0],
         signature,
         sizeof(result.sign_txn.signature.signature));
  starknet_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void starknet_sign_transaction(starknet_query_t *query) {
  starknet_txn_context =
      (starknet_txn_context_t *)malloc(sizeof(starknet_txn_context_t));
  memzero(starknet_txn_context, sizeof(starknet_txn_context_t));

  uint8_t signature[64] = {0};
  memzero(signature, sizeof(signature));

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(signature) &&
      send_signature(query, (const uint8_t *)signature)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  memzero(signature, sizeof(signature));

  if (starknet_txn_context) {
    free(starknet_txn_context);
    starknet_txn_context = NULL;
  }

  return;
}