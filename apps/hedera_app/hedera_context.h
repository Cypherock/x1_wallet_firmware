/**
 * @file    hedera_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the Hedera app.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#ifndef HEDERA_CONTEXT_H
#define HEDERA_CONTEXT_H

#include <stdbool.h>
#include <stdint.h>

#define HEDERA_NAME "Hedera"
#define HEDERA_LUNIT "HBAR"

#define HEDERA_COIN_DEPTH 5

// Derivation path: m/44'/3030'/0'/0'/i'
#define HEDERA_PURPOSE_INDEX (0x80000000 | 44)
#define HEDERA_COIN_INDEX    (0x80000000 | 3030)
#define HEDERA_ACCOUNT_INDEX (0x80000000 | 0)
#define HEDERA_CHANGE_INDEX  (0x00000000 | 0)

#define HEDERA_PUB_KEY_SIZE 32  // Raw Ed25519 public key
#define HEDERA_ADDRESS_STRING_SIZE (HEDERA_PUB_KEY_SIZE * 2 + 1) // Hex string + null
#define HEDERA_SIGNATURE_SIZE 64 // Raw Ed25519 signature

#define MAX_TXN_SIZE 512 // Maximum size of a serialized transaction body

#endif /* HEDERA_CONTEXT_H */