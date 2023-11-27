#include "mpc_api.h"
#include "mpc_context.h"
#include "mpc_helpers.h"

#include "ui_core_confirm.h"
#include "ui_screens.h"

static bool check_which_request(const mpc_poc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->dummy.which_request) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

void dummy_flow(mpc_poc_query_t *query) {
  if (MPC_POC_DUMMY_REQUEST_INITIATE_TAG !=
      query->dummy.which_request) {
    // set the relevant tags for error
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
  } else {
    mpc_poc_result_t result =
        init_mpc_result(MPC_POC_RESULT_DUMMY_TAG);

    mpc_poc_dummy_response_t response = MPC_POC_DUMMY_RESPONSE_INIT_ZERO;
    response.which_response = MPC_POC_DUMMY_RESPONSE_FIRST_RESPONSE_TAG;

    response.first_response.num = 6969;

    result.dummy = response;
    mpc_send_result(&result);

    delay_scr_init("6969 sent.", DELAY_TIME);

    if (!mpc_get_query(query, MPC_POC_QUERY_DUMMY_TAG) ||
        !check_which_request(query, MPC_POC_DUMMY_REQUEST_NEXT_TAG)) {
        return false;
    }

    result = init_mpc_result(MPC_POC_RESULT_DUMMY_TAG);

    response.which_response = MPC_POC_DUMMY_RESPONSE_SECOND_RESPONSE_TAG;

    response.second_response.condition = true;

    result.dummy = response;
    mpc_send_result(&result);
  }
}