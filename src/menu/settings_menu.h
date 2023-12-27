/**
 * @file    settings_menu.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the settings menu of X1 vault device
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

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
 * @brief This API returns the flow_step_t* for the settings menu.
 *
 * @return const flow_step_t* Pointer to the step for the settings menu
 */
const flow_step_t *settings_menu_get_step(void);

#endif /* SETTINGS_MENU_H */
