/**
 * @file    constellation_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the CONSTELLATION app
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CONSTELLATION_CONTEXT_H
#define CONSTELLATION_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define CONSTELLATION_NAME "CONSTELLATION"
#define CONSTELLATION_LUNIT "DAG"

#define CONSTELLATION_IMPLICIT_ACCOUNT_DEPTH 5

#define CONSTELLATION_PURPOSE_INDEX 0x8000002C        // 44'
#define CONSTELLATION_COIN_INDEX 0x80000000 + 1137    // 1137'
#define CONSTELLATION_ACCOUNT_INDEX 0x80000000        // 0'
#define CONSTELLATION_CHANGE_INDEX 0x00000000         // 0

#define CONSTELLATION_PUB_KEY_SIZE 65
#define CONSTELLATION_ACCOUNT_ADDRESS_SIZE 40
#define PKCS_PREFIX_SIZE 23
#define PKCS_PREFIXED_PUBKEY_SIZE PKCS_PREFIX_SIZE + CONSTELLATION_PUB_KEY_SIZE
#define SHA256_DIGEST_SIZE 32
#define BS58_ENCODED_SIZE 45
#define CONSTELLATION_TXN_HASH_LENGTH 64
#define CONSTELLATION_SIGN_MSG_PREFIX_LENGTH 31
#define CONSTELLATION_SIGN_DATA_PREFIX_LENGTH 28

#define MAX_ALLOWED_SIZE 20480

/**
 * TODO: update the size of msg data same as MAX_ALLOWED_SIZE.
 * Constraints : The LVGL buffer cannot handle more than 3Kb data size which
 * puts a limit on how much data can be displayed on the device. Possible fix is
 * to show the long messages in chunks in line with max LVGL buffer size.
 */
#define MAX_MSG_DATA_SIZE 3072

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* CONSTELLATION_CONTEXT_H */