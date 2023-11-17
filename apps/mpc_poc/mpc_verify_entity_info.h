#ifndef MPC_VERIFY_ENTITY_INFO_H
#define MPC_VERIFY_ENTITY_INFO_H
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
void verify_entity_info_flow(mpc_poc_query_t *query);

#endif /* MPC_VERIFY_ENTITY_INFO_H */