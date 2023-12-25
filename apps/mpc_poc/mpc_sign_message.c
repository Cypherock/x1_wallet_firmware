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

const uint8_t AUTHENTICATOR_DATA_BUFFER_SIZE = 70;

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

bool approve_message(mpc_poc_query_t *query, uint8_t *msg_hash) {
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

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2D);
    hasher_Update(&hasher, query->sign_message.approve_message.msg.bytes, 
                  query->sign_message.approve_message.msg.size);
    
    hasher_Final(&hasher, msg_hash);

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
    
    response.post_sequence_indices.my_index = *my_index;
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

void compute_lambda(bignum256 *lambda, uint32_t *participant_indices, size_t participants_len, uint32_t index) {
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;
  bn_one(lambda);

  for (int i = 0; i < participants_len; ++i) {
    if (i != index) {
      bignum256 bn_i;
      bn_read_uint32(participant_indices[i], &bn_i);

      bignum256 bn_j;
      bn_read_uint32(participant_indices[index], &bn_j);

      bignum256 bn_i_minus_j;

      bn_subtractmod(&bn_i, &bn_j, &bn_i_minus_j, &curve->order);
      bn_inverse(&bn_i_minus_j, &curve->order);
      bn_multiply(&bn_i, &bn_i_minus_j, &curve->order);

      bn_multiply(&bn_i_minus_j, lambda, &curve->order);
    }
  }
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

    if (mpc_sign_message(hash, 32, response2.mta_rcv_get_pk_sig.signature, priv_key) != 0) {
      mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool mta_send_snd_pub_keys(mpc_poc_query_t *query,
                           mpc_poc_group_info_t *group_info,
                           uint32_t index,
                           uint32_t *participant_indices,
                           uint32_t sender_times,
                           uint8_t *priv_key,
                           uint8_t *s_values,
                           uint8_t *ot_sender_sk_lists,
                           curve_point *ot_sender_received_pks) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < sender_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_PK_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd pk initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_PK_INITIATE_TAG;

    response.mta_snd_get_pk_initiate.to = participant_indices[index + 1 + i];
    response.mta_snd_get_pk_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher_verify;
    hasher_Init(&hasher_verify, HASHER_SHA2);

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);  

    uint8_t to_party = participant_indices[index + 1 + i];
    uint8_t my_index = participant_indices[index];

    hasher_Update(&hasher, &to_party, 1);
    hasher_Update(&hasher_verify, &my_index, 1); // my_index

    random_generate(&s_values[i * 16], 16);

    for (int j = 0; j < NUMBER_OF_OTS; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_PK_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta snd pk)", DELAY_TIME);
          return false;
      }

      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_PK_TAG;

      random_generate(&ot_sender_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], 32);

      bignum256 ot_sender_sk;
      bn_read_be(&ot_sender_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], &ot_sender_sk);

      curve_point ot_sender_pk;
      scalar_multiply(curve, &ot_sender_sk, &ot_sender_pk);

      curve_point ot_receiver_pk;
      ecdsa_read_pubkey(curve, query->sign_message.mta_snd_get_pk.public_key, &ot_receiver_pk);
      memcpy(&ot_sender_received_pks[i * NUMBER_OF_OTS + j], &ot_receiver_pk, sizeof(curve_point));

      hasher_Update(&hasher_verify, query->sign_message.mta_snd_get_pk.public_key, 33);

      uint8_t mask = 1 << (j % 8);
      if (s_values[(i * 16) + (j / 8)] & mask) {
        point_add(curve, &ot_receiver_pk, &ot_sender_pk);
      }

      response.mta_snd_get_pk.public_key[0] = 0x02 | ( ot_sender_pk.y.val[0] & 0x01);
      bn_write_be(&ot_sender_pk.x, response.mta_snd_get_pk.public_key + 1);

      hasher_Update(&hasher, response.mta_snd_get_pk.public_key, 33);

      result.sign_message = response;
      mpc_send_result(&result);
    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_PK_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd pk finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_PK_SIG_TAG;

    uint8_t hash[32] = {0};
    uint8_t hash_verify[32] = {0};

    hasher_Final(&hasher, hash);
    hasher_Final(&hasher_verify, hash_verify);

    uint8_t *verify_pub_key = malloc(33 * sizeof(uint8_t));
    if (!index_to_pub_key(group_info, participant_indices[index + 1 + i], verify_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      return false;
    }

    if (!mpc_verify_signature(hash_verify, 32, query->sign_message.mta_snd_get_pk_sig.signature, verify_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      return false;
    }

    if (mpc_sign_message(hash, 32, response2.mta_snd_get_pk_sig.signature, priv_key) != 0) {
      mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

void store_value_in_matrix(uint8_t *t_value, uint8_t *t_matrix, int index) {
  // t_matrix is a 1024 * (NUMBER_OF_OTS / 8) matrix
  // store t_value in the index column of the matrix

  for (int i = 0; i < 1024; ++i) {
    uint8_t mask = 1 << (i % 8);
    if (t_value[i / 8] & mask) {
      t_matrix[i * (NUMBER_OF_OTS / 8) + (index / 8)] |= (1 << (index % 8));
    } else {
      t_matrix[i * (NUMBER_OF_OTS / 8) + (index / 8)] &= ~(1 << (index % 8));
    }
  }
}

bool mta_send_rcv_enc(mpc_poc_query_t *query,
                      mpc_poc_group_info_t *group_info,
                      uint32_t index,
                      uint32_t *participant_indices,
                      uint32_t receiver_times,
                      uint8_t *priv_key,
                      uint8_t *ot_receiver_sk_lists,
                      uint8_t *b_value,
                      uint8_t *t_matrices) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < receiver_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_ENC_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv enc initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_ENC_INITIATE_TAG;

    response.mta_rcv_get_enc_initiate.to = participant_indices[i];
    response.mta_rcv_get_enc_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher;
    Hasher hasher_verify;

    hasher_Init(&hasher_verify, HASHER_SHA2);
    hasher_Init(&hasher, HASHER_SHA2);  

    uint8_t to_party = participant_indices[i];
    uint8_t my_index = participant_indices[index];

    hasher_Update(&hasher, &to_party, 1);
    hasher_Update(&hasher_verify, &my_index, 1); // my_index

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_ENC_TAG;

    for (int j = 0; j < NUMBER_OF_OTS; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_ENC_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta rcv enc)", DELAY_TIME);
          return false;
      }

      uint8_t *t_value = malloc(128 * sizeof(uint8_t));

      random_generate(t_value, 32);
      random_generate(t_value + 32, 32);
      random_generate(t_value + 64, 32);
      random_generate(t_value + 96, 32);

      store_value_in_matrix(t_value, &t_matrices[i * 1024 * (NUMBER_OF_OTS / 8)], j);

      bignum256 *ot_receiver_sk = malloc(sizeof(bignum256));
      bn_read_be(&ot_receiver_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], ot_receiver_sk);

      curve_point *ot_receiver_pk = malloc(sizeof(curve_point));
      scalar_multiply(curve, ot_receiver_sk, ot_receiver_pk);

      curve_point *ot_sender_pk = malloc(sizeof(curve_point));
      ecdsa_read_pubkey(curve, query->sign_message.mta_rcv_get_enc.public_key, ot_sender_pk);

      hasher_Update(&hasher_verify, query->sign_message.mta_rcv_get_enc.public_key, 33);

      curve_point *k0 = malloc(sizeof(curve_point));
      point_multiply(curve, ot_receiver_sk, ot_sender_pk, k0);

      curve_point *ot_receiver_pk_neg = malloc(sizeof(curve_point));
      bn_copy(&(ot_receiver_pk->x), &(ot_receiver_pk_neg->x));
      bn_subtractmod(&(curve->prime), &(ot_receiver_pk->y), &(ot_receiver_pk_neg->y), &(curve->prime));

      free(ot_receiver_pk);
      point_add(curve, ot_sender_pk, ot_receiver_pk_neg);
      free(ot_sender_pk);

      curve_point *k1 = malloc(sizeof(curve_point));
      point_multiply(curve, ot_receiver_sk, ot_receiver_pk_neg, k1);

      free(ot_receiver_sk);
      free(ot_receiver_pk_neg);

      uint8_t *key_hash = malloc(32 * sizeof(uint8_t));
      uint8_t *key_coordinate = malloc(32 * sizeof(uint8_t));

      Hasher hasher_key0;
      hasher_Init(&hasher_key0, HASHER_SHA2);

      bn_write_be(&(k0->x), key_coordinate);

      hasher_Update(&hasher_key0, key_coordinate, 32);
      bn_write_be(&(k0->y), key_coordinate);

      free(k0);

      hasher_Update(&hasher_key0, key_coordinate, 32);
      hasher_Final(&hasher_key0, key_hash);

      if (mpc_aes_encrypt(t_value, 128, response.mta_rcv_get_enc.enc_m0, key_hash) != 0) {

        mpc_delay_scr_init("Error: aes encrypt failed", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
      }

      Hasher hasher_key1;
      hasher_Init(&hasher_key1, HASHER_SHA2);

      bn_write_be(&(k1->x), key_coordinate);

      hasher_Update(&hasher_key1, key_coordinate, 32);
      bn_write_be(&(k1->y), key_coordinate);

      free(k1);

      hasher_Update(&hasher_key1, key_coordinate, 32);
      hasher_Final(&hasher_key1, key_hash);

      free(key_coordinate);

      // xor t_value with b_value
      for (int k = 0; k < 128; ++k) {
        t_value[k] ^= b_value[k];
      }

      if (mpc_aes_encrypt(t_value, 128, response.mta_rcv_get_enc.enc_m1, key_hash) != 0) {

        mpc_delay_scr_init("Error: aes encrypt failed", DELAY_TIME);
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
      }

      free(t_value);
      free(key_hash);

      hasher_Update(&hasher, response.mta_rcv_get_enc.enc_m0, 128);
      hasher_Update(&hasher, response.mta_rcv_get_enc.enc_m1, 128);

      result.sign_message = response;
      mpc_send_result(&result);
    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_GET_ENC_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv enc finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_GET_ENC_SIG_TAG;

    uint8_t hash[32] = {0};
    uint8_t hash_verify[32] = {0};

    hasher_Final(&hasher, hash);
    hasher_Final(&hasher_verify, hash_verify);

    uint8_t *verify_pub_key = malloc(33 * sizeof(uint8_t));
    if (!index_to_pub_key(group_info, participant_indices[i], verify_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      return false;
    }

    if (!mpc_verify_signature(hash_verify, 32, query->sign_message.mta_rcv_get_enc_sig.signature, verify_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      return false;
    }

    free(verify_pub_key);

    if (mpc_sign_message(hash, 32, response2.mta_rcv_get_enc_sig.signature, priv_key) != 0) {
      mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool mta_post_snd_enc(mpc_poc_query_t *query,
                      mpc_poc_group_info_t *group_info,
                      uint32_t index,
                      uint32_t *participant_indices,
                      uint32_t sender_times,
                      uint8_t *s_values,
                      uint8_t *ot_sender_sk_lists,
                      curve_point *ot_sender_received_pks,
                      uint8_t *q_matrices) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < sender_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_POST_ENC_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd post enc initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_POST_ENC_INITIATE_TAG;

    response.mta_snd_post_enc_initiate.to = participant_indices[index + 1 + i];
    response.mta_snd_post_enc_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher_verify;
    hasher_Init(&hasher_verify, HASHER_SHA2);

    uint8_t my_index = participant_indices[index];
    hasher_Update(&hasher_verify, &my_index, 1); // my_index

    for (int j = 0; j < NUMBER_OF_OTS; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_POST_ENC_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta snd post enc)", DELAY_TIME);
          return false;
      }

      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_POST_ENC_TAG;

      uint8_t *dec_m = malloc(128 * sizeof(uint8_t));
      uint8_t *dec_key = malloc(32 * sizeof(uint8_t));

      bignum256 ot_sender_sk;
      bn_read_be(&ot_sender_sk_lists[i * NUMBER_OF_OTS * 32 + j * 32], &ot_sender_sk);

      curve_point kc;
      point_multiply(curve, &ot_sender_sk, &ot_sender_received_pks[i * NUMBER_OF_OTS + j], &kc);

      Hasher hasher_key;
      hasher_Init(&hasher_key, HASHER_SHA2);

      uint8_t *key_coordinate = malloc(32 * sizeof(uint8_t));

      bn_write_be(&(kc.x), key_coordinate);

      hasher_Update(&hasher_key, key_coordinate, 32);
      bn_write_be(&(kc.y), key_coordinate);

      hasher_Update(&hasher_key, key_coordinate, 32);

      hasher_Final(&hasher_key, dec_key);


      uint8_t mask = 1 << (j % 8);
      if (s_values[(i * 16) + (j / 8)] & mask) {
        if (mpc_aes_decrypt(query->sign_message.mta_snd_post_enc.enc_m0, 128, dec_m, dec_key) != 0) {
          mpc_delay_scr_init("Error: aes decrypt failed", DELAY_TIME);
          return false;
        }
      } else {
        if (mpc_aes_decrypt(query->sign_message.mta_snd_post_enc.enc_m1, 128, dec_m, dec_key) != 0) {
          mpc_delay_scr_init("Error: aes decrypt failed", DELAY_TIME);
          return false;
        }  
      }

      store_value_in_matrix(dec_m, &q_matrices[i * 1024 * (NUMBER_OF_OTS / 8)], j);

      hasher_Update(&hasher_verify, query->sign_message.mta_snd_post_enc.enc_m0, 128);
      hasher_Update(&hasher_verify, query->sign_message.mta_snd_post_enc.enc_m1, 128);

      result.sign_message = response;
      mpc_send_result(&result);

    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_POST_ENC_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd post enc finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_POST_ENC_SIG_TAG;

    uint8_t hash_verify[32] = {0};
    hasher_Final(&hasher_verify, hash_verify);

    uint8_t *verify_pub_key = malloc(33 * sizeof(uint8_t));

    if (!index_to_pub_key(group_info, participant_indices[index + 1 + i], verify_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      return false;
    }

    if (!mpc_verify_signature(hash_verify, 32, query->sign_message.mta_snd_post_enc_sig.signature, verify_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool mta_mascot_snd(mpc_poc_query_t *query,
                    uint32_t index,
                    uint32_t *participant_indices,
                    uint32_t sender_times,
                    uint32_t threshold,
                    uint8_t *s_values,
                    uint8_t *q_matrices,
                    uint8_t *priv_key,
                    bignum256 *secret_share_list,
                    uint8_t *priv_key_share,
                    bignum256 *sender_additive_shares_list) {
  
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < sender_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_MASCOT_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd mascot initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_MASCOT_INITIATE_TAG;

    response.mta_snd_get_mascot_initiate.to = participant_indices[index + 1 + i];
    response.mta_snd_get_mascot_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);

    uint8_t to_party = participant_indices[index + 1 + i];
    hasher_Update(&hasher, &to_party, 1);

    for (int j = 0; j < 1024; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_MASCOT_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta snd mascot)", DELAY_TIME);
          return false;
      }

      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_MASCOT_TAG;

      uint8_t *q_value = malloc(16 * sizeof(uint8_t));
      memcpy(q_value, &q_matrices[i * 1024 * (NUMBER_OF_OTS / 8) + j * (NUMBER_OF_OTS / 8)], (NUMBER_OF_OTS / 8));

      uint8_t *rand_value = malloc(32 * sizeof(uint8_t));
      random_generate(rand_value, 32);

      bignum256 rand_bn;
      bn_read_be(rand_value, &rand_bn);

      uint8_t polynomial = j / 256;
      uint8_t round = j % 256;

      bignum256 share_bn;

      bignum256 two;
      bn_read_uint32(2, &two);

      bignum256 bn_i;
      bn_read_uint32((uint32_t) round, &bn_i);

      bignum256 res_x;
      bn_zero(&res_x);

      bn_power_mod(&two, &bn_i, &curve->order, &res_x);
      bn_multiply(&rand_bn, &res_x, &curve->order);

      bn_addmod(&sender_additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + polynomial], &res_x, &curve->order);

      if (polynomial < 3) {
        memcpy(&share_bn, &secret_share_list[ZERO_POLYNOMIALS + polynomial], sizeof(bignum256));
      } else {
        bn_read_be(priv_key_share, &share_bn);
      }

      if (mpc_aes_encrypt128(rand_value, 32, response.mta_snd_get_mascot.enc_m0, q_value) != 0) {
        mpc_delay_scr_init("Error: aes encrypt failed", DELAY_TIME);
        return false;
      }

      bn_addmod(&rand_bn, &share_bn, &curve->order);
      bn_write_be(&rand_bn, rand_value);

      // xor q_value with s_value
      for (int k = 0; k < 16; ++k) {
        q_value[k] ^= s_values[sender_times * 16 + k];
      }

      if (mpc_aes_encrypt128(rand_value, 32, response.mta_snd_get_mascot.enc_m1, q_value) != 0) {
        mpc_delay_scr_init("Error: aes encrypt failed", DELAY_TIME);
        return false;
      }

      hasher_Update(&hasher, response.mta_snd_get_mascot.enc_m0, 32);
      hasher_Update(&hasher, response.mta_snd_get_mascot.enc_m1, 32);

      result.sign_message = response;
      mpc_send_result(&result);
    }

    bignum256 lambda;
    compute_lambda(&lambda, participant_indices, threshold, index + 1 + i);

    for (int k = 0; k < 4; ++k) {
      bn_multiply(&lambda, &sender_additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + k], &curve->order);
    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_SND_GET_MASCOT_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta snd mascot finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_SND_GET_MASCOT_SIG_TAG;

    uint8_t hash[32] = {0};
    hasher_Final(&hasher, hash);

    if (mpc_sign_message(hash, 32, response2.mta_snd_get_mascot_sig.signature, priv_key) != 0) {
      mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool mta_mascot_rcv(mpc_poc_query_t *query,
                    mpc_poc_group_info_t *group_info,
                    uint32_t index,
                    uint32_t *participant_indices,
                    uint32_t receiver_times,
                    uint8_t *t_matrices,
                    bignum256 *receiver_additive_shares_list,
                    uint8_t *b_value) {
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  for (int i = 0; i < receiver_times; ++i) {
    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_POST_MASCOT_INITIATE_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv mascot initiate)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_POST_MASCOT_INITIATE_TAG;

    response.mta_rcv_post_mascot_initiate.to = participant_indices[i];
    response.mta_rcv_post_mascot_initiate.length = NUMBER_OF_OTS;

    result.sign_message = response;
    mpc_send_result(&result);

    Hasher hasher_verify;
    hasher_Init(&hasher_verify, HASHER_SHA2);

    uint8_t to_party = participant_indices[i];
    hasher_Update(&hasher_verify, &to_party, 1);

    for (int j = 0; j < 1024; ++j) {
      if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
          !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_POST_MASCOT_TAG)) {
          mpc_delay_scr_init("Error: wrong query (mta rcv mascot)", DELAY_TIME);
          return false;
      }

      mpc_poc_result_t result =
          init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

      mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
      response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_POST_MASCOT_TAG;

      hasher_Update(&hasher_verify, query->sign_message.mta_rcv_post_mascot.enc_m0, 32);
      hasher_Update(&hasher_verify, query->sign_message.mta_rcv_post_mascot.enc_m1, 32);

      // get j'th bit of b_value (128 bytes array)
      uint8_t mask = 1 << (j % 8);
      uint8_t b_bit = b_value[j / 8] & mask;
      
      uint8_t *dec_m = malloc(32 * sizeof(uint8_t));

      if (b_bit) {
        if (mpc_aes_decrypt128(query->sign_message.mta_rcv_post_mascot.enc_m0, 32, 
                                dec_m, &t_matrices[i * 1024 * (NUMBER_OF_OTS / 8) + j * (NUMBER_OF_OTS / 8)]) != 0) {
          mpc_delay_scr_init("Error: aes decrypt failed", DELAY_TIME);
          return false;
        }
      } else {
        if (mpc_aes_decrypt128(query->sign_message.mta_rcv_post_mascot.enc_m1, 32, 
                                dec_m, &t_matrices[i * 1024 * (NUMBER_OF_OTS / 8) + j * (NUMBER_OF_OTS / 8)]) != 0) {
          mpc_delay_scr_init("Error: aes decrypt failed", DELAY_TIME);
          return false;
        }
      }

      bignum256 bn_dec_m;
      bn_read_be(dec_m, &bn_dec_m);

      uint8_t polynomial = j / 256;
      uint8_t round = j % 256;

      bignum256 two;
      bn_read_uint32(2, &two);

      bignum256 bn_i;
      bn_read_uint32((uint32_t) round, &bn_i);

      bignum256 res_x;
      bn_zero(&res_x);

      bn_power_mod(&two, &bn_i, &curve->order, &res_x);
      bn_multiply(&bn_dec_m, &res_x, &curve->order);

      bn_addmod(&receiver_additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + polynomial], &res_x, &curve->order);

      result.sign_message = response;
      mpc_send_result(&result);
    }

    bignum256 lambda;
    compute_lambda(&lambda, participant_indices, group_info->threshold, i);

    for (int k = 0; k < 4; ++k) {
      bn_multiply(&lambda, &receiver_additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + k], &curve->order);
    }

    if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
        !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_MTA_RCV_POST_MASCOT_SIG_TAG)) {
        mpc_delay_scr_init("Error: wrong query (mta rcv mascot finish)", DELAY_TIME);
        return false;
    }

    mpc_poc_result_t result2 =
        init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

    mpc_poc_sign_message_response_t response2 = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
    response2.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_MTA_RCV_POST_MASCOT_SIG_TAG;

    uint8_t hash[32] = {0};
    hasher_Final(&hasher_verify, hash);

    // verify the signature
    uint8_t *verify_pub_key = malloc(33 * sizeof(uint8_t));

    if (!index_to_pub_key(group_info, participant_indices[i], verify_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      return false;
    }

    if (!mpc_verify_signature(hash, 32, query->sign_message.mta_rcv_post_mascot_sig.signature, verify_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      return false;
    }

    result2.sign_message = response2;
    mpc_send_result(&result2);
  }

  return true;
}

bool start_mta(mpc_poc_query_t *query, 
               mpc_poc_group_info_t *group_info,
               const uint32_t my_index, 
               uint32_t *participant_indices, 
               const uint32_t threshold,
               bignum256 *secret_share_list,
               uint8_t *priv_key_share,
               uint8_t *priv_key,
               bignum256 *additive_shares_list) {

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
  uint8_t *q_matrices;

  curve_point *ot_sender_received_pks;
  bignum256 *sender_additive_shares_list;

  // a B value of size 128 bytes created by concatenating the last three shares of secret_share_list
  uint8_t *b_value;

  // an array called t_matrices of size receiver_times where each t_matrix contains 1024 t_values, each of size 16 bytes
  uint8_t *t_matrices;

  // an array called ot_receiver_sk_lists of size receiver_times where each ot_receiver_sk_list contains 128 ot_receiver_sks, each of size 32 bytes
  uint8_t *ot_receiver_sk_lists;
  bignum256 *receiver_additive_shares_list;

  if (sender_times > 0) {
    s_values = malloc(sender_times * 16 * sizeof(uint8_t));
    ot_sender_sk_lists = malloc(sender_times * NUMBER_OF_OTS * 32 * sizeof(uint8_t));
    q_matrices = malloc(sender_times * 1024 * (NUMBER_OF_OTS / 8) * sizeof(uint8_t)); 

    ot_sender_received_pks = malloc(sender_times * NUMBER_OF_OTS * sizeof(curve_point));
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

  if (!mta_send_snd_pub_keys(query, group_info, index, 
                             participant_indices, sender_times, priv_key, s_values, 
                             ot_sender_sk_lists, ot_sender_received_pks)) {
    return false;
  }

  if (!mta_send_rcv_enc(query, group_info, index, participant_indices, receiver_times, 
                        priv_key, ot_receiver_sk_lists, b_value, t_matrices)) {
    return false;
  }

  if (!mta_post_snd_enc(query, group_info, index, participant_indices, sender_times, 
                        s_values, ot_sender_sk_lists, ot_sender_received_pks, q_matrices)) {
    return false;
  }

  free(ot_receiver_sk_lists);
  free(ot_sender_sk_lists);
  free(ot_sender_received_pks);

  if (sender_times > 0) {
    sender_additive_shares_list = malloc(sender_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) * sizeof(bignum256));

    for (int i = 0; i < sender_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1); ++i) {
      bn_zero(&sender_additive_shares_list[i]);
    }
  }

  if (!mta_mascot_snd(query, index, participant_indices, sender_times, group_info->threshold,
                      s_values, q_matrices, priv_key, secret_share_list,
                      priv_key_share, sender_additive_shares_list)) {
    return false;
  }

  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;

  bignum256 lambda;
  compute_lambda(&lambda, participant_indices, threshold, index);

  for (int i = 0; i < sender_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1); ++i) {
    bn_subtractmod(&curve->order, &sender_additive_shares_list[i], &sender_additive_shares_list[i], &curve->order);
    bn_multiply(&lambda, &sender_additive_shares_list[i], &curve->order);
  }

  memcpy(additive_shares_list, sender_additive_shares_list, sender_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) * sizeof(bignum256));

  if (receiver_times > 0) {
    receiver_additive_shares_list = malloc(receiver_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) * sizeof(bignum256));

    for (int i = 0; i < receiver_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1); ++i) {
      bn_zero(&receiver_additive_shares_list[i]);
    }
  }

  if (!mta_mascot_rcv(query, group_info, index, participant_indices, receiver_times,
                      t_matrices, receiver_additive_shares_list, b_value)) {
    return false;
  }

  for (int i = 0; i < receiver_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1); ++i) {
    bn_multiply(&lambda, &receiver_additive_shares_list[i], &curve->order);
  }

  memcpy(&additive_shares_list[sender_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1)], 
         receiver_additive_shares_list, receiver_times * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) * sizeof(bignum256));

  free(s_values);
  free(q_matrices);
  free(t_matrices);
  free(b_value);
  free(receiver_additive_shares_list);
  free(sender_additive_shares_list);

  return true;  
}

bool sig_get_authenticator(mpc_poc_query_t *query,
                           bignum256 *additive_shares_list,
                           size_t length,
                           bignum256 *self_product1,
                           bignum256 *a_share,
                           mpc_poc_group_key_info_t *group_key_info,
                           uint8_t *priv_key,
                           uint8_t *my_small_w_share,
                           uint8_t *my_big_w_share) {

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_SIG_GET_AUTHENTICATOR_TAG)) {
      mpc_delay_scr_init("Error: wrong query (get authenticator)", DELAY_TIME);
      return false;
  }

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_SIG_GET_AUTHENTICATOR_TAG;

  bignum256 small_w_share_bn;
  bn_copy(self_product1, &small_w_share_bn);

  for (int i = 0; i < length - 1; ++i) {
    bn_addmod(&small_w_share_bn, &additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + 0], &curve->order);
    bn_addmod(&small_w_share_bn, &additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + 1], &curve->order);
  }

  curve_point r_value;
  ecdsa_read_pubkey(curve, group_key_info->group_pub_key, &r_value);

  curve_point big_w_share_point;
  point_multiply(curve, a_share, &r_value, &big_w_share_point);

  mpc_poc_authenticator_data_t authenticator_data = MPC_POC_AUTHENTICATOR_DATA_INIT_ZERO;
  bn_write_be(&small_w_share_bn, authenticator_data.small_w_share);

  authenticator_data.big_w_share[0] = 0x02 | (big_w_share_point.y.val[0] & 0x01);
  bn_write_be(&big_w_share_point.x, authenticator_data.big_w_share + 1);

  memcpy(my_small_w_share, authenticator_data.small_w_share, 32);
  memcpy(my_big_w_share, authenticator_data.big_w_share, 33);

  mpc_poc_signed_authenticator_data_t signed_authenticator_data = MPC_POC_SIGNED_AUTHENTICATOR_DATA_INIT_ZERO;
  signed_authenticator_data.has_authenticator_data = true;
  signed_authenticator_data.authenticator_data = authenticator_data;

  // sign authenticator data
  if (!mpc_sign_struct(&authenticator_data, AUTHENTICATOR_DATA_BUFFER_SIZE, 
                       MPC_POC_AUTHENTICATOR_DATA_FIELDS, signed_authenticator_data.signature, priv_key)) {
    mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  response.sig_get_authenticator.has_signed_authenticator_data = true;
  response.sig_get_authenticator.signed_authenticator_data = signed_authenticator_data;

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

bool sig_compute_authenticator(mpc_poc_query_t *query,
                               mpc_poc_group_info_t *group_info,
                               uint32_t *participant_indices,
                               uint32_t index,
                               size_t length,
                               uint8_t *my_small_w_share,
                               uint8_t *my_big_w_share,
                               bignum256 *w_bn,
                               uint8_t *W) {

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_SIG_COMPUTE_AUTHENTICATOR_TAG)) {
      mpc_delay_scr_init("Error: wrong query (compute authenticator)", DELAY_TIME);
      return false;
  }

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_SIG_COMPUTE_AUTHENTICATOR_TAG;

  if (query->sign_message.sig_compute_authenticator.signed_authenticator_data_list_count != length - 1) {
    mpc_delay_scr_init("Error: wrong length of signed authenticator data list", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  bn_read_be(my_small_w_share, w_bn);

  const curve_point* points[length];
  uint32_t xcords[length];

  xcords[0] = participant_indices[index];

  curve_point my_point;
  ecdsa_read_pubkey(curve, my_big_w_share, &my_point);

  points[0] = &my_point;

  int ind = 1;

  int k = 0;
  for (int i = 0; i < length; ++i) {
    if (i == index) {
      continue;
    }

    mpc_poc_signed_authenticator_data_t signed_authenticator_data = 
      query->sign_message.sig_compute_authenticator.signed_authenticator_data_list[k];

    uint8_t *participant_pub_key = malloc(33 * sizeof(uint8_t));

    if (!index_to_pub_key(group_info, participant_indices[i], participant_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    if (!mpc_verify_struct_sig(&signed_authenticator_data.authenticator_data, AUTHENTICATOR_DATA_BUFFER_SIZE,
                           MPC_POC_AUTHENTICATOR_DATA_FIELDS, signed_authenticator_data.signature, 
                           participant_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    bignum256 small_w_share_bn;
    bn_read_be(signed_authenticator_data.authenticator_data.small_w_share, &small_w_share_bn);

    bn_addmod(w_bn, &small_w_share_bn, &curve->order);

    xcords[ind] = participant_indices[i];

    curve_point point;
    ecdsa_read_pubkey(curve, signed_authenticator_data.authenticator_data.big_w_share, &point);

    points[ind] = &point;

    ind++;
    k++;
  }

  curve_point W_point;
  lagarange_exp_interpolate(curve, points, xcords, 0, length, &W_point);

  W[0] = 0x02 | (W_point.y.val[0] & 0x01);
  bn_write_be(&W_point.x, W + 1);

  // check if w.G = W
  curve_point wG;
  scalar_multiply(curve, w_bn, &wG);

  if (!point_is_equal(&wG, &W_point)) {
    mpc_delay_scr_init("Error: w.G != W", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

bool sig_get_ka_share(mpc_poc_query_t *query,
                      bignum256 *w,
                      bignum256 *k_share,
                      bignum256 *a_share,
                      uint8_t *priv_key,
                      bignum256 *my_ka_share,
                      uint32_t *participant_indices,
                      uint32_t threshold,
                      uint32_t index,
                      bignum256 *k_inv_share) {
  
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_SIG_GET_KA_SHARE_TAG)) {
      mpc_delay_scr_init("Error: wrong query (get ka share)", DELAY_TIME);
      return false;
  }

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_SIG_GET_KA_SHARE_TAG;

  bignum256 ka_share;

  bn_copy(w, k_inv_share);
  bn_inverse(k_inv_share, &curve->order);
  bn_multiply(k_share, k_inv_share, &curve->order);

  bn_copy(k_inv_share, &ka_share);
  bn_addmod(&ka_share, a_share, &curve->order);

  bignum256 lambda;
  compute_lambda(&lambda, participant_indices, threshold, index);

  bn_multiply(&lambda, &ka_share, &curve->order);

  mpc_poc_signed_ka_share_t signed_ka_share = MPC_POC_SIGNED_KA_SHARE_INIT_ZERO;
  bn_write_be(&ka_share, signed_ka_share.ka_share);

  memcpy(my_ka_share, &ka_share, sizeof(bignum256));

  if (mpc_sign_message(signed_ka_share.ka_share, 32, signed_ka_share.signature, priv_key) != 0) {
    mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  response.sig_get_ka_share.has_signed_ka_share = true;
  response.sig_get_ka_share.signed_ka_share = signed_ka_share;

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

bool sig_compute_ka(mpc_poc_query_t *query,
                    mpc_poc_group_info_t *group_info,
                    uint32_t *participant_indices,
                    uint32_t threshold,
                    uint32_t index,
                    bignum256 *k_inv_plus_p) {
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_SIG_COMPUTE_KA_TAG)) {
      mpc_delay_scr_init("Error: wrong query (compute ka)", DELAY_TIME);
      return false;
  }

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_SIG_COMPUTE_KA_TAG;

  if (query->sign_message.sig_compute_ka.signed_ka_share_list_count != threshold - 1) {
    mpc_delay_scr_init("Error: wrong length of signed ka share list", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  int k = 0;
  for (int i = 0; i < threshold; ++i) {
    if (i == index) {
      continue;
    }

    mpc_poc_signed_ka_share_t signed_ka_share = 
      query->sign_message.sig_compute_ka.signed_ka_share_list[k];

    uint8_t *participant_pub_key = malloc(33 * sizeof(uint8_t));

    if (!index_to_pub_key(group_info, participant_indices[i], participant_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }
    if (!mpc_verify_signature(signed_ka_share.ka_share, 32, signed_ka_share.signature, participant_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    bignum256 ka_share;
    bn_read_be(signed_ka_share.ka_share, &ka_share);

    bn_addmod(k_inv_plus_p, &ka_share, &curve->order);
    k++;
  }

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

bool get_sig_share(mpc_poc_query_t *query,
                   bignum256 *k_inv_plus_p,
                   bignum256 *k_inv,
                   uint8_t *priv_key_share,
                   uint8_t *priv_key,
                   bignum256 *sig_share,
                   uint32_t *participant_indices,
                   uint32_t threshold,
                   uint32_t index,
                   bignum256 *additive_shares_list,
                   bignum256 *self_product2,
                   bignum256 *r,
                   uint8_t *message_hash,
                   bignum256 *d_share,
                   bignum256 *e_share) {
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;
  
  bignum256 pi_bn;
  bn_copy(self_product2, &pi_bn);

  for (int i = 0; i < threshold - 1; ++i) {
    bn_addmod(&pi_bn, &additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + 2], &curve->order);
    bn_addmod(&pi_bn, &additive_shares_list[i * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) + 3], &curve->order);
  }

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_GET_SIG_SHARE_TAG)) {
      mpc_delay_scr_init("Error: wrong query (get sig share)", DELAY_TIME);
      return false;
  }

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_GET_SIG_SHARE_TAG;

  bignum256 vi;
  bignum256 si;
  bignum256 xi;

  bn_read_be(priv_key_share, &xi);

  bn_multiply(k_inv_plus_p, &xi, &curve->order);

  bignum256 lambda;
  compute_lambda(&lambda, participant_indices, threshold, index);

  bn_multiply(&lambda, &xi, &curve->order);
  bn_subtractmod(&xi, &pi_bn, &vi, &curve->order);

  bignum256 mi;
  bn_read_be(message_hash, &mi);

  bn_copy(k_inv, &si);
  bn_multiply(&mi, &si, &curve->order);

  bn_multiply(d_share, &mi, &curve->order);
  bn_addmod(&si, &mi, &curve->order);

  bn_addmod(&si, e_share, &curve->order);

  bn_multiply(&lambda, &si, &curve->order);

  bn_multiply(r, &vi, &curve->order);
  bn_addmod(&si, &vi, &curve->order);

  bn_copy(&si, sig_share);

  mpc_poc_signed_sig_share_t signed_sig_share = MPC_POC_SIGNED_SIG_SHARE_INIT_ZERO;
  bn_write_be(sig_share, signed_sig_share.sig_share);

  if (mpc_sign_message(signed_sig_share.sig_share, 32, signed_sig_share.signature, priv_key) != 0) {
    mpc_delay_scr_init("Error: signing failed", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  response.get_sig_share.has_signed_sig_share = true;
  response.get_sig_share.signed_sig_share = signed_sig_share;

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

bool compute_sig(mpc_poc_query_t *query,
                 mpc_poc_group_info_t *group_info,
                 uint32_t *participant_indices,
                 uint32_t threshold,
                 uint32_t index,
                 bignum256 *r,
                 bignum256 *my_sig_share,
                 uint8_t *final_signature) {
  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!mpc_get_query(query, MPC_POC_QUERY_SIGN_MESSAGE_TAG) ||
      !check_which_request(query, MPC_POC_SIGN_MESSAGE_REQUEST_COMPUTE_SIG_TAG)) {
      mpc_delay_scr_init("Error: wrong query (compute sig)", DELAY_TIME);
      return false;
  }

  mpc_poc_result_t result =
      init_mpc_result(MPC_POC_RESULT_SIGN_MESSAGE_TAG);

  mpc_poc_sign_message_response_t response = MPC_POC_SIGN_MESSAGE_RESPONSE_INIT_ZERO;
  response.which_response = MPC_POC_SIGN_MESSAGE_RESPONSE_COMPUTE_SIG_TAG;

  if (query->sign_message.compute_sig.signed_sig_share_list_count != threshold - 1) {
    mpc_delay_scr_init("Error: wrong length of signed sig share list", DELAY_TIME);
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  int k = 0;
  for (int i = 0; i < threshold; ++i) {
    if (i == index) {
      continue;
    }

    mpc_poc_signed_sig_share_t signed_sig_share = 
      query->sign_message.compute_sig.signed_sig_share_list[k];

    uint8_t *participant_pub_key = malloc(33 * sizeof(uint8_t));

    if (!index_to_pub_key(group_info, participant_indices[i], participant_pub_key)) {
      mpc_delay_scr_init("Error: index to pubkey failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }
    if (!mpc_verify_signature(signed_sig_share.sig_share, 32, signed_sig_share.signature, participant_pub_key)) {
      mpc_delay_scr_init("Error: signature verification failed", DELAY_TIME);
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    bignum256 sig_share;
    bn_read_be(signed_sig_share.sig_share, &sig_share);

    bn_addmod(my_sig_share, &sig_share, &curve->order);

    k++;
  }

  bn_write_be(r, final_signature);
  bn_write_be(my_sig_share, final_signature + 32);

  memcpy(response.compute_sig.signature, final_signature, 64);

  result.sign_message = response;
  mpc_send_result(&result);

  return true;
}

void compute_self_product(bignum256 *self_product1, bignum256 *self_product2, 
                          bignum256 *secret_share_list, uint8_t *dec_share,
                          uint32_t threshold, uint32_t index, uint32_t *participant_indices) {
  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;

  bignum256 lambda;
  compute_lambda(&lambda, participant_indices, threshold, index);

  bn_multiply(&secret_share_list[POLYNOMIAL_K_INDEX], self_product1, &curve->order);
  bn_multiply(&secret_share_list[POLYNOMIAL_A_INDEX], self_product1, &curve->order);
  bn_multiply(&lambda, self_product1, &curve->order);
  bn_multiply(&lambda, self_product1, &curve->order);

  bn_read_be(dec_share, self_product2);
  bn_multiply(&secret_share_list[POLYNOMIAL_P_INDEX], self_product2, &curve->order);
  bn_multiply(&lambda, self_product2, &curve->order);
  bn_multiply(&lambda, self_product2, &curve->order);
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

    uint8_t *msg_hash = malloc(32 * sizeof(uint8_t));
    if (!approve_message(query, msg_hash)) {
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
    display_msg_on_screen("MTA process started.");

    uint8_t *dec_share = malloc(32 * sizeof(uint8_t));
    if (mpc_aes_decrypt(group_key_info.group_share.enc_share, 32, dec_share, priv_key) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    bignum256 *additive_shares_list = malloc((group_info.threshold - 1) * (POLYNOMIALS_COUNT - ZERO_POLYNOMIALS + 1) * sizeof(bignum256));

    bignum256 self_product1;
    bignum256 self_product2;

    bn_one(&self_product1);
    bn_one(&self_product2);

    uint32_t index = 0;
    for (uint32_t i = 0; i < group_info.threshold; i++) {
      if (participant_indices[i] == my_index) {
        index = i;
        break;
      }
    }

    compute_self_product(&self_product1, &self_product2, secret_share_list, dec_share, 
                         group_info.threshold, index, participant_indices);

    if (!start_mta(query, &group_info, 
                   my_index, participant_indices, 
                   group_info.threshold, secret_share_list, 
                   dec_share, priv_key, additive_shares_list)) {
      return;
    }

    mpc_delay_scr_init("MTA successfully finished.", DELAY_SHORT);
    display_msg_on_screen("Generating signature...");

    uint8_t *my_small_w_share = malloc(32 * sizeof(uint8_t));
    uint8_t *my_big_w_share = malloc(33 * sizeof(uint8_t));

    if (!sig_get_authenticator(query, additive_shares_list, group_info.threshold, 
                               &self_product1, &secret_share_list[POLYNOMIAL_A_INDEX],
                               &group_key_info_list[POLYNOMIAL_K_INDEX], priv_key,
                               my_small_w_share, my_big_w_share)) {

      return;
    }

    bignum256 w;
    bignum256 k_inv;

    uint8_t *W = malloc(33 * sizeof(uint8_t));

    if (!sig_compute_authenticator(query, &group_info, participant_indices, index, 
                                   group_info.threshold, my_small_w_share, my_big_w_share, 
                                   &w, W)) {
      return;
    }

    bignum256 my_ka_share;
    if (!sig_get_ka_share(query, &w, &secret_share_list[POLYNOMIAL_K_INDEX], 
                          &secret_share_list[POLYNOMIAL_P_INDEX], priv_key, &my_ka_share,
                          participant_indices, group_info.threshold, index, &k_inv)) {
      return;
    }

    if (!sig_compute_ka(query, &group_info, participant_indices, group_info.threshold, 
                        index, &my_ka_share)) {
      return;
    }
    
    curve_point R;

    const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;
    ecdsa_read_pubkey(curve, group_key_info_list[POLYNOMIAL_K_INDEX].group_pub_key, &R);

    bignum256 sig_share;
    if (!get_sig_share(query, &my_ka_share, &k_inv, dec_share, priv_key, &sig_share, 
                       participant_indices, group_info.threshold, index, 
                       additive_shares_list, &self_product2, &(R.x), msg_hash,
                       &secret_share_list[POLYNOMIAL_D_INDEX], &secret_share_list[POLYNOMIAL_E_INDEX])) {
      return;
    }

    uint8_t *final_signature = malloc(64 * sizeof(uint8_t));

    if (!compute_sig(query, &group_info, participant_indices, group_info.threshold, 
                     index, &R.x, &sig_share, final_signature)) {
      return;
    }

    mpc_delay_scr_init("Signature successfully generated.", DELAY_SHORT);

    char msg[130];
    byte_array_to_hex_string(final_signature, 64, msg, 130);

    if (!mpc_core_scroll_page("Match the signature", msg, mpc_send_error)) {
      return;
    }
  }
}