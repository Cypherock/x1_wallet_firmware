/**
 * @file    settings.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef SETTINGS_H
#define SETTINGS_H

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
 * @brief Set the tings initialize object
 *
 * @param ctx
 * @param data_ptr
 */
void settings_initialize(const engine_ctx_t *ctx, const void *data_ptr);

/**
 * @brief Set the tings p0 handler object
 *
 * @param ctx
 * @param p0_event
 * @param data_ptr
 */
void settings_p0_handler(const engine_ctx_t *ctx,
                         p0_evt_t p0_event,
                         const void *data_ptr);

/**
 * @brief Set the tings handler object
 *
 * @param ctx
 * @param ui_event
 * @param data_ptr
 */
void settings_handler(const engine_ctx_t *ctx,
                      ui_event_t ui_event,
                      const void *data_ptr);

/**
 * @brief Set the tings get step object
 *
 * @return const flow_step_t*
 */
const flow_step_t *settings_get_step(void);

#endif /* SETTINGS_H */
