/**
 * @file    ui_events_priv.h
 * @author  Cypherock X1 Team
 * @brief   UI Event getter module
 *          Provides UI event setter for different UI screens, used to pass
 *          an event to the OS.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef UI_EVENTS_PRIV
#define UI_EVENTS_PRIV

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

/**
 * @brief This API copies the input stored in the internal input buffer to the
 * buffer which the application can access from their context. It is assumed
 * that the application has appropriately allocated enough space to accomodate
 * the maximum input length.
 *
 * @param text_src_ptr Pointer to source of internal input buffer
 * @param input_text_ptr Pointer to application buffer
 * @param max_text_len Maximum length of input as requested by the application.
 */
void ui_fill_text(const char *text_src_ptr,
                  char *input_text_ptr,
                  const size_t max_text_len);

#endif
