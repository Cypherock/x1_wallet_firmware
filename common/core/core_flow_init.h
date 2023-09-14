/**
 * @file    core_flow_init.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CORE_FLOW_INIT_H
#define CORE_FLOW_INIT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stddef.h>
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
 * @brief Get the core flow ctx object
 *
 * @return const engine_ctx_t*
 */
engine_ctx_t *get_core_flow_ctx(void);

/**
 * @brief The function adds various app descriptions to the app registry.
 */
void core_init_app_registry();
#endif /* CORE_FLOW_INIT_H */
