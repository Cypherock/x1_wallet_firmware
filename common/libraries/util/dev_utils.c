#ifdef DEV_BUILD
#include "dev_utils.h"
static ekp_queue *q = NULL;

ekp_queue_node *ekp_new_queue_node(const lv_event_t event, const uint32_t delay) {
  ekp_queue_node *temp = (ekp_queue_node *)malloc(sizeof(ekp_queue_node));
  temp->event          = event;
  temp->delay          = delay;
  temp->next           = NULL;
  return temp;
}

ekp_queue *ekp_create_queue() {
  ekp_queue *q = (ekp_queue *)malloc(sizeof(ekp_queue));
  q->front = q->rear = NULL;
  q->count           = 0;
  return q;
}

int ekp_is_empty() {
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
  q->rear       = temp;
}

ekp_queue_node *ekp_dequeue() {
  if (ekp_is_empty(q))
    return NULL;
  ekp_queue_node *temp = q->front;
  q->front             = q->front->next;
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
      data->state        = LV_INDEV_STATE_PR;
      data->key          = qn->event;
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
#endif  //DEV_BUILD