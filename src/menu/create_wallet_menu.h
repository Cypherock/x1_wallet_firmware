/**
 * @file    create_wallet_menu.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef CREATE_WALLET_MENU_H
#define CREATE_WALLET_MENU_H

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

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Create a wallet menu initialize object
 *
 * @param ctx
 * @param data_ptr
 */
void create_wallet_menu_initialize(const engine_ctx_t *ctx,
                                   const void *data_ptr);

/**
 * @brief Create a wallet menu handler object
 *
 * @param ctx
 * @param ui_event
 * @param data_ptr
 */
void create_wallet_menu_handler(const engine_ctx_t *ctx,
                                ui_event_t ui_event,
                                const void *data_ptr);

/**
 * @brief Create a wallet menu get step object
 *
 * @return const flow_step_t*
 */
const flow_step_t *create_wallet_menu_get_step(void);

#endif /* CREATE_WALLET_MENU_H */
