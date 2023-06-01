/**
 * @file    card_operations.h
 * @author  Cypherock X1 Team
 * @brief   Card operations
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

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum card_errors_type {
  CARD_OPERATION_SUCCESS = 0,
  CARD_OPERATION_CARD_REMOVED,     /** When card is removed before operation
                                      completion*/
  CARD_OPERATION_PAIRING_REQUIRED, /** Returned after card is unpaired */
  CARD_OPERATION_LOCKED_WALLET,    /** Locked wallet detected during wallet
                                      operation */
  CARD_OPERATION_INCORRECT_PIN_ENTERED, /** Incorrect pin entered */
  CARD_OPERATION_P0_ABORT_OCCURED,      /** Abort command received from host */
  CARD_OPERATION_P0_TIMEOUT_OCCURED,    /** Timeout occured */

  /** The API setting error using below enums is required to also set an error
     message for corresponding user action or error specification */
  CARD_OPERATION_RETAP_BY_USER_REQUIRED, /** Error that can be resolved with a
                                            user action involving retapping of a
                                            card. */
  CARD_OPERATION_ABORT_OPERATION, /** Error that cannot be resolved by user
                                     action and operation should be aborted. */

  CARD_OPERATION_DEFAULT_INVALID = 0xFF, /** Default invalid value */
} card_error_type_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif