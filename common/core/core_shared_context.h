/**
 * @author  Cypherock X1 Team
 * @brief   Header file containing the session functions
 *          This file declares the functions used to create and manage the
 *          shared context.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef CORE_SHARED_CONTEXT
#define CORE_SHARED_CONTEXT

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "core.pb.h"
#include "device_authentication_api.h"

#define SHARED_CONTEXT_SIZE 256

extern uint8_t shared_context[SHARED_CONTEXT_SIZE];

void core_clear_shared_context();

#endif    // CORE_SHARED_CONTEXT
