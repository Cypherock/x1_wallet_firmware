/**
 * @file    controller_tap_cards.h
 * @author  Cypherock X1 Team
 * @brief   Header for tap card controllers.
 *          This file contains the declaration of the tap card controllers.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONTROLLER_TAP_CARDS_H
#define CONTROLLER_TAP_CARDS_H

#include "buzzer.h"
#include "controller_main.h"
#include "nfc.h"
#include "sha2.h"
#include "stdbool.h"

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
  uint8_t desktop_control;    // TODO: Remove deprecated
  uint8_t active_cmd_type;    // TODO: Remove deprecated
  int8_t keystore_index;
  uint8_t acceptable_cards;
  uint8_t tapped_card;
  uint8_t lvl3_retry_point;    // TODO: Remove deprecated
  uint8_t lvl4_retry_point;    // TODO: Remove deprecated
  uint8_t resume_point;        // TODO: Remove deprecated
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

extern NFC_connection_data tap_card_data;

/**
 * @brief Identifies a JavaCard in the field and triggers handshake and X1
 * Applet invocation.
 * @details The function handles the JavaCard identification and handshake with
 * the X1 Applet. It also identifies certain states of the card and triggers the
 * appropriate action if possible. Other critical errors are handled and
 * reported in the UI.<br/> This function is responsible for initializing the
 * NFC SecureChannel session with the appropriate IV and key.<br/> For scenarios
 * like using a X1 Card from a different set or not using the requested card,
 * the function informs the user about the situation and continues to retry.
 * This happens without any limit on the retry attempts.<br/> If the X1 Card is
 * absent right after the handshake, then the user is asked to tap and hold till
 * a feedback is received from the buzzer.
 *
 * @return bool true, false
 * @retval true if the card is identified and the handshake is successful.
 * @retval false if the card is not identified or the handshake fails.
 *
 * @see NFC_connection_data, nfc_select_card(), nfc_select_applet(),
 * tap_card_handle_applet_errors()
 * @since v1.0.0
 *
 * @note The tap_card_data structure needs to be initialized accordingly before
 * calling this function.
 */
bool tap_card_applet_connection();

/**
 * @brief A common handler for the X1 Applet and the NFC communication errors.
 * @details This is a one place error handler. The application can make a call
 * to this for handling the errors, if the application does not receive an
 * expected response from the X1 Applet.<br/> The function checks the status
 * member of tap_card_data for the error code. This function is supposed to be
 * called after making a call to any of the nfc functions. Based on the status,
 * the function performs the appropriate action which can be either displaying
 * a message to the user, or exiting the process, or retrying or a combination
 * of these.<br/> For certain types of errors, the function decrements its retry
 * counter. If the counter is zero, then the function exits the process and the
 * non-critical error is now considered as critical which is indicated by true.
 *
 * @return boolean true, false
 * @retval true if the error is handled. Application need not retry.
 * @retval false if the error is not critical. Application can retry right away.
 *
 * @see NFC_connection_data, nfc_select_card(), nfc_select_applet(),
 * tap_card_applet_connection()
 * @since v1.0.0
 *
 * @note The tap_card_data structure should be used to store the errors.
 * Otherwise the behavior is undefined.
 */
bool tap_card_handle_applet_errors();

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void tap_cards_for_write_and_verify_flow_controller();

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void tap_threshold_cards_for_reconstruction_flow_controller(uint8_t threshold);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void delete_from_cards_controller();

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void controller_update_card_id();

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void retrieve_key_from_card_flow_controller();

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void tap_card_take_to_pairing();

#endif
