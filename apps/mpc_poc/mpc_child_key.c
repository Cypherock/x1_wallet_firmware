#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "pb_encode.h"
#include "hasher.h"

#include "utils.h"
#include "controller_level_four.h"
#include "string.h"
#include "bignum.h"

#define ENTROPY_BYTES 32
#define MNEMONIC_STRENGTH (ENTROPY_BYTES * 8)
#define SEED_SIZE 512 / 8

static bool check_which_request(const mpc_poc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_child_key.which_request) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

bool get_child_key_initiate(mpc_poc_query_t *query, uint8_t *wallet_id, uint8_t *priv_key, uint8_t *pub_key) {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_GET_CHILD_KEY_TAG);

    mpc_poc_get_child_key_response_t response = MPC_POC_GET_CHILD_KEY_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_GET_CHILD_KEY_RESPONSE_INITIATE_TAG;

    memcpy(wallet_id, query->get_child_key.initiate.wallet_id, 32);

    if (!initiate_application(query->get_child_key.initiate.wallet_id, priv_key, pub_key)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    memcpy(response.initiate.pub_key, pub_key, 33);

    result.get_child_key = response;
    mpc_send_result(&result);

    return true;
}

bool get_child_key_process(mpc_poc_query_t *query, uint8_t *pub_key) {
  if (!mpc_get_query(query, MPC_POC_QUERY_GET_CHILD_KEY_TAG) ||
      !check_which_request(query, MPC_POC_GET_CHILD_KEY_REQUEST_GET_CHILD_PUBLIC_KEY_TAG)) {
    return false;
  }

  if (!query->get_child_key.get_child_public_key.has_group_key_info) {
    mpc_delay_scr_init("Error doesn't have group key info.", DELAY_SHORT);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;
  mpc_poc_result_t result = init_mpc_result(MPC_POC_RESULT_GET_CHILD_KEY_TAG);

  mpc_poc_get_child_key_response_t response = MPC_POC_GET_CHILD_KEY_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_GET_CHILD_KEY_RESPONSE_GET_CHILD_PUBLIC_KEY_TAG;

  uint8_t *group_key_info_bytes = malloc(GROUP_KEY_INFO_BUFFER_SIZE * sizeof(uint8_t));
  uint8_t *seed = malloc(SEED_SIZE * sizeof(uint8_t));

  pb_ostream_t stream = pb_ostream_from_buffer(group_key_info_bytes, GROUP_KEY_INFO_BUFFER_SIZE);

  if (!pb_encode(&stream, MPC_POC_GROUP_KEY_INFO_FIELDS, &query->get_child_key.get_child_public_key.group_key_info)) {
    mpc_delay_scr_init("Error while encoding.", DELAY_SHORT);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  size_t group_key_info_bytes_len = stream.bytes_written;

  // verify signature of group key info
  if (!mpc_verify_signature(group_key_info_bytes, group_key_info_bytes_len, 
                            query->get_child_key.get_child_public_key.signature, 
                            pub_key)) {
    
    mpc_delay_scr_init("Error while verifiying signature.", DELAY_SHORT);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  free(group_key_info_bytes);

  uint8_t *group_pub_key = query->get_child_key.get_child_public_key.group_key_info.group_pub_key;

  // hash group public key
  // uint8_t *group_pub_key_hash = malloc(32 * sizeof(uint8_t));
  // hash_sha256(group_pub_key, 33, group_pub_key_hash);

  const char *mnemonic;

  // Convert entropy to mnemonic
  mnemonic = mnemonic_from_data(group_pub_key, ENTROPY_BYTES);

  // Convert mnemonic to BIP39 seed
  mnemonic_to_seed(mnemonic, "", seed, NULL);

  HDNode node;

  if (!derive_hdnode_from_path(
          query->get_child_key.get_child_public_key.derivation_path, 
          query->get_child_key.get_child_public_key.derivation_path_count, 
          SECP256K1_NAME, seed, &node)) {
    mpc_delay_scr_init("Error during deriving.", DELAY_SHORT);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    memzero(&node, sizeof(HDNode));
    return false;
  }

  memzero(seed, SEED_SIZE);

  bignum256 r;
  bn_read_be(node.private_key, &r);

  curve_point rG;
  scalar_multiply(curve, &r, &rG);
  curve_point group_public_key_point;

  ecdsa_read_pubkey(curve, group_pub_key, &group_public_key_point);
  point_add(curve, &rG, &group_public_key_point);

  uint8_t *child_key = malloc(33 * sizeof(uint8_t));
  child_key[0] = 0x02 | (group_public_key_point.y.val[0] & 0x01);
  bn_write_be(&group_public_key_point.x, child_key + 1);

  memcpy(response.get_child_public_key.public_key, child_key, 33);

  memzero(&node, sizeof(HDNode));
  result.get_child_key = response;

  mpc_send_result(&result);

  return true;
}

void get_child_key_flow(mpc_poc_query_t *query) {
  if (MPC_POC_GET_CHILD_KEY_REQUEST_INITIATE_TAG !=
      query->get_child_key.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    uint8_t *priv_key = malloc(32 * sizeof(uint8_t));
    uint8_t *pub_key = malloc(33 * sizeof(uint8_t));
    uint8_t *wallet_id = malloc(32 * sizeof(uint8_t));

    if (!get_child_key_initiate(query, wallet_id, priv_key, pub_key)) {
      return;
    }

    if (!get_child_key_process(query, pub_key)) {
      return;
    }

    free(priv_key);
    free(pub_key);
    free(wallet_id);
  }
}