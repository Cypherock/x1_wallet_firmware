// #include "mpc_api.h"
// #include "mpc_context.h"

// #include "pb_encode.h"
// #include "utils.h"

// static mpc_poc_get_random_nonce_response_t get_random_nonce() {
//   mpc_poc_get_random_nonce_response_t response =
//       MPC_POC_GET_RANDOM_NONCE_RESPONSE_INIT_ZERO;
  
//   response.which_response = MPC_POC_GET_RANDOM_NONCE_RESPONSE_RESULT_TAG;

//   if (response.which_response == MPC_POC_GET_RANDOM_NONCE_RESPONSE_RESULT_TAG) {
//     mpc_poc_get_random_nonce_result_response_t *result = &response.result;

//     uint8_t random_nonce[32];

//     random_generate(random_nonce, 32);
//     memcpy(result->nonce, random_nonce, 32);
//   }

//   return response;
// }

// void get_random_nonce_flow(const mpc_poc_query_t *query) {
//   if (MPC_POC_GET_RANDOM_NONCE_REQUEST_INITIATE_TAG !=
//       query->get_random_nonce.which_request) {
//     // set the relevant tags for error
//     mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
//                        ERROR_DATA_FLOW_INVALID_REQUEST);
//   } else {
//     mpc_poc_result_t result =
//         init_mpc_result(MPC_POC_RESULT_GET_RANDOM_NONCE_TAG);
//     result.get_random_nonce = get_random_nonce();
//     mpc_send_result(&result);
//   }
// }