/**
 * @file    card_return_code.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining error codes for card operations
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_RETURN_CODE_H
#define CARD_RETURN_CODE_H

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
  CARD_OPERATION_P0_OCCURED,            /** P0 event occured */

  CARD_OPERATION_RETAP_BY_USER_REQUIRED, /** Errors that occur due to user
                                            mistakes, like wrong card number or
                                            card of wrong family tapped */
  CARD_OPERATION_ABORT_OPERATION, /** Error occurring due to internal handling
                                     of NFC or card communication. These errors
                                     can be associated to @ref ISO7816 errors or
                                     PN532 errors */

  CARD_OPERATION_DEFAULT_INVALID = 0xFF, /** Default invalid value */
} card_error_type_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* CARD_RETURN_CODES_H */