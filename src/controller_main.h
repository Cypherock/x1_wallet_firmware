/**
 * @file    controller_main.h
 * @author  Cypherock X1 Team
 * @brief   Main controller.
 *          This file defines global variables and functions used by other
 * flows.
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
 * task it to be executed next. Sometimes the controller file needs to take
 * decision based on the input by user as to which screen needs to be shown
 * next.
 *
 * The change of global Flow_level and Counter variable must be done in
 * controller files if possible.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef CONTROLLER_MAIN_H
#define CONTROLLER_MAIN_H

#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "assert_conf.h"
#include "communication.h"
#include "constant_texts.h"
#include "flash_api.h"
#include "logger.h"
#include "lvgl.h"
#include "sha2.h"
#include "sys_state.h"
#include "tasks.h"
#include "utils.h"
#include "wallet.h"

#ifdef DEV_BUILD
#define SKIP_ENTER_MNEMONICS_DEBUG
#endif

#define PRINT_FLOW_LVL()                                                       \
  LOG_SWV("ar=%d, af=%d, fl1=%d, fl2=%d, fl3=%d, fl4=%d, cr=%d, rst=%d\n",     \
          main_app_ready,                                                      \
          device_auth_flag,                                                    \
          flow_level.level_one,                                                \
          flow_level.level_two,                                                \
          flow_level.level_three,                                              \
          flow_level.level_four,                                               \
          counter.level,                                                       \
          sys_flow_cntrl_u.bits.reset_not_allowed)
#define CY_TRIGGER_SOURCE                                                      \
  (CY_External_Triggered() ? CY_APP_USB_TASK : CY_APP_DEVICE_TASK)

/**
 * @brief LEVELs ENUM for Counter struct.
 * The Counter.level is assigned one of these values which determines the depth
 * in the state transition.
 *
 * @see Counter
 */
enum LEVEL {
  LEVEL_ONE = 1,    ///< At depth level one we have the default view (Main menu
                    ///< in case of main application)
                    ///< @see Flow_level.level_one, Flow_level.level_two
  LEVEL_TWO,        ///< At depth level two the submenus are present @see
                    ///< Flow_level.level_three
  LEVEL_THREE,      ///< At depth level three the actual operations take place
                    ///< @see Flow_level.level_four
  LEVEL_FOUR,       ///< _Unused_
  LEVEL_FIVE,       ///< _Unused_
  LEVEL_SIX         ///< _Unused_
};

/**
 * @brief Commands received from desktop (or sent to the desktop) in status
 * command
 * @details The command is sent by both the device and the desktop to indicate
 * corresponding state of the flow from context of their respective context.
 *
 * @see READY_STATE_PACKET, mark_device_state(), is_device_ready(),
 * \_success_listener(), \_abort\_(),  timeout_task
 */
enum DESKTOP_STATUS {
  STATUS_CMD_FAILURE =
      0,    ///< Indicates the fail status of the ongoing process
  STATUS_CMD_SUCCESS = 1,    ///< Indicates the success status of the ongoing
                             ///< process @see \_success_listener()
  STATUS_CMD_READY =
      2,    ///< Indicates the desktop is ready to accept data or process @see
            ///< READY_STATE_PACKET, mark_device_state(), is_device_ready()
  STATUS_CMD_NOT_READY =
      3,    ///< Indicates the desktop is not ready to process @see
            ///< READY_STATE_PACKET, mark_device_state(), is_device_ready()
  STATUS_CMD_ABORT = 4    ///< Indicates an abort/exit request for current
                          ///< flow/process @see \_abort\_()
};

/**
 * @brief Records user choice/input
 * @details Temporarily store user input in the struct members. The possible
 * inputs are text/character inputs and choice in the list (for menu/submenu
 * options, back/next options, selection from list of choices).
 *
 * @see Flow_level
 * @since v1.0.0
 *
 * @note When handling sensitive information (such as PIN or mnemonics) as
 * input, care is taken that this structure is cleared out just after its use.
 * As a result of this strategy, back controllers will not have access to same
 * information.
 */
#pragma pack(push, 1)
typedef struct screen_input {
  uint16_t list_choice;    ///< Store the user selection from list of choices
                           ///< (choice list, menu/submenu, back/next)
  char input_text[250];    ///< Store text/character input from user (name,
                           ///< passphrase, mnemonics, PIN)
  uint8_t expected_list_choice;    ///< Store any expected choice. Useful for
                                   ///< wallet verification step
  uint8_t desktop_flow_choice;     ///< _Unused_
} Screen_input;
#pragma pack(pop)

/**
 * @brief Defines on which flow is system in, records user input and allows to
 *        mark any error at any point in the flow.
 * @details This is the core application-flow state variable for the state
 * machine based architecture of the application. It enables simple
 * navigation/jumps through flows.
 *
 * @see Screen_input, level_one_tasks(), level_one_tasks_initial(),
 * level_one_controller(), level_one_controller_initial(),
 * level_one_controller_b(), controller_level_four.h, tasks_level_four.h,
 * controller_tap_cards.h, tasks_tap_cards.h, controller_add_wallet.h,
 * tasks_add_wallet.h, controller_old_wallet.h, tasks_old_wallet.h,
 * controller_advanced_settings.h, tasks_advanced_settings.h, tasks.h
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct Flow_level {
  /**
   * Navigates to the default view (Main menu in case of main application)
   * alongside handling error prompts, user confirmation for desktop triggered
   * flows, etc. Refer tasks and controller for complete list. Choices available
   * to user can be among existing wallets and default main menu options under
   * ui_text_options_main_menu.
   *
   * @see Screen_input, level_one_tasks(), level_one_tasks_initial(),
   * level_one_controller(), level_one_controller_initial(),
   * level_one_controller_b(), Counter.level, ui_text_options_main_menu
   */
  uint8_t level_one;

  /**
   * Navigates to the submenus based on user choice at Flow_level.level_one. The
   * valid states would be accessible will be wallet operations, advanced
   * setting options, new wallet creation choices.
   *
   * @see Screen_input,  level_two_controller(),
   * level_two_controller_b(), LEVEL_TWO
   */
  uint8_t level_two;

  /**
   * Mostly targets the actual processing of the operations that are local to
   * the device. For example wallet creation, view seed, delete wallet, advanced
   * setting operations (factory reset, display rotate, etc.)
   *
   * @see Screen_input, level_three_old_wallet_tasks(),
   * level_three_old_wallet_controller(), level_three_old_wallet_controller_b(),
   * LEVEL_THREE_NEW_WALLET_TASKS, LEVEL_THREE_OLD_WALLET_TASKS,
   * LEVEL_THREE_ADVANCED_SETTINGS_TASKS, GENERATE_WALLET_TASKS,
   * DELETE_WALLET_TASKS, RESTORE_WALLET_TASKS, ARBITRARY_DATA_TASKS,
   * VIEW_SEED_TASKS, controller_add_wallet.h, tasks_add_wallet.h,
   * controller_old_wallet.h, tasks_old_wallet.h,
   * controller_advanced_settings.h, tasks_advanced_settings.h
   */
  uint8_t level_three;

  /**
   * The level_four has been divided into two modules: one handles the core
   * operations while the other handles card tap flow. The core module at
   * level_four handle the desktop triggered flows such as export wallet, verify
   * card, verify device, add coin, receive funds, send transaction. Some local
   * operations are also meant to be processed at this level such as verify
   * wallet, unlock wallet, device_provision (only in initial application).
   *
   * @see SYNC_CARDS_TASKS, VERIFY_WALLET_TASKS, WALLET_LOCKED_TASKS,
   * SEND_TRANSACTION_ETH,
   * RECEIVE_TRANSACTION_TASKS_ETH, RECEIVE_TRANSACTION_TASKS, SEND_TRANSACTION,
   * ADD_COINS_TASKS, EXPORT_WALLET_TASKS, VERIFY_CARD_FLOW, PAIR_CARD_FLOW,
   * TAP_CARDS_FLOW, TAP_ONE_CARD_FLOW, controller_level_four.h,
   * tasks_level_four.h, controller_tap_cards.h, tasks_tap_cards.h
   */
  uint8_t level_four;
  uint8_t level_five;    ///< _Unused_

  Screen_input screen_input;    ///< Temporary record of user choice/input

  bool show_error_screen;    ///< Flag for indicating any error in the previous
                             ///< states
  bool show_desktop_start_screen;    ///< Flag for indicating incoming desktop
                                     ///< request
  char error_screen_text[90];    ///< Error message to be shown to user. This
                                 ///< requires that the error flag
                                 ///< (#show_error_screen) is set to true.
  char confirmation_screen_text
      [90];    ///< Confirmation message to be shown to user. This requires that
               ///< the flag (#show_desktop_start_screen) is set to true.
} Flow_level;
#pragma pack(pop)

/**
 * @brief Stores the current level of application state system
 * @details The transition happens based on checks on Counter.level and
 * Counter.next_event_flag. The Counter.next_event_flag governs if there are any
 * events to be handled which initiates the chain of call to tasks from
 * level_one_tasks() or level_one_tasks_initial(). Each task and controller must
 * check the Counter.level to determine if the state is at higher level.
 *
 * @see main(), level_one_tasks(), level_one_tasks_initial()
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Counter {
  uint8_t level;    // holds level
  bool next_event_flag;
  bool previous_event_flag;
} Counter;
#pragma pack(pop)

extern Flow_level flow_level;
extern Counter counter;
extern Wallet wallet;
extern lv_task_t *address_timeout_task;
extern uint32_t inactivity_counter;

#pragma pack(push, 1)
typedef struct Provision_Data_Struct {
  uint8_t device_private_key[32];
  uint8_t device_public_key[ECDSA_PUB_KEY_SIZE];
  uint8_t self_key_path[FS_KEYSTORE_KEYPATH_LEN];
  uint8_t priv_key[FS_KEYSTORE_PRIVKEY_LEN];
  uint8_t card_root_xpub[FS_KEYSTORE_XPUB_LEN];
} Provision_Data_struct;
#pragma pack(pop)

/**
 * @brief Get the Global Flash_Wallet instance.
 *
 * @return Pointer to the global Flash_Wallet instance.
 *
 * @see wallet_for_flash
 * @since v1.0.0
 */
Flash_Wallet *get_flash_wallet();

/**
 * @brief This function is called to end particular event.
 * @details This function sets counter.next_event_flag to true and makes calls
 * to the controllers to update the flow_level and counters for the next event.
 * The function acts as bridge between lvgl registered UI event handlers and
 * application controllers to enable post processing in the application context
 * as the user progresses in a particular process/flow.
 *
 * @see Counter.next_event_flag, level_one_controller(),
 * level_one_controller_initial(), next_controller
 * @since v1.0.0
 */
void mark_event_over();

/**
 * @brief On user confirmation, assigns the value of choice made by user to the
 * flow_level.screen_input.list_choice.
 * @details This function is called by the lvgl UI components to handover the
 * user input choice to the application. The function copies the provided input
 * from UI component to the Flow_level.screen_input.list_choice.
 *
 * @param list_choice User's choice as provided by the lvgl UI component.
 *
 * @see Flow_level.screen_input.list_choice
 * @since v1.0.0
 */
void mark_list_choice(uint16_t list_choice);

/**
 * @brief This function is called to cancel a particular event.
 * @details This function sets Counter.next_event_flag to true and makes calls
 * to the back controllers to update the flow_level and counters for the
 * previous event. The function acts as bridge between lvgl registered UI event
 * handlers and the application back controllers to enable post processing in
 * the application context as the user tries to exit or cancel the process/flow.
 *
 * @see Counter.next_event_flag, level_one_controller_b(), back_controller
 * @since v1.0.0
 */
void mark_event_cancel();

/**
 * @brief Resets the entire flow and brings device to the main menu.
 * @details This functions sets the 1 to all flow_level levels and sets
 * Counter.level to LEVEL_ONE. Sets the Counter.next_event_flag to true and
 * makes the device state as ready. The Wallet.password_double_hash is cleared
 * and a call cy_free() releases all the allocated memory on heap using
 * cy_malloc().
 *
 * @see counter, flow_level, Wallet.password_double_hash, mark_device_state(),
 * cy_free(), cy_malloc()
 * @since v1.0.0
 */
void reset_flow_level();

/**
 * @brief Sets Counter.next_event_flag to false.
 *
 * @see counter, reset_flow_level(), mark_event_cancel()
 * @since v1.0.0
 */
void reset_next_event_flag();

/**
 * @brief Increments Counter.level by 1.
 *
 * @see counter
 * @since v1.0.0
 */
void increase_level_counter();

/**
 * @brief Resets the screen input choice. Sets the
 * Flow_level.screen_input.list_choice to 0.
 *
 * @see flow_level, Screen_input
 * @since v1.0.0
 */
void clear_list_choice();

/**
 * @brief [Obsolete] Shows error message passed as param on device screen.
 * @details Sets the Flow_level.show_error_screen to true and sets error message
 * string to Flow_level.error_screen_text. This is error message is shown to the
 * user in the next event loop iteration.
 *
 * Note: Obsolete function, new function @ref mark_core_error_screen in
 * core_error.h
 * TODO: Remove function after refactor complete
 * @param error_msg     The error description message to be shown to user
 *
 * @see flow_level
 * @since v1.0.0
 */
void mark_error_screen(const char *error_msg);

/**
 * @brief Resets the previous_event_flag property of Counter variable.
 * Sets the previous_event_flag to 0.
 *
 * @see counter
 * @since v1.0.0
 */
void reset_cancel_event_flag();

/**
 * @brief Decrements the counter by 1 without checking for underflow.
 *
 * @see counter
 * @since v1.0.0
 */
void decrease_level_counter();

/**
 * @brief Sets the input text entered by user on the device
 * @details The function internally calls snprintf to copy the input data to
 * application's temporary input buffer for processing in subsequent event
 * loops.
 *
 * @param [in] text     String entered by user as provided by lvgl UI component
 *
 * @see flow_level, Screen_input
 * @since v1.0.0
 */
void mark_input(char *text);

/**
 * @brief Sets the correct answer to the random word questions while creating a
 * new wallet.
 * @details The choice is stored in the
 * flow_level.screen_input.expected_list_choice for future reference in the
 * verification process.
 *
 * @param [in] expected_list_choice     Correct choice value as indicated by the
 * application
 *
 * @see flow_level, Screen_input,
 * @since v1.0.0
 */
void mark_expected_list_choice(uint8_t expected_list_choice);

/**
 * @brief Initializes the properties of global Wallet instance
 *
 * @see wallet, MAX_NUMBER_OF_MNEMONIC_WORDS, MINIMUM_NO_OF_SHARES,
 * TOTAL_NUMBER_OF_SHARES
 * @since v1.0.0
 *
 * @note
 */
void set_wallet_init();

/**
 * @brief Resets the value of particular properties global Flow_level instance
 * based on the value passed
 * @details The function resets all the level members of flow_level for all the
 * levels greater than specified level.
 *
 * @param level     LEVEL enum passed to reset flow_level
 *
 * @see flow_level, LEVEL, reset_flow_level()
 * @since v1.0.0
 */
void reset_flow_level_greater_than(enum LEVEL level);

/**
 * @brief Checks the messages from desktop and initiates/processes the request
 * sent by desktop.
 * @details The function is an lv_task handler registered with lvgl with a
 * priority of LV_TASK_PRIO_HIGH and period of 20 ms with NULL user data. This
 * is responsible for processing desktop requests and performing/triggering flow
 * jumps based on the request from desktop. The function triggers a confirmation
 * message which is shown to the user for confirmation before actually starting
 * the requested flow. It has the capacity to respond to certain requests
 * directly without user consent (such as DEVICE_INFO, START_CARD_AUTH,
 * START_DEVICE_PROVISION,
 *
 * @param task lv_task passed while registering the callback.
 *
 * @see listener_task, flow_level, counter, En_command_type_t, get_usb_msg(),
 * clear_message_received_data(), transmit_one_byte(), transmit_data_to_app(),
 * CY_Set_External_Triggered()
 * @since v1.0.0
 *
 * @note
 */
void desktop_listener_task(lv_task_t *data);

/**
 * @brief Callback function called by the task after a particular timeout to
 * show an error screen
 * @details The function does a generic work of showing a common error message
 * (refer ui_text_command_not_received) to the user. Then it resets the flow
 * level which will take the user to default view. The function also deletes the
 * success handler (ref success_task) if it was registered.
 *
 * @param [in] task lv_task passed while registering the callback.
 *
 * @see ui_text_command_not_received, reset_flow_level(), mark_error_screen()
 * @since v1.0.0
 */
void _timeout_listener(lv_task_t *task);

/**
 * @brief Aborts the events that are currently running and resets the flow of
 * device
 * @details The function wraps the exit method from ongoing flow to default
 * view. This is responsible for exiting flows in situations of errors or if
 * requested by desktop only in case of desktop initiated flows.
 *
 * @see START_DEVICE_PROVISION,  nfc_select_card(),
 * set_instant_abort(), nfc_set_early_exit_handler()
 * @since v1.0.0
 *
 * @note In case of nfc flow, the set_instant_abort(),
 * nfc_set_early_exit_handler() should be set to NULL for device local flows.
 * This is necessary for restricting accidental flow exits when not expected.
 */
void cy_exit_flow();

#endif