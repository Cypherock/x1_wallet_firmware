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
bool pair_card_operation(uint8_t card_number, char *heading, char *message);
#endif