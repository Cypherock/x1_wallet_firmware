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
 * @brief Initializes the applet on the card.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the applet
 * initialization process.
 */
card_error_type_e card_initialize_applet(card_operation_data_t *card_data);

/**
 * @brief Handles the errors encountered during a card operation.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the card
 * operation.
 */
card_error_type_e card_handle_errors(card_operation_data_t *card_data);
#endif