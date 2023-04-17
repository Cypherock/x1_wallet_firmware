/**
 * @file    nfc_events_priv.h
 * @author  Cypherock X1 Team
 * @brief   NFC Events module
 *          Provides NFC module setter for NFC Module
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NFC_EVENTS_PRIV
#define NFC_EVENTS_PRIV

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "nfc.h"
#include "nfc_events.h"

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
 * @brief   Set NFC_EVENT_CARD_DETECT type event
 *
 * @return  true if event set correctly, false otherwise
 */
void nfc_set_card_detect_event();

/**
 * @brief   Used to update nfc_time in nfc_ctx at ideal polling interval of 10ms
 * @arg     tick_inc: value in ms to update the nfc time with
 */
void nfc_tick_inc(uint16_t tick_inc);
#endif