/**
 * @file    nfc_events_priv.h
 * @author  Cypherock X1 Team
 * @brief   NFC Events module
 *          Provides NFC module setter for NFC Module
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
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
#endif
