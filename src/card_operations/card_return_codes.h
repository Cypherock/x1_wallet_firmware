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
#include <stdbool.h>
#include <stdint.h>

#include "app_error.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define ACCEPTABLE_CARDS_ALL 15

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum card_errors_type {
  CARD_OPERATION_SUCCESS = 0,
  CARD_OPERATION_CARD_REMOVED,  /** When card is removed before operation
                                   completion*/
  CARD_OPERATION_LOCKED_WALLET, /** Locked wallet detected during wallet
                                   operation */
  CARD_OPERATION_INCORRECT_PIN_ENTERED, /** Incorrect pin entered */
  CARD_OPERATION_P0_OCCURED,            /** P0 event occured */

  CARD_OPERATION_RETAP_BY_USER_REQUIRED, /** Errors that occur due to user
                                            mistakes, like wrong card number or
                                            card of wrong family tapped */
  CARD_OPERATION_ABORT_OPERATION, /** Error occurring due to internal handling
                                     of NFC or card communication. These errors
                                     can be associated to @ref
                                     card_error_status_word_e errors or PN532
                                     errors */

  CARD_OPERATION_DEFAULT_INVALID = 0xFF, /** Default invalid value */
} card_error_type_e;

typedef struct {
  uint8_t acceptable_cards;
  bool skip_card_removal;
} card_config_t;

typedef struct {
  uint32_t status;
  uint8_t tapped_card;
  uint8_t recovery_mode;
} card_info_t;

typedef struct {
  const char *heading;
  const char *msg;
} card_operation_frontent_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* CARD_RETURN_CODES_H */