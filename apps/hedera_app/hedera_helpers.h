/**
 * @file    hedera_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities API definitions for the Hedera app.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#ifndef HEDERA_HELPERS_H
#define HEDERA_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "proto/basic_types.pb.h"

// Verifies the derivation path for Hedera (m/44'/3030'/0'/0'/i').
bool hedera_derivation_path_guard(const uint32_t *path, uint8_t levels);

// Formats a raw public key into a hex string for display.
void hedera_format_pubkey(const uint8_t *pubkey, char *out_str);

// Formats an AccountID protobuf struct into a human-readable string "shard.realm.num".
void hedera_format_account_id(const Hedera_AccountID *account_id, char *out_str);

// Formats a tinybar amount into an HBAR string with 8 decimal places.
void hedera_format_tinybars_to_hbar_string(int64_t tinybars, char *out_str);

#endif // HEDERA_HELPERS_H