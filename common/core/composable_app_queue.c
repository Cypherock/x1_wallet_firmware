/**
 * @author  Cypherock X1 Team
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#include "composable_app_queue.h"

#include <stdlib.h>
static composable_app_queue_t *composable_app_queue = NULL;

static caq_node_t *caq_new_node(caq_node_data_t data) {
  caq_node_t *temp = (caq_node_t *)malloc(sizeof(caq_node_t));
  temp->data = data;
  temp->next = NULL;
  return temp;
}

static composable_app_queue_t *caq_create_queue() {
  composable_app_queue_t *queue =
      (composable_app_queue_t *)malloc(sizeof(composable_app_queue_t));
  queue->front = queue->rear = NULL;
  queue->count = 0;
  return queue;
}

bool caq_is_empty() {
  return (composable_app_queue == NULL || composable_app_queue->count == 0);
}

void caq_push(caq_node_data_t data) {
  caq_node_t *temp = caq_new_node(data);
  composable_app_queue->count++;

  if (composable_app_queue->rear == NULL) {
    composable_app_queue->front = composable_app_queue->rear = temp;
    return;
  }

  composable_app_queue->rear->next = temp;
  composable_app_queue->rear = temp;
}

bool caq_pop() {
  if (caq_is_empty()) {
    return false;
  }

  caq_node_t *temp = composable_app_queue->front;

  composable_app_queue->front = composable_app_queue->front->next;
  if (composable_app_queue->front == NULL) {
    composable_app_queue->rear = NULL;
  }
  composable_app_queue->count--;

  free(temp);
  return true;
}

caq_node_data_t caq_peek(bool *status) {
  *status = false;
  caq_node_data_t output = {0};

  if (caq_is_empty()) {
    return output;
  }

  caq_node_t *temp = composable_app_queue->front;

  output = temp->data;
  *status = true;

  return output;
}

bool caq_clear() {
  while (!caq_is_empty()) {
    bool status = caq_pop();
    if (!status) {
      return false;
    }
  }
  return true;
}

void caq_init() {
  composable_app_queue = caq_create_queue();
}
