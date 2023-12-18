#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"
#include "mpc_dkg.h"

#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "pb_encode.h"
#include "hasher.h"

#include "utils.h"
#include "controller_level_four.h"
#include "string.h"
#include "bignum.h"

const uint8_t ZERO_POLYNOMIALS = 2;
const uint8_t POLYNOMIALS_COUNT = ZERO_POLYNOMIALS + 2;


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

bool get_participant_indices(mpc_poc_query_t *query, uint8_t *pub_key,
                             mpc_poc_group_info_t *group_info, uint32_t *participant_indices) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_POST_SEQUENCE_INDICES_TAG)) {
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_POST_SEQUENCE_INDICES_TAG;

    if (query->sign_message.post_sequence_indices.sequence_indices_count != group_info->threshold) {
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    // convert pub_key to my_index
    uint32_t my_index = 0;
    if (!pub_key_to_index(group_info, pub_key, &my_index)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    // check that my_index is in the sequence_indices
    bool found_my_index = false;
    for (uint8_t i = 0; i < group_info->threshold; i++) {
      if (query->sign_message.post_sequence_indices.sequence_indices[i] == my_index) {
        found_my_index = true;
        break;
      }
    }

    if (!found_my_index) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    memcpy(participant_indices, query->sign_message.post_sequence_indices.sequence_indices, 
           group_info->threshold * sizeof(uint32_t));
    
    response.post_sequence_indices.success = true;
    result.sign_message = response;

    mpc_send_result(&result);

    return true;
}

bool transfer_share_data(mpc_poc_query_t *query,
                         mpc_poc_group_info_t *group_info,
                         uint8_t *pub_key,
                         uint32_t *participant_indices,
                         bignum256 *secret_share_list,
                         uint8_t *priv_key) {

    for (uint8_t i = 0; i < POLYNOMIALS_COUNT; i++) {
        if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
            !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_GET_SHARE_DATA_TAG)) {
            return false;
        }

        bignum256 *secret_share = &secret_share_list[i];

        mpc_poc_result_t result =
            init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
        mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
        response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_GET_SHARE_DATA_TAG;

        mpc_poc_signed_share_data_t signed_share_data = MPC_POC_SIGNED_SHARE_DATA_INIT_ZERO;

        if (!dkg_generate_signed_share_data(group_info, (i < ZERO_POLYNOMIALS), participant_indices, 
                                            group_info->threshold, pub_key, 
                                            secret_share, priv_key, &signed_share_data)) {
          return false;
        }

        response.get_share_data.has_signed_share_data = true;
        response.get_share_data.signed_share_data = signed_share_data;
        response.get_share_data.has_more = (i < POLYNOMIALS_COUNT - 1);

        result.sign_message = response;
        mpc_send_result(&result);
    }
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

    bignum256 secret_share_list[POLYNOMIALS_COUNT];

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

    uint32_t participant_indices[group_info.threshold];

    if (!get_participant_indices(query, pub_key, &group_info, participant_indices)) {
      return;
    }

    if (!transfer_share_data(query, &group_info, pub_key, participant_indices, 
                             secret_share_list, priv_key)) {
      return;
    }
  }
}