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
 * @return uint8_t The next state based on event received
 */
uint8_t confirm_next_state(uint8_t state_on_confirmation,
                           uint8_t state_on_rejection,
                           uint8_t state_on_p0_event);

/**
 * @brief Waits for UI event on an input screen and return next state
 * based on arguments
 *
 * @param state_on_text_input The state to be set on user text input
 * @param state_on_rejection The state to be set on user rejection
 * @param state_on_p0_event The state to be set on P0 event
 * @return uint8_t The next state based on event received
 */
uint8_t text_input_next_state(uint8_t state_on_text_input,
                              uint8_t state_on_rejection,
                              uint8_t state_on_p0_event);
#endif /* UI_STATE_MACHINE_H */
