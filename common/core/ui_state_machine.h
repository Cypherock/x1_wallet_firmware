/**
 * @file    ui_state_machine.h
 * @author  Cypherock X1 Team
 * @brief   Helpers to listen for UI events and advance the state of a flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef UI_STATE_MACHINE_H
#define UI_STATE_MACHINE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Waits for UI event on a confirmation screen and return next state
 * based on arguments
 *
 * @param state_on_confirmation The state to be set on user confirmation
 * @param state_on_rejection The state to be set on user rejection
 * @param state_on_p0_event The state to be set on P0 event
 * @return uint32_t The next state based on event received
 */
uint32_t get_state_on_confirm_scr(uint32_t state_on_confirmation,
                                  uint32_t state_on_rejection,
                                  uint32_t state_on_p0_event);

/**
 * @brief Waits for UI event on an input screen and return next state
 * based on arguments
 *
 * @param state_on_text_input The state to be set on user text input
 * @param state_on_rejection The state to be set on user rejection
 * @param state_on_p0_event The state to be set on P0 event
 * @return uint32_t The next state based on event received
 */
uint32_t get_state_on_input_scr(uint32_t state_on_text_input,
                                uint32_t state_on_rejection,
                                uint32_t state_on_p0_event);

/**
 * @brief Waits for UI event on an list input screen and return next state
 * based on arguments
 *
 * @param state_on_menu_input The state to be set on user list input
 * @param state_on_rejection The state to be set on user rejection
 * @param state_on_p0_event The state to be set on P0 event
 * @param list_choice The pointer in which the user list input is returned
 * @return uint32_t The next state based on event received
 */
uint32_t get_state_on_list_scr(uint32_t state_on_menu_input,
                               uint32_t state_on_rejection,
                               uint32_t state_on_p0_event,
                               uint16_t *list_choice);

#endif /* UI_STATE_MACHINE_H */
