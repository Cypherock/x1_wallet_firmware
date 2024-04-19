/**
 * @file    main.c
 * @author  Cypherock X1 Team
 * @brief   Main source file.
 *          Entry point to the application.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain"       \
                            issue*/
#include "application_startup.h"
#include "board.h"
#include "logger.h"
#include "onboarding.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys_state.h"
#include "time.h"
#include "ui_logo.h"

#define RUN_ENGINE
#ifdef RUN_ENGINE
#include "core_flow_init.h"
#endif /* RUN_ENGINE */

#if USE_SIMULATOR == 0
#include "cmsis_gcc.h"
#include "main.h"
#endif    // USE_SIMULATOR

#if USE_SIMULATOR == 1
#ifdef _WIN32
#define main SDL_main
#endif
#include "sim_usb.h"
extern lv_indev_t *indev_keypad;
#endif
extern Counter counter;
extern bool main_app_ready;

#if USE_SIMULATOR == 1

/*On OSX SDL needs different handling*/
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
#if __APPLE__ && TARGET_OS_MAC
#define SDL_APPLE
#endif
#endif

static int tick_thread(void *data);
static void memory_monitor(lv_task_t *param);

#endif
#ifdef DEV_BUILD
#include "dev_utils.h"
#endif
/**
 * @brief  The entry point to the application.
 * @retval int
 */

#include <pb_decode.h>
#include <pb_encode.h>

#include "bittensor_api.h"

int main(void) {
#ifdef DEV_BUILD
  ekp_queue_init();
#endif
  application_init();

  // bittensor_query_t query = {
  //     .which_request = 2,
  //     .sign_txn = {.which_request = 1,
  //                  .initiate = {
  //                      .derivation_path_count = 5,
  //                      .derivation_path = {NON_SEGWIT, ETHEREUM, BITCOIN, 0,
  //                      0}, .wallet_id = {}, .address_format = EVM_DEFAULT,
  //                      .transaction_size = 109,
  //                  }}};
  // bittensor_query_t query1 = {.which_request = 2,
  //                       .sign_txn = {.which_request = 2,
  //                                    .txn_data = {.has_chunk_payload = true,
  //                                                 .chunk_payload = {
  //                                                     .chunk =
  //                                                         {
  //                                                             .size = 109,
  //                                                         },
  //                                                     .remaining_size = 0,
  //                                                     .chunk_index = 0,
  //                                                     .total_chunks = 1,
  //                                                 }}}};
  // // raw Txn:
  // //
  // https://etherscan.io/getRawTx?tx=0xb8a6f4b82037dfc9aefa22d4cca2f695ada1d5d568673d001cf2d7dab27bea25
  // hex_string_to_byte_array(
  //     "f86b81a585043be57f958301725d94dac17f958d2ee523a2206206994597c13d831ec700"
  //     "b844a9059cbb0000000000000000000000001754b2d4414468d38bddea24b83cdb1a9b8c"
  //     "4355000000000000000000000000000000000000000000000000000000320effa6000180"
  //     "80",
  //     218,
  //     query1.sign_txn.txn_data.chunk_payload.chunk.bytes);
  // txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  // memzero(txn_context, sizeof(evm_txn_context_t));
  // memcpy(&txn_context->init_info,
  //        &query.sign_txn.initiate,
  //        sizeof(evm_sign_txn_initiate_request_t));
  // TEST_ASSERT_TRUE(pb_encode(&ostream, EVM_QUERY_FIELDS, &query1));
  // usb_set_event(sizeof(core_msg), core_msg, ostream.bytes_written, buffer);
  // TEST_ASSERT_TRUE(fetch_valid_transaction(&query));

#ifdef RUN_ENGINE
#if USE_SIMULATOR == 0
  if (fault_in_prev_boot()) {
    handle_fault_in_prev_boot();
  } else
#endif /* USE_SIMULATOR == 0 */
  {
    logo_scr_init(2000);
    device_provision_check();
  }

  while (1) {
    engine_ctx_t *main_engine_ctx = get_core_flow_ctx();
    engine_run(main_engine_ctx);
  }
#else /* RUN_ENGINE */
  while (true) {
    proof_of_work_task();

#if USE_SIMULATOR == 1
    usbsim_continue_loop();

#ifdef SDL_APPLE
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
#if USE_MOUSE != 0
      mouse_handler(&event);
#endif

#if USE_KEYBOARD
      keyboard_handler(&event);
#endif

#if USE_MOUSEWHEEL != 0
      mousewheel_handler(&event);
#endif
    }
#endif /*SDL_APPLE */
  }
#endif /* USE_SIMULATOR == 1 */

#endif /* RUN_ENGINE */
  return 0;
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

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

#endif    // USE_SIMULATOR
