/*!\file cQueue.c
** \author SMFSW
** \copyright BSD 3-Clause License (c) 2017-2022, SMFSW
** \brief Queue handling library (written in plain c)
** \details Queue handling library (written in plain c)
**/
/****************************************************************/
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "cQueue.h"

#include <stdlib.h>
#include <string.h>

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
/*!	\brief get initialization state of the queue
**	\param [in] pQ - pointer of queue to handle
**	\return Queue initialization status
**	\retval true if queue is allocated
**	\retval false is queue is not allocated
**/
static inline bool q_isInitialized(const queue_t *const pQ) {
  return (pQ->init == true) ? true : false;
}

/*!	\brief get emptiness state of the queue
**	\param [in] pQ - pointer of queue to handle
**	\return Queue emptiness status
**	\retval true if queue is empty
**	\retval false is not empty
**/
static inline bool q_isEmpty(const queue_t *const pQ) {
  return (!pQ->cnt) ? true : false;
}

/*!	\brief get fullness state of the queue
**	\param [in] pQ - pointer of queue to handle
**	\return Queue fullness status
**	\retval true if queue is full
**	\retval false is not full
**/
static inline bool q_isFull(const queue_t *const pQ) {
  return (pQ->cnt == pQ->rec_nb) ? true : false;
}

/*!	\brief get size of queue
**	\remark Size in bytes (like sizeof)
**	\param [in] pQ - pointer of queue to handle
**	\return Size of queue in bytes
**/
static inline uint32_t q_sizeof(const queue_t *const pQ) {
  return pQ->queue_sz;
}

/*!	\brief Peek previous record from queue
**	\warning If using q_push, q_pop, q_peek, q_drop, q_peekItem and/or
* q_peekPrevious in both interrupts and main application, *
* you shall disable interrupts in main application when using these functions
**	\note This inline is only useful with FIFO implementation, use q_peek
* instead with a LIFO (will lead to the same result)
**	\param [in] pQ - pointer of queue to handle *
**	\param [in,out] record - pointer to record to be peeked from queue
**	\return Peek status
**	\retval true if successfully peeked from queue
**  \retval false if queue is empty
**/
static inline bool q_peekPrevious(const queue_t *const pQ, void *const record) {
  const uint16_t idx =
      q_getCount(pQ) - 1;    // No worry about count - 1 when queue is empty,
                             // test is done by q_peekIdx
  return q_peekIdx(pQ, record, idx);
}

/*!	\brief Increment index
**	\details Increment buffer index \b pIdx rolling back to \b start when
* limit \b end is reached
**	\param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void inc_idx(uint16_t *const pIdx,
                           const uint16_t end,
                           const uint16_t start) {
  if (*pIdx < end - 1) {
    (*pIdx)++;
  } else {
    *pIdx = start;
  }
}

/*!	\brief Decrement index
**	\details Decrement buffer index \b pIdx rolling back to \b end when
* limit \b start is reached
**  \param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void dec_idx(uint16_t *const pIdx,
                           const uint16_t end,
                           const uint16_t start) {
  if (*pIdx > start) {
    (*pIdx)--;
  } else {
    *pIdx = end - 1;
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void *q_init_static(queue_t *const pQ,
                    const size_t size_rec,
                    const uint16_t nb_recs,
                    const queue_type_e type,
                    void *const pQDat,
                    const size_t lenQDat) {
  // De-Init queue (if previously initialized) and set structure
  // to 0 to ensure proper functions behavior when queue is not
  // allocated
  q_kill(pQ);

  const uint32_t size = nb_recs * size_rec;

  // Check static Queue data size
  if (lenQDat < size) {
    return NULL;
  }

  pQ->queue = (uint8_t *)pQDat;

  if (pQ->queue != NULL) {
    pQ->queue_sz = size;
    pQ->rec_sz = size_rec;
    pQ->rec_nb = nb_recs;
    pQ->impl = type;

    pQ->init = true;
  }

  // return NULL when queue not properly allocated, Queue
  // data address otherwise
  return pQ->queue;
}

void q_kill(queue_t *const pQ) {
  memset(pQ, 0, sizeof(queue_t));
}

void q_flush(queue_t *const pQ) {
  pQ->in = 0;
  pQ->out = 0;
  pQ->cnt = 0;
}

bool q_push(queue_t *const pQ, const void *const record) {
  if (q_isFull(pQ)) {
    return false;
  }

  uint8_t *const pStart = pQ->queue + (pQ->rec_sz * pQ->in);
  memcpy(pStart, record, pQ->rec_sz);

  inc_idx(&pQ->in, pQ->rec_nb, 0);

  if (!q_isFull(pQ)) {
    // Increase records count
    pQ->cnt++;
  }

  return true;
}

bool q_pop(queue_t *const pQ, void *const record) {
  const uint8_t *pStart;

  // No more records
  if (q_isEmpty(pQ)) {
    return false;
  }

  if (pQ->impl == FIFO) {
    pStart = pQ->queue + (pQ->rec_sz * pQ->out);
    inc_idx(&pQ->out, pQ->rec_nb, 0);
  } else if (pQ->impl == LIFO) {
    dec_idx(&pQ->in, pQ->rec_nb, 0);
    pStart = pQ->queue + (pQ->rec_sz * pQ->in);
  } else {
    return false;
  }

  memcpy(record, pStart, pQ->rec_sz);

  // Decrease records count
  pQ->cnt--;
  return true;
}

bool q_peek(const queue_t *const pQ, void *const record) {
  const uint8_t *pStart;

  // No more records
  if (q_isEmpty(pQ)) {
    return false;
  }

  if (pQ->impl == FIFO) {
    pStart = pQ->queue + (pQ->rec_sz * pQ->out);
    // No change on out var as it's just a peek
  } else if (pQ->impl == LIFO) {
    // Temporary var for peek (no change on pQ->in with dec_idx)
    uint16_t rec = pQ->in;
    dec_idx(&rec, pQ->rec_nb, 0);
    pStart = pQ->queue + (pQ->rec_sz * rec);
  } else {
    return false;
  }

  memcpy(record, pStart, pQ->rec_sz);
  return true;
}

bool q_drop(queue_t *const pQ) {
  // No more records
  if (q_isEmpty(pQ)) {
    return false;
  }

  if (pQ->impl == FIFO) {
    inc_idx(&pQ->out, pQ->rec_nb, 0);
  } else if (pQ->impl == LIFO) {
    dec_idx(&pQ->in, pQ->rec_nb, 0);
  } else {
    return false;
  }

  // Decrease records count
  pQ->cnt--;
  return true;
}

bool q_peekIdx(const queue_t *const pQ,
               void *const record,
               const uint16_t idx) {
  const uint8_t *pStart;

  // Index out of range
  if (idx + 1 > q_getCount(pQ)) {
    return false;
  }

  if (pQ->impl == FIFO) {
    pStart = pQ->queue + (pQ->rec_sz * ((pQ->out + idx) % pQ->rec_nb));
  } else if (pQ->impl == LIFO) {
    pStart = pQ->queue + (pQ->rec_sz * idx);
  } else {
    return false;
  }

  memcpy(record, pStart, pQ->rec_sz);
  return true;
}

uint16_t q_getRemainingCount(const queue_t *const pQ) {
  return pQ->rec_nb - pQ->cnt;
}

uint16_t q_getCount(const queue_t *const pQ) {
  return pQ->cnt;
}
