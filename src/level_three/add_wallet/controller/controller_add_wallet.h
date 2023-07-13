/**
 * @file    controller_add_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header for add wallet controllers.
 *          Houses the declarations of controllers for add wallet tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONTROLLER_ADD_WALLET_H
#define CONTROLLER_ADD_WALLET_H

#include "controller_main.h"

/**
 * @brief Back button controller for level three add wallet flow
 * @details Handles the back/cancel event processing for the generate wallet
 * flow.
 *
 * @see wallet.h, wallet_for_flash, wallet_shamir_data, wallet_credential_data,
 * generate_wallet_controller()
 * @since v1.0.0
 */
void generate_wallet_controller_b();

/**
 * @brief Back button controller for level three restore wallet flow
 * @details Handles the back/cancel event processing for the restore wallet
 * flow.
 *
 * @see restore_wallet_controller(),
 * restore_wallet_enter_mnemonics_controller_b()
 * @since v1.0.0
 */
void restore_wallet_controller_b();

/**
 * @brief Controller to add arbitrary data wallet flow
 * @details
 *
 * @see
 * @since v1.0.0
 */
void arbitrary_data_controller();

/**
 * @brief Back button controller for arbitrary data wallet flow
 * @details
 *
 * @see
 * @since v1.0.0
 */
void arbitrary_data_controller_b();

#endif