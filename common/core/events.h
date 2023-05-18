/**
 * @file    events.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the event getter module
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
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
#include "nfc_events.h"
#include "p0_events.h"
#include "ui_events.h"
#include "usb_api.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define EVENT_CONFIG_UI (1 << 0)
#define EVENT_CONFIG_USB (1 << 1)
#define EVENT_CONFIG_NFC (1 << 2)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  uint8_t evt_selection;
  uint32_t timeout;
} evt_config_t;

typedef struct {
  p0_evt_t p0_event;
  ui_event_t ui_event;
  usb_event_t usb_event;
  nfc_event_t nfc_event;
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
 * @param event_config
 * @param timeout
 * @return evt_status_t
 */
evt_status_t get_events(uint8_t event_config, uint32_t timeout);

#endif /* EVENTS_H */
