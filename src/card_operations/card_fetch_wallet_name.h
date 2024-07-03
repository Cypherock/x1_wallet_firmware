/**
 * @file    card_fetch_wallet_list.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting function to fetch wallet list from X1 card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_WALLET_NAME_H
#define CARD_FETCH_WALLET_NAME_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "card_operation_typedefs.h"
#include "wallet_list.h"

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
 * @brief This card operation extracts the wallet list from an X1 card
 *
 * @param config Reference to operation specific parameters such as acceptable
 * cards
 * @param response Reference to response structure which will be filled with the
 * wallet list and the card tapped
 * @return true If the operation was performed successfully
 * @return false If the operation did not complete
 */
void card_get_wallet_name(uint8_t *wallet_id, const char *wallet_name);

#endif /* CARD_FETCH_WALLET_LIST_H */