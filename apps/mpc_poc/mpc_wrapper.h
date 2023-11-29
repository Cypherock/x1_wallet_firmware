#ifndef MPC_WRAPPER_H
#define MPC_WRAPPER_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <mpc_poc/core.pb.h>
#include <stdint.h>

#include "mpc_context.h"

void get_public_key_flow(mpc_poc_query_t *query);

#endif /* MPC_WRAPPER_H */