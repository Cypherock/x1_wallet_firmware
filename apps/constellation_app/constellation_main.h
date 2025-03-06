/**
 * @file    constellation_main.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONSTELLATION_MAIN_H
#define CONSTELLATION_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

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
 * @brief Returns the config for CONSTELLATION chain app descriptor
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_constellation_app_desc();
#endif /* CONSTELLATION_MAIN_H */
