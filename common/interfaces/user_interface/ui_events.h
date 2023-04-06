/**
 * @file    ui_event.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef UI_EVENTS
#define UI_EVENTS

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert_conf.h"
#include "board.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum {
  UI_EVENT_CONFIRM = 1,
  UI_EVENT_REJECT,
  UI_EVENT_TEXT_INPUT,
  UI_EVENT_LIST_CHOICE,
  UI_EVENT_SKIP_EVENT
} ui_event_types_t;

typedef struct {
  uint32_t event_occured;
  ui_event_types_t event_type;
  char *text_ptr;
  uint16_t list_selection;
} ui_event_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief   Used to get the latest ui event occurance
 * @arg     *ui_event_os_obj   `ui_event_t` object used to get the ui event
 * details
 */
void ui_get_and_reset_event(ui_event_t *ui_event_os_obj);

/**
 * @brief   Used to pass UI cofirm event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
bool ui_set_confirm_event();

/**
 * @brief   Used to pass UI cancel event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
bool ui_set_cancel_event();

/**
 * @brief   Used to pass UI list event to os event getter
 * @arg     list_selection selection number from passed option,
 *          valid selection from 0-65535
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
bool ui_set_list_event(uint8_t list_selection);

/**
 * @brief   Used to pass UI Text Input event to os event getter
 *
 * @return  returns true if event was set correctly and ui status was updated
 */
bool ui_set_text_input_event(char *text_ptr);
#endif