/**
 * @file    card_fetch_challenge.h
 * @author  Cypherock X1 Team
 * @brief   API for fetching wallet challenge from a card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_CHALLENGE_H
#define CARD_FETCH_CHALLENGE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operation_typedefs.h"
#include "flash_struct.h"
#include "stdbool.h"
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
 * @brief Fetches wallet challenge data from a card.
 * @details This function initializes the applet, retrieves the wallet challenge
 * from the card. It handles various error cases and returns an appropriate
 * error code.
 *
 * NOTE:
 * 1. The instruction to tap card is not displayed by the
 * function as the api might be called as a continuation to error handling of a
 * separate flow.
 * 2. This function updates challenge data and lock status on flash for the
 * specified wallet.
 *
 * @param wallet_name The `wallet_name` parameter is a pointer to a uint8_t
 * array that represents the name of the wallet.
 * @param frontend Pointer to the object of @ref card_operation_frontend_t
 * structure, which represents the text to be displayed on retap errors.
 *
 * @return A card_error_type_e value representing the result of the operation.
 */
card_error_type_e card_fetch_challenge(const uint8_t *wallet_name,
                                       card_operation_frontend_t *frontend);
#endif
