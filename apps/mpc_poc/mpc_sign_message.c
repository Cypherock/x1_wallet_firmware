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
const uint8_t POLYNOMIALS_COUNT = ZERO_POLYNOMIALS + 3;

const uint8_t POLYNOMIAL_D_INDEX = 0;
const uint8_t POLYNOMIAL_E_INDEX = 1;

const uint8_t POLYNOMIAL_A_INDEX = ZERO_POLYNOMIALS + 0;
const uint8_t POLYNOMIAL_K_INDEX = ZERO_POLYNOMIALS + 1;
const uint8_t POLYNOMIAL_P_INDEX = ZERO_POLYNOMIALS + 2;

const uint8_t NUMBER_OF_OTS = 128;

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

bool sign_message_initiate(mpc_poc_query_t *query, uint8_t *priv_key, uint8_t *pub_key) {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_INITIATE_TAG;

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
        mpc_delay_scr_init("Error: wrong query (getGroupinfo)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_POST_GROUP_INFO_TAG;

    if (!query->sign_message.post_group_info.has_group_info ||
        !query->sign_message.post_group_info.has_group_key_info) {
        mpc_delay_scr_init("Error: request doesnt ahve groupinfo", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    mpc_poc_group_info_t group_info = query->sign_message.post_group_info.group_info;

    uint8_t *group_id = malloc(32 * sizeof(uint8_t));

    uint8_t group_info_bytes[GROUP_INFO_BUFFER_SIZE] = {0};
    size_t group_info_bytes_len = 0;

    pb_ostream_t stream = pb_ostream_from_buffer(group_info_bytes, GROUP_INFO_BUFFER_SIZE);

    if (!pb_encode(&stream, MPC_POC_GROUP_INFO_FIELDS, &group_info)) {
      return false;
    }

    group_info_bytes_len = stream.bytes_written;

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);

    hasher_Update(&hasher, group_info_bytes, group_info_bytes_len);
    hasher_Final(&hasher, group_id);

    if (!mpc_verify_signature(group_id, 32, query->sign_message.post_group_info.group_info_sig, pub_key)) {
        mpc_delay_scr_init("Error: goupinfo verification failed", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;

    }

    if (!mpc_verify_struct_sig(&query->sign_message.post_group_info.group_key_info, GROUP_KEY_INFO_BUFFER_SIZE, 
                               MPC_POC_GROUP_KEY_INFO_FIELDS, 
                               query->sign_message.post_group_info.group_key_info_sig, 
                               pub_key)) {

        mpc_delay_scr_init("Error: goupkey info verification failed", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    memcpy(group_info_arg, &query->sign_message.post_group_info.group_info, sizeof(mpc_poc_group_info_t));
    memcpy(group_key_info_arg, &query->sign_message.post_group_info.group_key_info, sizeof(mpc_poc_group_key_info_t));

    response.post_group_info.success = true;
    result.sign_message = response;
    mpc_send_result(&result);

    return true;
}

bool get_participant_indices(mpc_poc_query_t *query, uint8_t *pub_key,
                             mpc_poc_group_info_t *group_info, uint32_t *participant_indices,
                             uint32_t *my_index) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_POST_SEQUENCE_INDICES_TAG)) {

        mpc_delay_scr_init("Error: wrong query (sequenceIndices)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);
    
    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_POST_SEQUENCE_INDICES_TAG;

    if (query->sign_message.post_sequence_indices.sequence_indices_count != group_info->threshold) {
        mpc_delay_scr_init("Error: sequence indices len not equal to threshold", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }

    // convert pub_key to my_index
    if (!pub_key_to_index(group_info, pub_key, my_index)) {
        mpc_delay_scr_init("Error: error converting my pubkey to index", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    // check that my_index is in the sequence_indices
    bool found_my_index = false;
    for (uint8_t i = 0; i < group_info->threshold; i++) {
      if (query->sign_message.post_sequence_indices.sequence_indices[i] == *my_index) {
        found_my_index = true;
        break;
      }
    }

    if (!found_my_index) {
        mpc_delay_scr_init("Error: my index not in sequence indices", DELAY_TIME);
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
        mpc_delay_scr_init("Error: wrong query (getShareData)", DELAY_TIME);
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
        mpc_delay_scr_init("Error: dkg generate share data failed", DELAY_TIME);
          return false;
        }

        response.get_share_data.has_signed_share_data = true;
        response.get_share_data.signed_share_data = signed_share_data;
        response.get_share_data.has_more = (i < POLYNOMIALS_COUNT - 1);

        result.sign_message = response;
        mpc_send_result(&result);
    }

    return true;
}

bool get_QI(mpc_poc_query_t *query,
            mpc_poc_group_info_t *group_info,
            uint8_t *pub_key,
            uint8_t *priv_key,
            bignum256 *secret_share_list,
            uint8_t *public_keys[POLYNOMIALS_COUNT]) {
    
    for (uint8_t i = 0; i < POLYNOMIALS_COUNT; i++) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_GET_QI_TAG)) {
        mpc_delay_scr_init("Error: wrong query (getQI)", DELAY_TIME);
          return false;
      }
      
      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_GET_QI_TAG;

      uint8_t signature[64] = {0};
      uint32_t my_index = 0;

      if (!dkg_get_individual_public_key(group_info, group_info->threshold - 1, pub_key, priv_key, &secret_share_list[i], 
                                        query->sign_message.get_qi.share_data_list, 
                                        query->sign_message.get_qi.share_data_list_count, 
                                        public_keys[i], signature, &my_index)) {
        mpc_delay_scr_init("Error: dkg get qi failed", DELAY_TIME);
        return false;
      }

      response.get_qi.has_signed_pub_key = true;
      memcpy(response.get_qi.signed_pub_key.pub_key, public_keys[i], 33);
      memcpy(response.get_qi.signed_pub_key.signature, signature, 64);
      response.get_qi.signed_pub_key.index = my_index;

      result.sign_message = response;
      mpc_send_result(&result);
    }

    return true;
}

bool get_group_key(mpc_poc_query_t *query,
            mpc_poc_group_info_t *group_info,
            uint8_t *pub_key,
            uint8_t *priv_key,
            bignum256 *secret_share_list,
            uint8_t *public_keys[POLYNOMIALS_COUNT],
            mpc_poc_group_key_info_t group_key_info_list[POLYNOMIALS_COUNT]) {
  
  for (uint8_t i = 0; i < POLYNOMIALS_COUNT; i++) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_GET_GROUP_KEY_TAG)) {
        mpc_delay_scr_init("Error: wrong query (getgroukey)", DELAY_TIME);
        return false;
    }
    
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_GET_GROUP_KEY_TAG;

    uint8_t signature[64] = {0};

    if (!dkg_get_group_public_key(group_info, pub_key, priv_key, &secret_share_list[i], 
                                  query->group_setup.get_group_public_key.signed_pub_key_list, 
                                  query->group_setup.get_group_public_key.signed_pub_key_list_count, 
                                  public_keys[i], &group_key_info_list[i], signature)) {
        mpc_delay_scr_init("Error: dkg get group key failed", DELAY_TIME);
      return false;
    }

    response.get_group_key.has_group_key = true;
    response.get_group_key.group_key = group_key_info_list[i];
    memcpy(response.get_group_key.signature, signature, 64);

    result.sign_message = response;
    mpc_send_result(&result);
  }

  return true;
}

int compare_uint32(const void *a, const void *b) {
  return (*(uint32_t *)a - *(uint32_t *)b);
}

bool mta_send_rcv_pub_keys(mpc_poc_query_t *query,
                           uint32_t *participant_indices,
                           uint32_t receiver_times,
                           uint8_t *priv_key,
                           uint8_t *ot_receiver_sk_lists) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < receiver_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_PK_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv pk initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_PK_INITIATE_TAG;

    response.mta_rcv_get_pk_initiate.to = participant_indices[i];
    response.mta_rcv_get_pk_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);  

    uint8_t to_party = participant_indices[i];
    hasher_Update(&hasher, &to_party, 1);

    for (int j = 0; j < NUMBER_OF_OTS; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_PK_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta rcv pk)", DELAY_TIME);
          return false;
      }

      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_PK_TAG;

      random_generate(&ot_receiver_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], 32);

      bignum256 ot_receiver_sk;
      bn_read_be(&ot_receiver_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], &ot_receiver_sk);

      curve_point ot_receiver_pk;
      scalar_multiply(curve, &ot_receiver_sk, &ot_receiver_pk);

      response.mta_rcv_get_pk.public_key[0] = 0x02 | (ot_receiver_pk.y.val[0] & 0x01);
      bn_write_be(&ot_receiver_pk.x, response.mta_rcv_get_pk.public_key + 1);

      hasher_Update(&hasher, response.mta_rcv_get_pk.public_key, 33);

      result.sign_message = response;
      mpc_send_result(&result);
    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_PK_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv pk finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_PK_SIG_TAG;

    uint8_t hash[32] = {0};

    hasher_Final(&hasher, hash);

    if (!mpc_sign_message(hash, 32, response2.mta_rcv_get_pk_sig.signature, priv_key)) {
      mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool start_mta(mpc_poc_query_t *query, 
               const uint32_t my_index, 
              uint32_t *participant_indices, 
               const uint32_t threshold,
               bignum256 *secret_share_list,
               uint8_t *priv_key_share,
               uint8_t *priv_key) {
  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_START_MTA_TAG)) {
      mpc_delay_scr_init("Error: wrong query (startmta)", DELAY_TIME);
      return false;
  }

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_START_MTA_TAG;

  uint32_t sender_times = 0;
  uint32_t receiver_times = 0;

  // sort participant indices in place using qsort
  qsort(participant_indices, threshold, sizeof(uint32_t), compare_uint32);

  // find index of my_index in participant_indices
  uint32_t index = 0;
  for (uint32_t i = 0; i < threshold; i++) {
    if (participant_indices[i] == my_index) {
      index = i;
      break;
    }
  }

  receiver_times = index;
  sender_times = threshold - index - 1;

  uint8_t *s_values;
  uint8_t *ot_sender_sk_lists;
  uint8_t *q_dash_matrices;

  // a B value of size 128 bytes created by concatenating the last three shares of secret_share_list
  uint8_t *b_value;

  // an array called t_matrices of size receiver_times where each t_matrix contains 1024 t_values, each of size 16 bytes
  uint8_t *t_matrices;

  // an array called ot_receiver_sk_lists of size receiver_times where each ot_receiver_sk_list contains 128 ot_receiver_sks, each of size 32 bytes
  uint8_t *ot_receiver_sk_lists;

  if (sender_times > 0) {
    s_values = malloc(sender_times * 32 * sizeof(uint8_t));
    ot_sender_sk_lists = malloc(sender_times * NUMBER_OF_OTS * 32 * sizeof(uint8_t));
    q_dash_matrices = malloc(sender_times * NUMBER_OF_OTS * 128 * sizeof(uint8_t)); 
  }

  if (receiver_times > 0) {
    b_value = malloc(128 * sizeof(uint8_t));
    uint8_t *b_value_ptr = b_value;

    bn_write_be(&secret_share_list[POLYNOMIAL_K_INDEX], b_value_ptr);
    b_value_ptr += 32;
    bn_write_be(&secret_share_list[POLYNOMIAL_A_INDEX], b_value_ptr);
    b_value_ptr += 32;
    memcpy(b_value_ptr, priv_key_share, 32);
    b_value_ptr += 32;
    bn_write_be(&secret_share_list[POLYNOMIAL_P_INDEX], b_value_ptr);

    t_matrices = malloc(receiver_times * 1024 * (NUMBER_OF_OTS / 8) * sizeof(uint8_t));
    ot_receiver_sk_lists = malloc(receiver_times * NUMBER_OF_OTS * 32 * sizeof(uint8_t));
  }

  response.start_mta.sender_times = sender_times;
  response.start_mta.receiver_times = receiver_times;

  result.sign_message = response;
  mpc_send_result(&result);

  mpc_delay_scr_init("Everthing initialised", DELAY_TIME);

  if (!mta_send_rcv_pub_keys(query, participant_indices, receiver_times, 
                             priv_key, ot_receiver_sk_lists)) {
    return false;
  }

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

    mpc_poc_group_info_t group_info;
    mpc_poc_group_key_info_t group_key_info;

    bignum256 secret_share_list[POLYNOMIALS_COUNT];

    if (!sign_message_initiate(query, priv_key, pub_key)) {
      return;
    }

    mpc_delay_scr_init("App initiated.", DELAY_SHORT);

    if (!approve_message(query)) {
      return;
    }

    mpc_delay_scr_init("Getting Group info.", DELAY_SHORT);

    if (!get_group_info(query, pub_key, &group_info, &group_key_info)) {
      return;
    }

    uint32_t participant_indices[group_info.threshold];
    uint32_t my_index = 0;

    mpc_delay_scr_init("Getting indices.", DELAY_SHORT);

    if (!get_participant_indices(query, pub_key, &group_info, participant_indices, &my_index)) {
      return;
    }

    display_msg_on_screen("DKG process started.");
    if (!transfer_share_data(query, &group_info, pub_key, participant_indices, 
                             secret_share_list, priv_key)) {
      return;
    }

    uint8_t *public_keys[POLYNOMIALS_COUNT];

    for (uint8_t i = 0; i < POLYNOMIALS_COUNT; i++) {
      public_keys[i] = malloc(33 * sizeof(uint8_t));
    }

    if (!get_QI(query, &group_info, pub_key, priv_key, secret_share_list, public_keys)) {
      return;
    }

    mpc_poc_group_key_info_t group_key_info_list[POLYNOMIALS_COUNT];

    if (!get_group_key(query, &group_info, pub_key, priv_key, secret_share_list, 
                       public_keys, group_key_info_list)) {
      return;
    }

    mpc_delay_scr_init("DKG successfully finished.", DELAY_SHORT);

    // decrypt group_key_info's share

    uint8_t *dec_share = malloc(32 * sizeof(uint8_t));
    if (mpc_aes_decrypt(group_key_info.group_share.enc_share, 32, dec_share, priv_key) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    if (!start_mta(query, my_index, participant_indices, group_info.threshold, secret_share_list, dec_share, priv_key)) {
      return;
    }
  }
}