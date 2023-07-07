/**
 * @file    ui_instruction.c
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
#include "ui_instruction.h"

/*
 * Note : This screen displays some text for a given amount of time. Only text
 * is shown in this screen.
 *
 * Instruction Screen vs Delay Screen
 * Instruction screen has the ability to change the text shown on screen
 * without clearing and redoing the whole operation. For changing text
 * instruction screen is much more efficient.
 *
 * You'll have to call lv_task_handler() manually and delete screen
 * when not in use unlike delay screen.
 */

static lv_obj_t *instruction = NULL;
static lv_obj_t *heading = NULL;

void instruction_scr_init(const char *message, const char *heading_text) {
  ASSERT(message != NULL);

  lv_obj_clean(lv_scr_act());
  /* Leave references as the lv_objects will be freed by lv_obj_clean across
   * screen transitions */
  heading = NULL;
  instruction = NULL;

  if (heading_text != NULL) {
    heading = lv_label_create(lv_scr_act(), NULL);
    ui_heading(heading,
               heading_text,
               LV_HOR_RES - (strnlen(heading_text, MAX_HEADING_LEN) >> 1),
               LV_LABEL_ALIGN_CENTER);
  }
  instruction = lv_label_create(lv_scr_act(), NULL);
  ui_paragraph(instruction, message, LV_LABEL_ALIGN_CENTER);
  if (NULL == heading_text)
    lv_obj_align(instruction, NULL, LV_ALIGN_CENTER, 0, 0);
  else
    lv_obj_align(
        instruction, NULL, LV_ALIGN_CENTER, 0, lv_obj_get_height(heading) >> 1);

  lv_task_handler();
}

void instruction_scr_change_text(const char *new_message, bool immediate) {
  ASSERT(new_message != NULL);
  ASSERT(instruction != NULL);

  // Make sure instruction_scr_init is called first
  lv_label_set_static_text(instruction, new_message);
  if (heading == NULL)
    lv_obj_align(instruction, NULL, LV_ALIGN_CENTER, 0, 0);
  else
    lv_obj_align(
        instruction, NULL, LV_ALIGN_CENTER, 0, lv_obj_get_height(heading) >> 1);
  if (immediate == true) {
    lv_task_handler();
  }
}

void instruction_scr_destructor() {
  lv_obj_clean(lv_scr_act());    // screen will now be cleared
  heading = NULL;
  instruction = NULL;
}
