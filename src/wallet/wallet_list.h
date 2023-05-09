/**
 * @file    wallet_list.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef WALLET_LIST_H
#define WALLET_LIST_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <inttypes.h>
#include <stdbool.h>

#include "flash_api.h"

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
 * @brief This API returns the total number of wallets that exists on the
 * device. Additionally, it fills the array of char * with the wallet name
 *
 * @param wallet_list Array of char * which needs to be filled by the API
 * @return uint8_t Number of wallets which exist on the device
 */
uint8_t get_wallet_list(char *wallet_list[]);

#endif /* WALLET_LIST_H */
