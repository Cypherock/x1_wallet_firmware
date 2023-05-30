/**
 * @file    card_internal.h
 * @author  Cypherock X1 Team
 * @brief   Card internal operations
 * Exports all card APIs that enable card initialization and error handling
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_INTERNAL_H
#define CARD_INTERNAL_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operations.h"

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
 * @brief Initializes the applet on the card and performs necessary operations
 * for selecting the applet.
 * @details This API returns success if there is no error in applet
 * initialization and pairing requirements are ment. When an error is detected,
 * the API copies the corresponding error type to @ref error_type member of @ref
 * card_data object and returns it. Member of @ref card_data object @ref
 * error_message is also set with correspondig string in cases where error_type
 * is either @ref CARD_OPERATION_RETAP_BY_USER_REQUIRED or @ref
 * CARD_OPERATION_ABORT_OPERATION.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the applet
 * initialization process.
 */
card_error_type_e card_initialize_applet(card_operation_data_t *card_data);

/**
 * @brief Handles the errors encountered during a card operation.
 * @details If an error is detected,  the API copies the corresponding error
 * type to @ref error_type member of @ref card_data object and returns it.
 * Member of @ref card_data object @ref error_message is also set with
 * correspondig string in cases where error_type is either
 * CARD_OPERATION_RETAP_BY_USER_REQUIRED or CARD_OPERATION_ABORT_OPERATION.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the card
 * operation.
 */
card_error_type_e card_handle_errors(card_operation_data_t *card_data);
#endif