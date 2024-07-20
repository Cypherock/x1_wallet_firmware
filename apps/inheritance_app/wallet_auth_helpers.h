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
  WALLET_AUTH_OK = 0,
  WALLET_AUTH_TYPE_INVALID,
  WALLET_AUTH_INPUTS_INVALID,
} wallet_auth_error_type_e;

#pragma pack(push, 1)
typedef struct {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t challenge[CHALLENGE_SIZE_MAX];
  size_t challenge_size;

  const uint8_t entropy[ENTROPY_SIZE_LIMIT];
  uint8_t entropy_size;
  ed25519_secret_key private_key;

  ed25519_signature signature;
  ed25519_public_key public_key;

  bool is_setup;
  wallet_auth_error_type_e status;
} wallet_auth_t;
#pragma pack(pop)

/**
 * @brief Retrieves encrypted data (entropy) from the card based on the wallet ID.
 *
 * This function initializes a SecureData structure, fetches encrypted data from
 * the card, and stores the result in the auth structure. It checks if the operation
 * was successful and if the encrypted data size is within the allowed limit.
 *
 * @return true If the entropy was successfully fetched and stored.
 * @return false If there was an error in fetching the encrypted data or if the data size exceeds the limit.
 */
bool wallet_auth_get_entropy(wallet_auth_t *auth);

/**
 * @brief Generates the public and private key pairs based on the entropy.
 *
 * This function derives a seed from the entropy and generates an Ed25519 key pair
 * (private and public keys). It stores the keys in the auth structure.
 *
 * @return true Always returns true.
 */
bool wallet_auth_get_pairs(wallet_auth_t *auth);

/**
 * @brief Generates and verifies a digital signature for the wallet authentication.
 *
 * This function creates an unsigned transaction by concatenating the challenge and wallet ID.
 * It then signs the transaction using the private key and verifies the signature using the public key.
 *
 * @return true If the signature was successfully generated and verified.
 * @return false If the signature verification failed.
 */
bool wallet_auth_get_signature(wallet_auth_t *auth);

#endif