/**
 * @file    flow_engine.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef FLOW_ENGINE_H
#define FLOW_ENGINE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "events.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define ENGINE_STACK_DEPTH 10

#define ENGINE_LIFO_A 0
#define ENGINE_FIFO_A 1

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef void (*step_init_cb_t)(const void *);
typedef void (*flow_p0_evt_cb_t)(p0_evt_t, const void *);
typedef void (*step_ui_evt_cb_t)(ui_event_t, const void *);
typedef void (*step_usb_evt_cb_t)(usb_event_t, const void *);
typedef void (*step_nfc_evt_cb_t)(nfc_event_t, const void *);

/**
 * @brief This structure needs to be filled for each step for a flow.
 */
typedef struct {
  step_init_cb_t
      step_init_cb; /**< Initialization callback: which allows the step to
                       prepare the context - prepare UI components etc */
  flow_p0_evt_cb_t
      p0_cb; /**< P0 event callback: this callback will be called by the engine
                in case a P0 event arise during the current step */
  step_ui_evt_cb_t
      ui_cb; /**< UI event callback: this callback will be called by the engine
                in case an UI event arise during the current step */
  step_usb_evt_cb_t
      usb_cb; /**< USB event callback: this callback will be called by the
                 engine in case an USB event arise during the current step */
  step_nfc_evt_cb_t
      nfc_cb; /**< NFC event callback: this callback will be called by the
                 engine in case an NFC event arise during the current step */
  const evt_config_t
      *evt_cfg_ptr;    /**< Event configuration pointer: This pointer must store
                          the configuration related to the events which need to be
                          subscribed by a particular step */
  void *flow_data_ptr; /**< RFU: This pointer is kept as reserve for future use,
                          as there does not seem to be a use case currently */
} flow_step_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function initializes ALL the LIFO and FIFO queues implemented by
 * the engine. Therefore, this API must only be called if the complete engine
 * needs to be initialized or reset due to some requirement.
 */
void engine_initialize(void);

/**
 * @brief This function initializes only a specific LIFO or FIFO queue indicated
 * by the parameters. This API can be useful where-in a flow is using multiple
 * queues to maintain the flow.
 *
 * @param engine_stack The ID of the queue to be reset, it can be either the
 * LIFO or FIFO queues (ENGINE_LIFO_A,... or ENGINE_FIFO_A,....)
 * @return true If the queue is successfully reset
 * @return false If the ID of the queue passed as the parameter is incorrect or
 * represents an unimplemented queue
 */
bool engine_reset_flow(uint8_t engine_stack);

/**
 * @brief This function enqueues or push (based on ID passed as the paramter), a
 * pointer to the flow_step_t for a particular flow. A very important thing to
 * note here is that the data held by the pointer must be persistent, as the
 * reference to pointer is stored in the queue and not the data. The data
 * (flow_step_t) will be dereferenced at a later point in time using the pointer
 * (flow_step_t *) stored in the queue.
 * This API can be used to reach to the next step of a particular flow.
 *
 * @param engine_stack The ID of the queue to be selected, it can be either the
 * LIFO or FIFO queues (ENGINE_LIFO_A,... or ENGINE_FIFO_A,....)
 * @param flow_step_ptr Pointer to the step data which needs to be enqueued or
 * pushed on the the queue. It must be kept in mind that the pointer is being
 * enqueued and not the data held by the pointer. Therefore, the flow must
 * ensure that the data held in this pointer is persistent.
 * @return true If the pointer is successfully enqueued on to the selected queue
 * @return false If the pointer is not enqueued on the queue: It could be due to
 * incorrect paramters: like incorrect ID passed on as engine_stack parameter,
 * or because the queue is FULL!
 */
bool engine_next_flow_step(uint8_t engine_stack,
                           const flow_step_t *flow_step_ptr);

/**
 * @brief This function dequeues or pop an element of type flow_step_t *, from
 * the selected queue. An important thing to note here is that this API does not
 * return reference to the element dequeued.
 * This API can be used to reach to a previous step of a flow.
 *
 * @param engine_stack The ID of the queue to be selected, it can be either the
 * LIFO or FIFO queues (ENGINE_LIFO_A,... or ENGINE_FIFO_A,....)
 * @return true If the dequeue or pop occurred successfully
 * @return false If the dequeue or pop failed: It could be due to incorrect
 * parameters, or because the queue is EMPTY.
 */
bool engine_prev_flow_step(uint8_t engine_stack);

/**
 * @brief This function returns the a reference an element of type flow_step_t*,
 * from the selected queue which represents the current step of a flow.
 * This API can be used to get the current step of a flow in progress.
 *
 * @param engine_stack The ID of the queue to be selected, it can be either the
 * LIFO or FIFO queues (ENGINE_LIFO_A,... or ENGINE_FIFO_A,....)
 * @param flow_step_dptr A double pointer which will be filled with the element
 * of type flow_step_t*, which can be dereferenced by the caller.
 * @return true If the dequeue or pop occurred successfully
 * @return false If the dequeue or pop failed: It could be due to incorrect
 * parameters, or because the queue is EMPTY.
 */
bool engine_current_flow_step(uint8_t engine_stack,
                              flow_step_t **flow_step_dptr);

/**
 * @brief This API runs the flow represented by a queue until that particular
 * queue is empty.
 *
 * @param engine_stack The ID of the queue to be run, it can be either the
 * LIFO or FIFO queues (ENGINE_LIFO_A,... or ENGINE_FIFO_A,....)*
 */
void engine_run(uint8_t engine_stack);

#endif /* FLOW_ENGINE_H */
