/**
 * @file    canton_instruments.h
 * @author  Cypherock X1 Team
 * @brief   Canton instruments related definitions and types
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CANTON_INSTRUMENTS_H
#define CANTON_INSTRUMENTS_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "canton_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Number of entries in whitelisted canisters list
#define CANTON_WHITELISTED_INSTRUMENT_COUNT 5

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  canton_instrument_t instrument;
  /// Symbol (short alphabetical representation) of the canton instrument; to be
  /// used as unit also
  const char *symbol;
  /// Decimal value used to display the amount in instrument transfer in
  /// instrument units
  const uint8_t decimal;
} canton_instrument_data_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
/**
 * @brief Whitelisted instruments with respective instrument data
 * @details A map of canton instruments with their instrument data.
 * These will enable the device to verify the instrument transaction in a
 * user-friendly manner.
 */
extern const canton_instrument_data_t canton_instrument_data[];
#endif    // CANTON_INSTRUMENTS_H
