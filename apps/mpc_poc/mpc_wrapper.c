#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "ui_core_confirm.h"
#include "reconstruct_wallet_flow.h"
#include "coin_specific_data.h"

static int get_public_key(const pb_byte_t* wallet_id, mpc_poc_get_public_key_response_t *response) {
    uint8_t priv_key[32] = {0};
    uint8_t pub_key[33] = {0};

    response->which_response = MPC_POC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG;
    
    mpc_poc_get_public_key_result_response_t *result = &(response->result);

    if (!construct_mpc_key(wallet_id, &priv_key[0])) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return 1;
    }

    pub_key33_from_priv_key(priv_key, pub_key);

    memcpy(result->pub_key, pub_key, 33);

    memzero(priv_key, sizeof(priv_key));
    // memzero(pub_key, sizeof(pub_key));

    return 0;
}

void get_public_key_flow(mpc_poc_query_t *query) {
  if (MPC_POC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG !=
      query->get_public_key.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_GET_PUBLIC_KEY_TAG);

    const pb_byte_t *wallet_id = query->get_public_key.initiate.wallet_id;

    mpc_poc_get_public_key_response_t response = MPC_POC_GET_PUBLIC_KEY_RESPONSE_INIT_ZERO;
    result.get_public_key = response;

    if (get_public_key(wallet_id, &(result.get_public_key)) != 0) {
      return;
    }

    mpc_send_result(&result);
  }
}