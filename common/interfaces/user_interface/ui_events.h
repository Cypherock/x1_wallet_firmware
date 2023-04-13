/**
 * @file    ui_event.h
 * @author  Cypherock X1 Team
 * @brief   UI Event getter module
 *          Provides UI event getter used by os for fetching ui events.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
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
  bool event_occured;
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
 * @brief   Used to get the latest ui event occurance and reset it
 * @arg     *ui_event_os_obj   `ui_event_t` object used to get the ui event
 * details
 */
bool ui_get_and_reset_event(ui_event_t *ui_event_os_obj);

/**
 * @brief   Used to reset ui events from static `ui_event_t` object
 */
void ui_reset_event();
#endif