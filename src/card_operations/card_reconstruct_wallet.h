/**
 * @file    card_reconstruct_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Reconstruction of wallets from cards apis
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_RECONSTRUCT_WALLET_H
#define CARD_RECONSTRUCT_WALLET_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_return_codes.h"
#include "stdbool.h"
#include "stdint.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct card_fetch_share_cfg {
  uint8_t xcor;               /// xcor for share index for wallet reconstruction
  uint8_t remaining_cards;    /// Cards remaining to be tapped by user
  bool skip_card_removal;
  const char *heading;
  const char *message;
} card_fetch_share_cfg_t;

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
 * @return A card_error_type_e value representing the result of the operation.
 */
card_error_type_e card_fetch_share(card_fetch_share_cfg_t *config);
#endif
