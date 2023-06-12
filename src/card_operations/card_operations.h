/**
 * @file    card_operations.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting card operations
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_OPERATIONS_H
#define CARD_OPERATIONS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_fetch_share.h"
#include "card_pair.h"
#include "card_read_verify_share.h"
#include "card_return_codes.h"
#include "card_sign.h"
#include "card_write_share.h"
#include "check_pairing.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define CARD_HANDLE_P0_EVENTS(p0_event)                                        \
  do {                                                                         \
    if (true == (p0_event).flag) {                                             \
      return CARD_OPERATION_P0_OCCURED;                                        \
    }                                                                          \
  } while (0)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* CARD_OPERATIONS_H */