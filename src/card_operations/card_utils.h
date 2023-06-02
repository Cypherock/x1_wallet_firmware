/**
 * @file    card_utils.h
 * @author  Cypherock X1 Team
 * @brief   Card operations common utilities
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_UTILS_H
#define CARD_UTILS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operations.h"
#include "controller_tap_cards.h"
#include "stdbool.h"

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
 * @brief Initializes an NFC connection data struct with the provided
 * parameters.
 *
 * @param family_id Pointer to the family ID of the NFC connection.
 * @param acceptable_cards Number of acceptable cards for the NFC connection.
 *
 * @return The initialized NFC connection data.
 */
NFC_connection_data init_nfc_connection_data(const uint8_t *family_id,
                                             uint8_t acceptable_cards);

/**
 * @brief Displays an error message to the user and wait till user preses
 * tickmark or P0 occurs.
 *
 * @param error_message The error message to display.
 *
 * @return The type of error encountered during display, or CARD_ERROR_NONE if
 * successful.
 *
 * @details This function displays the provided error message to the user. If an
 * error occurs during display, the function returns the type of error
 * encountered.
 */
card_error_type_e display_error_message(const char *error_message);

/**
 * @brief Gets the serial number of an NFC card.
 * @details This function retrieves the serial number of an NFC card using the
 * provided NFC connection data and stores it in the provided buffer.
 *
 * @param nfc_data Pointer to the NFC connection data.
 * @param serial Pointer to the buffer to store the card serial number.
 */
void get_card_serial(NFC_connection_data *nfc_data, uint8_t *serial);

/**
 * @brief Waits for the removal of an NFC card or P0 event.
 *
 * @return CARD_OPERATION_SUCCESS for success, else error
 */
card_error_type_e wait_for_card_removal(void);
#endif
