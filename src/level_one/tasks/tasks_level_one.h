/**
 * @file    tasks_level_one.h
 * @author  Cypherock X1 Team
 * @brief   Header for level one tasks.
 *          Houses the declarations of tasks for level one state.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef TASKS_LEVEL_ONE_H
#define TASKS_LEVEL_ONE_H

/**
 * @brief Handles initialisation of display & pre-processing for level one tasks in the main application.
 * @details The level one tasks handler is responsible for handling error messages (ref Flow_level.show_error_screen,
 * Flow_level.error_screen_text, Flow_level.show_desktop_start_screen, Flow_level.confirmation_screen_text). Depending
 * on the value of Counter.level, the function either processes for level one or chains to the higher level task
 * handlers. At level one this task handler will populate and render Main menu options (ref ui_text_options_main_menu),
 * enable desktop request handler and set the device state to ready.
 *
 * @see flow_level, counter, wallet, get_wallet_count(), ui_text_options_main_menu, listener_task, LV_TASK_PRIO_MID,
 * mark_device_state()
 * @since v1.0.0
 */
void level_one_tasks();

/**
 * @brief Handles initialisation of display & pre-processing for level one tasks in the initial application.
 * @details The level one tasks handler is responsible for handling error messages (ref Flow_level.show_error_screen,
 * Flow_level.error_screen_text, Flow_level.show_desktop_start_screen, Flow_level.confirmation_screen_text). Depending
 * on the value of Counter.level, the function either processes for level one or chains to the higher level task
 * handlers. At level one, this task handler will enable desktop request handler and render appropriate display based
 * on the current level one state of the application.
 *
 * @see flow_level, counter, tasks_read_card_id(), instruction_scr_init()
 * @since v1.0.0
 */
void level_one_tasks_initial(void);


/**
 * @brief
 * @details 
 * 
 * @see
 * @since v1.0.0
 */
void level_one_tasks_wrapper(void);

#endif /* TASKS_LEVEL_ONE_H */