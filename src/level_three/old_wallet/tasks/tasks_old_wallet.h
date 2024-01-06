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
 * @see view_seed_task(), wallet_locked_task(),
 * verify_wallet_task(),
 * send_transaction_tasks_eth(), receive_transaction_tasks_eth(),
 * LEVEL_THREE_OLD_WALLET_TASKS, Flow_level.level_two,
 * level_three_old_wallet_controller_b() level_three_old_wallet_controller()
 * @since v1.0.0
 */
void level_three_old_wallet_tasks();

#endif