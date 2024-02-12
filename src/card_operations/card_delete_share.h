/**
 * @file    card_delete_share.h
 * @author  Cypherock X1 Team
 * @brief   API for deleting wallet share from a card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_DELETE_SHARE_H
#define CARD_DELETE_SHARE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operation_typedefs.h"
#include "stdbool.h"
#include "stdint.h"
#include "wallet.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct card_delete_share_cfg {
  Wallet *wallet;
  uint8_t card_number;    // Card numbers decoded [1, 4]
} card_delete_share_cfg_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Deletes wallet share data from a card.
 * @details This function initializes the applet, deletes the wallet data on
 * the card, and updates wallet data on flash accordingly. It handles various
 * error cases and returns an appropriate error code. For special case such as
 * incorrect pin, it indicates the no. of attempts left.
 *
 * @param delete_config A pointer to the configuration of the card delete
 * operation.
 * @param handle_wallet_deleted_from_card Function pointer that needs to be
 * called to handle successful deletion of wallet on a card. The function takes
 * the delete_config as an argument.
 *
 * @return A card_error_type_e value representing the result of the operation.
 */
card_error_type_e card_delete_share(
    card_delete_share_cfg_t *delete_config,
    void (*handle_wallet_deleted_from_card)(card_delete_share_cfg_t *));
#endif
