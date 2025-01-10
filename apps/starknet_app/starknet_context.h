/**
 * @file    starknet_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the STARKNET app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_CONTEXT_H
#define STARKNET_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define STARKNET_IMPLICIT_ACCOUNT_DEPTH 6

// m/2645'/1195502025'/1148870696'/0'/0'/i
#define STARKNET_PURPOSE_INDEX (0x80000000 + 0xA55)             // 2645'
#define STARKNET_LAYER_INDEX (0x80000000 + 0x4741E9C9)          // 1195502025'
#define STARKNET_APPLICATION_INDEX (0x80000000 + 0x447A6028)    // 1148870696'
#define STARKNET_ETH_1_INDEX 0x80000000                         // 0'
#define STARKNET_ETH_2_INDEX 0x80000000                         // 0'

/// this makes length of 5 with a termination NULL byte
#define STARKNET_SHORT_NAME_MAX_SIZE 6
/// this makes length of 5 with a termination NULL byte
#define STARKNET_LONG_NAME_MAX_SIZE 9

#define STARKNET_BIGNUM_SIZE 32

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /** Largest unit/denomination indicator/symbol. This will be used in UX for
   * displaying fees and amount.
   */
  const char lunit1_name[STARKNET_SHORT_NAME_MAX_SIZE];
  const char lunit2_name[STARKNET_SHORT_NAME_MAX_SIZE];
  /** Common name of the blockchain known to the users. This will be used in UX
   */
  const char name[STARKNET_LONG_NAME_MAX_SIZE];
} starknet_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern const starknet_config_t starknet_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* STARKNET_CONTEXT_H */
