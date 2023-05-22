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

#endif
