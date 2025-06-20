/**
 * @file    icrc_canisters.h
 * @author  Cypherock X1 Team
 * @brief   ICRC tokens related definitions and types
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef ICRC_CANISTERS_H
#define ICRC_CANISTERS_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "icp_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Number of entries in whitelisted canisters list
#define ICRC_WHITELISTED_TOKEN_COUNT 17

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /// upto 29-byte ledger cansister id of the token
  const uint8_t ledger_canister_id[ICP_LEDGER_CANISTER_ID_LENGTH];
  /// Symbol (short alphabetical representation) of the icrc token
  const char *symbol;
  /// Decimal value used to display the amount in token transfer in token units
  const uint8_t decimal;
} icrc_token_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
/**
 * @brief Whitelisted tokens with respective canister ids and symbols
 * @details A map of ICRC token ledger canister ids with their token symbols.
 * These will enable the device to verify the token transaction in a
 * user-friendly manner.
 */
extern const icrc_token_t icrc_tokens[];
/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif    // ICRC_CANISTERS_H
