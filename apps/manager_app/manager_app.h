/**
 * @file    manager_app.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef MANAGER_APP_H
#define MANAGER_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <manager/core.pb.h>
#include <manager/get_device_info.pb.h>
#include <stddef.h>
#include <stdint.h>

#include "events.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum {
  MANAGER_TASK_SUCCESS = 0,
  MANAGER_TASK_FAILED,
  MANAGER_TASK_REJECTED,
  MANAGER_TASK_INVALID_ARGS,
  MANAGER_TASK_DECODING_FAILED,
  MANAGER_TASK_ENCODING_FAILED,
  MANAGER_TASK_P0_ABORT_OCCURED,
  MANAGER_TASK_P0_TIMEOUT_OCCURED,
  MANAGER_TASK_UNKNOWN_QUERY_REQUEST,
  MANAGER_TASK_INVALID_STATE,

  MANAGER_TASK_INVALID_DEFAULT = 0xFF,
} manager_error_code_t;
/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the manager application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * manager app.
 *
 * @param usb_evt The USB event which triggered invocation of the manager app
 */
void manager_app_main(usb_event_t usb_evt);

/**
 * @brief Initiates the get device info flow.
 *
 * @param query Reference to the decoded query struct from the host app
 */
// TODO: Move to manager_app_priv.h
void get_device_info_flow(const manager_query_t *query);

/**
 * @brief Guided user training during the user on-boarding
 */
// TODO: Move to manager_app_priv.h
void manager_joystick_training(manager_query_t *query);

/**
 * @brief Initiates the device authentication flow. Exits in case the flow is
 * completed or a P0 event occurs.
 *
 * @param query Reference to the decoded query struct from the host app
 */
// TODO: Move to manager_app_priv.h
void device_authentication_flow(const manager_query_t *query);

#endif /* MANAGER_APP_H */
