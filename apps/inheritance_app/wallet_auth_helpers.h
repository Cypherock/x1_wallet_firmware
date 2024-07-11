/**
 * @file    session_utils.h
 * @author  Cypherock X1 Team
 * @brief   Header file containing the session utility functions
 *          This file declares the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef WALLET_AUTH_UTILS
#define WALLET_AUTH_UTILS

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "card_fetch_data.h"
#include "nfc.h"
#include "session_utils.h"

#define CHALLENGE_SIZE_MAX 100    // 32 ?
#define CHALLENGE_SIZE_MIN 16
#define ENTROPY_SIZE_LIMIT 150

typedef enum {
  WALLET_AUTH_OWNER,
  WALLET_AUTH_NOMINEE,
} wallet_auth_type_e;

typedef enum {
  WALLET_AUTH_OK = 0,
  WALLET_AUTH_TYPE_INVALID,
  WALLET_AUTH_INPUTS_INVALID,
  WALLET_AUTH_ERR_OWNER,
  WALLET_AUTH_ERR_NOMINEE,
} wallet_auth_error_type_e;

#pragma pack(push, 1)
typedef struct {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t challenge[CHALLENGE_SIZE_MAX];
  size_t challenge_size;

  uint8_t entropy[ENTROPY_SIZE_LIMIT];
  uint8_t entropy_size;
  ed25519_secret_key private_key;

  ed25519_signature signature;
  ed25519_public_key public_key;

  bool is_setup;
  wallet_auth_error_type_e status;
} wallet_auth_t;
#pragma pack(pop)

#endif