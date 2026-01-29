/**
 * @file    cardano_main.h
 * @author  Cypherock X1 Team
 * @brief   Header for cardano app descriptor
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef CARDANO_MAIN_H
#define CARDANO_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"

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
 * @brief Returns the config for Cardano app descriptor
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_cardano_app_desc();

#endif    // CARDANO_MAIN_H
