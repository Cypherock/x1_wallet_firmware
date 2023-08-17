/**
 * @file    near.c
 * @author  Cypherock X1 Team
 * @brief   Near coin support.
 *          Stores near coin related class.
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
#include "near.h"

#include "near_context.h"

void near_serialize_account_ids(const char **account_ids,
                                const size_t count,
                                uint8_t *data,
                                uint16_t *data_len) {
  *data_len = 0;
  data[(*data_len)++] = TAG_NEAR_DEFAULT_NETWORK;
  *data_len += 2;    // Leave 2 bytes for storing data length
  for (size_t i = 0; i < count; i++) {
    fill_flash_tlv(data,
                   data_len,
                   TAG_NEAR_REGISTERED_ACC,
                   strnlen(account_ids[i], NEAR_ACC_ID_MAX_LEN) + 1,
                   (const uint8_t *)account_ids[i]);
  }
  data[1] = (*data_len - 3);
  data[2] = (*data_len - 3) >> 8;
}

void near_deserialize_account_ids(const uint8_t *data,
                                  const uint16_t data_len,
                                  char **account_ids,
                                  size_t count) {
  uint16_t offset = 3;

  for (size_t i = 0; i < count; i++) {
    if (data[offset++] != TAG_NEAR_REGISTERED_ACC)
      return;

    uint16_t acc_id_len = U16_READ_LE_ARRAY(data + offset);
    offset += 2;

    account_ids[i] = (char *)(data + offset);
    offset += acc_id_len;

    if (offset > data_len)
      return;
  }
}

size_t near_get_account_ids_count(const uint8_t *data,
                                  const uint16_t data_len) {
  uint16_t offset = 3;
  size_t count = 0;
  while (offset < data_len) {
    if (data[offset++] != TAG_NEAR_REGISTERED_ACC)
      return count;

    uint16_t acc_id_len = 0;
    memcpy(&acc_id_len, data + offset, sizeof(acc_id_len));
    offset += 2;
    offset += acc_id_len;
    count++;
  }
  return count;
}
