/**
 * @file    solana_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various Solana coin actions
 supported.
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SOLANA_MAIN_H
#define SOLANA_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"
#include "events.h"
#include "solana_context.h"
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
 * @brief Returns the config for Solana chain app descriptors
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_solana_app_desc();
#endif /* SOLANA_MAIN_H */
