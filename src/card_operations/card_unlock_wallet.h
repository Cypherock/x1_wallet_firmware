/**
 * @file    card_fetch_challenge.h
 * @author  Cypherock X1 Team
 * @brief   API for fetching wallet challenge from a card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_UNLOCK_WALLET_H
#define CARD_UNLOCK_WALLET_H

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
 * @brief The function `card_unlock_wallet` is used to unlock a wallet by
 * verifying the user's password and updating the wallet's locked status.
 * @details The wallet being unlocked stays locked if
 * `CARD_OPERATION_LOCKED_WALLET` is returned. This occurs in two cases i.e.
 * incorrect pin or challenge mismatch/failure. In both the cases, new challenge
 * has to be fetched by the caller.
 *
 * @param wallet A pointer to a Wallet structure that contains information about
 * the wallet to be unlocked.
 *
 * @return a variable of type `card_error_type_e`, which represents the result
 * of the card unlocking operation.
 */
card_error_type_e card_unlock_wallet(const Wallet *wallet);
#endif
