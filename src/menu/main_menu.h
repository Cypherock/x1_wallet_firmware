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
 * @brief This API sets the internal flag, which informs the menu initializer
 * whether updating the menu is required or not.
 *
 * @param update_required Boolean value which depicts if the update is required.
 */
void main_menu_set_update_req(bool update_required);

/**
 * @brief This API returns the flow_step_t* for the main menu.
 *
 * @return const flow_step_t* Pointer to the step for the main menu
 */
const flow_step_t *main_menu_get_step(void);

#endif /* MAIN_MENU_H */
