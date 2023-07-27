/**
 * @file    delete_wallet_flow.h
 * @author  Cypherock X1 Team
 * @brief   Flow for delete wallet operation on an existing wallet
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef DELETE_WALLET_FLOW_H
#define DELETE_WALLET_FLOW_H

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
 * @brief This function deletes the user selected wallet from all cards and
 * device flash.
 *
 * @param wallet_id A pointer to a uint8_t variable that represents the wallet
 * ID.
 */
void delete_wallet_flow(const uint8_t *wallet_id);
#endif
