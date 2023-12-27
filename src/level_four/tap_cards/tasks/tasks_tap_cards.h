/**
 * @file    tasks_tap_cards.h
 * @author  Cypherock X1 Team
 * @brief   Header for tap card tasks.
 *          This file contains all the functions prototypes for the tap card
 * tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef TASKS_TAP_CARDS_H
#define TASKS_TAP_CARDS_H

/**
 * @brief Task for writing on the card flow , used to add wallet or restore
 * wallet on cards
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
void tap_cards_for_write_flow();

/**
 * @brief Task for verification of the card
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
void tap_cards_for_verification_flow();

/**
 * @brief Task to retrieve wallet from card
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
void tap_threshold_cards_for_reconstruction();

/**
 * @brief Task to update card id
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
void tasks_update_card_id();

/**
 * @brief Task to retrieve key from  id
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
void retrieve_key_from_card();

#endif