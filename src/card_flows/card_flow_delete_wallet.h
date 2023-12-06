/**
 * @file    card_flow_delete_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting card flow to delete wallet on the X1 cards
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FLOW_DELETE_WALLET_H
#define CARD_FLOW_DELETE_WALLET_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>

#include "card_operation_typedefs.h"
#include "wallet.h"

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
 * @brief This functions executes a sequential card flow to fetch(for
 * verification) and delete wallet on each of the 4 X1 cards
 *
 * @return true If the flow completed successfully and wallet share was written
 * and read back from all 4 cards
 * @return Card error code status, CARD_OPERATION_SUCCESS if deleted
 * successfully, else error
 */
card_error_type_e card_flow_delete_wallet(Wallet *wallet);
#endif /* CARD_FLOW_DELETE_WALLET_H */
