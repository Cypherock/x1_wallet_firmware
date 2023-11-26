#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "ui_core_confirm.h"
#include "reconstruct_wallet_flow.h"
#include "coin_specific_data.h"

static int init_application(const pb_byte_t* wallet_id, mpc_poc_init_application_response_t *response) {
    uint8_t seed[64] = {0};
    uint8_t priv_key[32] = {0};
    uint8_t pub_key[33] = {0};
    char msg[100] = "";
    char wallet_name[NAME_SIZE] = "";

    response->which_response = MPC_POC_INIT_APPLICATION_RESPONSE_RESULT_TAG;
    
    mpc_poc_init_application_result_response_t *result = &(response->result);

    if (!get_wallet_name_by_id(wallet_id, (uint8_t *)wallet_name, mpc_send_error)) {
      return 1;
    }

    snprintf(msg, sizeof(msg), "Start the MPC app using %s wallet?", wallet_name);

    if (!core_confirmation(msg, mpc_send_error)) {
        return 1;
    }

    if (!reconstruct_seed(wallet_id, &seed[0], mpc_send_error)) {
      memzero(seed, sizeof(seed));
      return 1;
    }

    priv_key_from_seed(seed, priv_key);
    pub_key33_from_priv_key(priv_key, pub_key);

    Coin_Specific_Data_Struct coin_specific_data;

    coin_specific_data.coin_type = MPC_APP;
    memcpy(coin_specific_data.wallet_id, wallet_id, WALLET_ID_SIZE);
    coin_specific_data.coin_data = priv_key;

    if (set_coin_data(&coin_specific_data, sizeof(priv_key)) != 0) {
      memzero(seed, sizeof(seed));
      memzero(priv_key, sizeof(priv_key));
      return 1;
    }

    memcpy(result->pub_key, pub_key, 33);
    
    memzero(seed, sizeof(seed));
    memzero(priv_key, sizeof(priv_key));
    memzero(pub_key, sizeof(pub_key));

    return 0;
}

static int exit_application(mpc_poc_exit_application_response_t *response) {
    response->which_response = MPC_POC_EXIT_APPLICATION_RESPONSE_RESULT_TAG;
    erase_flash_coin_specific_data();

    return 0;
}

static int get_public_key(const pb_byte_t* wallet_id, mpc_poc_get_public_key_response_t *response) {
    uint8_t pub_key[33] = {0};

    response->which_response = MPC_POC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG;
    
    mpc_poc_get_public_key_result_response_t *result = &(response->result);

    if (fetch_pub_key_from_flash(wallet_id, pub_key) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return 1;
    }

    memcpy(result->pub_key, pub_key, 33);
    
    memzero(pub_key, sizeof(pub_key));
    return 0;
}

void mpc_init(mpc_poc_query_t *query) {
  if (MPC_POC_INIT_APPLICATION_REQUEST_INITIATE_TAG !=
      query->init_application.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_INIT_APPLICATION_TAG);

    const pb_byte_t *wallet_id = query->init_application.initiate.wallet_id;

    mpc_poc_init_application_response_t response = MPC_POC_INIT_APPLICATION_RESPONSE_INIT_ZERO;
    result.init_application = response;

    if (init_application(wallet_id, &(result.init_application)) != 0) {
      return;
    }

    mpc_send_result(&result);
  }
}

void mpc_exit(mpc_poc_query_t *query) {
  if (MPC_POC_EXIT_APPLICATION_REQUEST_INITIATE_TAG !=
      query->exit_application.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_EXIT_APPLICATION_TAG);

    mpc_poc_exit_application_response_t response =
        MPC_POC_EXIT_APPLICATION_RESPONSE_INIT_ZERO;
    result.exit_application = response;

    if (exit_application(&(result.exit_application)) != 0) {
      return;
    }

    mpc_send_result(&result);
  }
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