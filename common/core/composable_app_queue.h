/**
 * @author  Cypherock X1 Team
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef COMPOSABLE_APP_QUEUE
#define COMPOSABLE_APP_QUEUE

#include <stdint.h>
#pragma once

#include <stdbool.h>

#define APP_QUEUE_PARAMS_SIZE 128
#define APP_QUEUE_CALLER_NAME_SIZE 32

typedef struct caq_node_data {
  uint32_t applet_id;
  uint8_t params[APP_QUEUE_PARAMS_SIZE];
  // Add a caller id here if multiple apps are using this in future
} caq_node_data_t;

typedef struct caq_node {
  caq_node_data_t data;
  struct caq_node *next;
} caq_node_t;

typedef struct composable_app_queue {
  caq_node_t *front;
  caq_node_t *rear;
  int count;
} composable_app_queue_t;

bool caq_is_empty();

void caq_push(caq_node_data_t data);

bool caq_pop();

caq_node_data_t caq_peek(bool *status);

bool caq_clear();

void caq_init();

#endif    // COMPOSABLE_APP_QUEUE
