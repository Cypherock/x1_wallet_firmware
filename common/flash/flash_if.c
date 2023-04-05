/**
 * @file    flash_if.c
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
#include "flash_if.h"

#include <string.h>

#include "board.h"
#include "logger.h"

void read_cmd(const uint32_t addr, uint32_t *source_addr, const uint32_t len) {
  ASSERT(addr != 0);
  ASSERT(
      (addr >= FLASH_DATA_ADDRESS && addr <= FLASH_DATA_END_ADDRESS) ||
      (addr >= FLASH_DATA_LOGGER_ADDRESS &&
       addr <= (FLASH_DATA_LOGGER_ADDRESS + (LOG_MAX_PAGES * LOG_PAGE_SIZE))) ||
      ((FLASH_END - (8 * FLASH_PAGE_SIZE) < addr) && (addr <= FLASH_END)));
  ASSERT(len != 0);
  ASSERT(source_addr != NULL);

  BSP_NonVolatileRead(addr, source_addr, len);
}

void write_cmd(const uint32_t addr, const uint32_t *data, const uint32_t len) {
  ASSERT(addr != 0);
  ASSERT(
      (addr >= FLASH_DATA_ADDRESS && addr <= FLASH_DATA_END_ADDRESS) ||
      (addr >= FLASH_DATA_LOGGER_ADDRESS &&
       addr <= (FLASH_DATA_LOGGER_ADDRESS + (LOG_MAX_PAGES * LOG_PAGE_SIZE))) ||
      ((FLASH_END - (8 * FLASH_PAGE_SIZE) < addr) && (addr <= FLASH_END)));
  ASSERT(len != 0);
  ASSERT(data != NULL);

  if (BSP_FlashSectorWrite((uint32_t *)addr, data, len) != BSP_OK) {
    BSP_DelayMs(10);
    BSP_FlashSectorWrite((uint32_t *)addr, data, len);
  }
}

void erase_cmd(const uint32_t addr, const uint32_t erase_size) {
  uint16_t pages_cnt = erase_size / FLASH_PAGE_SIZE +
                       ((erase_size % FLASH_PAGE_SIZE == 0) ? 0 : 1);

  ASSERT(addr != 0);
  ASSERT(
      (addr >= FLASH_DATA_ADDRESS && addr <= FLASH_DATA_END_ADDRESS) ||
      (addr >= FLASH_DATA_LOGGER_ADDRESS &&
       addr <= (FLASH_DATA_LOGGER_ADDRESS + (LOG_MAX_PAGES * LOG_PAGE_SIZE))) ||
      ((FLASH_END - (8 * FLASH_PAGE_SIZE) < addr) && (addr <= FLASH_END)));
  ASSERT(pages_cnt != 0);

  if (BSP_FlashSectorErase(addr, pages_cnt) != BSP_OK) {
    BSP_DelayMs(10);
    BSP_FlashSectorErase(addr, pages_cnt);
  }
}
