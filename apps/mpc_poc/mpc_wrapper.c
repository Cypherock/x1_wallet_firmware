#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "ui_core_confirm.h"
#include "reconstruct_wallet_flow.h"
#include "coin_specific_data.h"

static mpc_poc_init_application_response_t init_application(const pb_byte_t* wallet_id) {
    uint8_t seed[64] = {0};
    uint8_t priv_key[32] = {0};
    uint8_t pub_key[33] = {0};
    char msg[100] = "";
    char wallet_name[NAME_SIZE] = "";

    mpc_poc_init_application_response_t response =
        MPC_POC_INIT_APPLICATION_RESPONSE_INIT_ZERO;
    
    response.which_response = MPC_POC_INIT_APPLICATION_RESPONSE_RESULT_TAG;
    
    if (response.which_response == MPC_POC_INIT_APPLICATION_RESPONSE_RESULT_TAG) {
        mpc_poc_init_application_result_response_t *result = &response.result;
        result->initiated = false;

        if (!get_wallet_name_by_id(wallet_id, (uint8_t *)wallet_name, mpc_send_error)) {
          return response;
        }

        snprintf(msg, sizeof(msg), "Do you want to start the MPC TSS Application using %s wallet?", wallet_name);

        if (!core_confirmation(msg, mpc_send_error)) {
            return response;
        }

        if (!reconstruct_seed(wallet_id, &seed[0], mpc_send_error)) {
          memzero(seed, sizeof(seed));
          return response;
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
          return response;
        }

        result->initiated = true;
        memcpy(result->pub_key, pub_key, 33);
    }
    
    memzero(seed, sizeof(seed));
    memzero(priv_key, sizeof(priv_key));
    memzero(pub_key, sizeof(pub_key));

    return response;
}

static mpc_poc_exit_application_response_t exit_application() {
    mpc_poc_exit_application_response_t response =
        MPC_POC_EXIT_APPLICATION_RESPONSE_INIT_ZERO;
    
    response.which_response = MPC_POC_EXIT_APPLICATION_RESPONSE_RESULT_TAG;
    
    if (response.which_response == MPC_POC_EXIT_APPLICATION_RESPONSE_RESULT_TAG) {
        mpc_poc_exit_application_result_response_t *result = &response.result;

        erase_flash_coin_specific_data();
    
        result->exited = true;
    }
    
    return response;
}

static mpc_poc_get_public_key_response_t get_public_key(const pb_byte_t* wallet_id) {
    uint8_t priv_key[32] = {0};
    uint8_t pub_key[33] = {0};
    uint16_t len = 0;

    mpc_poc_get_public_key_response_t response =
        MPC_POC_GET_PUBLIC_KEY_RESPONSE_INIT_ZERO;
    
    response.which_response = MPC_POC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG;
    
    if (response.which_response == MPC_POC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG) {
        mpc_poc_get_public_key_result_response_t *result = &response.result;

        Coin_Specific_Data_Struct coin_specific_data;

        coin_specific_data.coin_type = MPC_APP;
        memcpy(coin_specific_data.wallet_id, wallet_id, WALLET_ID_SIZE);
        coin_specific_data.coin_data = priv_key;

        if (get_coin_data(&coin_specific_data, sizeof(priv_key), &len) != 0 || len != sizeof(priv_key)) {
          memzero(priv_key, sizeof(priv_key));
          mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                            ERROR_DATA_FLOW_INVALID_REQUEST);
          return;
        }

        pub_key33_from_priv_key(priv_key, pub_key);
        memcpy(result->pub_key, pub_key, 33);
    }
    
    memzero(priv_key, sizeof(priv_key));
    memzero(pub_key, sizeof(pub_key));

    return response;
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

    result.init_application = init_application(wallet_id);
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
    result.exit_application = exit_application();
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

    result.get_public_key = get_public_key(wallet_id);
    mpc_send_result(&result);
  }
}