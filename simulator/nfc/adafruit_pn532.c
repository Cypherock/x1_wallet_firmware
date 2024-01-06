/*
 * Adafruit PN532 library adapted to use in NRF51 and NRF52
 *
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2012, Adafruit Industries
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// #include "sdk_config.h"
#define __INLINE inline

#ifndef ADAFRUIT_PN532_ENABLED
#define ADAFRUIT_PN532_ENABLED 1
#endif

#ifndef TAG_DETECT_TIMEOUT
#define TAG_DETECT_TIMEOUT 10000
#endif

#ifndef PN532_IRQ
#define PN532_IRQ 0
#endif

#ifndef PN532_RESET
#define PN532_RESET 0
#endif

#ifndef PN532_CONFIG_TWI_INSTANCE
#define PN532_CONFIG_TWI_INSTANCE 0
#endif

#ifndef ADAFRUIT_PN532_LOG_ENABLED
#define ADAFRUIT_PN532_LOG_ENABLED 0
#endif

#ifndef ADAFRUIT_PN532_LOG_LEVEL
#define ADAFRUIT_PN532_LOG_LEVEL 3
#endif

#ifndef ADAFRUIT_PN532_INFO_COLOR
#define ADAFRUIT_PN532_INFO_COLOR 0
#endif

#ifndef PN532_PACKBUFF_SIZE
#define PN532_PACKBUFF_SIZE 256
#endif

#if ADAFRUIT_PN532_ENABLED
#include "adafruit_pn532.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "board.h"

#define STM_LOG_MODULE_NAME adafruit_pn532
#if ADAFRUIT_PN532_LOG_ENABLED
#define STM_LOG_LEVEL ADAFRUIT_PN532_LOG_LEVEL
#define STM_LOG_INFO_COLOR ADAFRUIT_PN532_INFO_COLOR
#else    // ADAFRUIT_PN532_LOG_ENABLED
#define STM_LOG_LEVEL 0
#endif    // ADAFRUIT_PN532_LOG_ENABLED

/**@brief Function for decoding a uint32 value in big-endian format.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static __INLINE uint32_t uint32_big_decode(const uint8_t *p_encoded_data) {
  return ((((uint32_t)((uint8_t *)p_encoded_data)[0]) << 24) |
          (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 16) |
          (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 8) |
          (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 0));
}

// Type 2 Tag page/block read/write restrictions.
#define T2T_MAX_READ_PAGE_NUMBER 255
#define T2T_MIN_WRITE_PAGE_NUMBER 4
#define T2T_MAX_WRITE_PAGE_NUMBER 255

// Lengths and offsets for specific commands and responses.
#define COMMAND_GETFIRMWAREVERSION_LENGTH 1
#define REPLY_GETFIRMWAREVERSION_LENGTH (5 + PN532_FRAME_OVERHEAD)

#define COMMAND_SAMCONFIGURATION_LENGTH 4
#define REPLY_SAMCONFIGURATION_LENGTH (1 + PN532_FRAME_OVERHEAD)

#define COMMAND_POWERDOWN_BASE_LENGTH 2    // No GenerateIRQ parameter.
#define REPLY_POWERDOWN_LENGTH (2 + PN532_FRAME_OVERHEAD)

#define COMMAND_RFCONFIGURATION_MAXRETRIES_LENGTH 5
#define COMMAND_RFCONFIGURATION_RFFIELD_LENGTH 3
#define REPLY_RFCONFIGURATION_LENGTH (1 + PN532_FRAME_OVERHEAD)

#define COMMAND_INLISTPASSIVETARGET_BASE_LENGTH 3
#define REPLY_INLISTPASSIVETARGET_106A_TARGET_LENGTH (17 + PN532_FRAME_OVERHEAD)
#define REPLY_INLISTPASSIVETARGET_106A_NBTG_OFFSET 7
#define REPLY_INLISTPASSIVETARGET_106A_TG_OFFSET 8
#define REPLY_INLISTPASSIVETARGET_106A_SENS_RES_BYTE_1_OFFSET 10
#define REPLY_INLISTPASSIVETARGET_106A_SENS_RES_BYTE_2_OFFSET 9
#define REPLY_INLISTPASSIVETARGET_106A_SEL_RES_OFFSET 11
#define REPLY_INLISTPASSIVETARGET_106A_UID_LEN_OFFSET 12
#define REPLY_INLISTPASSIVETARGET_106A_UID_OFFSET 13

#define COMMAND_INDATAEXCHANGE_BASE_LENGTH 2
#define REPLY_INDATAEXCHANGE_BASE_LENGTH (2 + PN532_FRAME_OVERHEAD)

// Configuration parameters for SAMCONFIGURATION command.
#define SAMCONFIGURATION_MODE_NORMAL 0x01
#define SAMCONFIGURATION_MODE_VIRTUAL_CARD 0x02
#define SAMCONFIGURATION_MODE_WIRED_CARD 0x03
#define SAMCONFIGURATION_MODE_DUAL_CARD 0x04

#define SAMCONFIGURATION_IRQ_ENABLED 0x01
#define SAMCONFIGURATION_IRQ_DISABLED 0x00

// Configuration parameters for POWERDOWN command.
#define POWERDOWN_WAKEUP_IRQ 0x80
#define POWERDOWN_WAKEUP_SPI 0x20

// Configuration parameters for RFCONFIGURATION command.
#define RFCONFIGURATION_CFGITEM_RFFIELD 0x01
#define RFCONFIGURATION_CFGITEM_MAXRETRIES 0x05
#define RFCONFIGURATION_RFFIELD_ON 0x01
#define RFCONFIGURATION_RFFIELD_OFF 0x00

// Error mask for the status mask in INDATAEXCHANGE frame.
#define PN532_STATUS_ERROR_MASK 0x3F

// Size of the PN532 size packet.
#define PN532_ACK_PACKET_SIZE 6

// Default time-out for read_passive_target_id (time required for field scan).
#define PN532_DEFAULT_WAIT_FOR_READY_TIMEOUT 100

/**
 * @brief Information about the communication between the host and the Adafruit
 * PN532 Shield.
 */
typedef struct {
  uint8_t ss;               // !< Slave select signal for SPI.
  uint8_t clk;              // !< Clock signal for SPI.
  uint8_t mosi;             // !< Master output, slave input signal for SPI.
  uint8_t miso;             // !< Master input, slave output signal for SPI.
  uint8_t irq;              // !< Interrupt pin for Adafruit.
  uint8_t reset;            // !< Reset pin for Adafruit.
  uint8_t in_listed_tag;    // !< Tag number of in listed tags.
  bool using_spi;           // !< True if using SPI, false if using I2C.
  bool hardware_spi;        // !< True if using hardware SPI, false if using
                            // software SPI.
} adafruit_pn532;

// ACK frame format.
static const uint8_t m_pn532_ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
// Firmware version reply frame format (preamble to command byte).
static const uint8_t m_pn532_rsp_firmware_ver[] =
    {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

static adafruit_pn532 m_pn532_object = {.clk = 0,
                                        .miso = 0,
                                        .mosi = 0,
                                        .ss = 0,
                                        .irq = PN532_IRQ,
                                        .reset = PN532_RESET,
                                        .using_spi = false,
                                        .hardware_spi = false};

static uint8_t m_pn532_packet_buf[PN532_PACKBUFF_SIZE];

static uint8_t
    m_pn532_rxtx_buffer[PN532_PACKBUFF_SIZE];    /// Buffer for low level
                                                 /// communication.

static bool m_lib_initialized = false;

/**
 * @brief Function to configure pins in host chip.
 *
 * This function configures specific pins to interact with the PN532 module.
 */
static void adafruit_pn532_pin_setup(void) {
}

/**
 * @brief Function to calculate the checksum byte.
 *
 * This function calculates the checksum byte, so that the sum of all verified
 * bytes and the checksum byte is equal to 0.
 *
 * @param  current_sum[in]  Sum of all bytes used to calculate checksum.
 *
 * @retval Value of the checksum byte.
 */
static uint8_t adafruit_pn532_cs_complement_calc(uint8_t current_sum) {
  return ~current_sum + 1;
}

/**
 * @brief Function to check correctness of PN532 Normal information frame
 * header.
 *
 * @param  p_buffer[in]  Pointer to the buffer containing frame header.
 * @param  p_length[out] Pointer to the variable where the data length will be
 * stored.
 *
 * @retval STM_SUCCESS             If the header was correct.
 * @retval STM_ERROR_INVALID_DATA  Otherwise.
 */
static ret_code_t adafruit_pn532_header_check(uint8_t const *p_buffer,
                                              uint8_t *p_length) {
  // Preamble
  if ((p_buffer[PN532_PREAMBLE_OFFSET] != PN532_PREAMBLE) ||
      (p_buffer[PN532_STARTCODE1_OFFSET] != PN532_STARTCODE1) ||
      (p_buffer[PN532_STARTCODE2_OFFSET] != PN532_STARTCODE2)) {
    return STM_ERROR_INVALID_DATA;
  }
  // Data length
  if (p_buffer[PN532_LENGTH_CS_OFFSET] !=
      adafruit_pn532_cs_complement_calc(p_buffer[PN532_LENGTH_OFFSET])) {
    return STM_ERROR_INVALID_DATA;
  }
  // Direction byte
  if ((p_buffer[PN532_TFI_OFFSET] != PN532_PN532TOHOST) &&
      (p_buffer[PN532_TFI_OFFSET] != PN532_HOSTTOPN532)) {
    return STM_ERROR_INVALID_DATA;
  }

  *p_length = p_buffer[PN532_LENGTH_OFFSET];

  return STM_SUCCESS;
}

ret_code_t adafruit_pn532_init(bool force) {
  uint32_t ver_data;    // Variable to store firmware version read from PN532.

  if (m_lib_initialized && !(force)) {
    return STM_SUCCESS;
  }

  if (force) {
  }

  if (m_pn532_object.using_spi) {
    return STM_ERROR_INTERNAL;
  }

  ret_code_t err_code = adafruit_pn532_i2c_create();
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  adafruit_pn532_pin_setup();

  usleep(100 * 1000);

  err_code = adafruit_pn532_firmware_version_get(&ver_data);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  err_code = adafruit_pn532_sam_config(SAMCONFIGURATION_MODE_NORMAL);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  err_code = adafruit_pn532_passive_activation_retries_set(0xFF);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  m_lib_initialized = true;

  return STM_SUCCESS;
}

ret_code_t adafruit_pn532_i2c_create(void) {
  return STM_SUCCESS;
}

// always returns success
ret_code_t adafruit_pn532_firmware_version_get(uint32_t *p_response) {
  m_pn532_packet_buf[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  memcpy(m_pn532_packet_buf + 1,
         m_pn532_rsp_firmware_ver,
         sizeof(m_pn532_rsp_firmware_ver));

  if (memcmp(m_pn532_packet_buf + 1,
             m_pn532_rsp_firmware_ver,
             sizeof(m_pn532_rsp_firmware_ver))) {
    return STM_ERROR_NOT_FOUND;
  }

  // Extract firmware version from the frame.
  *p_response = uint32_big_decode(m_pn532_packet_buf + PN532_DATA_OFFSET + 1);

  return STM_SUCCESS;
}

// always success
ret_code_t adafruit_pn532_cmd_send(uint8_t *p_cmd,
                                   uint8_t cmd_len,
                                   uint16_t timeout) {
  ret_code_t err_code = adafruit_pn532_command_write(p_cmd, cmd_len);

  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  // Wait for ACK
  if (!adafruit_pn532_waitready_ms(timeout)) {
    return STM_ERROR_INTERNAL;
  }

  return adafruit_pn532_ack_read();
}

// always returns success
ret_code_t adafruit_pn532_sam_config(uint8_t mode) {
  ret_code_t err_code;

  if ((mode != SAMCONFIGURATION_MODE_NORMAL) &&
      (mode != SAMCONFIGURATION_MODE_VIRTUAL_CARD) &&
      (mode != SAMCONFIGURATION_MODE_WIRED_CARD) &&
      (mode != SAMCONFIGURATION_MODE_DUAL_CARD)) {
    return STM_ERROR_INVALID_PARAM;
  }

  return STM_SUCCESS;
  // m_pn532_packet_buf[0] = PN532_COMMAND_SAMCONFIGURATION;
  // m_pn532_packet_buf[1] = mode;
  // m_pn532_packet_buf[2] = 0x14; // Time-out value
  // m_pn532_packet_buf[3] = SAMCONFIGURATION_IRQ_ENABLED;

  // err_code = adafruit_pn532_cmd_send(m_pn532_packet_buf,
  // COMMAND_SAMCONFIGURATION_LENGTH, 1000); if (err_code != STM_SUCCESS)
  // {
  //     return err_code;
  // }

  // err_code = adafruit_pn532_data_read(m_pn532_packet_buf,
  // REPLY_SAMCONFIGURATION_LENGTH); if (err_code != STM_SUCCESS)
  // {
  //     return err_code;
  // }

  // if (!(m_pn532_packet_buf[PN532_DATA_OFFSET] ==
  // PN532_COMMAND_SAMCONFIGURATION + 1))
  // {
  //     return STM_ERROR_NOT_FOUND;
  // }
}

ret_code_t adafruit_pn532_passive_activation_retries_set(uint8_t max_retries) {
  ret_code_t err_code;

  m_pn532_packet_buf[0] = PN532_COMMAND_RFCONFIGURATION;
  m_pn532_packet_buf[1] = RFCONFIGURATION_CFGITEM_MAXRETRIES;
  m_pn532_packet_buf[2] = 0xFF;    // MxRtyATR retries (default value)
  m_pn532_packet_buf[3] = 0x01;    // MxRtyPSL retries (default value)
  m_pn532_packet_buf[4] =
      max_retries;    // MxRtyPassiveActivation retries (user value)

  err_code = adafruit_pn532_cmd_send(
      m_pn532_packet_buf, COMMAND_RFCONFIGURATION_MAXRETRIES_LENGTH, 1000);

  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  return STM_SUCCESS;
}

ret_code_t pn532_set_nfca_target_init_command() {
  return STM_SUCCESS;
}

ret_code_t pn532_read_nfca_target_init_resp(nfc_a_tag_info *p_tag_info) {
  return STM_SUCCESS;
}

// simply return success; simulator nfc is always initialised
ret_code_t adafruit_pn532_nfc_a_target_init(nfc_a_tag_info *p_tag_info,
                                            uint16_t timeout) {
  return STM_SUCCESS;
}

// TODO: logically, no change required; yet to debug
ret_code_t adafruit_pn532_in_data_exchange(uint8_t *p_send,
                                           uint8_t send_len,
                                           uint8_t *p_response,
                                           uint8_t *p_response_len) {
  if ((uint16_t)send_len + 2 > PN532_PACKBUFF_SIZE) {
    return STM_ERROR_INTERNAL;
  }

  if ((uint16_t)(*p_response_len) + REPLY_INDATAEXCHANGE_BASE_LENGTH >
      PN532_PACKBUFF_SIZE) {
    return STM_ERROR_INTERNAL;
  }

  // Prepare command.
  m_pn532_packet_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
  m_pn532_packet_buf[1] = m_pn532_object.in_listed_tag;
  memcpy(m_pn532_packet_buf + 2, p_send, send_len);

  ret_code_t err_code =
      adafruit_pn532_cmd_send(m_pn532_packet_buf, send_len + 2, 1000);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  // Note : The wait time was increased from 1 sec to 10 sec as some APDU in
  // card upgrade take longer than 1 sec
  if (!adafruit_pn532_waitready_ms(10000)) {
    return STM_ERROR_INTERNAL;
  }

  err_code = adafruit_pn532_data_read(
      m_pn532_packet_buf, *p_response_len + REPLY_INDATAEXCHANGE_BASE_LENGTH);
  // + 2 for command and status byte
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  uint8_t length = 0;
  err_code = adafruit_pn532_header_check(m_pn532_packet_buf, &length);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  if ((m_pn532_packet_buf[PN532_TFI_OFFSET] != PN532_PN532TOHOST) ||
      (m_pn532_packet_buf[PN532_DATA_OFFSET] !=
       PN532_COMMAND_INDATAEXCHANGE + 1)) {
    return STM_ERROR_INTERNAL;
  }

  // Check InDataExchange Status byte.
  if ((m_pn532_packet_buf[PN532_DATA_OFFSET + 1] & PN532_STATUS_ERROR_MASK) !=
      0x00) {
    return STM_ERROR_INTERNAL;
  }

  length -= 3;    // Calculate the actual data length

  // Silently truncate response to fit into reply desired data size.
  if (length > *p_response_len) {
    length = *p_response_len;
  }

  memcpy(p_response, m_pn532_packet_buf + PN532_DATA_OFFSET + 2, length);
  *p_response_len = length;

  return STM_SUCCESS;
}

// should always returns successful ack
ret_code_t adafruit_pn532_ack_read(void) {
  uint8_t ack_buf[PN532_ACK_PACKET_SIZE] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
  ret_code_t err_code;

  if (!adafruit_pn532_waitready_ms(PN532_DEFAULT_WAIT_FOR_READY_TIMEOUT)) {
    return STM_ERROR_INTERNAL;
  }

  // Wait for irq to be taken off.
  for (uint16_t i = 0; i < 1000; i++) {
    if (!adafruit_pn532_is_ready()) {
      break;
    }
  }

  if (memcmp(ack_buf, m_pn532_ack, PN532_ACK_PACKET_SIZE) != 0) {
    return STM_ERROR_INTERNAL;
  }

  return STM_SUCCESS;
}

bool adafruit_pn532_is_ready(void) {
  return true;
}

bool adafruit_pn532_waitready_ms(uint16_t timeout) {
  uint16_t timer = 0;
  bool result = false;

  result =
      adafruit_pn532_is_ready();    // adafruit is always ready in simulator
  while ((!result) && (timer < timeout)) {
    timer += 1;
    usleep(1000);
    result = adafruit_pn532_is_ready();
  }

  return result;
}

// TODO
ret_code_t adafruit_pn532_data_read(uint8_t *p_buff, uint8_t n) {
  if (!adafruit_pn532_waitready_ms(PN532_DEFAULT_WAIT_FOR_READY_TIMEOUT)) {
    return STM_ERROR_INTERNAL;
  }

  if (m_pn532_object.using_spi) {
    return STM_ERROR_INTERNAL;
  }

  if ((uint16_t)n + 1 > PN532_PACKBUFF_SIZE) {
    return STM_ERROR_INVALID_PARAM;
  }

  if (n == UINT8_MAX) {
    return STM_ERROR_NOT_SUPPORTED;
  }

  ret_code_t err_code;
  // In case of I2C, read the additional status byte.

  err_code = applet_read(m_pn532_rxtx_buffer, n + 1);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }
  memcpy(p_buff, m_pn532_rxtx_buffer + 1, n);

  return STM_SUCCESS;
}

// TODO
ret_code_t adafruit_pn532_command_write(uint8_t *p_cmd, uint8_t cmd_len) {
  ret_code_t err_code;
  uint8_t checksum;

  if (m_pn532_object.using_spi) {
    return STM_ERROR_INTERNAL;
  }

  if ((uint16_t)cmd_len + PN532_FRAME_OVERHEAD > PN532_PACKBUFF_SIZE) {
    return STM_ERROR_INVALID_PARAM;
  }

  // Compose header part of the command frame.
  m_pn532_rxtx_buffer[0] = PN532_PREAMBLE;
  m_pn532_rxtx_buffer[1] = PN532_STARTCODE1;
  m_pn532_rxtx_buffer[2] = PN532_STARTCODE2;
  m_pn532_rxtx_buffer[3] = cmd_len + 1;    // Data length + TFI byte.
  m_pn532_rxtx_buffer[4] = adafruit_pn532_cs_complement_calc(cmd_len + 1);
  m_pn532_rxtx_buffer[5] = PN532_HOSTTOPN532;

  // Copy the payload data.
  memcpy(m_pn532_rxtx_buffer + HEADER_SEQUENCE_LENGTH, p_cmd, cmd_len);

  // Calculate checksum.
  checksum = PN532_HOSTTOPN532;
  for (uint8_t i = 0; i < cmd_len; i++) {
    checksum += p_cmd[i];
  }
  checksum = adafruit_pn532_cs_complement_calc(checksum);

  // Compose checksum part of the command frame.
  m_pn532_rxtx_buffer[HEADER_SEQUENCE_LENGTH + cmd_len] = checksum;
  m_pn532_rxtx_buffer[HEADER_SEQUENCE_LENGTH + cmd_len + 1] = PN532_POSTAMBLE;

  err_code = applet_write(m_pn532_rxtx_buffer, cmd_len + PN532_FRAME_OVERHEAD);
  if (err_code != STM_SUCCESS) {
    return err_code;
  }

  return STM_SUCCESS;
}

ret_code_t adafruit_pn532_field_on(void) {
  return STM_SUCCESS;
}

ret_code_t adafruit_pn532_field_off(void) {
  return STM_SUCCESS;
}

ret_code_t adafruit_pn532_release() {
  return STM_SUCCESS;
}

/**  @brief Function for clearing sensitive data from the local buffer.
 */
void adafruit_pn532_clear_buffers(void) {
}

ret_code_t adafruit_diagnose_comm_line(uint8_t *p_send, uint8_t send_len) {
  return STM_SUCCESS;
}

ret_code_t adafruit_diagnose_card_presence() {
  return STM_SUCCESS;
}

ret_code_t adafruit_diagnose_self_antenna(uint8_t threshold) {
  return STM_SUCCESS;
}

#endif    // ADAFRUIT_PN532_ENABLED
