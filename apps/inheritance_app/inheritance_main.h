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
#include "session_utils.h"

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
void inheritance_wallet_login(inheritance_query_t *query);

/**
 * @brief This API processes a login session request for inheritance operations.
 *
 * This function handles login sessions by processing requests of type
 * `inheritance_query_t`. Depending on the type of request specified in
 * `query.which_request`, it either sets up an inheritance session or performs
 * a recovery operation. The function uses a simulator for debugging purposes
 * if enabled, and sends results accordingly.
 *
 * @param query A pointer to the `inheritance_query_t` structure containing the
 *        request details.
 */
void inheritance_session_login(inheritance_query_t *query);

/**
 * @brief This API handles the setup phase of an inheritance session.
 *
 * This function processes an inheritance setup request by converting plain data
 * into secure messages, encrypting them, and then packaging them into an
 * encrypted packet. The resulting encrypted data is then stored in the response
 * structure. If any errors occur during the process, appropriate error logging
 * is performed.
 *
 * @param query A pointer to the `inheritance_query_t` structure containing the
 *        setup request details, including plain data and wallet ID.
 * @param msgs A pointer to an array of `secure_data_t` structures to hold the
 *        converted plain data.
 * @param response A pointer to the `inheritance_result_t` structure where the
 *        encrypted result will be stored.
 */
void inheritance_setup(inheritance_query_t *query,
                       secure_data_t *msgs,
                       inheritance_result_t *response);

/**
 * @brief This API handles the recovery phase of an inheritance session.
 *
 * This function processes an inheritance recovery request by decrypting an
 * encrypted packet into secure messages, decrypting the secure data, and then
 * converting it back into plain data. The resulting plain data is then stored
 * in the response structure. If any errors occur during the process,
 * appropriate error logging is performed.
 *
 * @param query A pointer to the `inheritance_query_t` structure containing the
 *        recovery request details, including encrypted data and wallet
 *        authentication information.
 * @param msgs A pointer to an array of `secure_data_t` structures to hold the
 *        decrypted secure data.
 * @param response A pointer to the `inheritance_result_t` structure where the
 *        decrypted result will be stored.
 */
void inheritance_recovery(inheritance_query_t *query,
                          secure_data_t *msgs,
                          inheritance_result_t *response);

#endif /* INHERITANCE_MAIN_H */
