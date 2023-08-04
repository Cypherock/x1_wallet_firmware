/**
 * @file    tasks_advanced_settings.h
 * @author  Cypherock X1 Team
 * @brief   Header for advanced settings tasks.
 *          This file contains the prototypes of the functions used to manage
 * the advanced settings tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef TASKS_ADVANCED_SETTINGS_H
#define TASKS_ADVANCED_SETTINGS_H

#pragma once

#include "controller_main.h"
#include "tasks.h"

/**
 * @brief This function is used to manage the pre-processing & rendering in
 * advanced settings tasks.
 * @details All the settings features are managed here. Some of the desktop
 * triggered requests are also managed here such as export of logs, firmware
 * upgrade, card verification along with card pairing.
 *
 * @see LEVEL_THREE_ADVANCED_SETTINGS_TASKS, initial_verify_card_task(),
 * verify_card_task(), task_device_provision()
 * @since v1.0.0
 */
void level_three_advanced_settings_tasks();

#endif