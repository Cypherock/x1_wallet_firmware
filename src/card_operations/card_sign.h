/**
 * @file    card_sign.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_SIGN_H
#define CARD_SIGN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "card_return_codes.h"
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
  CARD_SIGN_SERIAL =
      1,    /// Card's serial to be signed is copied to data member of
            /// card_sign_data_config_t object after applet initializatoin
  CARD_SIGN_CUSTOM,    /// Custom data to be signed is provided through data
                       /// member of card_sign_data_config_t pointer object
} card_sign_type_e;

typedef struct card_sign_data_config {
  uint8_t acceptable_cards;
  uint8_t *family_id;
  card_sign_type_e sign_type;
  bool skip_card_removal;    /// If true, handler will not wait for card removal
                             /// in success case
  const char *heading;
  const char *message;

  uint8_t *
      data;    /// Buffer pointer for serial or custom data, in CARD_SIGN_SERIAL
               /// the tapped card's serial is copied to this buffer
  uint16_t data_size;
  uint8_t signature[64];
} card_sign_data_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Signs authentication data with the provided configuration.
 * @details This function signs the authentication data using the provided
 * configuration. The signed data is stored in the signature field of the
 * sign_data parameter. If an error occurs during signing, the function returns
 * the type of error encountered. NOTE: Max size accepted for data member of
 * card_sign_data_config_t is 64 bytes
 *
 * @param sign_data Pointer to the configuration for signing the authentication
 * data.
 *
 * @return CARD_OPERATION_SUCCESS for success, else error
 */
card_error_type_e card_sign_auth_data(card_sign_data_config_t *sign_data);
#endif