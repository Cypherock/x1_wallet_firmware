/**
 * @file    restore_seed_phrase_flow.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the import seed phrase flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef RESTORE_SEED_PHRASE_FLOW_H
#define RESTORE_SEED_PHRASE_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "stdbool.h"
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
 * @brief This API executes the seed phrase import flow on the device
 * @details This function inputs seed phrase from user, takes user inputs for
 * wallet configuration, writes the wallet shares on the X1 vault flash and X1
 * cards and verifies each share
 *
 * @param wallet_name buffer to copy wallet name of the created wallet. Can be
 * used to handle wallet creation failure by the caller
 *
 * @return true if wallet creation was completed successfully else false
 */
bool restore_seed_phrase_flow(char *wallet_name);

#endif /* RESTORE_SEED_PHRASE_FLOW_H */
