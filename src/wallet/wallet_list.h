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
#include <error.pb.h>
#include <stdbool.h>
#include <stdint.h>

#include "flash_api.h"
#include "wallet.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  uint8_t info;
  uint8_t locked;
  uint8_t name[NAME_SIZE];
  uint8_t id[WALLET_ID_SIZE];
} wallet_metadata_t;

typedef struct {
  uint8_t count;
  wallet_metadata_t wallet[MAX_WALLETS_ALLOWED];
} wallet_list_t;

typedef void(rejection_cb)(pb_size_t which_error, uint32_t error_code);

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
 * @brief This API fills metadata for all the wallets present on X1 vault and
 * are not in VALID_WALLET_WITHOUT_DEVICE_SHARE state.
 *
 * @param wallet_list Refernce to buffer which will be filled by this function
 * @return uint8_t The number of wallets returned
 */
uint8_t get_filled_wallet_meta_data_list(wallet_list_t *wallet_list);

/**
 * @brief This API searches for wallet on the flash using wallet_id as key and
 * fills the wallet structure if it is in usable state.
 * @details In case if the wallet is not in usable state or is not found on the
 * device, the function executes the provided rejection callback if it exists to
 * the host app.
 *
 * @param wallet_id The wallet_id that needs to be searched
 * @param wallet Reference to Wallet structure which will be populated by the
 * @param reject_cb Callback to execute if wallet is not found or is not in
 * usable state function
 * @return true If the wallet corresponding to wallet_id is found in the flash
 * and is in usable state
 * @return false If the wallet corresponding to wallet_id is not found or is not
 * in usable state.
 */
bool get_wallet_data_by_id(const uint8_t *wallet_id,
                           Wallet *wallet,
                           rejection_cb *reject_cb);

/**
 * @brief This API searches for wallet on the flash using wallet_id as key and
 * returns the name of the wallet if it is in usable state.
 * @details In case if the wallet is not in usable state or is not found on the
 * device, the function executes the provided rejection callback if it exists to
 * the host app.
 *
 * @param wallet_id The wallet_id that needs to be searched
 * @param wallet_name The buffer in which wallet name will be filled or NULL if
 * wallet name is not required
 * @param reject_cb Callback to execute if wallet is not found or is not in
 * usable state function
 * @return true If the wallet corresponding to wallet_id is found in the flash
 * and is in usable state
 * @return false If the wallet corresponding to wallet_id is not found or is not
 * in usable state.
 */
bool get_wallet_name_by_id(const uint8_t *wallet_id,
                           uint8_t *wallet_name,
                           rejection_cb *reject_cb);
#endif /* WALLET_LIST_H */
