/**
 * @file    manager_app.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef MANAGER_APP_H
#define MANAGER_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <manager/core.pb.h>
#include <manager/get_device_info.pb.h>
#include <stddef.h>
#include <stdint.h>

#include "app_registry.h"
#include "events.h"

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
 * The function returns a pointer to the "manager_desc" variable of type
 * "cy_app_desc_t".
 *
 * @return a pointer to a constant structure of type `cy_app_desc_t`.
 */
const cy_app_desc_t *get_manager_app_desc();

/**
 * The function returns a pointer to the "manager_restricted_desc"
 * object.
 *
 * @return a pointer to a constant structure of type `cy_app_desc_t`.
 */
const cy_app_desc_t *get_restricted_manager_app_desc();
#endif /* MANAGER_APP_H */
