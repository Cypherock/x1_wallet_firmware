/**
 * @file    events.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef EVENTS_H
#define EVENTS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "lvgl.h"
#include "p0_events.h"
#include "ui_events.h"
#include "usb_api.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef union {
  struct {
    uint8_t ui_events : 1;
    uint8_t usb_events : 1;
    uint8_t nfc_events : 1;
    uint8_t rfu1 : 1;
    uint8_t rfu2 : 1;
    uint8_t rfu3 : 1;
    uint8_t rfu4 : 1;
    uint8_t rfu5 : 1;
  } bits;
  uint8_t byte;
} evt_select_t;

typedef struct {
  evt_select_t evt_selection;
  bool abort_disabled;
  uint32_t timeout;
} evt_config_t;

typedef struct {
  p0_evt_t p0_event;
  ui_event_t ui_event;
  usb_event_t usb_event;
  //   nfc_evt_t nfc_event;
} evt_status_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Get the events object
 *
 * @param evt_config
 * @param p_evt_status
 */
void get_events(evt_config_t evt_config, evt_status_t *p_evt_status);

#endif /* EVENTS_H */
