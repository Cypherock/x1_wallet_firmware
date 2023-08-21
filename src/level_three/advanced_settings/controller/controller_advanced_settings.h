/**
 * @file    controller_advanced_settings.h
 * @author  Cypherock X1 Team
 * @brief   Header for advanced settings controller.
 *          This file contains the prototypes of the functions used to control
 * the advanced settings.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONTROLLER_ADV_SETTINGS_H
#define CONTROLLER_ADV_SETTINGS_H

#pragma once

#include "controller_main.h"
#include "tasks.h"

/**
 * @brief Controller for post-event processing of advanced settings tasks.
 * @details This function is used to control the flow of the advanced settings
 * controller and is responsible for transition from one state to an appropriate
 * state based on the user's input (as communication via lvgl's UI components).
 *
 * @see LEVEL_THREE_ADVANCED_SETTINGS_TASKS, initial_verify_card_controller(),
 * level_three_advanced_settings_tasks(),
 * get_usb_msg_by_cmd_type(), device_provision_controller(),
 * tap_card_pair_card_controller()
 * @since v1.0.0
 */
void level_three_advanced_settings_controller();

#endif