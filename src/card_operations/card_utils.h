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
 * @brief Indicates an error during the card tap operation to the user and wait
 * till user acknowledges the error or P0 occurs.
 * @details This function displays the provided error message to the user and
 * beeps the buzzer. If an error occurs during display, the function returns the
 * type of error encountered.
 *
 * @param error_message The error message to display.
 *
 * @return The type of error encountered during display, or
 * CARD_OPERATION_SUCCESS if successful.
 */
card_error_type_e indicate_card_error(const char *error_message);

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

/**
 * @brief Indicates an incorrect PIN error to the user.
 * @details This function checks if the error is due to an incorrect PIN and
 * indicates the error to user with remaining pin attempts.
 *
 * @param status The card_error_status_word_e status value representing the
 * error.
 * @return The type of error encountered during display, or
 * CARD_OPERATION_SUCCESS if successful.
 */
card_error_type_e indicate_wrong_pin(card_error_status_word_e status);
#endif
