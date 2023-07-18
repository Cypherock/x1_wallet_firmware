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
 * @param wallet_index The wallet index is an unsigned 8-bit integer that
 * represents the index of the wallet on flash, acceptable index [0-3]
 */
void view_seed_flow(uint8_t wallet_index);
#endif