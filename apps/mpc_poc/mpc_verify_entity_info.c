#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "pb_encode.h"
#include "hasher.h"

#include "ui_core_confirm.h"
#include "ui_screens.h"

static mpc_poc_verify_entity_info_response_t verify_entity_info(const pb_byte_t *fingerprint, 
    const mpc_poc_entity_info_t entity_info, const pb_byte_t *signature) {

  mpc_poc_verify_entity_info_response_t response =
      MPC_POC_VERIFY_ENTITY_INFO_RESPONSE_INIT_ZERO;
  
  response.which_response = MPC_POC_VERIFY_ENTITY_INFO_RESPONSE_RESULT_TAG;

  if (response.which_response == MPC_POC_VERIFY_ENTITY_INFO_RESPONSE_RESULT_TAG) {
    mpc_poc_verify_entity_info_result_response_t *result = &response.result;
    
    uint8_t entity_info_buffer[ENTITY_INFO_BUFFER_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(entity_info_buffer, ENTITY_INFO_BUFFER_SIZE);

    bool status = pb_encode(&stream, MPC_POC_ENTITY_INFO_FIELDS, &entity_info);

    if (status == false) {
        delay_scr_init("Error: Unable to encode entity info", DELAY_TIME);
        
        mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_REQUEST);
        return response; // have to check how to exit flow
    }

    size_t entity_info_len = 0;
    entity_info_len = stream.bytes_written;

    delay_scr_init("Verifying fingerprint...", DELAY_SHORT);

    uint8_t sha256_hash[32];

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);  

    hasher_Update(&hasher, entity_info_buffer, entity_info_len);
    hasher_Final(&hasher, sha256_hash);

    if (memcmp(fingerprint, sha256_hash, 32) != 0) {
      delay_scr_init("Error: Fingerprint doesn't match", DELAY_TIME);
      result->verified = false;
      return response;
    }

    delay_scr_init("Fingerprint verified.", DELAY_SHORT);
    delay_scr_init("Verifying signature...", DELAY_SHORT);

    if (mpc_verify_signature(entity_info_buffer, entity_info_len, signature, (uint8_t *)entity_info.pub_key) == false) {
      delay_scr_init("Error: Signature doesn't match", DELAY_TIME);
      result->verified = false;
      return response;
    }

    delay_scr_init("Signature verified.", DELAY_SHORT);
    result->verified = true;
  }

  return response;
}

void verify_entity_info_flow(const mpc_poc_query_t *query) {
  if (MPC_POC_VERIFY_ENTITY_INFO_REQUEST_INITIATE_TAG !=
      query->verify_entity_info.which_request || !query->verify_entity_info.initiate.has_entity_info) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_VERIFY_ENTITY_INFO_TAG);

    const mpc_poc_entity_info_t entity_info = query->verify_entity_info.initiate.entity_info;
    const pb_byte_t *fingerprint = query->verify_entity_info.initiate.fingerprint;
    const pb_byte_t *signature = query->verify_entity_info.initiate.signature;

    result.verify_entity_info = verify_entity_info(fingerprint, entity_info, signature);
    mpc_send_result(&result);
  }
}