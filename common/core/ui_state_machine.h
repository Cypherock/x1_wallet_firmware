/**
 * @file    core_error.h
 * @author  Cypherock X1 Team
 * @brief   Core error handling apis
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
 * @brief
 *
 * @param state_on_confirmation
 * @param state_on_rejection
 * @param state_on_p0_event
 * @return uint8_t
 */
uint8_t confirm_next_state(uint8_t state_on_confirmation,
                           uint8_t state_on_rejection,
                           uint8_t state_on_p0_event);

/**
 * @brief
 *
 * @param state_on_text_input
 * @param state_on_rejection
 * @param state_on_p0_event
 * @return uint8_t
 */
uint8_t ui_text_next_state(uint8_t state_on_text_input,
                           uint8_t state_on_rejection,
                           uint8_t state_on_p0_event);
#endif /* UI_STATE_MACHINE_H */
