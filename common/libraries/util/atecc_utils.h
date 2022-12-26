/**
 * @file atecc_utils.h
 * @author  Cypherock X1 Team
 * @brief Util functions related to atecc signing
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef ATECC_UTILS_H
#define ATECC_UTILS_H

#pragma once

#include "atca_host.h"
#include "stm32l4xx_it.h"

#define SIGNATURE_SIZE          64
#define POSTFIX1_SIZE           7
#define POSTFIX2_SIZE           23
#define DEVICE_SERIAL_SIZE      32
#define AUTH_DATA_SERIAL_SIGN_MSG_SIZE      (POSTFIX1_SIZE + POSTFIX2_SIZE + SIGNATURE_SIZE + DEVICE_SERIAL_SIZE)
#define AUTH_DATA_CHALLENGE_SIGN_MSG_SIZE   (POSTFIX1_SIZE + POSTFIX2_SIZE + SIGNATURE_SIZE)

/**
 * @brief
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
typedef struct auth_data_struct {
  uint8_t postfix1[POSTFIX1_SIZE],
      postfix2[POSTFIX2_SIZE],
      signature[SIGNATURE_SIZE],
      serial[DEVICE_SERIAL_SIZE];
} auth_data_t;

typedef enum {
  slot_0_unused = 0U,
  slot_1_unused = 1U,
  slot_2_auth_key = 2U,
  slot_3_nfc_pair_key = 3U,
  slot_4_unused = 4U,
  slot_5_challenge = 5U,
  slot_6_io_key = 6U,
  slot_7_unused = 7U,
  slot_8_serial = 8U,
  slot_9_unused = 9U,
  slot_10_unused = 10U,
  slot_11_unused = 11U,
  slot_12_unused = 12U,
  slot_13_unused = 13U,
  slot_14_unused = 14U,
  slot_15_unused = 15U
} atecc_slot_define_t;

typedef struct {
  uint8_t device_serial[DEVICE_SERIAL_SIZE], retries;
  ATCA_STATUS status;
  ATCAIfaceCfg *cfg_atecc608a_iface;
} atecc_data_t;

/**
 * @brief
 * @details
 *
 * @see swap_transaction_controller(), device_authentication_controller()
 *
 * @param slot
 * @param data
 * @param digest
 * @param postfix
 * @param atecc_data
 */
void helper_get_gendig_hash(atecc_slot_define_t slot,
                            uint8_t *data,
                            uint8_t *digest,
                            uint8_t *postfix,
                            atecc_data_t atecc_data);

/**
 * @brief
 * @details
 *
 * @see swap_transaction_controller(), device_authentication_controller()
 *
 * @param param
 * @param mode
 * @param priv_key_id
 * @param data_key_id
 * @param atecc_data
 * @return
 */
ATCA_STATUS helper_sign_internal_msg(struct atca_sign_internal_in_out *param,
                                     uint8_t mode,
                                     uint8_t priv_key_id,
                                     uint8_t data_key_id,
                                     atecc_data_t atecc_data);

#endif //ATECC_UTILS_H
