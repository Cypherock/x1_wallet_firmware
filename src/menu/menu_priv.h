/**
 * @file    menu_priv.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef MENU_PRIV_H
#define MENU_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "flow_engine.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern const evt_config_t main_menu_evt_config;
extern const evt_config_t device_nav_evt_config;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This is the initializer callback for the main menu. It determines if
 * the main menu requires an update and renders the options if needed.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param data_ptr Currently unused pointer set by the engine
 */
void main_menu_initialize(engine_ctx_t *ctx, const void *data_ptr);

/**
 * @brief This is the UI event handler for the main menu. It decodes the user
 * selection and sets the next step into the engine ctx
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param ui_event The ui event object which triggered the callback
 * @param data_ptr Currently unused pointer set by the engine
 */
void main_menu_handler(engine_ctx_t *ctx,
                       ui_event_t ui_event,
                       const void *data_ptr);

#endif /* MENU_PRIV_H */
