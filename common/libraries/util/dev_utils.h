/**
 * @file dev_utils.h
 * @author Cypherock X1 Team
 * @brief Utilities for DEV build
 *        This file contains utilities for the DEV build which helps
 *        the debugging and testing process.
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 *
 */
#ifdef DEV_BUILD
#ifndef DEV_UTILS_H
#define DEV_UTILS_H
#include <lvgl/lvgl.h>
#include <stdlib.h>

// EMULATE KEY PRESSES FUNCTIONALITY
// This is intended for the purpose of testing UI intensive flows which require
// user interaction This functionality adds the ability to emulate key inputs
// which by default is added to every UI element which supports an accept button
// i.e. every UI screen with an accept button will automatically be activated
// when this feature is enabled.

/**
 * @brief Delays used by Emulated Key Presses functionality
 */
#define DEFAULT_DELAY 50
#define RELEASE_DELAY 100

/**
 * @brief A queue node for emulate key press events
 *  this contains an lv_event which denotes the event to be dispatched
 *  a delay which dictates how long the key press should be held
 */
typedef struct ekp_queue_node {
  lv_event_t event;
  uint32_t delay;
  struct ekp_queue_node *next;
} ekp_queue_node;

/**
 * @brief Queue structure to which contains front rear and count of the queue
 */
typedef struct ekp_queue {
  ekp_queue_node *front;
  ekp_queue_node *rear;
  int count;
} ekp_queue;

/**
 * @brief Create a new queue node with given parameters
 *
 * @param event event which must be triggered
 * @param delay the delay for which the key must be held down
 * @return ekp_queue_node*
 */
ekp_queue_node *ekp_new_queue_node(const lv_event_t event,
                                   const uint32_t delay);

/**
 * @brief Creates a new Queue and returns its pointer
 *
 * @return ekp_queue*
 */
ekp_queue *ekp_create_queue();
/**
 * @brief Checks whether the queue is empty
 *
 * @return bool returns true if the queue is empty and false otherwise
 */
bool ekp_is_empty();

/**
 * @brief Creates and inserts a new queue node into the queue
 *
 * @param event event which must be triggered
 * @param delay the delay for which the key must be held down
 */
void ekp_enqueue(const lv_event_t event, const uint32_t delay);

/**
 * @brief Pops the last inserted element from the queue and returns it if it
 * exists and returns NULL otherwise
 *
 * @return ekp_queue_node*
 */
ekp_queue_node *ekp_dequeue();

/**
 * @brief Initializes the global queue instance
 */
void ekp_queue_init();

/**
 * @brief Queue processing function which pops the queue and and updates the
 * indev data to be used by the lvgl
 *
 * @param data pointer to lv_indev_data_t object which needs to be updated
 */
void ekp_process_queue(lv_indev_data_t *data);

#endif    // DEV_UTILS_H
#endif    // DEV_BUILD