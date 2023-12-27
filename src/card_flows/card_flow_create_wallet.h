/**
 * @file    card_flow_create_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting card flow to create wallet on the X1 cards
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FLOW_CREATE_WALLET_H
#define CARD_FLOW_CREATE_WALLET_H

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
 * @brief This functions executes a sequential card flow to create wallet on
 * each of the 4 X1 cards
 *
 * @return true If the flow completed successfully and wallet share was written
 * and read back from all 4 cards
 * @return false If the flow did not complete
 */
bool card_flow_create_wallet(void);

/**
 * @brief Verifies card data after readback from each card during wallet
 * creation
 * @details Internally performs shamir reconstruction using wallet shares from
 * the device and cards and compares against wallet id of the new wallet
 *
 * @return int Integer depicting status of verification. 1 if the verification
 * is done successfully.
 * TODO: Move this function to a better location
 */
int verify_card_share_data();

#endif /* CARD_FLOW_CREATE_WALLET_H */
