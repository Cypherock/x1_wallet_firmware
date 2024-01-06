/**
 * @file    logger.c
 * @author  Cypherock X1 Team
 * @brief   Logger class for device.
 *          Class for logging and fetching messages to and from the device
 *respectively.
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

#include "logger.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app_error.h"
#include "board.h"
#include "communication.h"
#include "flash_api.h"
#include "flash_struct.h"

extern const char *GIT_REV;
extern const char *GIT_TAG;
extern const char *GIT_BRANCH;

/// Stores log details
static logger_data_s_t sg_log_data;

/**
 * @brief Move to the next page and adds spaces to fill gap
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void logger_switch_page(void);

void logger(char *fmt, ...) {
  ASSERT(fmt != NULL);

  char temp_str1[LOG_MAX_SIZE];
  char temp_str2[LOG_MAX_SIZE];
  int16_t n;
  uint8_t cnt2;
  va_list args;

  if (sg_log_data.initialized == true) {
    memset(temp_str1, 0, LOG_MAX_SIZE);
    memset(temp_str2, 0, LOG_MAX_SIZE);
    va_start(args, fmt);
    n = vsnprintf(temp_str1, LOG_MAX_SIZE, fmt, args);
    va_end(args);

    n = snprintf(
        temp_str2, LOG_MAX_SIZE, "%d. %s\n", sg_log_data.log_count, temp_str1);
    cnt2 = n % sizeof(uint64_t);
    cnt2 = cnt2 == 0
               ? 0
               : (sizeof(uint64_t) -
                  cnt2); /*Here finding how many bytes needed to U64 alignment*/
    for (uint8_t cnt1 = 0; cnt1 < cnt2; cnt1++) {
      temp_str2[n + cnt1] = ' ';
    }

    /*Here check if enough space is available in current page*/
    if ((sg_log_data.next_write_loc + n + cnt2) >=
        (((sg_log_data.page_index + 1) * LOG_PAGE_SIZE) + LOG_SECTION_START)) {
      logger_switch_page();
    }

    write_cmd(sg_log_data.next_write_loc, (uint32_t *)temp_str2, n + cnt2);
    sg_log_data.next_write_loc += (n + cnt2);
    sg_log_data.log_count++;

    /*Here is the page is completely filled.
    Putting this check so if at anypoint device reset when current page was
    full, we should be able to fine atleast 1 next page with space to write*/
    if ((sg_log_data.next_write_loc) >=
        (((sg_log_data.page_index + 1) * LOG_PAGE_SIZE) + LOG_SECTION_START)) {
      logger_switch_page();
    }
  }
}

void logger_reset_flash(void) {
  erase_cmd(LOG_SECTION_START, LOG_MAX_PAGES * FLASH_PAGE_SIZE);
  sg_log_data.page_index = 0;
  sg_log_data.next_write_loc = LOG_SECTION_START;
  sg_log_data.log_count = 1;
}

// void logger_init(uint32_t head, uint32_t tail) {
void logger_init(void) {
#if USE_SIMULATOR == 0
  void *addr_loc = NULL;
  uint8_t next_loc_found = false;

  sg_log_data.page_index = -1;
  sg_log_data.log_count = 1;

  for (uint16_t cnt = 0; (cnt < LOG_MAX_PAGES); cnt++) {
    addr_loc = (void *)(LOG_SECTION_START + (cnt * LOG_PAGE_SIZE));
    if (*(uint64_t *)addr_loc == -1) {
      sg_log_data.page_index = cnt;
      break;
    }
  }

  if (sg_log_data.page_index < LOG_MAX_PAGES) {
    addr_loc = (void *)(LOG_SECTION_START +
                        ((sg_log_data.page_index + 1) * LOG_PAGE_SIZE));
    for (uint16_t cnt2 = 0; cnt2 < (LOG_PAGE_SIZE / sizeof(uint64_t)) - 1;
         cnt2++) {
      addr_loc -= sizeof(uint64_t);
      if (*(uint64_t *)addr_loc == -1) {
        sg_log_data.next_write_loc = (uint32_t)addr_loc;
        next_loc_found = true;
      } else {
        break;
      }
    }
  }

  if (next_loc_found == false) {
    sg_log_data.page_index = 0;
    addr_loc =
        (void *)(LOG_SECTION_START + sg_log_data.page_index * LOG_PAGE_SIZE);
    erase_cmd((uint32_t)addr_loc, FLASH_PAGE_SIZE);
    sg_log_data.next_write_loc = (uint32_t)(addr_loc + sizeof(uint64_t));
  }

  sg_log_data.initialized = true;
  sg_log_data.read_sm_e = LOG_READ_FINISH;
#endif
}

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void logger_switch_page(void) {
  uint32_t page_addr = 0;
  uint16_t cnt2;
  /*Fill Remaining Space in page with ' '*/
  cnt2 = sg_log_data.next_write_loc % LOG_PAGE_SIZE;
  cnt2 = LOG_PAGE_SIZE - cnt2;
  for (uint16_t cnt1 = 0; cnt1 < cnt2; cnt1 += sizeof(uint64_t)) {
    page_addr = sg_log_data.next_write_loc + cnt1;
    write_cmd(page_addr, (uint32_t *)"        ", sizeof(uint64_t));
  }

  page_addr = LOG_SECTION_START + sg_log_data.page_index * LOG_PAGE_SIZE;
  write_cmd(page_addr, (uint32_t *)"        ", sizeof(uint64_t));

  sg_log_data.page_index =
      CYCLIC_INCREMENT(sg_log_data.page_index, LOG_MAX_PAGES);
  page_addr = LOG_SECTION_START + sg_log_data.page_index * LOG_PAGE_SIZE;
  erase_cmd(page_addr, FLASH_PAGE_SIZE);
  sg_log_data.next_write_loc = page_addr + sizeof(uint64_t);
}

void logger_task(uint8_t *data, size_t *size) {
  ASSERT((NULL != data) && (NULL != size));
#if USE_SIMULATOR == 0
  printf("#GRN#%s: lc=%d, wl=%ld, pi=%d, i=%d, rpi=%d, sm=%d, rc=%d\n",
         __func__,
         sg_log_data.log_count,
         sg_log_data.next_write_loc,
         sg_log_data.page_index,
         sg_log_data.initialized,
         sg_log_data.read_page_index,
         sg_log_data.read_sm_e,
         sg_log_data.total_page_read);
  uint16_t cnt = 0;
  uint16_t packet_len = 0;
  void *addr_loc = NULL;
  uint8_t next_loc_found = false;
  const char *start_of_log = "startofpacket\r\n";
  const char *end_of_log = "endofpacket\r\n";

  switch (sg_log_data.read_sm_e) {
    case LOG_READ_INIT:
      sg_log_data.read_sm_e = LOG_READ_ONGOING;

      for (cnt = sg_log_data.page_index + 1; cnt != sg_log_data.page_index;
           cnt = CYCLIC_INCREMENT(cnt, LOG_MAX_PAGES)) {
        addr_loc = (void *)(LOG_SECTION_START + (cnt * LOG_PAGE_SIZE));
        if (*(uint64_t *)addr_loc == 0x2020202020202020) {
          sg_log_data.read_page_index = cnt;
          next_loc_found = true;
          break;
        }
      }

      if (next_loc_found == false)
        sg_log_data.read_page_index = sg_log_data.page_index;

      sg_log_data.total_page_read = 0;
      sg_log_data.initialized = false;
      char extended_start_of_log[MAXIMUM_DATA_SIZE] = {'\0'};
      snprintf(extended_start_of_log,
               MAXIMUM_DATA_SIZE - 1,
               "\r\n%s%s Bl:%lX\r\n",
               start_of_log,
               GIT_REV,
               FW_get_bootloader_version());
      memcpy(data, extended_start_of_log, MAXIMUM_DATA_SIZE);
      *size = MAXIMUM_DATA_SIZE;
      break;

    case LOG_READ_ONGOING:
      packet_len = LOG_PAGE_SIZE;
      if (sg_log_data.page_index == sg_log_data.read_page_index) {
        packet_len =
            sg_log_data.next_write_loc -
            (LOG_SECTION_START + sg_log_data.read_page_index * LOG_PAGE_SIZE);
        sg_log_data.read_sm_e = LOG_READ_END;
      }
      if (packet_len == 0 || sg_log_data.total_page_read == LOG_MAX_PAGES) {
        sg_log_data.read_sm_e = LOG_READ_END;
        break;
      }
      if (packet_len > 0) {
        uint8_t *page_read_location =
            (uint8_t *)(LOG_SECTION_START +
                        sg_log_data.read_page_index * LOG_PAGE_SIZE);
        memcpy(data, page_read_location, packet_len);
        *size = packet_len;
      }
      sg_log_data.read_page_index =
          CYCLIC_INCREMENT(sg_log_data.read_page_index, LOG_MAX_PAGES);
      sg_log_data.total_page_read += 1;
      break;

    case LOG_READ_END:
      sg_log_data.read_sm_e = LOG_READ_FINISH;
      memcpy(data, end_of_log, strlen(end_of_log));
      *size = strlen(end_of_log);
      sg_log_data.initialized = true;
      break;

    default:
      sg_log_data.read_sm_e = LOG_READ_FINISH;
      sg_log_data.initialized = true;
      break;
  }
#endif
}

void log_hex_array(const char text[],
                   const uint8_t *arr,
                   const uint8_t length) {
  if (!is_logging_enabled())
    return;
  if (text == NULL || arr == NULL || length == 0)
    return;

  char str[131];
  uint8_t j = 0;
  LOG_ERROR("%s %d\n", text, length);

  str[130] = '\0';
  for (uint8_t i = 0; i < length; i++) {
    snprintf(str + j, sizeof(str) - j, "%02X", arr[i]);
    j += 2;
    if (j > 121) {
      LOG_ERROR("%s", str, 122);
      j = 0;
    }
  }
  str[j] = '\0';
  LOG_ERROR("%s", str, length * 2);
}

void set_start_log_read() {
  sg_log_data.read_sm_e = LOG_READ_INIT;
}

log_read_e_t get_log_read_status() {
  return sg_log_data.read_sm_e;
}
