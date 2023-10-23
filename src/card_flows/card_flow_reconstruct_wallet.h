/**
 * @file    card_flow_reconstruct_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting card flow reconstruct wallet seed using the X1
 *          cards
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FLOW_RECONSTRUCT_WALLET_H
#define CARD_FLOW_RECONSTRUCT_WALLET_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "card_operation_typedefs.h"

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
 * @brief This functions executes a sequential card flow to read wallet share
 * from atleast threshold number of X1 cards.
 *
 * @param threshold The number of X1 cards need to be read from
 * @param error_status_OUT card error status for error reporting to host
 * @return card_errors_type The return code of the flow
 */
card_error_type_e card_flow_reconstruct_wallet(uint8_t threshold,
                                               uint32_t *error_status_OUT);

#endif /* CARD_FLOW_RECONSTRUCT_WALLET_H */
