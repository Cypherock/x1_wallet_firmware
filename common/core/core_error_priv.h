/**
 * @file    core_error_priv.h
 * @author  Cypherock X1 Team
 * @brief   Private core error handling apis
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CORE_ERROR_PRIV_H
#define CORE_ERROR_PRIV_H

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
 * @brief Handle P0 timeout event and display error message set by core
 * opearations using @ref mark_core_error_screen. In case of P0 timeout event,
 * message is displayed (if local error message buffer was empty) and a core
 * error is sent to host. In case of abort event from the host, the usb event
 * is cleared so that stale event does not go to application in next event loop.
 * After P0 handling, if a core error message was set, that message is displayed
 * if inactivity was the reason.
 *
 * NOTE:
 * 1. Should be called on initialization of a core flow like main menu or
 * onboarding.
 * 2. Core device idle status is set to CORE_DEVICE_IDLE_STATE_DEVICE when error
 * messsage is displayed
 */
void handle_core_errors();

/**
 * @brief This function clears the p0 event. Should be called in engine p0
 * callbacks to ignore repeated p0 events.
 *
 */
void ignore_p0_event();
#endif
