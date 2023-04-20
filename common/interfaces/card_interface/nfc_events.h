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
typedef enum {
  NFC_EVENT_CARD_DETECT = 1,
  NFC_EVENT_CARD_REMOVED
} nfc_event_type_t;

typedef struct {
  bool event_occured;
  nfc_event_type_t event_type;
} nfc_event_t;

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
void nfc_reset_event(void);

/**
 * @brief   Used to enable task which detects and selects NFC Type A card
 */
void nfc_en_select_card_task(void);

/**
 * @brief   Used to enable task which waits for card removal. Before enabling
 * the task, card's presence in field is checked and only if card is detected
 * the task is enabled. This event is only applicable when waiting for removal
 * of an already activated card. The wait for card removal event would always
 * return an error if the card is not activated in correct state. Any return
 * other than PN532_DIAGNOSE_CARD_DETECTED_RESP indicates either card is not
 * detected or card is not activated.
 *
 * @return  PN532_DIAGNOSE_CARD_DETECTED_RESP  Card is detected in field and
 * task has been enabled, otherwise error code is returned
 */
uint32_t nfc_en_wait_for_card_removal_task(void);

/**
 * @brief   Handle NFC tasks and set events
 *          Should be called by os event getter @ref get_events to handle NFC
 * tasks and detect NFC events
 */
void nfc_task_handler(void);

/**
 * @brief   Resets local variables, states and PN532 state
 */
void nfc_ctx_destroy(void);
#endif
