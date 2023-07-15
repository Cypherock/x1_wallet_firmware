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
 * @brief The function returns a pointer to a flow_step_t structure based on the
 * given selection index[1-4].
 *
 * @param selection_index The `selection_index` parameter is an unsigned 8-bit
 * integer that represents the index of a wallet selection.
 *
 * @return a pointer to a constant `flow_step_t` object or NULL if invalid
 * selection index is passed
 */
const flow_step_t *wallet_menu_get_step(uint8_t selection_index);
#endif /* WALLET_MENU_H */
