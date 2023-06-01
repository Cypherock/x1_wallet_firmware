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
#include <manager/get_device_info.pb.h>
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

/**
 * @brief This API returns the last completed step of the onboarding flow
 *
 * @return manager_onboarding_step_t Enum depicting last recorded step
 */
manager_onboarding_step_t onboarding_get_last_step(void);

/**
 * @brief This API updates the progress of the onboarding flow. An important
 * thing to note here is that it only sets the step if the last_step is not
 * MANAGER_ONBOARDING_STEP_COMPLETE (depicting onboarding is already complete)
 * and the next_step is actually a new step of the onboarding flow (revocation
 * is not allowed).
 *
 * It is to be noted that this API does not check if the next_step is a
 * sequential step, just to satisfy 2 requirements
 * 1. We need to set onboarding step directly to
 * MANAGER_ONBOARDING_STEP_COMPLETE in case of in-field device.
 * 2. We need to skip ONBOARDING_CARD_AUTHENTICATION in case old cards are
 * tapped and ONBOARDING takes a different route.
 *
 * @param next_step The step that needs to be recorded
 */
void onboarding_set_step_done(const manager_onboarding_step_t next_step);

/**
 * @brief This API checks if a particular step of the onboarding flow is allowed
 * or not. It is to ensure that the flow occurs sequentially only. In case the
 * last_step already depicts MANAGER_ONBOARDING_STEP_COMPLETE, this API returns
 * true.
 *
 * @param step The step that needs to be queried for allowance
 * @return true If the step is allowed to be executed
 * @return false If the step is not allowed to be executed
 */
bool onboarding_step_allowed(const manager_onboarding_step_t step);

#endif /* ONBOARDING_H */
