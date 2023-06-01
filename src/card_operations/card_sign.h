/**
 * @file    card_sign.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INCLUDE_GUARD
#define INCLUDE_GUARD

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_operations.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum card_sign_type {
  CARD_SIGN_SERIAL = 1,
  CARD_SIGN_CUSTOM,
} card_sign_type_e;

typedef struct card_sign_data_config {
  uint8_t acceptable_cards;
  uint8_t family_id[FAMILY_ID_SIZE];
  const char *heading;
  const char *message;
  card_sign_type_e sign_type;
  uint8_t signature[64];
  uint8_t *data;
  uint16_t data_size;
  bool skip_card_removal;
} card_sign_data_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
card_error_type_e card_sign_auth_data(card_sign_data_config_t *sign_data);

__IO inline card_sign_data_config_t init_sign_serial_config(
    const char *heading,
    const char *message,
    const uint8_t *family_id,
    uint8_t acceptable_cards,
    bool skip_card_removal,
    uint8_t *serial_buffer) {
  card_sign_data_config_t sign_data = {
      .acceptable_cards = acceptable_cards,
      .family_id = {0},
      .heading = heading,
      .message = message,
      .sign_type = CARD_SIGN_SERIAL,
      .signature = {0},
      .data = serial_buffer,
      .data_size = 0,
      .skip_card_removal = skip_card_removal,
  };

  memcpy(sign_data.family_id, family_id, FAMILY_ID_SIZE);

  return sign_data;
}

__IO inline card_sign_data_config_t init_sign_custom_config(
    const char *heading,
    const char *message,
    const uint8_t *family_id,
    uint8_t acceptable_cards,
    bool skip_card_removal,
    uint8_t *data,
    uint16_t data_size) {
  card_sign_data_config_t sign_data = {
      .acceptable_cards = acceptable_cards,
      .family_id = {0},
      .heading = heading,
      .message = message,
      .sign_type = CARD_SIGN_SERIAL,
      .signature = {0},
      .data = data,
      .data_size = data_size,
      .skip_card_removal = skip_card_removal,
  };

  memcpy(sign_data.family_id, family_id, FAMILY_ID_SIZE);

  return sign_data;
}

#endif