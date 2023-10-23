/**
 * @file    check_pairing.h
 * @author  Cypherock X1 Team
 * @brief   Api to tap a card and tell if it is paired.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CHECK_PAIRING_H
#define CHECK_PAIRING_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "card_operation_typedefs.h"
#include "flash_config.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct check_pairing_result {
  bool is_paired;
  uint8_t card_number;
  uint8_t family_id[FAMILY_ID_SIZE];
  uint32_t error_code;
} check_pairing_result_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handles card tap and pairing check
 * @details The function depends on card_initialize_applet for initializing
 * applet and populating necessary card information. The function returns the
 * operation status and only for CARD_OPERATION_SUCCESS will the result hold any
 * valid information. The function queries the keystore entries (@ref
 * get_paired_card_index) to decide if the tapped card is paired.
 *
 * @param result Reference to the check_pairing_result_t
 *
 * @return card_error_type_s Enum indicating operation status
 */
card_error_type_e card_check_pairing(check_pairing_result_t *result);

#endif
