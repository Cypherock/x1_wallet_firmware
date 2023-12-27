/**
 * @file    card_flow_pairing.h
 * @author  Cypherock X1 Team
 * @brief   Header file for card pair flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FLOW_PAIRING_H
#define CARD_FLOW_PAIRING_H

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
 * @brief This card flow allows pairing of any unpaired X1 card with the X1
 * vault
 *
 * @param number_of_cards_paired Reference to a variable which will be filled by
 * the function. It will represent the number of cards paired by the flow
 * @return true If the flow execution occurred successfully
 * @return false If any CARD_OPERATION_CARD_ABORT error occurred or any P0 event
 * occurred during the card flow
 */
bool card_flow_pairing(uint8_t *cards_paired);
#endif /* CARD_FLOW_PAIRING_H */
