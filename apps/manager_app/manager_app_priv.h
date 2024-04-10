/**
 * @file    manager_app_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for manager app internal operations
 * This file is defined to separate manager's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef MANAGER_APP_PRIV_H
#define MANAGER_APP_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "events.h"
#include "manager_api.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for card authentication, this flow expects
 * MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG as initial query, otherwise the flow
 * is aborted
 *
 * @param query object for card auth query
 */
void card_auth_handler(manager_query_t *query);

/**
 * @brief Initiates the get device info flow.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void get_device_info_flow(const manager_query_t *query);

/**
 * @brief Guided user training during the user on-boarding
 */
void manager_joystick_training(manager_query_t *query);

/**
 * @brief Initiates the device authentication flow. Exits in case the flow is
 * completed or a P0 event occurs.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void device_authentication_flow(manager_query_t *query);

/**
 * @brief Guided card training during the user on-boarding
 *
 * @param query Reference to the decoded query struct from the host app
 */
void manager_card_training(manager_query_t *query);

/**
 * @brief Transfer ascii encoded logs from the device to the host
 *
 * @param query Reference to the decoded query struct from the host app
 */
void manager_get_logs(manager_query_t *query);

/**
 * @brief Export wallet list from the device to the host
 *
 * @param query Reference to the decoded query struct from the host app
 */
void manager_export_wallets(manager_query_t *query);

/**
 * @brief User confirmation for firmware update based on request from host
 *
 * @param query Reference to the decoded query struct from the host app
 */
void manager_confirm_firmware_update(manager_query_t *query);

/**
 * @brief
 *
 * @param query Reference to the decoded query struct from the host app
 */
void manager_wallet_selector(manager_query_t *query);
#endif
