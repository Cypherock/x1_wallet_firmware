/**
 * @file    inheritance_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various inheritance actions
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
#include "inheritance_api.h"
#include "inheritance_wallet_auth.h"

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
 * @brief Returns the config for INHERITANCE app descriptors
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_inheritance_app_desc();

/**
 * @brief Handles the wallet login process, including authentication and key
 * generation.
 *
 * This function initializes the wallet authentication structure, populates it
 * with data from the query, and performs a series of authentication steps
 * (input verification, entropy fetching, key pair generation, and signature
 * generation). If successful, it sends the result, including the public key and
 * signature if setup is required.
 *
 * @param query The reference to the inheritance query containing wallet
 * authentication data.
 */
void wallet_login(inheritance_query_t *query);

#endif /* INHERITANCE_MAIN_H */
