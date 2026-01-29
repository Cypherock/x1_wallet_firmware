/**
 * @file    canton_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Canton chains
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "canton_helpers.h"

#include <stddef.h>

#include "canton_context.h"
#include "coin_utils.h"
#include "sha2.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
// For references to values, check get_party_id function
static uint8_t CANTON_HASH_PURPOSE[CANTON_HASH_PURPOSE_SIZE] = {0x00,
                                                                0x00,
                                                                0x00,
                                                                0x0c};
static uint8_t CANTON_HASH_PREFIX[CANTON_HASH_PREFIX_SIZE] = {0x12, 0x20};
static const char CANTON_PARTY_ID_SEPARATOR[] = "::";

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool canton_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != CANTON_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/6767'/0'/0'/i'
  status = (CANTON_PURPOSE_INDEX == purpose && CANTON_COIN_INDEX == coin &&
            CANTON_ACCOUNT_INDEX == account && CANTON_CHANGE_INDEX == change &&
            is_hardened(address));

  return status;
}

void sha256_with_prefix(const uint8_t *data, size_t data_size, uint8_t *hash) {
  if (!data || !hash || data_size == 0) {
    return;
  }

  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(data, data_size, digest);

  memcpy(hash, CANTON_HASH_PREFIX, CANTON_HASH_PREFIX_SIZE);
  memcpy(hash + CANTON_HASH_PREFIX_SIZE, digest, SHA256_DIGEST_LENGTH);
}

bool get_party_id(const uint8_t *public_key, char *party_id) {
  if (!public_key || !party_id) {
    return false;
  }

  // Ref:
  // https://docs.digitalasset.com/integrate/devnet/party-management/index.html#choosing-a-party-hint
  // party_id = party_hint_str + :: + fingerprint_str
  // party_hint can by anything like "alice", "bob", "my-wallet", etc.
  // We are using first 5 bytes of sha256_hash(fingerprint) to keep it
  // deterministic
  // party_hint = sha256_hash(fingerprint)[:5]
  // Ref:
  // https://github.com/hyperledger-labs/splice-wallet-kernel/blob/main/core/ledger-client/src/topology-write-service.ts#L143
  // Ref:
  // https://github.com/hyperledger-labs/splice-wallet-kernel/blob/main/core/ledger-client/src/topology-write-service.ts#L62
  // fingerprint = 0x1220 + sha256(HASH_PURPOSE + public_key)
  uint8_t party_hint[CANTON_PARTY_HINT_SIZE] = {0};
  uint8_t fingerprint[CANTON_FINGERPRINT_SIZE] = {0};
  char party_hint_str[CANTON_PARTY_HINT_STR_SIZE] = {'\0'};
  char fingerprint_str[CANTON_FINGERPRINT_STR_SIZE] = {'\0'};

  // HASH_PURPOSE + public_key
  uint8_t hash_buf[CANTON_HASH_PURPOSE_SIZE + CANTON_PUB_KEY_SIZE] = {0};
  memcpy(hash_buf, CANTON_HASH_PURPOSE, CANTON_HASH_PURPOSE_SIZE);
  memcpy(hash_buf + CANTON_HASH_PURPOSE_SIZE, public_key, CANTON_PUB_KEY_SIZE);

  sha256_with_prefix(hash_buf, sizeof(hash_buf), fingerprint);

  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(fingerprint, CANTON_FINGERPRINT_SIZE, digest);
  memcpy(party_hint, digest, CANTON_PARTY_HINT_SIZE);

  if (!byte_array_to_hex_string(party_hint,
                                CANTON_PARTY_HINT_SIZE,
                                party_hint_str,
                                CANTON_PARTY_HINT_STR_SIZE)) {
    return false;
  }
  if (!byte_array_to_hex_string(fingerprint,
                                CANTON_FINGERPRINT_SIZE,
                                fingerprint_str,
                                CANTON_FINGERPRINT_STR_SIZE)) {
    return false;
  }

  strncpy(party_id, party_hint_str, CANTON_PARTY_HINT_STR_SIZE);
  strncat(party_id, CANTON_PARTY_ID_SEPARATOR, CANTON_PARTY_ID_SEPARATOR_SIZE);
  strncat(party_id, fingerprint_str, CANTON_FINGERPRINT_STR_SIZE);

  return true;
}

bool verify_party_id(uint8_t *public_key, char *party_id) {
  if (!public_key || !party_id) {
    return false;
  }

  char derived_party_id[CANTON_PARTY_ID_SIZE] = {0};
  get_party_id(public_key, derived_party_id);
  if (strcmp(derived_party_id, party_id) != 0) {
    return false;
  }

  return true;
}
