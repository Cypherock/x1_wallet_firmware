/**
 * @file    device_authentication_api.h
 * @author  Cypherock X1 Team
 * @brief   Header file for APIs to support X1 vault device authentication
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef DEVICE_AUTHENTICATION_API_H
#define DEVICE_AUTHENTICATION_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <manager/core.pb.h>

#include "atca_iface.h"
#include "atca_status.h"
#include "stddef.h"
#include "stdint.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define DEFAULT_ATECC_RETRIES 5
#define DEVICE_SERIAL_SIZE 32

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
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

extern atecc_data_t atecc_data;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This function requests ATECC SE chip to sign the serial number using
 * the device root key
 *
 * @return manager_auth_device_response_t Postfix1, postfix2, device serial and
 * ECDSA signature of the device serial of the particular device
 */
manager_auth_device_response_t sign_serial_number(void);

/**
 * @brief This function requests ATECC SE chip to sign a random number XOR'ed
 * with the hash of the installed firmware on the device serial number using the
 * device root key
 *
 * @param challenge The random challenge of 32 bytes sent by the host
 * @return manager_auth_device_response_t Postfix1, postfix2, device serial and
 * ECDSA signature of the device serial of the particular device
 */
manager_auth_device_response_t sign_random_challenge(uint8_t *challenge);

#endif /* DEVICE_AUTHENTICATION_API_H */
