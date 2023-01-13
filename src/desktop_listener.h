/**
 * @file    desktop_listener.h
 * @author  Cypherock X1 Team
 * @brief   Main controller.
 *          This file defines global variables and functions used by other flows.
 * @details
 * This is main file for controller module.
 *
 * Controller module is divided into levels which are traversed recursively.
 *
 * Each state of the device is uniquely represented by two variables counter
 * flow_level.
 *
 * Each level has a task file and a controller file. The task file contains
 * task such as showing a particular screen. The controller file decides which
 * task it to be executed next. Sometimes the controller file needs to take decision
 * based on the input by user as to which screen needs to be shown next.
 *
 * The change of global Flow_level and Counter variable must be done in controller files
 * if possible.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef DESKTOP_LISTENER_H
#define DESKTOP_LISTENER_H

#include "lvgl.h"

/**
 * @brief Callback function called periodically to check for success message from desktop.
 * @details This is a generic success listener for the application to work with while waiting for status from the
 * desktop app. The functions looks for any STATUS_PACKET data and only handles STATUS_CMD_ABORT or STATUS_CMD_SUCCESS.
 * In both the cases, the types of STATUS_PACKET, the function resets flow level and de-registers the lv_tasks success_task
 * and timeout_task. Additionally, if the request is for STATUS_CMD_ABORT it displays a message
 * (ref ui_text_operation_has_been_cancelled) to the user. <br/>
 * If any STATUS_PACKET is received, it is cleared from the application buffer regardless of it is processed by this
 * function or not.
 *
 * @param [in] task lv_task passed while registering the callback.
 *
 * @see STATUS_PACKET, STATUS_CMD_ABORT, STATUS_CMD_SUCCESS, ui_text_operation_has_been_cancelled, reset_flow_level(),
 * success_task, timeout_task, mark_error_screen(), mark_event_over(), clear_message_received_data()
 * @since v1.0.0
 */
void _success_listener(lv_task_t* task);

/**
 * @brief Checks the messages from desktop and initiates/processes the request sent by desktop.
 * @details The function is an lv_task handler registered with lvgl with a priority of LV_TASK_PRIO_HIGH and period of
 * 20 ms with NULL user data. This is responsible for processing desktop requests and performing/triggering flow jumps
 * based on the request from desktop. The function triggers a confirmation message which is shown to the user for
 * confirmation before actually starting the requested flow. It has the capacity to respond to certain requests directly
 * without user consent (such as DEVICE_INFO, START_CARD_AUTH, START_DEVICE_PROVISION, START_DEVICE_AUTHENTICATION).
 *
 * @param task lv_task passed while registering the callback.
 *
 * @see listener_task, flow_level, counter, En_command_type_t, get_usb_msg(), clear_message_received_data(), transmit_one_byte(),
 * transmit_data_to_app(), CY_Set_External_Triggered()
 * @since v1.0.0
 *
 * @note
 */
void desktop_listener_task(lv_task_t* data);

#ifndef PROVISIONING_FIRMWARE
/**
 * @brief Checks the messages from desktop and initiates/processes restricted requests sent by desktop.
 * @details The function is an lv_task handler registered with lvgl with a priority of LV_TASK_PRIO_MID.
 * It decodes the request from desktop and allows restricted requests. Allowed requests are handled further by
 * invoking the desktop_listener_task(lv_task_t* data) function directly.
 * @param task lv_task passed while registering the callback.
 * @see 
 * @since
 *
 * @note
 */
void __authentication_listener(lv_task_t* task);
#endif /* PROVISIONING_FIRMWARE */

#endif /* DESKTOP_LISTENER_H */
