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

const uint8_t (*global_fingerprints)[32];

// Comparator function for qsort
int compare_fingerprints(const void *a, const void *b) {
    uint16_t index_a = *(const uint16_t *)a;
    uint16_t index_b = *(const uint16_t *)b;

    char str_a[65], str_b[65]; // 32 bytes -> 64 hex chars + 1 for null terminator
    byte_array_to_hex_string(global_fingerprints[index_a], 32, str_a, sizeof(str_a));
    byte_array_to_hex_string(global_fingerprints[index_b], 32, str_b, sizeof(str_b));

    return strcmp(str_a, str_b);
}

static bool check_which_request(const mpc_poc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->group_setup.which_request) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

bool populate_entity_info(uint32_t timestamp, 
                          uint32_t threshold, 
                          uint32_t total_participants, 
                          const uint8_t *wallet_id,
                          const uint8_t *pub_key,
                          mpc_poc_entity_info_t *entity_info) {

  if (timestamp == 0 || threshold == 0 || total_participants == 0) {
    return false;
  }

  uint8_t random_nonce[32] = {0};

  entity_info->timestamp = timestamp;
  entity_info->threshold = threshold;
  entity_info->total_participants = total_participants;

  random_generate(random_nonce, 32);
  memcpy(entity_info->random_nonce, random_nonce, 32);

  entity_info->has_device_info = true;

  memcpy(entity_info->device_info.wallet_id, wallet_id, 32);
  memcpy(entity_info->device_info.pub_key, pub_key, 33);
  memcpy(entity_info->device_info.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);

  return true;
}

bool compute_fingerprint(const mpc_poc_entity_info_t *entity_info, uint8_t *fingerprint) {
  uint8_t entity_info_bytes[ENTITY_INFO_BUFFER_SIZE] = {0};
  size_t entity_info_bytes_len = 0;

  pb_ostream_t stream = pb_ostream_from_buffer(entity_info_bytes, ENTITY_INFO_BUFFER_SIZE);

  if (!pb_encode(&stream, MPC_POC_ENTITY_INFO_FIELDS, entity_info)) {
    return false;
  }

  entity_info_bytes_len = stream.bytes_written;

  Hasher hasher;
  hasher_Init(&hasher, HASHER_SHA2);  

  hasher_Update(&hasher, entity_info_bytes, entity_info_bytes_len);
  hasher_Final(&hasher, fingerprint);

  return true;
}

bool verify_participant_info_list(uint16_t threshold, 
                                  uint16_t total_participants, 
                                  const mpc_poc_participant_info_t *participant_info_list, 
                                  size_t participant_info_list_count) {
  if (participant_info_list_count + 1 != total_participants) {
    return false;
  }
  
  uint8_t fingerprint[32] = {0};
  for (int i = 0; i < participant_info_list_count; i++) {
    mpc_poc_participant_info_t participant_info = participant_info_list[i];

    if (participant_info.entity_info.threshold != threshold ||
        participant_info.entity_info.total_participants != total_participants) {
      return false;
    }

    if (!compute_fingerprint(&participant_info.entity_info, fingerprint)) {
      return false;
    }

    if (memcmp(participant_info.fingerprint, fingerprint, 32) != 0) {
      return false;
    }
  }

  return true;
}

bool compute_group_id(uint16_t threshold, 
                      uint16_t total_participants, 
                      const mpc_poc_entity_info_t *entity_info_list, 
                      const uint8_t (*fingerprint_list)[32],
                      uint8_t *group_id,
                      mpc_poc_group_info_t *group_info) {
  
  uint16_t indexes[total_participants];

  for (int i = 0; i < total_participants; i++) {
    indexes[i] = i;
  }

  global_fingerprints = fingerprint_list;
  qsort(indexes, total_participants, sizeof(uint16_t), compare_fingerprints);

  group_info->threshold = threshold;
  group_info->total_participants = total_participants;
  group_info->participants_count = total_participants;

  for (int i = 0; i < total_participants; i++) {
    group_info->participants[i] = entity_info_list[indexes[i]].device_info;
  }

  uint8_t group_info_bytes[GROUP_INFO_BUFFER_SIZE] = {0};
  size_t group_info_bytes_len = 0;

  pb_ostream_t stream = pb_ostream_from_buffer(group_info_bytes, GROUP_INFO_BUFFER_SIZE);

  if (!pb_encode(&stream, MPC_POC_GROUP_INFO_FIELDS, group_info)) {
    return false;
  }

  group_info_bytes_len = stream.bytes_written;

  Hasher hasher;
  hasher_Init(&hasher, HASHER_SHA2);

  hasher_Update(&hasher, group_info_bytes, group_info_bytes_len);
  hasher_Final(&hasher, group_id);

  return true;
}

void copy_entity_info(mpc_poc_entity_info_t *dest, mpc_poc_entity_info_t *src) {
  dest->timestamp = src->timestamp;
  dest->threshold = src->threshold;
  dest->total_participants = src->total_participants;
  memcpy(dest->random_nonce, src->random_nonce, 32);
  dest->has_device_info = src->has_device_info;
  memcpy(dest->device_info.wallet_id, src->device_info.wallet_id, 32);
  memcpy(dest->device_info.pub_key, src->device_info.pub_key, 33);
  memcpy(dest->device_info.device_id, src->device_info.device_id, 32);
}

bool group_setup_initiate(mpc_poc_query_t *query, uint8_t *wallet_id, uint8_t *priv_key, uint8_t *pub_key) {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_GROUP_SETUP_TAG);

    mpc_poc_group_setup_response_t response = MPC_POC_GROUP_SETUP_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_GROUP_SETUP_RESPONSE_INITIATE_TAG;

    memcpy(wallet_id, query->group_setup.initiate.wallet_id, 32);

    if (!initiate_application(query->group_setup.initiate.wallet_id, priv_key, pub_key)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    memcpy(response.initiate.pub_key, pub_key, 33);

    result.group_setup = response;
    mpc_send_result(&result);

    return true;
}

bool group_setup_get_entity_info(mpc_poc_query_t *query, uint8_t *wallet_id, 
                                 uint8_t *pub_key, 
                                 uint16_t *threshold, uint16_t *total_participants,
                                 mpc_poc_entity_info_t *entity_info, uint8_t *fingerprint) {

    char *msg = malloc(100 * sizeof(char));

    if (!mpc_get_query(query, MPC_POC_QUERY_GROUP_SETUP_TAG) ||
      !check_which_request(query, MPC_POC_GROUP_SETUP_REQUEST_GET_ENTITY_INFO_TAG)) {
      return false;
    }

    mpc_poc_result_t result = init_mpc_result(MPC_POC_RESULT_GROUP_SETUP_TAG);

    mpc_poc_group_setup_response_t response = MPC_POC_GROUP_SETUP_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_GROUP_SETUP_RESPONSE_GET_ENTITY_INFO_TAG;

    *threshold = query->group_setup.get_entity_info.threshold;
    *total_participants = query->group_setup.get_entity_info.total_participants;

    snprintf(msg, 100, "Create the group with:\nThreshold: %d\nTotal participants: %d", *threshold, *total_participants);
    if (!mpc_core_confirmation(msg, mpc_send_error)) {
      return false;
    }

    if (!populate_entity_info(query->group_setup.get_entity_info.timestamp,
                         query->group_setup.get_entity_info.threshold,
                         query->group_setup.get_entity_info.total_participants,
                         wallet_id,
                         pub_key,
                         entity_info)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }
    
    // copy_entity_info(&(response.get_entity_info.entity_info), entity_info);
    mpc_poc_entity_info_t ei = MPC_POC_ENTITY_INFO_INIT_ZERO;
    copy_entity_info(&ei, entity_info);

    response.get_entity_info.has_entity_info = true;
    response.get_entity_info.entity_info = ei;
    result.group_setup = response;

    mpc_send_result(&result);

    if (!compute_fingerprint(entity_info, fingerprint)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    byte_array_to_hex_string(fingerprint, 32, msg, 100);

    if (!mpc_core_scroll_page("Match the fingerprint", msg, mpc_send_error)) {
      return false;
    }

    free(msg);

    return true;
}

bool group_setup_verify(mpc_poc_query_t *query, 
                        uint16_t threshold, uint16_t total_participants,
                        mpc_poc_entity_info_t *entity_info_list,
                        uint8_t (*fingerprint_list)[32]) {

    char *msg = malloc(100 * sizeof(char));

    if (!mpc_get_query(query, MPC_POC_QUERY_GROUP_SETUP_TAG) ||
      !check_which_request(query, MPC_POC_GROUP_SETUP_REQUEST_VERIFY_PARTICIPANT_INFO_LIST_TAG)) {
      return false;
    }

    mpc_poc_result_t result = init_mpc_result(MPC_POC_RESULT_GROUP_SETUP_TAG);

    mpc_poc_group_setup_response_t response = MPC_POC_GROUP_SETUP_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_GROUP_SETUP_RESPONSE_VERIFY_PARTICIPANT_INFO_LIST_TAG;

    size_t participant_info_list_count = query->group_setup.verify_participant_info_list.participant_info_list_count;

    for (int i = 0; i < participant_info_list_count; i++) {
      char temp[50] = {0};
      uint8_t *participant_fingerprint = 
        query->group_setup.verify_participant_info_list.participant_info_list[i].fingerprint;

      byte_array_to_hex_string(participant_fingerprint, 32, msg, 100);
      snprintf(temp, sizeof(temp), "Participant %d", i+1);
      if (!mpc_core_scroll_page(temp, msg, mpc_send_error)) {
        return false;
      }
    }

    response.verify_participant_info_list.verified = 
      verify_participant_info_list(threshold,
                                   total_participants,
                                   query->group_setup.verify_participant_info_list.participant_info_list,
                                   query->group_setup.verify_participant_info_list.participant_info_list_count);


    if (response.verify_participant_info_list.verified) {
      mpc_delay_scr_init("Verification successful", DELAY_TIME);

      for (int i = 1; i < total_participants; i++) {
        copy_entity_info(&entity_info_list[i], 
          &query->group_setup.verify_participant_info_list.participant_info_list[i-1].entity_info);

        memcpy(fingerprint_list[i], 
          query->group_setup.verify_participant_info_list.participant_info_list[i-1].fingerprint, 
          32);
      }

      result.group_setup = response;
      mpc_send_result(&result);
    }
    else {
      mpc_delay_scr_init("Verification failed", DELAY_TIME);
      result.group_setup = response;
      mpc_send_result(&result);
      return false;
    }

    free(msg);
    return true;
}

bool group_setup_get_group_id(mpc_poc_query_t *query, 
                              uint16_t threshold, uint16_t total_participants,
                              mpc_poc_entity_info_t *entity_info_list,
                              uint8_t (*fingerprint_list)[32],
                              uint8_t *priv_key) {
    if (!mpc_get_query(query, MPC_POC_QUERY_GROUP_SETUP_TAG) ||
      !check_which_request(query, MPC_POC_GROUP_SETUP_REQUEST_GET_GROUP_ID_TAG)) {
      return false;
    }

    uint8_t group_id[32] = {0};
    uint8_t signature[64] = {0};
    mpc_poc_group_info_t group_info = MPC_POC_GROUP_INFO_INIT_ZERO;
    
    if (!compute_group_id(threshold,
                          total_participants,
                          &entity_info_list[0],
                          &fingerprint_list[0],
                          &group_id[0],
                          &group_info)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    if (mpc_sign_message(&group_id[0], 32, &signature[0], &priv_key[0]) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    mpc_delay_scr_init("Group ID generated.", DELAY_SHORT);

    mpc_poc_result_t result = init_mpc_result(MPC_POC_RESULT_GROUP_SETUP_TAG);

    mpc_poc_group_setup_response_t response = MPC_POC_GROUP_SETUP_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_GROUP_SETUP_RESPONSE_GET_GROUP_ID_TAG;
    memcpy(response.get_group_id.group_id, group_id, 32);
    memcpy(response.get_group_id.signature, signature, 64);

    result.group_setup = response;
    mpc_send_result(&result);

    return true;
}

void group_setup_flow(mpc_poc_query_t *query) {
  if (MPC_POC_GROUP_SETUP_REQUEST_INITIATE_TAG !=
      query->group_setup.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    uint8_t *priv_key = malloc(32 * sizeof(uint8_t));
    uint8_t *pub_key = malloc(33 * sizeof(uint8_t));
    uint8_t *wallet_id = malloc(32 * sizeof(uint8_t));

    uint16_t threshold = 0;
    uint16_t total_participants = 0;


    if (!group_setup_initiate(query, wallet_id, priv_key, pub_key)) {
      return;
    }

    mpc_poc_entity_info_t *entity_info = malloc(sizeof(mpc_poc_entity_info_t));
    uint8_t *fingerprint = malloc(32 * sizeof(uint8_t));

    if (!group_setup_get_entity_info(query, wallet_id, pub_key, &threshold, &total_participants, entity_info, fingerprint)) {
      return;
    }

    mpc_poc_entity_info_t *entity_info_list = malloc(total_participants * sizeof(mpc_poc_entity_info_t));
    uint8_t (*fingerprint_list)[32] = malloc(total_participants * sizeof(*fingerprint_list));

    if (!entity_info_list || !fingerprint_list) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    copy_entity_info(&entity_info_list[0], entity_info);
    memcpy(&fingerprint_list[0], fingerprint, 32);

    free(entity_info);
    free(fingerprint);

    free(wallet_id);
    free(pub_key);


    if (!group_setup_verify(query, threshold, total_participants, entity_info_list, fingerprint_list)) {
      return;
    }

    if (!group_setup_get_group_id(query, threshold, total_participants, entity_info_list, fingerprint_list, priv_key)) {
      return;
    }

    free(priv_key);
    free(entity_info_list);
    free(fingerprint_list);
                                  
    stop_msg_display();
  }
}