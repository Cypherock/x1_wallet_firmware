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
#define __EVENT_CONFIG__(n) (1 << n)

#define EVENT_CONFIG_UI __EVENT_CONFIG__(0)
#define EVENT_CONFIG_USB __EVENT_CONFIG__(1)
#define EVENT_CONFIG_NFC __EVENT_CONFIG__(2)

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
 * @details This API provides events occuring based on the event configuration,
 * timeout, and device state(device_idle_state). P0 events are prioritised over
 * other events, and p0 events persist(calling get_events will return the same
 * event again) till they've been reset by calling @ref p0_reset_event. If a P0
 * event hasn't occured, other selected P1 events(from event_config param) are
 * polled continously. In a race condition where multiple P1 events have
 * occured, both will be returned to the caller.
 *
 * @param event_config One or more event config can be passed here, refer @ref
 * __EVENT_CONFIG__. To pass multiple event config use OR(|) operator. Only
 * passed event config will be polled for evevnts.
 * @param timeout Timeout value in mili seconds. Max allowed value is
 * MAX_INACTIVITY_TIMEOUT
 * @return evt_status_t Object with events status.
 */
evt_status_t get_events(uint8_t event_config, uint32_t timeout);

#endif /* EVENTS_H */
