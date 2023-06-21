/**
 * @file    p0_events_test.c
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "p0_events_test.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define P0_UNIT_TESTS_TIMEOUT_5S (5 * 1000)

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

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(p0_events_test);

TEST_SETUP(p0_events_test) {
  return;
}

TEST_TEAR_DOWN(p0_events_test) {
  return;
}

TEST(p0_events_test, inactivity_evt) {
  p0_evt_t p0_evt;

  p0_ctx_init(P0_UNIT_TESTS_TIMEOUT_5S);
#if USE_SIMULATOR == 1
  p0_set_inactivity_evt(true);
#endif /* USE_SIMULATOR == 1 */

  while (p0_get_evt(&p0_evt) != true)
    ;

  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == true);
  TEST_ASSERT(p0_evt.abort_evt == false);

  /* Re-read variables before destroying context */
  memzero(&p0_evt, sizeof(p0_evt));
  TEST_ASSERT(p0_get_evt(&p0_evt) == true);
  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == true);
  TEST_ASSERT(p0_evt.abort_evt == false);

  p0_ctx_destroy();
  p0_reset_evt();

  /* Re-check after destroying context */
  TEST_ASSERT(p0_get_evt(&p0_evt) == false);
  TEST_ASSERT(p0_evt.flag == false);
  TEST_ASSERT(p0_evt.inactivity_evt == false);
  TEST_ASSERT(p0_evt.abort_evt == false);

  return;
}

TEST(p0_events_test, abort_evt) {
  p0_evt_t p0_evt;

  p0_ctx_init(P0_UNIT_TESTS_TIMEOUT_5S);

  /* Set abort manually - it will be set by communication module */
  p0_set_abort_evt(true);

  while (p0_get_evt(&p0_evt) != true)
    ;

  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == false);
  TEST_ASSERT(p0_evt.abort_evt == true);

  /* Re-read variables before destroying context */
  memzero(&p0_evt, sizeof(p0_evt));
  TEST_ASSERT(p0_get_evt(&p0_evt) == true);
  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == false);
  TEST_ASSERT(p0_evt.abort_evt == true);

  p0_ctx_destroy();
  p0_reset_evt();

  /* Re-check after destroying context */
  TEST_ASSERT(p0_get_evt(&p0_evt) == false);
  TEST_ASSERT(p0_evt.flag == false);
  TEST_ASSERT(p0_evt.inactivity_evt == false);
  TEST_ASSERT(p0_evt.abort_evt == false);

  return;
}

TEST(p0_events_test, abort_inactivity_race) {
  p0_evt_t p0_evt;

  p0_ctx_init(P0_UNIT_TESTS_TIMEOUT_5S);

  /* Manually disable systick module as we want to emulate race scenario here */
  systick_set_timeout_config(false);
  systick_reset_timer();

  /* Set abort & inactivity event manually */
  p0_set_abort_evt(true);
  p0_set_inactivity_evt(true);

  while (p0_get_evt(&p0_evt) != true)
    ;

  /* If a race condition arises, wherein inactivity event and abort event come
   * at the same time, the system should report both events */
  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == true);
  TEST_ASSERT(p0_evt.abort_evt == true);

  p0_ctx_destroy();
  p0_reset_evt();

  return;
}

TEST(p0_events_test, inactivity_refresh_on_joystick_movement) {
  p0_evt_t p0_evt;

  p0_ctx_init(P0_UNIT_TESTS_TIMEOUT_5S);

  uint32_t return_status = false;
  uint16_t refresh_count = 0;
  while (return_status != true) {
    return_status = p0_get_evt(&p0_evt);
    if (refresh_count < 1000) {
      /* Emulate joystick movement: ideally it will be done in a loop if a
       * keypad press is detected */
      systick_reset_timer();
      refresh_count += 1;
    } else {
#if USE_SIMULATOR == 1
      p0_set_inactivity_evt(true);
#endif /* USE_SIMULATOR == 1 */
    }
  }

  /* As of now, we need to notice if inactivity timeout took longer than
   * MAX_INACTIVITY_TIMEOUT milliseconds */
  TEST_ASSERT(p0_evt.flag == true);
  TEST_ASSERT(p0_evt.inactivity_evt == true);
  TEST_ASSERT(p0_evt.abort_evt == false);

  p0_ctx_destroy();
  p0_reset_evt();
  return;
}
