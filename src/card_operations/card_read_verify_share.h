/**
 * @file    card_read_verify_share.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting APIs to support wallet share read from X1
 *          cards.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef READ_CARD_SHARE_H
#define READ_CARD_SHARE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "card_return_codes.h"

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
 * @brief This API provides read backs wallet share from an X1 card
 * @details
 *
 * @param card_num The X1 card number to read the share from
 * @param heading The heading of the instruction to be shown on the screen
 * @param msg The message to be shown on the screen
 * @return card_error_type_e Error code depicting the status of card operation.
 * If the share was successfully read back, then CARD_OPERATION_SUCCESS is
 * returned
 */
card_error_type_e read_card_share(uint8_t card_num,
                                  const char *heading,
                                  const char *msg);

#endif /* READ_CARD_SHARE_H */
