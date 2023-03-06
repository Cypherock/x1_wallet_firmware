#ifdef DEV_BUILD
#ifndef DEV_UTILS_H
#define DEV_UTILS_H
#include <stdlib.h>
#include <lvgl/lvgl.h>
#define DEFAULT_DELAY 50
#define RELEASE_DELAY 100

typedef struct ekp_queue_node {
    lv_event_t event;
    uint32_t delay;
    struct ekp_queue_node *next;
} ekp_queue_node;

typedef struct ekp_queue {
  ekp_queue_node *front;
  ekp_queue_node *rear;
  int count;
} ekp_queue;

ekp_queue_node *ekp_new_queue_node(const lv_event_t event, const uint32_t delay);
ekp_queue *ekp_create_queue();
int ekp_is_empty();
void ekp_enqueue( const lv_event_t event, const uint32_t delay);
ekp_queue_node *ekp_dequeue();
void ekp_queue_init();

#endif //DEV_UTILS_H
#endif //DEV_BUILD