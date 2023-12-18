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

static bool check_which_request(const mpc_poc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_message.which_request) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    char msg[100];
    sprintf(msg, "Check failed: %d", which_request);
    mpc_delay_scr_init(msg, DELAY_SHORT);
    return false;
  }

  return true;
}

bool sign_message_initiate(mpc_poc_query_t *query, uint8_t *wallet_id, uint8_t *priv_key, uint8_t *pub_key) {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_INITIATE_TAG;

    memcpy(wallet_id, query->sign_message.initiate.wallet_id, 32);

    if (!initiate_application(query->sign_message.initiate.wallet_id, priv_key, pub_key)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    memcpy(response.initiate.pub_key, pub_key, 33);

    result.sign_message = response;
    mpc_send_result(&result);

    return true;
}

bool approve_message(mpc_poc_query_t *query) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_APPROVE_MESSAGE_TAG)) {
        return false;
    }

    char *msg = malloc(300 * sizeof(char));

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_APPROVE_MESSAGE_TAG;

    byte_array_to_hex_string(query->sign_message.approve_message.group_id, 32, msg, 300);

    if (!mpc_core_scroll_page("Match the GroupID", msg, mpc_send_error)) {
      free(msg);
      return false;
    }

    byte_array_to_hex_string(query->sign_message.approve_message.msg.bytes, 
                             query->sign_message.approve_message.msg.size, msg, 300);

    if (!mpc_core_scroll_page("Match the message to sign", msg, mpc_send_error)) {
      free(msg);
      return false;
    }

    free(msg);
    
    response.approve_message.approved = true;
    result.sign_message = response;
    mpc_send_result(&result);
    
    return true;
}

bool get_group_info(mpc_poc_query_t *query, uint8_t *pub_key, 
                    mpc_poc_group_info_t *group_info_arg,
                    mpc_poc_group_key_info_t *group_key_info_arg) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_POST_GROUP_INFO_TAG)) {
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_POST_GROUP_INFO_TAG;

    if (!query->sign_message.post_group_info.has_group_info ||
        !query->sign_message.post_group_info.has_group_key_info) {
          
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    mpc_poc_group_info_t group_info = query->sign_message.post_group_info.group_info;
    mpc_poc_group_key_info_t group_key_info = query->sign_message.post_group_info.group_key_info;

    if (!mpc_verify_struct_sig(&group_info, GROUP_INFO_BUFFER_SIZE, 
                               MPC_POC_GROUP_INFO_FIELDS, 
                               query->sign_message.post_group_info.group_info_sig, 
                               pub_key)) {

        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    if (!mpc_verify_struct_sig(&group_key_info, GROUP_KEY_INFO_BUFFER_SIZE, 
                               MPC_POC_GROUP_KEY_INFO_FIELDS, 
                               query->sign_message.post_group_info.group_key_info_sig, 
                               pub_key)) {

        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    memcpy(group_info_arg, &group_info, sizeof(mpc_poc_group_info_t));
    memcpy(group_key_info_arg, &group_key_info, sizeof(mpc_poc_group_key_info_t));

    response.post_group_info.success = true;
    result.sign_message = response;
    mpc_send_result(&result);

    return true;
}

void sign_message_flow(mpc_poc_query_t *query) {
  if (MPC_POC_SIGN_MESSAGE_REQUEST_INITIATE_TAG !=
      query->sign_message.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    uint8_t *priv_key = malloc(32 * sizeof(uint8_t));
    uint8_t *pub_key = malloc(33 * sizeof(uint8_t));
    uint8_t *wallet_id = malloc(32 * sizeof(uint8_t));

    mpc_poc_group_info_t group_info;
    mpc_poc_group_key_info_t group_key_info;

    if (!sign_message_initiate(query, wallet_id, priv_key, pub_key)) {
      return;
    }

    mpc_delay_scr_init("App initiated.", DELAY_SHORT);

    if (!approve_message(query)) {
      return;
    }

    if (!get_group_info(query, pub_key, &group_info, &group_key_info)) {
      return;
    }
  }
}