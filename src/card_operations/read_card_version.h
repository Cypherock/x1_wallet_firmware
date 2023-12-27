/**
 * @file    read_card_version.h
 * @author  Cypherock X1 Team
 * @brief   API to read card version details
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef READ_CARD_VERSION_H
#define READ_CARD_VERSION_H

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
 * @brief This card operation reads the card version from an X1 card
 * irrespective of it's pairing status
 *
 * @param card_version Pointer to buffer of size CARD_VERSION_SIZE which will be
 * filled by this function in case a valid X1 card is detected
 * @param heading The heading that needs to be displayed
 * @param msg The message prompt that needs to be displayed to the user
 * @return true If the operation was successful, and the card_version populated
 * with valid data
 * @return false If the operation was un-successful (eg: Card abort error or P0
 * error occurred)
 */
bool read_card_version(uint8_t *card_version,
                       const char *heading,
                       const char *msg);

#endif /* READ_CARD_VERSION_H */
