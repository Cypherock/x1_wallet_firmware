/*!\file cQueue.h
** \author SMFSW
** \copyright BSD 3-Clause License (c) 2017-2022, SMFSW
** \brief Queue handling library (written in plain c)
** \details Queue handling library (written in plain c)
**/
/****************************************************************/
#ifndef __CQUEUE_H
#define __CQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <inttypes.h>
#include <stdbool.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum {
  FIFO = 0,    //!< First In First Out behavior
  LIFO = 1     //!< Last In First Out behavior
} queue_type_e;

typedef struct {
  queue_type_e impl;    //!< Queue implementation: FIFO LIFO
  size_t queue_sz;      //!< Size of the full queue
  uint16_t rec_nb;      //!< number of records in the queue
  size_t rec_sz;        //!< Size of a record
  uint8_t *queue;       //!< Queue start pointer (when allocated)
  uint16_t in;          //!< number of records pushed into the queue
  uint16_t out;    //!< number of records pulled from the queue (only for FIFO)
  uint16_t cnt;    //!< number of records not retrieved from the queue
  bool init;       //!< set to true after successful init of the
                   //!< queue and reset when killing queue
} queue_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/*!	\brief Queue initialization (using static queue)
**	\param [in,out] pQ - pointer of queue to handle
**	\param [in] size_rec - size of a record in the queue (in bytes)
**	\param [in] nb_recs - number of records in the queue
**	\param [in] type - Queue implementation type: FIFO, LIFO
**	\param [in] pQDat - Pointer to static data queue
**	\param [in] lenQDat - Length of static data queue (in bytes) for static
*array size check against required size for queue *
**  \return Queue tab address (to remain consistent with \ref q_init)
**/
void *q_init_static(queue_t *const pQ,
                    const size_t size_rec,
                    const uint16_t nb_recs,
                    const queue_type_e type,
                    void *const pQDat,
                    const size_t lenQDat);

/*!	\brief Queue destructor: release dynamically allocated queue
**	\param [in,out] pQ - pointer of queue to handle
**/
void q_kill(queue_t *const pQ);

/*!	\brief Flush queue, restarting from empty queue
**	\param [in,out] pQ - pointer of queue to handle
**/
void q_flush(queue_t *const pQ);

/*!	\brief Push record to queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
*q_peekPrevious in both interrupts and main application, *
*you shall disable interrupts in main application when using these functions
**	\param [in,out] pQ - pointer of queue to handle
**	\param [in] record - pointer to record to be pushed into queue
**	\return Push status
**	\retval true if successfully pushed into queue
**	\retval false if queue is full
**/
bool q_push(queue_t *const pQ, const void *const record);

/*!	\brief Pop record from queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
*q_peekPrevious in both interrupts and main application, *
*you shall disable interrupts in main application when using these functions
**	\param [in] pQ - pointer of queue to handle
**	\param [in,out] record - pointer to record to be popped from queue
**	\return Pop status
**	\retval true if successfully pulled from queue
**	\retval false if queue is empty
**/
bool q_pop(queue_t *const pQ, void *const record);

/*!	\brief Peek record from queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
*q_peekPrevious in both interrupts and main application, *
*you shall disable interrupts in main application when using these functions
**	\note This function is most likely to be used in conjunction with q_drop
**	\param [in] pQ - pointer of queue to handle
**	\param [in,out] record - pointer to record to be peeked from queue
**	\return Peek status
**	\retval true if successfully peeked from queue
**	\retval false if queue is empty
**/
bool q_peek(const queue_t *const pQ, void *const record);

/*!	\brief Drop current record from queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
*q_peekPrevious in both interrupts and main application, *
*you shall disable interrupts in main application when using these functions
**	\note This function is most likely to be used in conjunction with q_peek
**	\param [in,out] pQ - pointer of queue to handle
**	\return drop status
**	\retval true if successfully dropped from queue
**	\retval false if queue is empty
**/
bool q_drop(queue_t *const pQ);

/*!	\brief Peek record at index from queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
*q_peekPrevious in both interrupts and main application, *
*you shall disable interrupts in main application when using these functions
**	\note This function is only useful if searching for a duplicate record
*and shouldn't be used in conjunction with q_drop
**	\param [in] pQ - pointer
*of queue to handle
**  \param [in,out] record - pointer to record to be peeked
*from queue
**  \param [in] idx - index of the record to pick
**  \return Peek status
**  \retval true if successfully peeked from queue
**	\retval false if index is out of range
**/
bool q_peekIdx(const queue_t *const pQ, void *const record, const uint16_t idx);

/*!	\brief get number of records in the queue
**	\param [in] pQ - pointer of queue to handle
**	\return Number of records stored in the queue
**/
uint16_t q_getCount(const queue_t *const pQ);

/*!	\brief get number of records left in the queue
**	\param [in] pQ - pointer of queue to handle
**	\return Number of records left in the queue
**/
uint16_t q_getRemainingCount(const queue_t *const pQ);

/****************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* __CQUEUE_H */

/****************************************************************/