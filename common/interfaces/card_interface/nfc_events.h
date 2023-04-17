/**
 * @file    nfc_events.h
 * @author  Cypherock X1 Team
 * @brief   NFC Events module
 *          Provides NFC module setter and getter
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NFC_EVENTS
#define NFC_EVENTS

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "nfc.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum { NFC_EVENT_CARD_DETECT = 1 } nfc_event_type_t;

typedef struct {
  bool event_occured;
  nfc_event_type_t event_type;
} nfc_event_t;

typedef struct {
  bool card_detect_enabled;
  bool nfc_field_off;
  uint16_t nfc_time;
} nfc_ctx_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief   Used to get the latest NFC event occurance
 * @arg     *nfc_event_os_obj   `nfc_event_t` object used to get the NFC event
 *          details
 *
 * @return  true for event occured, else otherwise
 */
bool nfc_get_event(nfc_event_t *nfc_event_os_obj);

/**
 * @brief   Used to reset the latest NFC events from static `nfc_event_t` object
 */
void nfc_reset_event();

/**
 * @brief   Enable NFC Card detect event
 */
void nfc_enable_card_detect_event();

/**
 * @brief   Disable NFC Card detect event
 */
void nfc_disable_card_detect_event();

/**
 * @brief   Performs operations to support NFC tasks
 */
void nfc_ctx_init();

/**
 * @brief   Handle NFC tasks and set events
 */
void nfc_task_handler();

/**
 * @brief   Performs resetting on PN532 states
 */
void nfc_ctx_destroy();
#endif
