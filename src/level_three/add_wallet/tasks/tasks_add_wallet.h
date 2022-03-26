/**
 * @file    tasks_add_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header for add wallet tasks.
 *          This file contains all the functions prototypes for the add wallet tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
/**
 * @file task_new_wallet.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-05-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef TASKS_ADD_WALLET_H
#define TASKS_ADD_WALLET_H

#include "controller_main.h"
#include "tasks.h"

/**
 * @brief Task to generate new wallet
 * @details This function will guide the user to generate a new wallet and store it in the X1Cards. The task handler helps
 * take the user input for wallet configuration. Based on the settings of the device, the option to enable/disable the
 * passphrase will be displayed. The task handler is also responsible for a quick check to make sure the user has
 * temporary knowledge of the seed phrase to aid in double check of the validity of data on cards.
 *
 * @see GENERATE_WALLET_TASKS, tap_cards_for_write_flow(), generate_wallet_random_word()
 * @since v1.0.0
 */
void tasks_add_new_wallet();

/**
 * @brief Task to restore wallet
 * @details This function will guide the user to restore a wallet from the X1Cards. The task handler helps take the user input
 * for wallet configuration. Based on the settings of the device, the option to enable/disable the passphrase will be displayed.
 *
 * @see
 * @since v1.0.0
 */
void tasks_restore_wallet();

/**
 * @brief Task to create a wallet to add arbitrary data
 * @details
 *
 * @see
 * @since v1.0.0
 */
void tasks_arbitrary_data();

#endif