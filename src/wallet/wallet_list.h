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
uint8_t get_wallet_list(const char *wallet_list[]);

/**
 * @brief This API searches for wallet on the flash using wallet_id as key if it
 * is in VALID_WALLET state and not locked.
 * @details If a VALID_WALLET is found in the flash, then this API fills the
 * wallet_name, wallet_info, wallet_id fields of the Wallet structure
 *
 * @param wallet_id The wallet_id that needs to be searched
 * @param wallet Reference to Wallet structure which will be populated by the
 * function
 * @return true If the wallet corresponding to wallet_id is found in the flash
 * and is in VALID_WALLET state and not locked.
 * @return false If the wallet corresponding to wallet_id is not found or is not
 * in VALID_WALLET state and/or is locked.
 */
bool get_wallet_data_by_id(const uint8_t *wallet_id, Wallet *wallet);

/**
 * @brief This API searches for wallet on the flash using wallet_id as key and
 * returns the name of the wallet if it is in VALID_WALLET state and not locked.
 * @details If a VALID_WALLET is found in the flash, then this API fills the
 * wallet_name buffer if provided.
 *
 * @param wallet_id The wallet_id that needs to be searched
 * @param wallet_name The buffer in which wallet name will be filled or NULL if
 * wallet name is not required
 * @return true If the wallet corresponding to wallet_id is found in the flash
 * and is in VALID_WALLET state and not locked.
 * @return false If the wallet corresponding to wallet_id is not found or is not
 * in VALID_WALLET state and/or is locked.
 */
bool get_wallet_name_by_id(const uint8_t *wallet_id, uint8_t *wallet_name);
#endif /* WALLET_LIST_H */
