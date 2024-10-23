/**
 * @file atecc_utils.h
 * @author  Cypherock X1 Team
 * @brief Util functions related to atecc signing
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef ATECC_UTILS_H
#define ATECC_UTILS_H

#pragma once

#include "atca_host.h"
// #include "stm32l4xx_it.h"
#include "device_authentication_api.h"
#include "nist256p1.h"

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
  uint8_t postfix1[POSTFIX1_SIZE], postfix2[POSTFIX2_SIZE],
      signature[SIGNATURE_SIZE], serial[DEVICE_SERIAL_SIZE];
} auth_data_t;

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
 * @param atecc_value
 */
void helper_get_gendig_hash(atecc_slot_define_t slot,
                            uint8_t *data,
                            uint8_t *digest,
                            uint8_t *postfix,
                            atecc_data_t *atecc_value);

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
 * @param atecc_value
 * @return
 */
ATCA_STATUS helper_sign_internal_msg(struct atca_sign_internal_in_out *param,
                                     uint8_t mode,
                                     uint8_t priv_key_id,
                                     uint8_t data_key_id,
                                     atecc_data_t *atecc_value);

auth_data_t atecc_sign(uint8_t *hash);

#endif    // ATECC_UTILS_H