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
 * @brief
 *
 * @return A card_error_type_e value representing the result of the operation.
 */
card_error_type_e card_fetch_encrypt_data(uint8_t *wallet_id,
                                          secure_data_t *msgs,
                                          size_t msg_count);

#endif
