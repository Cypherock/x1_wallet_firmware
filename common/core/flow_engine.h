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
#include <stddef.h>
#include <stdint.h>

#include "array_list_type.h"
#include "events.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef array_list_t engine_ctx_t;

typedef void (*step_init_cb_t)(engine_ctx_t *, const void *);
typedef void (*flow_p0_evt_cb_t)(engine_ctx_t *, p0_evt_t, const void *);
typedef void (*step_ui_evt_cb_t)(engine_ctx_t *, ui_event_t, const void *);
typedef void (*step_usb_evt_cb_t)(engine_ctx_t *, usb_event_t, const void *);
typedef void (*step_nfc_evt_cb_t)(engine_ctx_t *, nfc_event_t, const void *);

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
 * @brief This function initializes only a specific buffer out of all the
 * buffers implemented and maintained by the engine. This API can be useful
 * where-in a flow is using multiple buffer to maintain the flow.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @return true If the buffer is successfully reset
 * @return false If the ID of the queue passed as the parameter is incorrect or
 * represents an unimplemented buffer
 */
bool engine_reset_flow(engine_ctx_t *ctx);

/**
 * @brief This function inserts a pointer to the flow_step_t for a particular
 * flow at the end. A very important thing to note here is that the data held by
 * the pointer must be persistent, as the reference is stored in the
 * buffer and not it's data. The data (flow_step_t) will be dereferenced at a
 * later point in time using the pointer (flow_step_t *) stored in the buffer.
 * This API can be used to insert a step in the flow. However, note that this
 * inserts the element in the end of the buffer.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @param flow_step_ptr Pointer to the step data which needs to be added in the
 * particular flow. It must be kept in mind that the pointer is being inserted
 * and not the data held by the pointer. Therefore, the flow must ensure that
 * the data held in this pointer is persistent.
 * @return true If the pointer is successfully insterted on to the selected
 * buffer
 * @return false If the pointer is not inserted on the buffer: It could be due
 * to incorrect paramters: like incorrect ID passed on as engine_buffer
 * parameter, or because the buffer is FULL!
 */
bool engine_add_next_flow_step(engine_ctx_t *ctx,
                               const flow_step_t *flow_step_ptr);

/**
 * @brief This function can be used to increment the index of the current engine
 * buffer and move to the next step of a flow.
 * This API can be useful to reach the next step. Important thing to note here
 * is that the current step will not be deleted and user may choose to return to
 * this step at a later point.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @return true If the index is incremented
 * @return false If the index could not be incremented: It could be due to
 * incorrect parameters: like incorrect ID passed on a engine_buffer, or because
 * the flow has already reached the end.
 */
bool engine_goto_next_flow_step(engine_ctx_t *ctx);

/**
 * @brief This function can be used to decrement the index of the current engine
 * buffer and move to the previous step of a flow.
 * This API can be useful to reach the previous step. Important thing to note
 here
 * is that the current step will not be deleted and user may choose to return to
 * this step at a later point.

 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @return true If the index is decremented
 * @return false If the index could not be decremented: It could be due to
 * incorrect parameters: like incorrect ID passed on a engine_buffer, or because
 * the flow has already reached the starting point.
 */
bool engine_goto_prev_flow_step(engine_ctx_t *ctx);

/**
 * @brief This function deletes the current step held in the buffer, and
 * internally moves to the previous step in the flow.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @return true If the element was deleted successfully
 * @return false If the element was not deleted: It could be due to incorrect
 * parameters, or because the buffer is EMPTY.
 */
bool engine_delete_current_flow_step(engine_ctx_t *ctx);

/**
 * @brief This API runs the flow represented by a queue until that particular
 * queue is empty.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 */
void engine_run(engine_ctx_t *ctx);

#endif /* FLOW_ENGINE_H */
