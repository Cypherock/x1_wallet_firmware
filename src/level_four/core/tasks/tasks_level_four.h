/**
 * @file    tasks_level_four.h
 * @author  Cypherock X1 Team
 * @brief   Header for level four tasks.
 *          This file contains all the functions prototypes for level four
 * tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef TASKS_LEVEL_FOUR_H
#define TASKS_LEVEL_FOUR_H

#pragma once

#include "coin_utils.h"
#include "controller_main.h"
#include "eth.h"
#include "tasks.h"

extern evm_unsigned_txn eth_unsigned_txn_ptr;
extern uint8_t challenge_no[32];

/**
 * @brief This task is executed for processing and signing unsigned transaction
 *  of Ethereum.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void send_transaction_tasks_eth();

/**
 * @brief This task is executed for processing and signing unsigned transaction
 * of Ethereum.
 *
 */
void sign_message_tasks_eth();

/**
 * @brief This task is executed for processing and signing unsigned transaction.
 */
void send_transaction_tasks_solana();

/**
 * @brief This task is executed for generating address using xpub for Ethereum.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void receive_transaction_tasks_eth();

/**
 * @brief This task is executed for handling near registered accounts and
 * generating near implicit account.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void receive_transaction_tasks_near();

/**
 * @brief This task is executed for generating solana address using the public
 * key
 * @details
 *
 * @see
 * @since v1.0.0
 */
void receive_transaction_tasks_solana();

/**
 * @brief This task is executed for verifying wallet added or restored
 * on the device and cards.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void verify_wallet_tasks();

/**
 * @brief Device provision task handler
 * @details
 *
 * @see
 * @since v1.0.0
 */
void task_device_provision();

/**
 * @brief This task is executed when the name of a wallet is clicked
 * and it is locked.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void wallet_locked_task();

/**
 * @brief This task is executed for verifying card.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void verify_card_task();

/**
 * @brief This task is executed for upgrading cards.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void card_upgrade_task();

#endif