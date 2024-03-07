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
#include "apdu.h"
#include "buzzer.h"
#include "card_operation_typedefs.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/**
 * @brief
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
typedef struct NFC_connection_data {
  int8_t keystore_index;
  uint8_t acceptable_cards;
  uint8_t tapped_card;
  uint8_t retries;
  uint8_t family_id[FAMILY_ID_SIZE +
                    2];    // TODO: Review(need to find reason for extra byte)
  uint8_t card_key_id[4];
  bool pairing_error;
  uint8_t recovery_mode;
  uint8_t card_absent_retries;
  uint8_t *card_version;
  bool init_session_keys;
  ISO7816 status;
} NFC_connection_data;

typedef struct card_operation_data {
  NFC_connection_data nfc_data;
  const char *error_message; /** Error message to be displayed for user action
                                or error specification */
  card_error_type_e error_type;
} card_operation_data_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Initializes the applet on the card and loads session keys if encrypted
 * communication is required.
 * @details This API handles card applet initialization and populates card_data
 * object. Wheather in success or failure, the API populates
 * card_data.error_type to indicate result. If applet initialization
 * requirements are met, success is returned, else either @ref
 * CARD_OPERATION_RETAP_BY_USER_REQUIRED or
 * @ref CARD_OPERATION_ABORT_OPERATION error is returned. For retap errors a
 * message is populated to @ref card_data.error_message, while in abort error
 * cases, error message is set to core error using @ref mark_core_error_screen.
 * In case a P0 event occurs, @ref CARD_OPERATION_P0_OCCURED is returned.
 *
 * NOTE:
 * - Pre-requisite: An instruction screen should always be initialized before
 * calling this API
 * - Retap error is returned in only two cases, Wrong card number tapped or
 * wrong family of card tapped
 * - If encrypted communication is required, member of @ref NFC_connection_data
 * @ref init_session_keys must be set true.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the applet
 * initialization process.
 */
card_error_type_e card_initialize_applet(card_operation_data_t *card_data);

/**
 * @brief Handles the errors encountered during a card operation.
 * @details This API handles response from last card
 * action(card_data.nfc_data.status). Wheather success or failure, the API
 * populates and returns card_data.error_type to indicate result. The errors
 * returned by this handler are defined in @ref card_error_type_e. For retap
 * errors a message is populated to card_data.error_message, while in abort and
 * locked wallet error cases, error message is set to core error using @ref
 * mark_core_error_screen.
 *
 * NOTE:
 * - Pre-requisite: An instruction screen should always be initialized before
 * calling this API
 * - In case of CARD_OPERATION_CARD_REMOVED, appropriate text is set to the
 * instruction screen body.
 *
 * @param card_data Pointer to the data structure containing information about
 * the card operation.
 * @return card_error_type_e Type of error encountered during the card
 * operation.
 */
card_error_type_e card_handle_errors(card_operation_data_t *card_data);
#endif