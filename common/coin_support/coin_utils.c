/**
 * @file    coin_utils.c
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
#include "coin_utils.h"

#include "arbitrum.h"
#include "avalanche.h"
#include "bsc.h"
#include "etc.h"
#include "fantom.h"
#include "harmony.h"
#include "near.h"
#include "optimism.h"
#include "polygon.h"
#include "segwit_addr.h"
#include "solana.h"

void s_memcpy(uint8_t *dst,
              const uint8_t *src,
              uint32_t size,
              uint64_t len,
              int64_t *offset) {
  if (*offset == -1 || (*offset + len > size)) {
    *offset = -1;
    return;
  }
  memcpy(dst, &src[*offset], len);
  *offset += len;
}

bool derive_hdnode_from_path(const uint32_t *path,
                             const size_t path_length,
                             const char *curve,
                             const uint8_t *seed,
                             HDNode *hdnode) {
  hdnode_from_seed(seed, 512 / 8, curve, hdnode);
  for (size_t i = 0; i < path_length; i++) {
    if (0 == hdnode_private_ckd(hdnode, path[i])) {
      // hdnode_private_ckd returns 1 when the derivation succeeds
      return false;
    }
  }
  hdnode_fill_public_key(hdnode);
  return true;
}

ui_display_node *ui_create_display_node(const char *title,
                                        const size_t title_size,
                                        const char *value,
                                        const size_t value_size) {
  ui_display_node *result = cy_malloc(sizeof(ui_display_node));
  memzero(result, sizeof(ui_display_node));

  size_t title_length = strnlen(title, title_size) + 1;
  result->title = cy_malloc(title_length);
  memzero(result->title, title_length);
  strncpy(result->title, title, title_length - 1);

  size_t value_length = strnlen(value, value_size) + 1;
  result->value = cy_malloc(value_length);
  memzero(result->value, value_length);
  strncpy(result->value, value, value_length - 1);

  result->next = NULL;
  return result;
}

void bech32_addr_encode(char *output,
                        char *hrp,
                        uint8_t *address_bytes,
                        uint8_t byte_len) {
  uint8_t data[65] = {0};
  size_t datalen = 0;
  convert_bits(data, &datalen, 5, address_bytes, byte_len, 8, 1);
  bech32_encode(output, hrp, data, datalen);
}

FUNC_RETURN_CODES hd_path_array_to_string(const uint32_t *path,
                                          const size_t path_length,
                                          const bool harden_all,
                                          char *output,
                                          const size_t out_len) {
  if (out_len == 0 || output == NULL || path == NULL)
    return FRC_INVALID_ARGUMENTS;

  int offset = 0;
  offset += snprintf(output + offset, out_len - offset, "m");

  for (int i = 0; i < path_length; i++) {
    const bool hardened = path[i] & 0x80000000;
    const uint32_t value = path[i] & 0x7FFFFFFF;

    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;

    offset += snprintf(output + offset, out_len - offset, "/%ld", value);

    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;

    if (harden_all || hardened)
      offset += snprintf(output + offset, out_len - offset, "'");

    // extra check needed as snprintf returns estimated size rather than actual
    // size written
    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;
  }
  return FRC_SUCCESS;
}
