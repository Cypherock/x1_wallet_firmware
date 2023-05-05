/**
 * @file    main_menu.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

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
void main_menu_initialize(const engine_ctx_t *ctx, const void *data_ptr);

/**
 * @brief
 *
 * @param ctx
 * @param ui_event
 * @param data_ptr
 */
void main_menu_handler(const engine_ctx_t *ctx,
                       ui_event_t ui_event,
                       const void *data_ptr);

/**
 * @brief
 *
 * @param update_required
 */
void main_menu_set_update_req(bool update_required);

/**
 * @brief
 *
 * @return const flow_step_t*
 */
const flow_step_t *main_menu_get_step(void);

#endif /* MAIN_MENU_H */
