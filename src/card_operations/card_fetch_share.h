/**
 * @file    card_fetch_share.h
 * @author  Cypherock X1 Team
 * @brief   API for fetching wallet share from a card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_SHARE_H
#define CARD_FETCH_SHARE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operation_typedefs.h"
#include "stdbool.h"
#include "stdint.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  uint8_t xcor;    /// xcor for share index for wallet reconstruction
  card_operation_config_t operation;
  card_operation_frontend_t frontend;
} card_fetch_share_config_t;

typedef struct {
  card_info_t card_info;
} card_fetch_share_response_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Fetches wallet share data from a card.
 * @details This function initializes the applet, retrieves the wallet data from
 * the card, and shares the retrieved wallet data. It handles various error
 * cases and returns an appropriate error code. For special case such as
 * incorrect pin, it indicates the no. of attempts left.
 *
 * @param config A pointer to the configuration of the card fetch and share
 * operation.
 * @param response Pointer to buffer where response will be filled
 * @return A card_error_type_e value representing the result of the operation.
 */
card_error_type_e card_fetch_share(const card_fetch_share_config_t *config,
                                   card_fetch_share_response_t *response);
#endif
