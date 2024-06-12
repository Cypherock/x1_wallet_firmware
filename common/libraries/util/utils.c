/**
 * @file    utils.c
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
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "assert_conf.h"
#include "bip32.h"
#include "bip39.h"
#include "crypto_random.h"
#include "cryptoauthlib.h"
#include "curves.h"
#include "device_authentication_api.h"
#include "logger.h"
#include "lv_font.h"
#include "lv_txt.h"
#include "sha2.h"
#include "wallet.h"

/**
 * @brief struct for
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
typedef struct {
  void *mem;
  size_t mem_size;
} memory_list_t;

/**
 * @brief struct for
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
typedef struct cy_linked_list {
  struct cy_linked_list *next;
  memory_list_t mem_entry;
} cy_linked_list_t;

static cy_linked_list_t *memory_list = NULL;

void *cy_malloc(size_t mem_size) {
  cy_linked_list_t *new_entry =
      (cy_linked_list_t *)malloc(sizeof(cy_linked_list_t));
  new_entry->mem_entry.mem = malloc(mem_size);
  ASSERT(new_entry != NULL && new_entry->mem_entry.mem != NULL);

  new_entry->next = memory_list;
  new_entry->mem_entry.mem_size = mem_size;
  memory_list = new_entry;
  memzero(new_entry->mem_entry.mem, mem_size);
  return new_entry->mem_entry.mem;
}

void cy_free() {
  cy_linked_list_t *temp = memory_list;
  while ((temp = memory_list) != NULL) {
    memzero(temp->mem_entry.mem, temp->mem_entry.mem_size);
    free(temp->mem_entry.mem);
    memory_list = memory_list->next;
    memzero(temp, sizeof(cy_linked_list_t));
    free(temp);
  }
}

int is_zero(const uint8_t *bytes, const uint8_t len) {
  if (len == 0)
    return 1;

  for (int i = 0; i < len; i++) {
    if (bytes[i])
      return 0;
  }

  return 1;
}

/// Array of hex characters
uint8_t map[] = {'0',
                 '1',
                 '2',
                 '3',
                 '4',
                 '5',
                 '6',
                 '7',
                 '8',
                 '9',
                 'a',
                 'b',
                 'c',
                 'd',
                 'e',
                 'f'};

uint32_t byte_array_to_hex_string(const uint8_t *bytes,
                                  const uint32_t byte_len,
                                  char *hex_str,
                                  size_t str_len) {
  if (bytes == NULL || hex_str == NULL || byte_len < 1 || str_len < 2)
    return 0;
  if ((2 * byte_len + 1) > str_len)
    return 0;
  uint32_t i = 0;
  for (i = 0; i < byte_len; i++) {
    hex_str[i * 2] = map[(bytes[i] & 0xf0) >> 4];
    hex_str[i * 2 + 1] = map[bytes[i] & 0x0f];
  }
  hex_str[i * 2] = '\0';
  return (i * 2 + 1);
}

void __single_to_multi_line(
    const char *input,
    const uint16_t input_len,
    char output[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH]) {
  if (input == NULL || output == NULL)
    return;
  uint16_t i = 0U;
  uint16_t j;
  uint16_t l = 0U;
  while (1) {
    j = 0U;
    while (input[l] != ' ' && l < input_len) {
      output[i][j] = input[l];
      l++;
      j++;
    }
    output[i][j] = 0U;
    i++;
    if (l >= input_len) {
      return;
    }
    l++;
  }
}

void __multi_to_single_line(
    const char input[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH],
    const uint8_t number_of_mnemonics,
    char *output) {
  if (input == NULL || output == NULL)
    return;
  uint8_t word_len;
  uint16_t offset = 0U;
  uint16_t i = 0U;
  for (; i < number_of_mnemonics; i++) {
    word_len =
        strnlen(input[i], 15);    // TODO: Define handling of overflow cases
    memcpy(output + offset, input[i], word_len);
    offset += word_len;
    memcpy(output + offset, " ", 1);
    offset++;
  }
  offset--;
  uint8_t zero = 0;
  memcpy(output + offset, &zero, 1);
}

void hex_string_to_byte_array(const char *hex_string,
                              const uint32_t string_length,
                              uint8_t *byte_array) {
  char hex[3] = {'\0'};

  for (int i = 0; i < string_length; i += 2) {
    hex[0] = hex_string[i];
    hex[1] = hex_string[i + 1];
    byte_array[i / 2] = (uint8_t)strtol(hex, NULL, 16);
  }
}

void print_hex_array(const char text[],
                     const uint8_t *arr,
                     const uint8_t length) {
  printf("%s %d\n", text, length);

  for (uint8_t i = 0U; i < length; i++) {
    printf("%02X ", arr[i]);
  }
  printf("\n");
}

void byte_array_re(uint8_t *input_output, uint16_t len) {
  uint8_t *temp_output = (uint8_t *)malloc(len);
  ASSERT(temp_output != NULL);
  uint16_t output_array_index = 0U;
  while (output_array_index < len) {
    temp_output[output_array_index] =
        input_output[len - 1 - output_array_index];
    output_array_index++;
  }
  memcpy(input_output, temp_output, len);
  free(temp_output);
}

uint8_t decode_card_number(const uint8_t encoded_card_number) {
  switch (encoded_card_number) {
    case 1U:
      return 1;
    case 2U:
      return 2;
    case 4U:
      return 3;
    case 8U:
      return 4;
    default:
      return 1;
  }
}

uint8_t encode_card_number(const uint8_t decoded_card_number) {
  uint8_t output = 1U;

  for (uint8_t i = 0U; i < (decoded_card_number - 1U); i++) {
    output = output << 1U;
  }

  return output;
}
void get_firmaware_version(uint16_t pid,
                           const char *product_hash,
                           char message[20]) {
  uint16_t len = 0;
  char pid_array[4] = {'0', '0', '0', '0'};
  while (pid > 0 && len < sizeof(pid_array)) {
    pid_array[3 - len] = (char)('0' + pid % 10);
    pid = pid / 10;
    len = len + 1;
  }
  message[0] = pid_array[0], message[1] = '.', message[2] = pid_array[1],
  message[3] = '.';
  if (pid_array[2] != '0') {
    message[4] = pid_array[2], message[5] = pid_array[3];
    strcpy(&message[6], product_hash);
    message[16] = 0;
  } else {
    message[4] = pid_array[3];
    strcpy(&message[5], product_hash);
    message[15] = 0;
  }
}

// TODO: Update len return size to 16 bit
void random_generate(uint8_t *arr, int len) {
  ASSERT(len <= 32);

  ASSERT(crypto_random_generate(arr, len) == true);

  // using atecc
  uint8_t temp[32] = {0};
  atecc_data.retries = DEFAULT_ATECC_RETRIES;

  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
    atecc_data.status = atcab_random(temp);
  } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  ASSERT((atecc_data.status == ATCA_SUCCESS) && (!is_zero(temp, sizeof(temp))));

  for (int i = 0; i < len; ++i) {
    arr[i] ^= temp[i];
  }
}

uint8_t get_floating_precision(uint64_t num, uint64_t den) {
  uint8_t precision = 0;
  while (den > 1) {
    if (num % den == 0)
      break;
    den /= 10;
    precision++;
  }
  return precision;
}

void der_to_sig(const uint8_t *der, uint8_t *sig) {
  if (!der || !sig)
    return;
  memzero(sig, 64);
  uint8_t len, offset = 0;
  if (der[offset++] != 0x30)
    return;    // Verify the starting byte is 0x30
  offset++;    // Skip the length byte
  if (der[offset++] != 0x02)
    return;               // Verify the marker byte is 0x02
  len = der[offset++];    // Get the length of the r value
  if (len == 0x21) {
    offset++;
    len--;
  }
  // Copy the r component of signature prepending 0's
  memcpy(sig + (32 - len), der + offset, len);
  offset += len;
  if (der[offset++] != 0x02)
    return;               // Verify the marker byte is 0x02
  len = der[offset++];    // Get the length of the s value
  if (len == 0x21) {
    offset++;
    len--;
  }
  // Copy the s component of signature prepending 0's
  memcpy(sig + 32 + (32 - len), der + offset, len);
}

/**
 * @brief The function writes the specified number of bytes in escaped string
 * format.
 * @details The escape format returned by the function of the form '\xhh'. This
 * is done for each byte that is specified in the input buffer. Each byte is
 * encoded as set of 4-character strings meaning the output buffer is assumed to
 * be of length `4*count + 1`. The function always ensures that the result
 * buffer is NULL terminated.
 *
 * @param bytes       Pointer to the uint8_t buffer containing bytes to be
 * escaped
 * @param count       Number of bytes to be escaped from the specifed input
 * buffer
 * @param escaped_str Output buffer for storing the escaped string result
 */
static void escape_bytes(const uint8_t *bytes,
                         uint8_t count,
                         char *escaped_str) {
  uint8_t str_off = 0;
  uint8_t byte_idx = 0;

  while (byte_idx < count) {
    uint8_t byte = bytes[byte_idx++];

    escaped_str[str_off++] = '\\';
    escaped_str[str_off++] = 'x';
    escaped_str[str_off++] = map[(byte & 0xf0) >> 4];
    escaped_str[str_off++] = map[byte & 0x0f];
  }
  escaped_str[str_off] = '\0';
}

/**
 * @brief hex char to integer.
 *
 * @param ch char
 * @return int output.
 */
static int hexchartoint(const uint8_t ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  else if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  return '\0';
}

/**
 * @brief intermediate step to add the last hex
 * in the partial input.
 * @details
 *
 * @param Out output reference
 * @param NewHex attached hex.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void updateDec(uint8_t *Out, const uint8_t NewHex, const uint8_t size) {
  if (NULL == Out) {
    return;
  }
  int power = NewHex;
  for (int i = size - 1; i >= 0; i--) {
    int temp = Out[i] * 16 + power;
    Out[i] = temp % 10;
    power = temp / 10;
  }
}

void convertbase16tobase10(const uint8_t size_inp,
                           const char *u_Inp,
                           uint8_t *Out,
                           const uint8_t size_out) {
  if (NULL == u_Inp || NULL == Out) {
    return;
  }
  memzero(Out, size_out);
  for (int i = 0; i < size_inp; i++) {
    updateDec(Out, hexchartoint(u_Inp[i]), size_out);
  }
}

uint8_t dec_to_hex(const uint64_t dec, uint8_t *bytes, uint8_t len) {
  uint8_t hex[8];
  uint8_t size = 0;

  hex[0] = (dec & 0xff00000000000000) >> 56;
  size = size == 0 && hex[0] > 0 ? 8 : size;
  hex[1] = (dec & 0x00ff000000000000) >> 48;
  size = size == 0 && hex[1] > 0 ? 7 : size;
  hex[2] = (dec & 0x0000ff0000000000) >> 40;
  size = size == 0 && hex[2] > 0 ? 6 : size;
  hex[3] = (dec & 0x000000ff00000000) >> 32;
  size = size == 0 && hex[3] > 0 ? 5 : size;
  hex[4] = (dec & 0x00000000ff000000) >> 24;
  size = size == 0 && hex[4] > 0 ? 4 : size;
  hex[5] = (dec & 0x0000000000ff0000) >> 16;
  size = size == 0 && hex[5] > 0 ? 3 : size;
  hex[6] = (dec & 0x000000000000ff00) >> 8;
  size = size == 0 && hex[6] > 0 ? 2 : size;
  hex[7] = dec & 0x00000000000000ff;
  size = size == 0 && hex[7] > 0 ? 1 : size;

  if (bytes)
    memcpy(bytes, hex + 8 - len, len);

  return size;
}

uint8_t cy_reverse_byte_array(uint8_t *byte_data, uint16_t len) {
  uint8_t i = 0;
  uint8_t j = len - 1;
  uint8_t temp = 0;
  while (i < j) {
    temp = byte_data[i];
    byte_data[i] = byte_data[j];
    byte_data[j] = temp;
    i++;
    j--;
  }
  return 0;
}

uint64_t cy_read_be(const uint8_t *bytes, uint8_t size) {
  if (bytes == NULL || size == 0)
    return 0;

  uint64_t value = 0;
  uint8_t offset = 0;
  while (offset < size) {
    value = (bytes[offset++] | (value << 8));
  }
  return value;
}

bool convert_byte_array_to_decimal_string(
    const uint8_t len,
    const uint8_t decimal,
    char *amount_string,
    char *amount_decimal_string,
    const size_t amount_decimal_string_size) {
  uint8_t decimal_val_s[32 * 3] = {0};
  if (sizeof(decimal_val_s) / sizeof(decimal_val_s[0]) > UINT8_MAX) {
    LOG_ERROR("0xxx#");
    return false;
  }
  const uint8_t dec_val_len =
      sizeof(decimal_val_s) /
      sizeof(decimal_val_s[0]);    // logbase10(2pow256) roughly equals 78
  convertbase16tobase10(len, amount_string, decimal_val_s, dec_val_len);

  int i = 0, j = dec_val_len - 1;
  bool pre_dec_digit = false, post_dec_digit = false;
  uint8_t point_index = dec_val_len - decimal;

  uint8_t offset = 0;
  while (i <= j) {
    if (i == point_index && post_dec_digit) {
      if (!pre_dec_digit) {
        offset += snprintf(amount_decimal_string + offset,
                           (amount_decimal_string_size)-offset,
                           "0");
      }
      offset += snprintf(amount_decimal_string + offset,
                         (amount_decimal_string_size)-offset,
                         ".");
    }
    if (j >= point_index) {
      if (!decimal_val_s[j] && !post_dec_digit) {
        j--;
      } else if (decimal_val_s[j]) {
        post_dec_digit = true;
      }
    }
    if (decimal_val_s[i] || i == point_index) {
      pre_dec_digit = true;
    }
    if (pre_dec_digit || decimal_val_s[i]) {
      // attach non zero leading value detected or decimal digits till j(should
      // be the last non zero decimal digit index).
      offset += snprintf(amount_decimal_string + offset,
                         (amount_decimal_string_size)-offset,
                         "%d",
                         decimal_val_s[i]);
    }
    i++;
  }
  if (!post_dec_digit && !pre_dec_digit) {
    snprintf(amount_decimal_string, (amount_decimal_string_size)-1, "0");
  }

  LOG_INFO("amt %s %d:%d", amount_string, decimal, i);
  return true;
}

uint8_t UTIL_CheckBound(const uint8_t *pBaseAddr,
                        const uint32_t totalSizeOfChunk,
                        const uint8_t *pCurrentSrcAddr,
                        const uint32_t readSize) {
  uint8_t returnCode = UTIL_OUT_OF_BOUNDS;

  if ((NULL == pBaseAddr) || (NULL == pCurrentSrcAddr) ||
      ((uint32_t)pCurrentSrcAddr < (uint32_t)pBaseAddr)) {
    returnCode = UTIL_INVALID_ARGUMENTS;
  } else {
    if (((uint32_t)(pCurrentSrcAddr + readSize) - (uint32_t)pBaseAddr) <=
        (totalSizeOfChunk)) {
      returnCode = UTIL_IN_BOUNDS;
    }
  }

  return returnCode;
}

uint8_t string_to_escaped_string(const char *input,
                                 char *escaped_string,
                                 size_t out_len) {
  uint32_t in_idx = 0;
  uint32_t letter = 0;
  uint32_t out_idx = 0;
  lv_font_glyph_dsc_t g;
  bool non_printable = false;
  bool invalid_encoding = false;
  const lv_font_t *font_p = LV_FONT_DEFAULT;
  uint8_t zero_bytes[7] = {0, 0, 0, 0, 0, 0, 0};

  if (NULL == escaped_string || NULL == input || 0 == out_len ||
      input == escaped_string) {
    return 1;
  }

  // ensure result is always NULL terminated
  memset(escaped_string, 0, out_len);

  while (input[in_idx] != '\0') {
    size_t old_idx = in_idx;

    // read the unicode character; expects 0 if invalid encoding
    letter = lv_txt_encoded_next(input, &in_idx);
    // try to get the glyph descriptor for this unicode character
    bool g_ret = lv_font_get_glyph_dsc(font_p, &g, letter, '\0');
    uint8_t bytes_read = in_idx - old_idx;

    if (0 != letter && ('\n' == letter || '\r' == letter || true == g_ret)) {
      // the glyph exists; keep the encoded character as it is.
      // Additionally, CR/LF characters are handled as special case by LVGL
      // hence glyphs for CR/LF are meaningless so we should keep it as it is
      // refer common/lvgl/src/lv_misc/lv_txt.c#174
      if ((out_idx + bytes_read + 1) >= out_len)
        return 5;
      memcpy(&escaped_string[out_idx], &input[old_idx], bytes_read);
      out_idx += bytes_read;
    } else if (0 != letter && false == g_ret) {
      // the glyph does not exist; escape the utf-8 bytes
      if ((out_idx + 4 * bytes_read + 1) >= out_len)
        return 5;
      escape_bytes(
          (uint8_t *)&input[old_idx], bytes_read, &escaped_string[out_idx]);
      out_idx += (4 * bytes_read);
      non_printable = true;
    } else {
      // invalid utf-8 encoding detected; escape each byte with "\x00" string
      if ((out_idx + 4 * bytes_read + 1) >= out_len)
        return 5;
      escape_bytes(zero_bytes, bytes_read, &escaped_string[out_idx]);
      out_idx += (4 * bytes_read);
      invalid_encoding = true;
    }
  }

  escaped_string[out_idx] = '\0';
  if (non_printable && invalid_encoding) {
    return 4;
  } else if (invalid_encoding) {
    return 3;
  } else if (non_printable) {
    return 2;
  } else {
    return 0;
  }
}
