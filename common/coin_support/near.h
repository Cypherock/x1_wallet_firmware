/**
 * @file    near.h
 * @author  Cypherock X1 Team
 * @brief   Header for near coin.
 *          Stores declarations for near coin functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef NEAR_HEADER
#define NEAR_HEADER

#include <stdint.h>

#include "coin_specific_data.h"
#include "coin_utils.h"

/**
 * @brief serialize a list of account ids to a byte array in tlv format.
 *
 * @param account_ids   list of account ids.
 * @param count         count of account ids.
 * @param data          byte array to store the result.
 * @param data_len      length of byte array.
 */
void near_serialize_account_ids(const char **account_ids,
                                const size_t count,
                                uint8_t *data,
                                uint16_t *data_len);

/**
 * @brief  deserialize a byte array in tlv format to a list of account ids.
 *
 * @param data          byte array to deserialize.
 * @param data_len      length of byte array.
 * @param account_ids   list of account ids to store the result.
 * @param count         count of account ids.
 */
void near_deserialize_account_ids(const uint8_t *data,
                                  const uint16_t data_len,
                                  char **account_ids,
                                  size_t count);

/**
 * @brief get count of account ids stored in the byte array.
 *
 * @param data          byte array to get the count.
 * @param data_len      length of byte array.
 * @return size_t       count of account ids.
 */
size_t near_get_account_ids_count(const uint8_t *data, const uint16_t data_len);

#endif    // NEAR_HEADER
