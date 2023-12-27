/**
 * @file    nfc.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */
#include "nfc.h"

#include <math.h>

#include "app_error.h"
#include "application_startup.h"
#include "assert_conf.h"
#include "sys_state.h"
#include "utils.h"
#include "wallet_utilities.h"

#define SEND_PACKET_MAX_LEN 236
#define RECV_PACKET_MAX_ENC_LEN 242
#define RECV_PACKET_MAX_LEN 225

static void (*early_exit_handler)() = NULL;
static uint8_t nfc_device_key_id[4];
static bool nfc_secure_comm = true;
static uint8_t request_chain_pkt[] = {0x00, 0xCF, 0x00, 0x00};

/**
 * @brief Check if any error is received from NFC.
 *
 * @param err_code Error code with err macro.
 */
void my_error_check(ret_code_t err_code) {
  // Some kind of error handler which can be defined globally
  if (err_code != STM_SUCCESS) {
  }
}

ret_code_t nfc_init() {
  // Init PN532. Call this at start of program
  return adafruit_pn532_init(false);
}

uint32_t nfc_diagnose_antenna_hw() {
  uint32_t result = 0;
  uint32_t err;

  LOG_ERROR("NFC hw error logging");

  // Poll and log different thresholds for NFC antenna self test
  for (int i = 0; i < 8; i++) {
    err = adafruit_diagnose_self_antenna(0x20 + (i << 1));
    if (err != 0) {
      LOG_ERROR("NFC-HW l:%dmA, h:%dmA, %02x", 25, 45 + (i * 15), err);
    }
    err = adafruit_diagnose_self_antenna(0x30 + (i << 1));
    if (err != 0) {
      LOG_ERROR("NFC-HW l:%dmA, h:%dmA, %02x", 35, 45 + (i * 15), err);
    }
  }

  // Detect self antenna fault with 105mA high current threshold and 25mA low
  // current threshold
  if (adafruit_diagnose_self_antenna(NFC_ANTENNA_CURRENT_TH) != 0)
    result += (1 << NFC_ANTENNA_STATUS_BIT);
  return result;
}

uint32_t nfc_diagnose_card_presence() {
  return adafruit_diagnose_card_presence();
}

void nfc_detect_card_removal() {
#if DEV_BUILD == 0
  uint32_t err = 0;
  uint8_t err_count = 0;
  do {
    err = adafruit_diagnose_card_presence();
    if (err != 0) {
      err_count++;
    } else {
      err_count = 0;
    }
  } while (err_count <= 5);
  if (err != 1)
    LOG_CRITICAL("xxx34 diag fault:%ld", err);
#endif
}

ret_code_t nfc_select_card() {
  // tag_info stores data of card selected like UID. Useful for identifying
  // card.
  ret_code_t err_code = STM_ERROR_NULL;    // random error. added to remove
                                           // warning
  nfc_a_tag_info tag_info;
  sys_flow_cntrl_u.bits.nfc_off = false;
  uint32_t system_clock = uwTick;
  while (err_code != STM_SUCCESS && !CY_Read_Reset_Flow()) {
    reset_inactivity_timer();
    err_code =
        adafruit_pn532_nfc_a_target_init(&tag_info, DEFAULT_NFC_TG_INIT_TIME);
    if (CY_Read_Reset_Flow() && early_exit_handler) {
      (*early_exit_handler)();
      return STM_ERROR_NULL;
    }
    //        }
    // TAG_DETECT_TIMEOUT is used to specify the wait time for a card. Defined
    // in sdk_config.h
  }
  LOG_SWV("Card selected in %lums\n", uwTick - system_clock);

  return err_code;
}

void nfc_deselect_card() {
  sys_flow_cntrl_u.bits.nfc_off = true;
  adafruit_pn532_release();
  adafruit_pn532_field_off();
  BSP_DelayMs(50);
}

ret_code_t nfc_wait_for_card(const uint16_t wait_time) {
  nfc_a_tag_info tag_info;
  sys_flow_cntrl_u.bits.nfc_off = false;
  return adafruit_pn532_nfc_a_target_init(&tag_info, wait_time);
}

ISO7816 nfc_select_applet(uint8_t expected_family_id[],
                          uint8_t *acceptable_cards,
                          uint8_t *version,
                          uint8_t *card_key_id,
                          uint8_t *recovery_mode) {
  ASSERT(expected_family_id != NULL);
  ASSERT(acceptable_cards != NULL);

  ISO7816 status_word;
  uint8_t send_apdu[255], recv_apdu[255] = {0},
                          _version[CARD_VERSION_SIZE] = {0};
  uint16_t send_len = 5, recv_len = 236;

  send_len = create_apdu_select_applet(send_apdu);

  nfc_secure_comm = false;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Applet selected in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    uint8_t actual_family_id[FAMILY_ID_SIZE + 2], card_number;

    status_word = extract_card_detail_from_apdu(recv_apdu,
                                                recv_len,
                                                actual_family_id,
                                                _version,
                                                &card_number,
                                                card_key_id,
                                                recovery_mode);
    LOG_ERROR("cno %08X%02X", U32_READ_BE_ARRAY(actual_family_id), card_number);

    if (status_word == SW_NO_ERROR) {
      bool first_time = true;
      if (_version[0] == 0x01)
        return SW_INCOMPATIBLE_APPLET;
      if (version)
        memcpy(version, _version, CARD_VERSION_SIZE);

      uint8_t compareIndex = 0;
      for (; compareIndex < FAMILY_ID_SIZE; compareIndex++) {
        if (expected_family_id[compareIndex] != DEFAULT_VALUE_IN_FLASH) {
          first_time = false;
        }
      }

      if (!first_time) {
        // compare family id
        if (memcmp(actual_family_id, expected_family_id, FAMILY_ID_SIZE) != 0) {
          return SW_FILE_INVALID;    // Invalid Family ID
        }
      } else {    // first time. so set the family id.
        memcpy(expected_family_id, actual_family_id, FAMILY_ID_SIZE + 2);
      }

      if (((*acceptable_cards) >> (card_number - 1)) & 1) {
        // card number is accepted
        (*acceptable_cards) &= ~(1 << (card_number - 1));    // clear the bit
        return SW_NO_ERROR;
      } else {
        return SW_CONDITIONS_NOT_SATISFIED;    // wrong card number
      }

    } else {
      return status_word;
    }
  }
  return 0;
}

ISO7816 nfc_pair(uint8_t *data_inOut, uint8_t *length_inOut) {
  ASSERT(data_inOut != NULL);
  ASSERT(length_inOut != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[255], recv_apdu[255] = {0};
  uint16_t send_len = 5, recv_len = 236;

  send_len = create_apdu_pair(data_inOut, *length_inOut, send_apdu);

  nfc_secure_comm = false;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Pairing in %lums\n", uwTick - system_clock);

  memset(send_apdu, 0, sizeof(send_apdu));
  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] << 8);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      // Extracting Data from APDU
      *length_inOut = recv_len;
      memcpy(data_inOut, recv_apdu, recv_len);
    }
  }

  memset(recv_apdu, 0, sizeof(recv_apdu));
  return status_word;
}

ISO7816 nfc_unpair() {
  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[255], recv_apdu[255] = {0};
  uint16_t send_len = 5, recv_len = 236;

  hex_string_to_byte_array("00130000", 8, send_apdu);
  nfc_secure_comm = true;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] << 8);
    status_word += recv_apdu[recv_len - 1];
  }

  return status_word;
}

ISO7816 nfc_list_all_wallet(wallet_list_t *wallet_list) {
  // Select card before.
  ISO7816 status_word;
  uint8_t send_apdu[300], send_len;

  // Use the same buffer to receive NFC apdu
  uint8_t *recv_apdu = send_apdu;

  // recv_len receives the length of response APDU. It also
  // acts as expected length of response APDU.
  uint16_t recv_len = 236;

  send_len = create_apdu_list_wallet(send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("List wallet in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      apdu_extract_wallet_list(wallet_list, recv_apdu, recv_len);
    }
    return status_word;
  }
  return 0;
}

ISO7816 nfc_add_wallet(const struct Wallet *wallet) {
  ASSERT(wallet != NULL);

  // Call nfc_select_card() before
  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  calculate_checksum(wallet, (uint8_t *)wallet->checksum);
  if (WALLET_IS_ARBITRARY_DATA(wallet->wallet_info))
    send_len = create_apdu_add_arbitrary_data(wallet, send_apdu);
  else
    send_len = create_apdu_add_wallet(wallet, send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Add wallet in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    if (recv_len != ADD_WALLET_EXPECTED_LENGTH)
      my_error_check(STM_ERROR_INVALID_LENGTH);
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];
  }
  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_retrieve_wallet(struct Wallet *wallet) {
  ASSERT(wallet != NULL);

  // Call nfc_select_card() before
  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = RETRIEVE_WALLET_EXPECTED_LENGTH + 32;

  if (WALLET_IS_ARBITRARY_DATA(wallet->wallet_info))
    recv_len = 244;
  send_len = create_apdu_retrieve_wallet(wallet, send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Retrieve wallet in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      // Extract data from APDU and add it to struct Wallet
      extract_from_apdu(wallet, recv_apdu, recv_len);
      Card_Data_errors_t status = validate_wallet(wallet);
      if (status != VALID_DATA) {
        LOG_CRITICAL("edat", status);
        status_word = 0;
      }
    }
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_delete_wallet(const struct Wallet *wallet) {
  ASSERT(wallet != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_delete_wallet(wallet, send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Delete wallet in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    if (recv_len != DELETE_WALLET_EXPECTED_LENGTH)
      my_error_check(STM_ERROR_INVALID_LENGTH);
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];
  }
  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_ecdsa(uint8_t data_inOut[ECDSA_SIGNATURE_SIZE],
                  uint16_t *length_inOut) {
  ASSERT(data_inOut != NULL);
  ASSERT(length_inOut != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_ecdsa(data_inOut, *length_inOut, send_apdu);

  nfc_secure_comm = false;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("ECDSA in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  status_word = (recv_apdu[recv_len - 2] * 256);
  status_word += recv_apdu[recv_len - 1];

  if (status_word == SW_NO_ERROR) {
    if (recv_len != ECDSA_EXPECTED_LENGTH) {
      status_word = CARD_SIGNATURE_INCORRECT_LEN;
      LOG_ERROR("Card error: %04x", status_word);
    } else {
      // Extracting Data from APDU
      *length_inOut = recv_apdu[1];
      memcpy(data_inOut, recv_apdu + 2, recv_apdu[1]);
    }
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_verify_challenge(const uint8_t name[NAME_SIZE],
                             const uint8_t nonce[POW_NONCE_SIZE],
                             const uint8_t password[BLOCK_SIZE]) {
  ASSERT(name != NULL);
  ASSERT(nonce != NULL);
  ASSERT(password != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_verify_challenge(name, nonce, password, send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Verify challenge in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_get_challenge(const uint8_t name[NAME_SIZE],
                          uint8_t target[SHA256_SIZE],
                          uint8_t random_number[POW_RAND_NUMBER_SIZE]) {
  ASSERT(name != NULL);
  ASSERT(target != NULL);
  ASSERT(random_number != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_get_challenge(name, send_apdu);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Get challenge in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      extract_apdu_get_challenge(target, random_number, recv_apdu, recv_len);
    }
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_encrypt_data(const uint8_t name[NAME_SIZE],
                         const uint8_t *plain_data,
                         const uint16_t plain_data_size,
                         uint8_t *encrypted_data,
                         uint16_t *encrypted_data_size) {
  ASSERT(name != NULL);
  ASSERT(plain_data != NULL);
  ASSERT(encrypted_data != NULL);
  ASSERT(encrypted_data_size != NULL);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_inheritance(name,
                                     plain_data,
                                     plain_data_size,
                                     send_apdu,
                                     P1_INHERITANCE_ENCRYPT_DATA);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Encrypt data in %lums\n", uwTick - system_clock);

  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      // Extracting Data from APDU
      *encrypted_data_size = recv_apdu[1];
      memcpy(encrypted_data, recv_apdu + 2, recv_apdu[1]);
    }
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ISO7816 nfc_decrypt_data(const uint8_t name[NAME_SIZE],
                         uint8_t *plain_data,
                         uint16_t *plain_data_size,
                         const uint8_t *encrypted_data,
                         const uint16_t encrypted_data_size) {
  ASSERT(name != NULL);
  ASSERT(plain_data != NULL);
  ASSERT(encrypted_data != NULL);
  ASSERT(encrypted_data_size != 0);

  ISO7816 status_word = CLA_ISO7816;
  uint8_t send_apdu[600] = {0}, *recv_apdu = send_apdu;
  uint16_t send_len = 0, recv_len = 236;

  send_len = create_apdu_inheritance(name,
                                     encrypted_data,
                                     encrypted_data_size,
                                     send_apdu,
                                     P1_INHERITANCE_DECRYPT_DATA);

  nfc_secure_comm = true;
  uint32_t system_clock = uwTick;
  ret_code_t err_code =
      nfc_exchange_apdu(send_apdu, send_len, recv_apdu, &recv_len);
  LOG_SWV("Decrypt data in %lums\n", uwTick - system_clock);
  if (err_code != STM_SUCCESS) {
    return err_code;
  } else {
    status_word = (recv_apdu[recv_len - 2] * 256);
    status_word += recv_apdu[recv_len - 1];

    if (status_word == SW_NO_ERROR) {
      // Extracting Data from APDU
      *plain_data_size = recv_apdu[1];
      memcpy(plain_data, recv_apdu + 2, recv_apdu[1]);
    }
  }

  memzero(recv_apdu, sizeof(send_apdu));
  return status_word;
}

ret_code_t nfc_exchange_apdu(uint8_t *send_apdu,
                             uint16_t send_len,
                             uint8_t *recv_apdu,
                             uint16_t *recv_len) {
  ASSERT(send_apdu != NULL);
  ASSERT(recv_apdu != NULL);
  ASSERT(recv_len != NULL);
  ASSERT(send_len != 0);

  ret_code_t err_code = adafruit_diagnose_card_presence();
  if (err_code != 0)
    return NFC_CARD_ABSENT;

  uint8_t total_packets = 0, header[5], status[2] = {0};
  uint8_t recv_pkt_len = 236, send_pkt_len;
  uint16_t off = OFFSET_CDATA;

  memcpy(header, send_apdu, OFFSET_CDATA);
  if (nfc_secure_comm) {
    if (send_apdu[OFFSET_LC] > 0) {
      send_len -= OFFSET_CDATA;
      if ((err_code = apdu_encrypt_data(send_apdu + OFFSET_CDATA, &send_len)) !=
          STM_SUCCESS)
        return err_code;
      send_len += OFFSET_CDATA;
    }
    memcpy(send_apdu + send_len, nfc_device_key_id, sizeof(nfc_device_key_id));
    send_len += sizeof(nfc_device_key_id);
    send_apdu[OFFSET_LC] += sizeof(nfc_device_key_id);
  }

  total_packets = ceil(send_len / (1.0 * SEND_PACKET_MAX_LEN));
  for (int packet = 1; packet <= total_packets;) {
    recv_pkt_len = RECV_PACKET_MAX_ENC_LEN; /* On every request set acceptable
                                               packet length */

    /**
     * Sets appropriate CLA byte for each packet. CLA byte (first byte of
     * packet) is used to determine the packet type in a multi-packet C-APDU
     * (Command APDU). The classification is as follows: <br/> <ul> <li>0x01 :
     * First packet of a multi-packet APDU</li> <li>0x00 : Last packet of a
     * multi-packet APDU</li> <li>0x80 : Middle packets of a multi-packet
     * APDU</li>
     * </ul>
     */
    send_apdu[off - OFFSET_CDATA] =
        packet == total_packets ? 0x00 : (packet == 1 ? 0x10 : 0x80);

    /** Copy rest of the header (INS,P1,P2,Lc : 4 bytes after CLA) as it is. */
    if (off > OFFSET_CDATA)
      memcpy(send_apdu + off - OFFSET_CDATA + 1, header + 1, OFFSET_CDATA - 1);

    /** Fix on length of data to be sent in the current packet. @see
     * SEND_PACKET_MAX_LEN puts an upper limit */
    if ((send_len - off) > SEND_PACKET_MAX_LEN)
      send_pkt_len = SEND_PACKET_MAX_LEN;
    else
      send_pkt_len = send_len - off;
    send_apdu[off - 1] = send_pkt_len;

    /** Exchange the C-APDU */
    err_code = adafruit_pn532_in_data_exchange(send_apdu + off - OFFSET_CDATA,
                                               send_pkt_len + OFFSET_CDATA,
                                               recv_apdu,
                                               &recv_pkt_len);

    /** Verify card's response. */
    if (err_code != STM_SUCCESS) {
      LOG_ERROR("err:%08X\n", err_code);
      return err_code;
    }
    if (recv_pkt_len < 2)
      return STM_ERROR_INVALID_LENGTH;
    if (packet == total_packets)
      break;
    off += SEND_PACKET_MAX_LEN;

    /**
     * Check if card properly handled the current packet and has sufficient
     * buffer left with it.
     */
    if (recv_pkt_len != 2 ||
        (recv_apdu[1] != 0xFF && recv_apdu[1] < (send_len - off))) {
      return STM_ERROR_INVALID_LENGTH;
    }
    packet++;
  }

  /** Check response status of received packet then decrypt the packet if
   * necessary */
  if (nfc_secure_comm && recv_pkt_len > 2)
    err_code = apdu_decrypt_data(recv_apdu, &recv_pkt_len);
  if (err_code != STM_SUCCESS)
    return err_code;

  /** Prepare to request next packet from the card */
  *recv_len = recv_pkt_len;
  recv_pkt_len = RECV_PACKET_MAX_ENC_LEN;
  request_chain_pkt[2] = ceil(*recv_len * 1.0 / RECV_PACKET_MAX_LEN);

  /** Request all the remaining packets of multi-packet response */
  while (recv_apdu[*recv_len - 2] == 0x61) {
    *recv_len -= 2;
    err_code = adafruit_pn532_in_data_exchange(request_chain_pkt,
                                               sizeof(request_chain_pkt),
                                               recv_apdu + *recv_len,
                                               &recv_pkt_len);

    /** Verify card's response */
    if (err_code != STM_SUCCESS) {
      LOG_ERROR("err:%08X\n", err_code);
      return err_code;
    }
    if (recv_pkt_len < 2)
      return STM_ERROR_INVALID_LENGTH;

    /** Check response status of the packet then decrypt the packet if necessary
     */
    status[0] = recv_apdu[*recv_len + recv_pkt_len - 2];
    status[1] = recv_apdu[*recv_len + recv_pkt_len - 1];
    if (nfc_secure_comm && recv_pkt_len > 2)
      err_code = apdu_decrypt_data(recv_apdu + *recv_len, &recv_pkt_len);
    if (err_code != STM_SUCCESS)
      return err_code;

    /** Prepare to request next packet from the card */
    *recv_len += recv_pkt_len;
    recv_pkt_len = RECV_PACKET_MAX_ENC_LEN;
    request_chain_pkt[2] = *recv_len / RECV_PACKET_MAX_LEN + 1;
  }

  adafruit_pn532_clear_buffers();
  *recv_len = extract_card_data_health(recv_apdu, *recv_len);
  return err_code;
}

void nfc_set_early_exit_handler(void (*handler)()) {
  early_exit_handler = handler;
}

void nfc_set_device_key_id(const uint8_t *device_key_id) {
  memcpy(nfc_device_key_id, device_key_id, 4);
}

void nfc_set_secure_comm(bool state) {
  nfc_secure_comm = state;
}
