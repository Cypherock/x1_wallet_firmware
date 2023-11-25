#ifndef MPC_WRAPPER_H
#define MPC_WRAPPER_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <mpc_poc/core.pb.h>
#include <stdint.h>

#include "mpc_context.h"

/**
 * @brief Handler for SOLANA public key derivation.
 * @details This flow expects SOLANA_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void mpc_init(mpc_poc_query_t *query);

/**
 * @brief Handler for SOLANA public key derivation.
 * @details This flow expects SOLANA_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void mpc_exit(mpc_poc_query_t *query);

/**
 * @brief Handler for SOLANA public key derivation.
 * @details This flow expects SOLANA_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void get_public_key_flow(mpc_poc_query_t *query);

#endif /* MPC_WRAPPER_H */