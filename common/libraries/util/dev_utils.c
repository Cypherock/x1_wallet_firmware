/**
 * @file dev_utils.c
 * @author Cypherock X1 Team
 * @brief Utilities for DEV build
 *
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
#ifdef DEV_BUILD
#include "dev_utils.h"
static ekp_queue *q = NULL;

ekp_queue_node *ekp_new_queue_node(const lv_event_t event,
                                   const uint32_t delay) {
  ekp_queue_node *temp = (ekp_queue_node *)malloc(sizeof(ekp_queue_node));
  temp->event = event;
  temp->delay = delay;
  temp->next = NULL;
  return temp;
}

ekp_queue *ekp_create_queue() {
  ekp_queue *q = (ekp_queue *)malloc(sizeof(ekp_queue));
  q->front = q->rear = NULL;
  q->count = 0;
  return q;
}

bool ekp_is_empty() {
  return (q->count == 0);
}

void ekp_enqueue(const lv_event_t event, const uint32_t delay) {
  ekp_queue_node *temp = ekp_new_queue_node(event, delay);
  q->count++;
  if (q->rear == NULL) {
    q->front = q->rear = temp;
    return;
  }
  q->rear->next = temp;
  q->rear = temp;
}

ekp_queue_node *ekp_dequeue() {
  if (ekp_is_empty(q))
    return NULL;
  ekp_queue_node *temp = q->front;
  q->front = q->front->next;
  if (q->front == NULL)
    q->rear = NULL;
  q->count--;
  return temp;
}

void ekp_queue_init() {
  q = ekp_create_queue();
}

void ekp_process_queue(lv_indev_data_t *data) {
  static bool alternate = false;
  if (!alternate) {
    if (!ekp_is_empty()) {
      ekp_queue_node *qn = ekp_dequeue();
      data->state = LV_INDEV_STATE_PR;
      data->key = qn->event;
      BSP_DelayMs(qn->delay);
      free(qn);
      alternate = !alternate;
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
    BSP_DelayMs(RELEASE_DELAY);
    alternate = !alternate;
  }
}
#endif    // DEV_BUILD