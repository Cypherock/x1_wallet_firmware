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

#include "base58.h"
#include "bip32.h"
#include "bip39.h"
#include "curves.h"
#include "nist256p1.h"
#include "sec_flash_priv.h"
#include "utils.h"
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
#ifdef DEV_BUILD
#include "dev_utils.h"
#endif
void RunAllTests(void) {
  RUN_TEST_GROUP(event_getter_test);
  RUN_TEST_GROUP(p0_events_test);
  RUN_TEST_GROUP(ui_events_test);
  RUN_TEST_GROUP(usb_evt_api_test);
  RUN_TEST_GROUP(nfc_events_test);
  RUN_TEST_GROUP(core_session_test);
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
  RUN_TEST_GROUP(inheritance_auth_wallet_tests);
  RUN_TEST_GROUP(utils_tests);
  RUN_TEST_GROUP(inheritance_encryption_tests);
  RUN_TEST_GROUP(inheritance_decryption_tests);
}

/**
 * @brief  The entry point to the unit test framework
 * This entry point is a parallel entry point to the int main(void) of the
 * actual firmware.
 */

#if USE_SIMULATOR == 1
void create_flash_keys() {
  const uint32_t version = 0x0488b21e;
  const char *curve = NIST256P1_NAME;
  const char *mnemonic = "embody pepper stumble";
  printf("%s\n\n", mnemonic);

  uint8_t seed[64];
  mnemonic_to_seed(mnemonic, "", seed, NULL);
  printf("\n");

  HDNode m;
  hdnode_from_seed(seed, 64, curve, &m);
  hdnode_fill_public_key(&m);

  const uint32_t h = 0x80000000;
  // m/1000'/0'/2'/0
  uint32_t path[] = {1000 | h, 0 | h, 2 | h, 0};    // path to generate nodes

  const size_t path_size = sizeof(path) / 4;
  HDNode nodes[path_size];
  HDNode last = m;
  for (size_t i = 0; i < path_size; i++) {
    nodes[i] = last;
    hdnode_private_ckd(&nodes[i], path[i]);
    hdnode_fill_public_key(&nodes[i]);
    last = nodes[i];
  }
  uint8_t card_root_xpub[FS_KEYSTORE_XPUB_LEN] = {0};
  char xpub[XPUB_SIZE];
  hdnode_serialize_public(&last,
                          hdnode_fingerprint(&nodes[path_size - 2]),
                          version,
                          xpub,
                          XPUB_SIZE);
  base58_decode_check(
      xpub, nist256p1_info.hasher_base58, card_root_xpub, XPUB_SIZE);
  get_flash_perm_instance();
  memcpy(flash_perm_instance.permKeyData.ext_keys.card_root_xpub,
         card_root_xpub,
         FS_KEYSTORE_XPUB_LEN);
}
#endif

int main(void) {
#if USE_SIMULATOR == 1
  create_flash_keys();
#endif
  application_init();
#ifdef DEV_BUILD
  ekp_queue_init();
#endif
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
