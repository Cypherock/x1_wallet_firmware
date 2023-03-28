/**
 * @file    tasks_old_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header for old wallet tasks.
 *          This file contains all the prototypes for the old wallet tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef TASKS_OLD_WALLET_H
#define TASKS_OLD_WALLET_H

#pragma once

#include "controller_main.h"

/**
 * @brief Handles all the possible operations on the old wallets.
 * @details This function handles all the possible operations on the old wallets
 * such as view seed, delete/unlock/verify wallet and other desktop triggered
 * operations such export wallet, send/receive transactions.
 *
 * @see view_seed_task(), delete_wallet_task(), wallet_locked_task(),
 * verify_wallet_task(), export_wallet_task(), add_coin_tasks(),
 * send_transaction_tasks(), receive_transaction_tasks(),
 * send_transaction_tasks_eth(), receive_transaction_tasks_eth(),
 * LEVEL_THREE_OLD_WALLET_TASKS, Flow_level.level_two,
 * level_three_old_wallet_controller_b() level_three_old_wallet_controller()
 * @since v1.0.0
 */
void level_three_old_wallet_tasks();

/**
 * @brief Handles the pre-processing & display rendering of delete wallet
 * operation.
 * @details This function handles the pre-processing of delete wallet operation.
 *
 * @see DELETE_WALLET_TASKS, tap_cards_for_delete_flow(), wallet.h,
 * delete_wallet_controller()
 * @since v1.0.0
 */
void delete_wallet_task();

/**
 * @brief Handles the pre-processing & display rendering of view seed operation.
 * @details This function handles the actual processing of seed retrieval to
 * view mnemonics.
 *
 * @see VIEW_SEED_TASKS, tap_threshold_cards_for_reconstruction(),
 * recover_secret_from_shares(), wallet.h, view_seed_controller()
 * @since v1.0.0
 */
void view_seed_task();

#endif