#ifdef DEV_BUILD
#include "dev_utils.h"
ekp_queue_node *ekp_new_queue_node(const lv_event_t event, const uint32_t delay) {
  ekp_queue_node *temp = (ekp_queue_node *)malloc(sizeof(ekp_queue_node));
  temp->event  = event;
  temp->delay  = delay;
  temp->next = NULL;
  return temp;
}

ekp_queue *ekp_create_queue() {
  ekp_queue *q = (ekp_queue *)malloc(sizeof(ekp_queue));
  q->front = q->rear = NULL;
  q->count           = 0;
  return q;
}

int ekp_is_empty(ekp_queue *q) {
  return (q->count == 0);
}

void ekp_enqueue(ekp_queue *q, const lv_event_t event, const uint32_t delay) {
  ekp_queue_node *temp = ekp_new_queue_node(event, delay);
  q->count++;
  if (q->rear == NULL) {
    q->front = q->rear = temp;
    return;
  }
  q->rear->next = temp;
  q->rear       = temp;
}

ekp_queue_node *ekp_dequeue(ekp_queue *q) {
  if (ekp_is_empty(q))
    return NULL;
  ekp_queue_node *temp = q->front;
  q->front         = q->front->next;
  if (q->front == NULL)
    q->rear = NULL;
  q->count--;
  return temp;
}
#endif //DEV_BUILD