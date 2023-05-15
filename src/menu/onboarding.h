/**
 * @file    onboarding.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef ONBOARDING_H
#define ONBOARDING_H

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
 * @brief This API sets the internal flag, which informs the onboarding menu
 * initializer that the static screen is required to be rendered on the screen.
 * This is to satisfy the requirement where-in the onboarding flow needs to show
 * a static screen on host connection.
 */
void onboarding_set_static_screen(void);

/**
 * @brief This API returns the flow_step_t* for the onboarding menu.
 *
 * @return const flow_step_t* Pointer to the step for the onboarding menu
 */
const flow_step_t *onboarding_get_step(void);

#endif /* ONBOARDING_H */
