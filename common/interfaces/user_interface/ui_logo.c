/**
 * @file    ui_logo.c
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
#include "ui_logo.h"

#include "events.h"
#include "ui_multi_instruction.h"

#if VENDOR_ID == 0  // cypherock
LV_IMG_DECLARE(cypherock_logo);
#define VENDOR_LOGO_LIST  cypherock_logo
#define VENDOR_LOGO_COUNT 1

#elif VENDOR_ID == 1  // odix
LV_IMG_DECLARE(odix_logo);
LV_IMG_DECLARE(cypherock_vendor_logo);
#define VENDOR_LOGO_LIST  odix_logo, cypherock_vendor_logo
#define VENDOR_LOGO_COUNT 2

#endif


void logo_scr_init(const uint16_t delay_in_ms) {
  const lv_img_dsc_t logos[] = { VENDOR_LOGO_LIST };

  for (int i = 0; i < VENDOR_LOGO_COUNT; i++) {
    instruction_content_t logo_content = {
        .img = &logos[i],
        .img_x_offset = (128 - (logos[i].header.w)) >> 1,
        .img_y_offset = (64 - (logos[i].header.h)) >> 1,
        .text = "",
        .text_align = LV_ALIGN_CENTER};
  
    multi_instruction_with_image_init(&logo_content, 1, 0, false);
    lv_task_handler();
    BSP_DelayMs(delay_in_ms);
  }
}
