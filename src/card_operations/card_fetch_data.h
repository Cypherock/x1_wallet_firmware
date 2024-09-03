/**
 * @author  Cypherock X1 Team
 * @brief   API for fetching data from card
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_FETCH_DATA_H
#define CARD_FETCH_DATA_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <assert.h>

#include "card_fetch_wallet_list.h"
#include "card_internal.h"
#include "card_operation_typedefs.h"
#include "stdbool.h"
#include "stdint.h"
#include "wallet.h"

#define PLAIN_DATA_BUFFER_SIZE 100    // Card data encryption limit <100 chars>
#define ENCRYPTED_DATA_BUFFER_SIZE 112

#define DATA_CHUNKS_MAX 10

#define PLAIN_DATA_SIZE (PLAIN_DATA_BUFFER_SIZE * DATA_CHUNKS_MAX)
#define ENCRYPTED_DATA_SIZE (ENCRYPTED_DATA_BUFFER_SIZE * DATA_CHUNKS_MAX)

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
#pragma pack(push, 1)
typedef struct {
  uint8_t plain_data[PLAIN_DATA_SIZE];
  uint16_t plain_data_size;
  uint8_t encrypted_data[ENCRYPTED_DATA_SIZE];
  uint16_t encrypted_data_size;
} secure_data_t;
#pragma pack(pop)

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Fetches wallet list from card after applet initialization.
 *
 * @param card_data card_operation_data_t used for applet init.
 *
 * @return The card error type indicating the result of the operation.
 */
card_error_type_e card_get_wallet_list(
    card_operation_data_t card_data,
    const card_fetch_wallet_list_config_t *configuration,
    card_fetch_wallet_list_response_t *response);

/**
 * @brief Fetches and encrypts data for a given wallet ID.
 *
 * This function retrieves plain data associated with a wallet ID, encrypts it,
 * and stores the encrypted data in the provided secure_data_t structures.
 *
 * @param wallet_id Pointer to the wallet ID.
 * @param msgs Pointer to an array of secure_data_t structures to store the
 * encrypted data.
 * @param msg_count Number of secure_data_t structures in the array.
 * @return The card error type indicating the result of the operation.
 */
card_error_type_e card_fetch_encrypt_data(const uint8_t *wallet_id,
                                          secure_data_t *msgs,
                                          size_t msg_count);

/**
 * @brief Fetches and decrypts data from the card.
 *
 * This function retrieves encrypted data associated with a wallet ID from the
 * card, decrypts it, and stores the plain data in the provided secure_data_t
 * structures.
 *
 * @param wallet_id Pointer to the wallet ID.
 * @param msgs Pointer to an array of secure_data_t structures to store the
 * decrypted data.
 * @param msg_count Number of secure_data_t structures in the array.
 * @return The card error type indicating the result of the operation.
 */
card_error_type_e card_fetch_decrypt_data(const uint8_t *wallet_id,
                                          secure_data_t *msgs,
                                          size_t msg_count);
#endif
