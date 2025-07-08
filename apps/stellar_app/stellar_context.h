/**
 * @file    stellar_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the STELLAR app
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STELLAR_CONTEXT_H
#define STELLAR_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Stellar network constants
#define STELLAR_NAME "Stellar"
#define STELLAR_LUNIT "XLM"

// Key and address sizes
#define STELLAR_PUB_KEY_SIZE 33
#define STELLAR_PUBKEY_RAW_SIZE 32
#define STELLAR_PRIVATE_KEY_SIZE 32
#define STELLAR_ADDRESS_LENGTH 57
#define STELLAR_SECRET_KEY_LENGTH 57
#define STELLAR_SIGNATURE_SIZE 64

// Network passphrases
#define TESTNET_PASSPHRASE "Test SDF Network ; September 2015"
#define MAINNET_PASSPHRASE "Public Global Stellar Network ; September 2015"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// TODO: Populate structure for STELLAR
typedef struct {
} stellar_config_t;

// Stellar Memo types
typedef enum {
  MEMO_NONE = 0,
  MEMO_TEXT = 1,
  MEMO_ID = 2,
  MEMO_HASH = 3,
  MEMO_RETURN = 4
} stellar_memo_type_t;

// Stellar transaction structures
typedef struct {
  uint8_t source_account[32];
  uint64_t sequence_number;
  uint32_t fee;
  uint32_t operation_count;
  uint32_t operation_type;    // CREATE_ACCOUNT = 0, PAYMENT = 1
  stellar_memo_type_t memo_type;
  union {
    char text[29];       // MEMO_TEXT (max 28 bytes + 1 byte delimiter)
    uint64_t id;         // MEMO_ID
    uint8_t hash[32];    // MEMO_HASH or MEMO_RETURN(32 bytes)
  } memo;
} stellar_transaction_t;

typedef struct {
  uint8_t destination[32];
  uint64_t amount;
} stellar_payment_t;

typedef enum {
  STELLAR_NETWORK_MAINNET = 0,
  STELLAR_NETWORK_TESTNET = 1
} stellar_network_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Generates a Stellar address from a public key
 * @details Follows the Stellar address generation algorithm:
 * 1. Creates a payload with account ID type (0x30) and the public key
 * 2. Calculates CRC16 checksum
 * 3. Encodes the result using base32
 *
 * @param public_key The 32-byte ED25519 public key
 * @param address Buffer to store the resulting address (must be at least
 * STELLAR_ADDRESS_LENGTH bytes)
 * @return true if the address was generated successfully, false otherwise
 */
bool stellar_generate_address(const uint8_t *public_key, char *address);

#endif /* STELLAR_CONTEXT_H */