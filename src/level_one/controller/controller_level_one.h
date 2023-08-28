/**
 * @file    controller_level_one.h
 * @author  Cypherock X1 Team
 * @brief   Header for level one controllers.
 *          Houses the declarations of controllers for level one tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef CONTROLLER_LEVEL_ONE_H
#define CONTROLLER_LEVEL_ONE_H

#include "communication.h"
#include "controller_main.h"
#include "flash_api.h"
#include "tasks.h"

/**
 * @brief Handles post event (only next events) processing for level one tasks
 * in the main application.
 * @details The level one controller for main is responsible for clearing out
 * error and desktop request flags (ref Flow_level.show_error_screen,
 * Flow_level.show_desktop_start_screen). Since it does post event processing,
 * it is responsible for performing state transitions based on the events so any
 * updates to flow_level and counter must happen here. In case of a wallet
 * selection, the information is copied to the wallet instance for application
 * to work with. The device state is also changed from ready to busy using
 * mark_device_state(). The controller also disables processing of new requests
 * from desktop app (i.e., the listener_task is disabled by setting its priority
 * to LV_TASK_PRIO_OFF) for safety.
 *
 * @see flow_level, counter, wallet, increase_level_counter(),
 * mark_device_state(), listener_task, LV_TASK_PRIO_OFF
 * @since v1.0.0
 *
 * @note New desktop requests won't be processed until the priority of
 * listener_task is set higher. This should happen in the level_one_task itself.
 */
void level_one_controller();

/**
 * @brief Handles post event (only back/cancel events) processing for level one
 * tasks.
 * @details This is the back controller for level one tasks and its events which
 * chains up to higher level back controllers based on the level set in
 * Counter.level.
 *
 * @see flow_level, counter, level_two_controller_b()
 * @since v1.0.0
 */
void level_one_controller_b();
#endif