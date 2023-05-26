/**
 * @file    card_operations.h
 * @author  Cypherock X1 Team
 * @brief   Card operations module
 *          Exports all card APIs to application
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_OPERATIONS_H
#define CARD_OPERATIONS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "controller_tap_cards.h" /* TODO: Remove the include and move required struct with cleanup*/

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum card_errors_type {
  CARD_OPERATION_SUCCESS = 0,
  CARD_OPERATION_CARD_REMOVED,
  CARD_OPERATION_PAIRING_REQUIRED,
  CARD_OPERATION_LOCKED_WALLET,
  CARD_OPERATION_INCORRECT_PIN_ENTERED,
  CARD_OPERATION_P0_ABORT_OCCURED,
  CARD_OPERATION_P0_TIMEOUT_OCCURED,
  CARD_OPERATION_RETAP_BY_USER_REQUIRED,
  CARD_OPERATION_ABORT_OPERATION,

  CARD_OPERATION_DEFAULT_INVALID = 0xFF,
} card_error_type_e;

typedef struct card_operation_data {
  NFC_connection_data nfc_data;
  const char *error_message;
  card_error_type_e error_type;
} card_operation_data_t;
/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif