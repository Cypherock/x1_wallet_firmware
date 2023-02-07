/**
 * @file session_utils.h
 * @author  Cypherock X1 Team
 * @brief Functions for session management
 * @version 0.1
 * @date 2023-02-07
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SESSION_UTILS
#define SESSION_UTILS

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "bip32.h"
#include "base58.h"
#include "nist256p1.h"
#include "nfc.h"

bool verify_session_digest(uint8_t *payload, uint16_t payload_length,
                           uint8_t *buffer);

#endif //SESSION_UTILS
