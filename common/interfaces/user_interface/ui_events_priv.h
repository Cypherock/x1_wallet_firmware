/**
 * @file    ui_events_priv.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INCLUDE_GUARD
#define INCLUDE_GUARD

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

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
 * @brief   Used to pass UI cofirm event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
void ui_set_confirm_event();

/**
 * @brief   Used to pass UI cancel event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
void ui_set_cancel_event();

/**
 * @brief   Used to pass UI list event to os event getter
 * @arg     list_selection selection number from passed option,
 *          valid selection from 0-65535
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
void ui_set_list_event(uint16_t list_selection);

/**
 * @brief   Used to pass UI Text Input event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
void ui_set_text_input_event(char *text_ptr);
#endif
