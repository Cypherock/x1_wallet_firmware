/**
 * @file    coin_specific_data.h
 * @author  Cypherock X1 Team
 * @brief   Header for coin specific data.
 *          Contains declarations for coin specific data functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef FLASH_COIN_SPECIFIC_DATA_H
#define FLASH_COIN_SPECIFIC_DATA_H
#include <coin_utils.h>
#include <flash_api.h>
#include <flash_if.h>
#include <stdint.h>

#define FLASH_COIN_SPECIFIC_PAGE_COUNT 8

#define FLASH_COIN_SPECIFIC_BASE_ADDRESS                                       \
  (1 + FLASH_END - FLASH_PAGE_SIZE * FLASH_COIN_SPECIFIC_PAGE_COUNT)
#define MAX_COIN_DATA_LENGTH 512
#define MAX_UNIQUE_COIN_COUNT 1
#define GET_NEXT_MULTIPLE_OF_8(x) (((x) + 7) & ~7)

typedef enum Coin_Specific_Data_Tag {
  TAG_CSD_WALLET_ID = 0x01,
  TAG_CSD_COIN_DATA = 0x02,
} Coin_Specific_Data_Tag;

typedef enum Coin_Specific_Data_Status {
  CSD_STATUS_OK = 0x00,
  CSD_STATUS_NOT_ENOUGH_SPACE = 0x01,
  CSD_STATUS_DATA_NOT_FOUND = 0x02,
} Coin_Specific_Data_Status;

typedef struct Coin_Specific_Data_Struct {
  Coin_Type coin_type;
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t *coin_data;
} Coin_Specific_Data_Struct;

/**
 * @brief fills an byte array in tlv format
 *
 * @param array
 * resultant array to be filled.
 * @param starting_index 				starting index of the
 * array.
 * @param tag tag of the data.
 * @param length length of the data.
 * @param data data to be filled in the array.
 */
void fill_flash_tlv(uint8_t *array,
                    uint16_t *starting_index,
                    uint8_t tag,
                    uint16_t length,
                    const uint8_t *data);

/**
 * @brief finds the coin specific data by coin_specific_data_struct in the
 * flash.
 *
 * @param coin_specific_data		coin specific data struct to be found
 * and filled.
 * @param coin_data_max_len 		maximum length of the coin data.
 * @param coin_data_len 				length of the coin data.
 */
int get_coin_data(Coin_Specific_Data_Struct *coin_specific_data,
                  const uint16_t coin_data_max_len,
                  uint16_t *coin_data_len);

/**
 * @brief insert the latest coin data in flash by coin_specific_data_struct.
 *
 * @param coin_specific_data    coin specific data struct to be updated.
 * @param coin_data_size 				size of the coin data in
 * the given struct.
 */
int set_coin_data(const Coin_Specific_Data_Struct *coin_specific_data,
                  const uint16_t coin_data_size);

/**
 * @brief erase the coin specific data in flash.
 */
void erase_flash_coin_specific_data();

#endif    // FLASH_COIN_SPECIFIC_DATA_H