/**
 * @file    near_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the NEAR app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_CONTEXT_H
#define NEAR_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
// NEAR implicit account derivation path: m/44'/397'/0'/0'/i'
#define NEAR_IMPLICIT_ACCOUNT_DEPTH 5

#define NEAR_PURPOSE_INDEX (0x8000002C)    // 44'
#define NEAR_COIN_INDEX (0x8000018D)       // 397'
#define NEAR_ACCOUNT_INDEX 0x80000000      // 0'
#define NEAR_CHANGE_INDEX 0x80000000       // 0'

#define NEAR_TOKEN_NAME "NEAR"
#define NEAR_CHAIN_NAME "Near"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// TODO: Populate structure for NEAR
typedef struct {
} near_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* NEAR_CONTEXT_H */
