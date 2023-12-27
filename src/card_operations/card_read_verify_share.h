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
#include <stdbool.h>
#include <stdint.h>

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
 * @return true If the process was completed successfully
 * @return false If the process could not be completed
 */
bool read_card_share(uint8_t card_num, const char *heading, const char *msg);

#endif /* READ_CARD_SHARE_H */
