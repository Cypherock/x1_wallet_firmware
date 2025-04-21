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

/**
 * @brief Initializes the composable app queue.
 *
 * This function creates and initializes the composable app queue. It must be
 * called before any other queue operations.
 */
void caq_init(void);

/**
 * @brief Checks if the composable app queue is empty.
 *
 * @return true If the queue is empty.
 * @return false If the queue is not empty.
 */
bool caq_is_empty(void);

/**
 * @brief Pushes data onto the composable app queue.
 *
 * @param data The data to be pushed onto the queue.
 */
void caq_push(caq_node_data_t data);

/**
 * @brief Pops data from the composable app queue.
 *
 * @return true If data was successfully popped from the queue.
 * @return false If the queue was empty.
 */
bool caq_pop(void);

/**
 * @brief Peeks at the data at the front of the composable app queue.
 *
 * @param status Pointer to a boolean variable to indicate if the peek was
 * successful.
 *               Will be set to true if successful, false otherwise.
 *
 * @return caq_node_data_t The data at the front of the queue.  If the queue is
 * empty,
 *                         it returns a zeroed out data. Check the status
 * parameter
 *                         to see if the queue was empty.
 */
caq_node_data_t caq_peek(bool *status);

/**
 * @brief Clears all elements from the composable app queue.
 *
 * @return true If the queue was successfully cleared.
 * @return false If there was an error during clearing.
 */
bool caq_clear(void);

#endif    // COMPOSABLE_APP_QUEUE
