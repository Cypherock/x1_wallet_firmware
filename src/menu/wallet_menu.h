/**
 * @file    create_wallet_menu.h
 * @author  Cypherock X1 Team
 * @brief   Populate and handle old wallet menu options
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef WALLET_MENU_H
#define WALLET_MENU_H

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
 * @brief This function returns a pointer to a `flow_step_t` struct and copies
 * the selected wallet ID into a variable.
 *
 * @param selected_wallet_id A pointer to a uint8_t variable that represents the
 * selected wallet ID.
 *
 * @return a pointer to a constant variable of type `flow_step_t`.
 */
const flow_step_t *wallet_menu_get_step(uint8_t *selected_wallet_id);
#endif /* WALLET_MENU_H */
