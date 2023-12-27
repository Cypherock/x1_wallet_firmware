/**
 * @file    common_error.h
 * @author  Cypherock X1 Team
 * @brief   Get an initialized instance of common error.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <error.pb.h>

#include "app_error.h"

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
 * @brief Returns an initialized instance of error_common_error_t.
 * @details The error_code is uint32_t. The function works because size of union
 * is uint32_t.
 * NOTE: Any updates in error_common_error_t might effect the working of this
 * function.
 *
 * @param which_error The error type to initialize
 * @param error_code The error code to be filled
 * @return An instance of error_common_error_t filled with the provided values
 */
error_common_error_t init_common_error(pb_size_t which_error,
                                       uint32_t error_code);

/**
 * The function maps NFC status codes to specific card error codes.
 *
 * @param nfc_status An enumeration representing the status word returned by an
 * NFC communication with a smart card.
 *
 * @return an error_card_error_t value based on the input parameter nfc_status,
 * which is of type card_error_status_word_e. The returned value corresponds to
 * the specific error code associated with the input nfc_status.
 */
error_card_error_t get_card_error_from_nfc_status(
    card_error_status_word_e nfc_status);
#endif
