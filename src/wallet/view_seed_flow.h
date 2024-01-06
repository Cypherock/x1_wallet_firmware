/**
 * @file    view_seed_flow.h
 * @author  Cypherock X1 Team
 * @brief   Flow for view seed operation on an existing wallet
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef VIEW_SEED_FLOW_H
#define VIEW_SEED_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "stdint.h"
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
 * @brief The function displays the mnemonics for a given wallet index.
 *
 * @param wallet_id A pointer to a uint8_t variable that represents the wallet
 * ID.
 */
void view_seed_flow(const uint8_t *wallet_id);
#endif
