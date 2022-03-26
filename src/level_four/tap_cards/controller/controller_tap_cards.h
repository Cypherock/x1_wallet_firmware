/**
 * @file    controller_tap_cards.h
 * @author  Cypherock X1 Team
 * @brief   Header for tap card controllers.
 *          This file contains the declaration of the tap card controllers.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */

#ifndef CONTROLLER_TAP_CARDS_H
#define CONTROLLER_TAP_CARDS_H

#include "controller_main.h"
#include "nfc.h"
#include "buzzer.h"
#include "sha2.h"

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
    uint8_t desktop_control;
    uint8_t active_cmd_type;
    int8_t keystore_index;
    uint8_t acceptable_cards;
    uint8_t tapped_card;
    uint8_t lvl3_retry_point;
    uint8_t lvl4_retry_point;
    uint8_t resume_point;
    uint8_t retries;
    uint8_t family_id[FAMILY_ID_SIZE + 2];
    uint8_t card_key_id[4];
    ISO7816 status;
} NFC_connection_data;

extern NFC_connection_data tap_card_data;

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
bool tap_card_applet_connection();

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
void tap_card_pair_card_controller();

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
void tap_cards_for_write_flow_controller();

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
void tap_cards_for_verification_flow_controller();

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
void tap_a_card_and_sync_controller();

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
void controller_read_card_id();

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
