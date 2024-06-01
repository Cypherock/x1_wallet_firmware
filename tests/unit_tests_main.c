/**
 * @file    unit_tests_main.c
 * @author  Cypherock X1 Team
 * @brief   MMain file to handle execution of all unit tests
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

#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain"       \
                            issue*/
#include "application_startup.h"
#include "unity_fixture.h"

#if USE_SIMULATOR == 1
#ifdef _WIN32
#define main SDL_main
#endif
#include "sim_usb.h"
extern lv_indev_t *indev_keypad;

/*On OSX SDL needs different handling*/
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
#if __APPLE__ && TARGET_OS_MAC
#define SDL_APPLE
#endif
#endif
#endif /* USE_SIMULATOR == 1 */

void RunAllTests(void) {
  RUN_TEST_GROUP(event_getter_test);
  RUN_TEST_GROUP(p0_events_test);
  RUN_TEST_GROUP(ui_events_test);
  RUN_TEST_GROUP(usb_evt_api_test);
  RUN_TEST_GROUP(nfc_events_test);
#ifdef NFC_EVENT_CARD_DETECT_MANUAL_TEST
  RUN_TEST_GROUP(nfc_events_manual_test);
#endif
  RUN_TEST_GROUP(xpub);
  RUN_TEST_GROUP(array_lists_tests);
  RUN_TEST_GROUP(flow_engine_tests);
  RUN_TEST_GROUP(manager_api_test);
  RUN_TEST_GROUP(btc_txn_helper_test);
  RUN_TEST_GROUP(btc_helper_test);
  RUN_TEST_GROUP(btc_script_test);
  RUN_TEST_GROUP(evm_eip1559_tests);
  RUN_TEST_GROUP(evm_txn_test);
  RUN_TEST_GROUP(evm_sign_msg_test);
  RUN_TEST_GROUP(near_helper_test);
  RUN_TEST_GROUP(solana_add_account_test);
#ifdef NEAR_FLOW_MANUAL_TEST
  RUN_TEST_GROUP(near_txn_user_verification_test);
#endif
  RUN_TEST_GROUP(utils_tests);
}

/**
 * @brief  The entry point to the unit test framework
 * This entry point is a parallel entry point to the int main(void) of the
 * actual firmware.
 */
int main(void) {
  application_init();

  UnityBegin("unit_tests_main.c");
  RunAllTests();
  UnityEnd();
}

#if USE_SIMULATOR == 0
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

/**
 * @brief Function to transmit data in real-time over SWV channel
 * @param file unused
 * @param *ptr string pointer for data to send
 * @param len  length of data to send
 *
 * @ret len of data transmitted
 */
int _write(int file, char *ptr, int len) {
#ifndef NDEBUG    // Disable printf in release mode
  int DataIdx;
  for (DataIdx = 0; DataIdx < len; DataIdx++) {
    ITM_SendChar(*ptr++);
  }
  return len;
#endif
}

#endif /* USE_SIMULATOR == 0 */
