/**
 * @file    inheritance_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various Inheritance actions
 supported.
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef INHERITANCE_MAIN_H
#define INHERITANCE_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"
#include "events.h"
#include "inheritance/core.pb.h"
#include "inheritance_context.h"
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
 * @brief Returns the config for Inheritance chain app descriptors
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_inheritance_app_desc();

/**
 * @brief Entry point to auth wallet flow
 *
 */
void inheritance_wallet_login(inheritance_query_t *query);

#endif /* INHERITANCE_MAIN_H */
