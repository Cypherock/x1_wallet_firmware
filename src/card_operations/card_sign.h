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
#include "stdbool.h"
#include "stdint.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define FAMILY_ID_SIZE 4
#define INIT_SIGN_SERIAL_CONFIG(acceptable_cards,                              \
                                family_id,                                     \
                                skip_card_removal,                             \
                                heading,                                       \
                                message,                                       \
                                serial_buffer)                                 \
                                                                               \
  {                                                                            \
    acceptable_cards, family_id, CARD_SIGN_SERIAL, skip_card_removal, heading, \
        message, serial_buffer, 0, {                                           \
      0                                                                        \
    }                                                                          \
  }

#define INIT_SIGN_CUSTOM_CONFIG(acceptable_cards,                              \
                                family_id,                                     \
                                skip_card_removal,                             \
                                heading,                                       \
                                message,                                       \
                                challenge_buffer,                              \
                                challenge_size)                                \
                                                                               \
  {                                                                            \
    acceptable_cards, family_id, CARD_SIGN_CUSTOM, skip_card_removal, heading, \
        message, challenge_buffer, challenge_size, {                           \
      0                                                                        \
    }                                                                          \
  }

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum card_sign_type {
  CARD_SIGN_SERIAL = 1,
  CARD_SIGN_CUSTOM,
} card_sign_type_e;

typedef struct card_sign_data_config {
  uint8_t acceptable_cards;
  const uint8_t *family_id;
  card_sign_type_e sign_type;
  bool skip_card_removal;
  const char *heading;
  const char *message;

  uint8_t *data;
  uint16_t data_size;
  uint8_t signature[64];
} card_sign_data_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
card_error_type_e card_sign_auth_data(card_sign_data_config_t *sign_data);
#endif