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
 * @brief This API executes a new wallet creation flow on the device
 * @details This function generate random mnemonics, takes user inputs for
 * wallet configuration, writes the wallet shares on the X1 vault flash and X1
 * cards and verifies each share
 */
void create_new_wallet_flow(void);

#endif /* CREATE_WALLET_FLOW_H */
