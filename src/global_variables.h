/**
 * @file    global_variables.h
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

#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include "onboarding_storage.h"
#include "controller_main.h"
#include "application_startup.h"
#include "communication.h"
#include "lvgl.h"
#include "tasks.h"
#include <stdbool.h>
#include "tasks_level_four.h"
#include "controller_level_four.h"

extern lv_task_t* success_task;
extern lv_task_t* timeout_task;
extern uint8_t device_auth_flag;
extern bool main_app_ready;
extern lv_task_t* listener_task;
extern lv_task_t* authentication_task;
extern Flow_level flow_level;
extern Counter counter;
extern Add_Coin_Data add_coin_data;
extern Receive_Transaction_Data receive_transaction_data;\
extern volatile uint8_t gBoolOnboardingComplete;
extern Flash_Wallet wallet_for_flash;

#ifndef PROVISIONING_FIRMWARE
extern Onboarding_StageData gOnboardingStg;
#endif /* PROVISIONING_FIRMWARE */

/**
 * @brief TODO: 
 * 
 */
extern const char *GIT_REV;
extern const char *GIT_TAG;
extern const char *GIT_BRANCH;


#endif /* #define GLOBAL_VARIABLES_H */
