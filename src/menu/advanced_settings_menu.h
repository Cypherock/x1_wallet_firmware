/**
 * @file    advanced_settings_menu.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the advanced settings menu of X1 vault device
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef ADVANCED_SETTINGS_MENU_H
#define ADVANCED_SETTINGS_MENU_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
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
 * @brief This API returns the flow_step_t* for the advanced settings menu.
 *
 * @return const flow_step_t* Pointer to the step for the advanced settings menu
 */
const flow_step_t *advanced_settings_menu_get_step(void);

#endif /* ADVANCED_SETTINGS_MENU_H */
