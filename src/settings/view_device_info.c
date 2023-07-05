/**
 * @file    view_device_info.c
 * @author  Cypherock X1 Team
 * @brief   Source file with helper functions to display information for the
 *          X1 vault firmware, bootloader and X1 cards
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "constant_texts.h"
#include "flash_api.h"
#include "settings_api.h"
#include "ui_screens.h"
#include "ui_state_machine.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
extern const char *GIT_REV;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void view_firmware_version(void) {
  uint32_t blVersion = FW_get_bootloader_version(), fwVersion = get_fwVer();
  uint16_t fwMajor = (fwVersion >> 24) & 0xFF,
           fwMinor = (fwVersion >> 16) & 0xFF, fwPatch = fwVersion & 0xFFFF;
  uint16_t blMajor = (blVersion >> 24) & 0xFF,
           blMinor = (blVersion >> 16) & 0xFF, blPatch = blVersion & 0xFFFF;
  char fw_msg[60] = {0}, bl_msg[60] = {0};
  const char *msg[2] = {fw_msg, bl_msg};

  snprintf(fw_msg,
           sizeof(fw_msg),
           UI_TEXT_FIRMWARE_VERSION,
           fwMajor,
           fwMinor,
           fwPatch,
           GIT_REV);
  snprintf(bl_msg,
           sizeof(bl_msg),
           UI_TEXT_BOOTLOADER_VERSION,
           blMajor,
           blMinor,
           blPatch);

  multi_instruction_init(msg, 2, DELAY_TIME, true);

  // Do not care about the return value from confirmation screen
  (void)get_state_on_confirm_scr(0, 0, 0);
  return;
}

void view_device_regulatory_information(void) {
  instruction_content_t content[7] = {0};
  LV_IMG_DECLARE(fcc_logo_black_2020);
  LV_IMG_DECLARE(ce_mark);
  content[0].img = &fcc_logo_black_2020;
  content[0].img_x_offset =
      (LV_HOR_RES_MAX - fcc_logo_black_2020.header.w) >> 1;
  content[0].img_y_offset =
      (LV_VER_RES_MAX - fcc_logo_black_2020.header.h) >> 1;
  content[0].text_align = LV_ALIGN_OUT_BOTTOM_MID;
  content[6].img = &ce_mark;
  content[6].img_x_offset = (LV_HOR_RES_MAX - ce_mark.header.w) >> 1;
  content[6].img_y_offset = (LV_VER_RES_MAX - ce_mark.header.h) >> 1;

  for (uint8_t slide = 0; slide < NUMBER_OF_SLIDES_REGULATORY_INFO; slide++) {
    snprintf(content[slide + 1].text,
             sizeof(content[slide + 1].text),
             ui_text_regulatory_info[slide]);
  }

  multi_instruction_with_image_init(content, 7, DELAY_TIME, true);

  // Do not care about the return value from confirmation screen
  (void)get_state_on_confirm_scr(0, 0, 0);
  return;
}