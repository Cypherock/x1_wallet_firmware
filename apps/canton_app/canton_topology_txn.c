/**
 * @file    canton_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for CANTON protocol
 *
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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

#include <stdint.h>

#include "canton/canton_topology_txn.pb.h"
#include "canton/sign_topology_txn.pb.h"
#include "canton_api.h"
#include "canton_context.h"
#include "canton_helpers.h"
#include "canton_priv.h"
#include "coin_utils.h"
#include "curves.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "utils.h"
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
typedef canton_sign_topology_txn_signature_response_t canton_topology_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of canton_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const canton_query_t *query,
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
 * @param request Reference to an instance of canton_sign_topology_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(
    const canton_sign_topology_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * CANTON_SIGN_topology_TXN_REQUEST_INITIATE_TAG type of request. Additionally,
 * the wallet-id is validated for sanity and the derivation path for the account
 * is also validated. After the validations, user is prompted about the action
 * for confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the canton_topology_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const canton_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type canton_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(canton_query_t *query);

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
static bool sign_topology_txn(canton_topology_sig_t *sig);

/**
 * @brief Sends signature of the CANTON unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * CANTON_SIGN_topology_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type canton_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(canton_query_t *query,
                           const canton_topology_sig_t *sig);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static canton_topology_txn_context_t *canton_topology_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const canton_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_topology_txn.which_request) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  canton_result_t result =
      init_canton_result(CANTON_RESULT_SIGN_TOPOLOGY_TXN_TAG);
  result.sign_topology_txn.which_response = which_response;
  canton_send_result(&result);
}

static bool validate_request_data(
    const canton_sign_topology_txn_request_t *request) {
  bool status = true;

  if (!canton_derivation_path_guard(request->initiate.derivation_path,
                                    request->initiate.derivation_path_count)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  return status;
}

static bool handle_initiate_query(const canton_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query,
                           CANTON_SIGN_TOPOLOGY_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_topology_txn) ||
      !get_wallet_name_by_id(query->sign_topology_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             canton_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SIGN_TOPOLOGY_TXN_PROMPT,
           CANTON_NAME,
           wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_scroll_page(NULL, msg, canton_send_error)) {
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TOPOLOGY_TXN_STATUS_CONFIRM);
  memcpy(&canton_topology_txn_context->init_info,
         &query->sign_topology_txn.initiate,
         sizeof(canton_sign_topology_txn_initiate_request_t));

  send_response(CANTON_SIGN_TOPOLOGY_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool validate_and_store_party_txn_proposal(
    const canton_generate_transaction_request_proposal_t *proposal,
    canton_unsigned_topology_txn *ut_txn) {
  // topology txns are of type ADD_REPLACE only
  if (proposal->operation != CANTON_TOPOLOGY_CHANGE_OP_ADD_REPLACE) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  if (proposal->has_mapping) {
    switch (proposal->mapping.which_mapping) {
      case CANTON_TOPOLOGY_MAPPING_NAMESPACE_DELEGATION_TAG: {
        canton_namespace_delegation_t namespace_delegation =
            proposal->mapping.namespace_delegation;
        if (namespace_delegation.which_restriction !=
            CANTON_NAMESPACE_DELEGATION_CAN_SIGN_ALL_MAPPINGS_TAG) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }
        if (!namespace_delegation.has_target_key) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        // TODO: research about the first 12 bytes in the public key field
        uint8_t *public_key = namespace_delegation.target_key.public_key + 12;
        if (ut_txn->has_public_key == true) {
          if (memcmp(ut_txn->public_key, public_key, CANTON_PUB_KEY_SIZE) !=
              0) {
            canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                              ERROR_DATA_FLOW_INVALID_DATA);
            return false;
          }
        } else {
          memcpy(ut_txn->public_key, public_key, CANTON_PUB_KEY_SIZE);
          ut_txn->has_public_key = true;
        }

        // derive party id from the public key
        char party_id[CANTON_PARTY_ID_SIZE] = {0};
        get_party_id(public_key, party_id);

        if (ut_txn->has_party_id == true) {
          if (strcmp(ut_txn->party_id, party_id) != 0) {
            canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                              ERROR_DATA_FLOW_INVALID_DATA);
            return false;
          }
        } else {
          memcpy(ut_txn->party_id, party_id, CANTON_PARTY_ID_SIZE);
          ut_txn->has_party_id = true;
        }

        // compare the namespace(fingerprint) with the namespace in the
        // proposal
        char *namespace_ptr = party_id + CANTON_PARTY_HINT_STR_SIZE +
                              CANTON_PARTY_ID_SEPARATOR_SIZE - 2;
        if (strcmp(namespace_delegation.namespace, namespace_ptr) != 0) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        // TODO: validate other fields
        break;
      }

      case CANTON_TOPOLOGY_MAPPING_PARTY_TO_PARTICIPANT_TAG: {
        canton_party_to_participant_t party_to_participant =
            proposal->mapping.party_to_participant;
        if (ut_txn->has_party_id == true) {
          if (strcmp(ut_txn->party_id, party_to_participant.party) != 0) {
            canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                              ERROR_DATA_FLOW_INVALID_DATA);
            return false;
          }
        } else {
          memcpy(ut_txn->party_id,
                 party_to_participant.party,
                 CANTON_PARTY_ID_SIZE);
          ut_txn->has_party_id = true;
        }

        if (party_to_participant.threshold != 1 ||
            party_to_participant.participants_count != 1) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        if (party_to_participant.participants[0].permission !=
            CANTON_PARTICIPANT_PERMISSION_CONFIRMATION) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        // TODO: validate the participant id
        break;
      }

      case CANTON_TOPOLOGY_MAPPING_PARTY_TO_KEY_MAPPING_TAG: {
        canton_party_to_key_mapping_t party_to_key_mapping =
            proposal->mapping.party_to_key_mapping;
        if (party_to_key_mapping.threshold != 1 ||
            party_to_key_mapping.signing_keys_count != 1) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        uint8_t *public_key =
            party_to_key_mapping.signing_keys[0].public_key + 12;
        if (ut_txn->has_public_key == true) {
          if (memcmp(ut_txn->public_key, public_key, CANTON_PUB_KEY_SIZE) !=
              0) {
            canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                              ERROR_DATA_FLOW_INVALID_DATA);
            return false;
          }
        } else {
          memcpy(ut_txn->public_key, public_key, CANTON_PUB_KEY_SIZE);
          ut_txn->has_public_key = true;
        }

        // derive party id from the public key
        char party_id[CANTON_PARTY_ID_SIZE] = {0};
        get_party_id(public_key, party_id);

        if (strcmp(party_to_key_mapping.party, party_id) != 0) {
          canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_DATA);
          return false;
        }

        if (ut_txn->has_party_id == true) {
          if (strcmp(ut_txn->party_id, party_id) != 0) {
            canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                              ERROR_DATA_FLOW_INVALID_DATA);
            return false;
          }
        } else {
          memcpy(ut_txn->party_id, party_id, CANTON_PARTY_ID_SIZE);
          ut_txn->has_party_id = true;
        }

        break;
      }

      default: {
        canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_DATA);
        return false;
      }
    }
  }
  return true;
}

static bool parse_and_hash_party_txn_proposal(
    const uint8_t *serialized_txn,
    uint32_t serialized_txn_size,
    canton_unsigned_topology_txn *ut_txn,
    uint8_t *hash) {
  // decode the proto serialized txn
  canton_generate_transaction_request_proposal_t decoded_proposal =
      CANTON_GENERATE_TRANSACTION_REQUEST_PROPOSAL_INIT_ZERO;
  if (!decode_canton_serialized_data(
          serialized_txn + 3,    // skip the length prefix
          serialized_txn_size - 3,
          CANTON_GENERATE_TRANSACTION_REQUEST_PROPOSAL_FIELDS,
          &decoded_proposal)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  // validate the proposal and store only required fields
  if (!validate_and_store_party_txn_proposal(&decoded_proposal, ut_txn)) {
    return false;
  }

  // hash the serialized txn and store the hash in the
  // canton_topology_txn_context
  memzero(hash, CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH);
  sha256_canton(CANTON_TOPOLOGY_TXN_HASH_PURPOSE,
                serialized_txn,
                serialized_txn_size,
                hash);

  return true;
}

static bool fetch_valid_input(canton_query_t *query) {
  canton_unsigned_topology_txn *ut_txn =
      &canton_topology_txn_context->unsigned_topology_txn;
  ut_txn->has_party_id = false;
  ut_txn->has_public_key = false;

  uint32_t topology_txns_count =
      canton_topology_txn_context->init_info.topology_txns_count;
  if (topology_txns_count < 1 ||
      topology_txns_count > CANTON_MAX_TOPOLOGY_TXNS_COUNT) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }
  // receives all topology_txns_count number of topology txns one by one
  for (int i = 0; i < topology_txns_count; i++) {
    if (!canton_get_query(query, CANTON_QUERY_SIGN_TOPOLOGY_TXN_TAG) ||
        !check_which_request(query,
                             CANTON_SIGN_TOPOLOGY_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!parse_and_hash_party_txn_proposal(
            query->sign_topology_txn.txn_data.serialized_txn.bytes,
            query->sign_topology_txn.txn_data.serialized_txn.size,
            ut_txn,
            ut_txn->party_txns[i].hash)) {
      return false;
    }

    // send the response
    send_response(CANTON_SIGN_TOPOLOGY_TXN_RESPONSE_DATA_ACCEPTED_TAG);
  }

  return true;
}

static bool sign_topology_txn(canton_topology_sig_t *sig) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(canton_topology_txn_context->init_info.wallet_id,
                        seed,
                        canton_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TOPOLOGY_TXN_STATUS_SEED_GENERATED);

  uint8_t digest[CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH] = {0};
  hash_party_txns(canton_topology_txn_context->unsigned_topology_txn.party_txns,
                  canton_topology_txn_context->init_info.topology_txns_count,
                  digest);

  HDNode hdnode = {0};
  derive_hdnode_from_path(
      canton_topology_txn_context->init_info.derivation_path,
      canton_topology_txn_context->init_info.derivation_path_count,
      ED25519_NAME,
      seed,
      &hdnode);

  // match public key with the public key in the proposal
  if (!canton_topology_txn_context->unsigned_topology_txn.has_party_id ||
      !verify_party_id(
          hdnode.public_key + 1,
          canton_topology_txn_context->unsigned_topology_txn.party_id)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    memzero(&hdnode, sizeof(hdnode));
    memzero(digest, sizeof(digest));
    memzero(seed, sizeof(seed));
    return false;
  }

  ed25519_sign(digest,
               CANTON_HASH_SIZE,
               hdnode.private_key,
               hdnode.public_key + 1,
               sig->signature);

  memzero(digest, sizeof(digest));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));

  return true;
}

static bool send_signature(canton_query_t *query,
                           const canton_topology_sig_t *sig) {
  canton_result_t result =
      init_canton_result(CANTON_RESULT_SIGN_TOPOLOGY_TXN_TAG);
  result.sign_topology_txn.which_response =
      CANTON_SIGN_TOPOLOGY_TXN_RESPONSE_SIGNATURE_TAG;

  if (!canton_get_query(query, CANTON_QUERY_SIGN_TOPOLOGY_TXN_TAG) ||
      !check_which_request(query,
                           CANTON_SIGN_TOPOLOGY_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(
      &result.sign_topology_txn.signature, sig, sizeof(canton_topology_sig_t));

  canton_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void canton_sign_topology_transaction(canton_query_t *query) {
  canton_topology_txn_context = (canton_topology_txn_context_t *)malloc(
      sizeof(canton_topology_txn_context_t));
  memzero(canton_topology_txn_context, sizeof(canton_topology_txn_context_t));

  canton_topology_sig_t sig = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      sign_topology_txn(&sig) && send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (canton_topology_txn_context) {
    free(canton_topology_txn_context);
    canton_topology_txn_context = NULL;
  }

  return;
}
