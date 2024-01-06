/**
 * @file    create_wallet_menu.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef CREATE_WALLET_MENU_H
#define CREATE_WALLET_MENU_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "flow_engine.h"

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
 * @brief This API returns the flow_step_t* for the create wallet menu.
 *
 * @return const flow_step_t* Pointer to the step for the create wallet menu
 */
const flow_step_t *create_wallet_menu_get_step(void);

#endif /* CREATE_WALLET_MENU_H */
