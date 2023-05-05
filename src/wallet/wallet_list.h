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
 * @brief Get the wallet list object
 *
 * @param wallet_list
 * @param filter
 * @return uint8_t
 */
uint8_t get_wallet_list(char **wallet_list);

#endif /* WALLET_LIST_H */
