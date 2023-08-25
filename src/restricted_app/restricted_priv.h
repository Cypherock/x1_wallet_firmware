/**
 * @file    restricted_priv.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef ONBOARDING_PRIV_H
#define ONBOARDING_PRIV_H

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
 * @brief This is the initializer callback for the restricted app menu.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param data_ptr Currently unused pointer set by the engine
 */
void restricted_app_initialize(engine_ctx_t *ctx, const void *data_ptr);

#endif /* ONBOARDING_PRIV_H */
