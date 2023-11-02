/**
 * @file    verify_wallet_flow.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the verify an unverified wallet
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef VERIFY_WALLET_FLOW_H
#define VERIFY_WALLET_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
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
 * @brief This function is used to verify a wallet in unverified state.
 *
 * @param flash_wallet pointer to ram instance of the selected wallet.
 */
void verify_wallet_flow(const Flash_Wallet *flash_wallet);

#endif /* VERIFY_WALLET_FLOW_H */
