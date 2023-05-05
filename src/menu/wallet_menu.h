/**
 * @file    create_wallet_menu.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef WALLET_MENU_H
#define WALLET_MENU_H

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
 * @brief
 *
 * @param ctx
 * @param data_ptr
 */
void wallet_menu_initialize(const engine_ctx_t *ctx, const void *data_ptr);

/**
 * @brief
 *
 * @param ctx
 * @param ui_event
 * @param data_ptr
 */
void wallet_menu_handler(const engine_ctx_t *ctx,
                         ui_event_t ui_event,
                         const void *data_ptr);

/**
 * @brief
 *
 * @param menu_wallet_index
 * @return const flow_step_t*
 */
const flow_step_t *wallet_menu_get_step(uint8_t menu_wallet_index);

#endif /* WALLET_MENU_H */
