/**
 * @file    sia_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the SIA app
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SIA_CONTEXT_H
#define SIA_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Sia network constants
#define SIA_NAME "Sia"
#define SIA_LUNIT "SC"

// Key and address sizes
#define SIA_PUBLIC_KEY_SIZE 32
#define SIA_PRIVATE_KEY_SIZE 32
#define SIA_ADDRESS_SIZE 77
#define SIA_SIGNATURE_SIZE 64

// Transaction limits
#define SIA_MAX_INPUTS 200    // Referred from Btc
#define SIA_MAX_OUTPUTS 2

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  uint64_t input_count;
  uint8_t parent_ids[SIA_MAX_INPUTS][32];
  uint64_t output_count;
  struct {
    uint8_t address_hash[32];
    uint64_t value_lo, value_hi;
  } outputs[SIA_MAX_OUTPUTS];
  uint64_t fee_lo, fee_hi;
} sia_transaction_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* SIA_CONTEXT_H */