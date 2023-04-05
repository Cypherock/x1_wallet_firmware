#include "buzzer.h"

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

static volatile bool buzzer_status = 0;    // A flag indicating PWM status.

static void buzzer_timer_handler(void *p_context);

static void buzzer_timer_start(uint32_t msec);

static void buzzer_stop();

/* @function : software_timer_init
 * @brief    :
 */
void buzzer_timer_create(void) {
}

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
  // ready_flag = true;
}

void buzzer_init() {
}

void buzzer_start(uint32_t on_time_msec) {
}

/**@brief Timeout handler for the repeated timer.
 * This is called every time
  timer ends.
 */
static void buzzer_timer_handler(void *p_context) {
}

static void buzzer_timer_start(uint32_t msec) {
}

static void buzzer_stop() {
}
