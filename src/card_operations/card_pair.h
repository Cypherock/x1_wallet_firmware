/**
 * @file    card_pair.h
 * @author  Cypherock X1 Team
 * @brief   Pair card handler
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_PAIR_H
#define CARD_PAIR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operations.h"
#include "stdbool.h"
#include "stdint.h"

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
 * This function pairs a specific card without handling of retap errors.
 *
 * @param card_number The number of the card to be paired. It should be an
 * integer between 1 and 4.
 * @param status A pointer to a uint32_t variable that will be used to store the
 * error status of the function. If the function returns
 * CARD_OPERATION_ABORT_OPERATION, the error status will be stored in this
 * variable. If this parameter is NULL, the error status will not be stored.
 *
 * @return a value of type `card_error_type_e`.
 */
card_error_type_e card_pair_without_retap(uint8_t card_number,
                                          uint32_t *status);

/**
 * This function performs a card pairing operation and returns an error status.
 *
 * @param card_number The number of the card to be paired. It should be an
 * integer between 1 and 4.
 * @param heading A string that represents the heading of the card pairing
 * operation. It is displayed on the screen to provide context to the user.
 * @param message A pointer to a character array (string) containing the message
 * to be displayed on the screen during the card pairing operation.
 *
 * @return a value of type `card_error_type_e`.
 */
card_error_type_e card_pair_operation(uint8_t card_number,
                                      char *heading,
                                      const char *message);
#endif