/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "mpc_main.h"

#include "mpc_api.h"
#include "status_api.h"
#include "mpc_get_random_nonce.h"
#include "mpc_verify_entity_info.h"
#include "mpc_sign_entity_info.h"
#include "mpc_wrapper.h"

#include "mpc_dummy.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the MPC application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * Solana app.
 *
 * @param usb_evt The USB event which triggered invocation of the mpc app
 */
void mpc_main(usb_event_t usb_evt, const void *app_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const cy_app_desc_t mpc_app_desc = {.id = 18,
                                              .version =
                                                  {
                                                      .major = 1,
                                                      .minor = 0,
                                                      .patch = 0,
                                                  },
                                              .app = mpc_main,
                                              .app_config = NULL};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void mpc_main(usb_event_t usb_evt, const void *app_config) {
  mpc_poc_query_t query = MPC_POC_QUERY_INIT_DEFAULT;

  if (false == decode_mpc_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  switch ((uint8_t)query.which_request) {
    case MPC_POC_QUERY_INIT_APPLICATION_TAG: {
      mpc_init(&query);
      break;
    }
    case MPC_POC_QUERY_GET_RANDOM_NONCE_TAG: {
      get_random_nonce_flow(&query);
      break;
    }
    case MPC_POC_QUERY_VERIFY_ENTITY_INFO_TAG: {
      verify_entity_info_flow(&query);
      break;
    }
    case MPC_POC_QUERY_SIGN_ENTITY_INFO_TAG: {
      sign_entity_info_flow(&query);
      break;
    }
    case MPC_POC_QUERY_EXIT_APPLICATION_TAG: {
      mpc_exit(&query);
      break;
    }
    case MPC_POC_QUERY_GET_PUBLIC_KEY_TAG: {
      get_public_key_flow(&query);
      break;
    }
    case MPC_POC_QUERY_DUMMY_TAG: {
      dummy_flow(&query);
      break;
    }
    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_QUERY);
    } break;
  }

  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
const cy_app_desc_t *get_mpc_app_desc() {
  return &mpc_app_desc;
}