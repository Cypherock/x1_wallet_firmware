/**
 * @file    create_wallet_flow.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the create wallet flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CREATE_WALLET_FLOW_H
#define CREATE_WALLET_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>

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
 * @brief This API executes the wallet creation flow on the device
 * @details This function generate random mnemonics or inputs seed phrase from
 * user, takes user inputs for wallet configuration, writes the wallet shares on
 * the X1 vault flash and X1 cards and verifies each share
 *
 * @param new_wallet true if a new wallet needs to be created, false if the user
 * wants to recover the wallet from a seed phrase
 */
void create_wallet_flow(bool new_wallet);

#endif /* CREATE_WALLET_FLOW_H */
