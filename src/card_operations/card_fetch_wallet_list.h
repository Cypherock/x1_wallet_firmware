/**
 * @file    card_fetch_wallet_list.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting function to fetch wallet list from X1 card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_WALLET_LIST_H
#define CARD_FETCH_WALLET_LIST_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "card_internal.h"
#include "card_operation_typedefs.h"
#include "wallet_list.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  card_operation_config_t operation;
  card_operation_frontend_t frontend;
} card_fetch_wallet_list_config_t;

typedef struct {
  wallet_list_t *wallet_list;
  card_info_t card_info;
} card_fetch_wallet_list_response_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This card operation extracts the wallet list from an X1 card
 *
 * @param config Reference to operation specific parameters such as acceptable
 * cards
 * @param response Reference to response structure which will be filled with the
 * wallet list and the card tapped
 * @return true If the operation was performed successfully
 * @return false If the operation did not complete
 */
card_error_type_e card_fetch_wallet_list(
    const card_fetch_wallet_list_config_t *config,
    card_fetch_wallet_list_response_t *response);

/**
 * @brief Fetches the wallet name associated with a given wallet ID.
 *
 * This function retrieves the list of wallets from the card and searches for
 * the wallet with the specified wallet ID. If found, it copies the wallet name
 * to the provided buffer.
 *
 * @param wallet_id Pointer to the wallet ID.
 * @param wallet_name Pointer to the buffer where the wallet name will be
 * copied.
 * @param reject_cb Callback to execute if wallet is not found or card abort
 * error occurs.
 * @return true if the wallet name is successfully fetched, false otherwise.
 */
bool card_fetch_wallet_name(const uint8_t *wallet_id,
                            char *wallet_name,
                            rejection_cb *reject_cb);
#endif /* CARD_FETCH_WALLET_LIST_H */
