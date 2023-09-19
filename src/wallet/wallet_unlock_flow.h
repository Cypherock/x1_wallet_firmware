/**
 * @file    wallet_unlock_flow.h
 * @author  Cypherock X1 Team
 * @brief   Header file for wallet unlock flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef WALLET_UNLOCK_FLOW_H
#define WALLET_UNLOCK_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "flash_struct.h"
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
 * @brief The function `wallet_unlock_flow` unlocks a wallet by fetching a
 * challenge and handling the unlocking process. Wallet unlock is performed in
 * the following steps:
 * 1. Fetch challenge from card
 * 2. Solve challenge by finding correct nonce for acheiving the target hash
 * 3. Ask user to enter the pin
 * 4. Unlock wallet on card
 *
 * @param flash_wallet A pointer to a structure of type Flash_Wallet, which
 * contains information about a wallet stored in flash memory.
 */
void wallet_unlock_flow(const Flash_Wallet *flash_wallet);
#endif /* WALLET_UNLOCK_FLOW_H */
