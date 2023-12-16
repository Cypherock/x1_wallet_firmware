#ifndef MPC_SIGN_MESSAGE_H
#define MPC_SIGN_MESSAGE_H

#include <mpc_poc/core.pb.h>
#include <stdint.h>

#include "mpc_context.h"

void sign_message_flow(mpc_poc_query_t *query);

#endif