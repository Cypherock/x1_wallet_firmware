/**
 * @file    card_fetch_data.h
 * @author  Cypherock X1 Team
 * @brief   API for deleting wallet share from a card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_DATA_H
#define CARD_FETCH_DATA_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operation_typedefs.h"
#include "stdbool.h"
#include "stdint.h"
#include "wallet.h"

#define PLAIN_DATA_SIZE 200
#define ENCRYPTED_DATA_SIZE (PLAIN_DATA_SIZE * 2)

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
#pragma pack(push, 1)
typedef struct {
  uint8_t plain_data[PLAIN_DATA_SIZE];
  uint16_t plain_data_size;
  bool is_private;
  uint8_t encrypted_data[ENCRYPTED_DATA_SIZE];
  uint16_t encrypted_data_size;
} SessionMsg;
#pragma pack(pop)

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
card_error_type_e card_fetch_encrypt_data(uint8_t *wallet_id,
                                          SessionMsg *msgs,
                                          size_t msg_array_size);
#endif
