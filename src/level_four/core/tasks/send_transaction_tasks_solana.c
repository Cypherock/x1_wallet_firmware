/**
 * @file    send_transaction_tasks_solana.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction for SOLANA.
 *          This file contains functions to send transaction for SOLANA.
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
#include "constant_texts.h"
#include "controller_level_four.h"
#include "int-util.h"
#include "math.h"
#include "solana.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "utils.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

extern lv_task_t *timeout_task;

void send_transaction_tasks_solana() {
  switch (flow_level.level_three) {
    case SEND_TXN_VERIFY_COIN_SOLANA: {
      instruction_scr_init(ui_text_processing, NULL);
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_RECEIVED_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_VERIFY_CONTRACT_ADDRESS: {
      mark_event_over();
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SOLANA: {
      instruction_scr_destructor();
      char address[45] = {0};
      char top_heading[225] = {0};
      char display[70] = {0};

      instruction_scr_destructor();
      size_t address_size = sizeof(address);
      b58enc(address,
             &address_size,
             solana_unsigned_txn_ptr.instruction.program.transfer
                 .recipient_account,
             SOLANA_ACCOUNT_ADDRESS_LENGTH);
      snprintf(top_heading, sizeof(top_heading), "%s", ui_text_verify_address);
      snprintf(display, sizeof(display), "%s%s", ui_text_20_spaces, address);
      address_scr_init(top_heading, display, true);
    } break;

    case SEND_TXN_CALCULATE_AMOUNT_SOLANA: {
      instruction_scr_init(ui_text_processing, NULL);
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT_SOLANA: {
      char amount_string[40] = {'\0'}, amount_decimal_string[30] = {'\0'};
      char display[110] = {'\0'};
      uint8_t be_lamports[8] = {0};
      int i = 8;
      while (i--)
        be_lamports[i] =
            solana_unsigned_txn_ptr.instruction.program.transfer.lamports >>
            8 * (7 - i);
      byte_array_to_hex_string(
          be_lamports, 8, amount_string, sizeof(amount_string));
      if (!convert_byte_array_to_decimal_string(16,
                                                solana_get_decimal(),
                                                amount_string,
                                                amount_decimal_string,
                                                sizeof(amount_decimal_string)))
        break;
      instruction_scr_destructor();
      snprintf(display,
               sizeof(display),
               UI_TEXT_VERIFY_AMOUNT,
               amount_decimal_string,
               "");
      confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES_SOLANA: {
      char amount_string[40] = {'\0'}, amount_decimal_string[30] = {'\0'};
      char display[110] = {'\0'};
      byte_array_to_hex_string((const uint8_t *)var_send_transaction_data
                                   .transaction_metadata.transaction_fees,
                               8,
                               amount_string,
                               sizeof(amount_string));
      uint8_t decimal_val_s[32 * 3] = {0};

      if (sizeof(decimal_val_s) / sizeof(decimal_val_s[0]) > UINT8_MAX) {
        LOG_ERROR("0xxx#");
        break;
      }

      if (!convert_byte_array_to_decimal_string(16,
                                                solana_get_decimal(),
                                                amount_string,
                                                amount_decimal_string,
                                                sizeof(amount_decimal_string)))
        break;
      instruction_scr_destructor();
      snprintf(display,
               sizeof(display),
               UI_TEXT_VERIFY_AMOUNT,
               amount_decimal_string,
               "");
      confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_ENTER_PASSPHRASE_SOLANA: {
      if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three =
            SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_SOLANA;
        break;
      }
      input_text_init(PASSPHRASE,
                      26,
                      ui_text_enter_passphrase,
                      0,
                      DATA_TYPE_PASSPHRASE,
                      64);

    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE_SOLANA: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_passphrase, display, false);
      memzero(display, sizeof(display));
    } break;

    case SEND_TXN_CHECK_PIN_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_ENTER_PIN_SOLANA: {
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = SEND_TXN_CHECK_PIN_SOLANA;
        break;
      }
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);

    } break;

    case SEND_TXN_TAP_CARD_SOLANA: {
      tap_threshold_cards_for_reconstruction();
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD_SOLANA: {
      instruction_scr_init(ui_text_processing, NULL);
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case SEND_TXN_UPDATE_BLOCKHASH_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_READ_DEVICE_SHARE_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_SIGN_TXN_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_WAITING_SCREEN_SOLANA: {
      mark_event_over();
    } break;

    case SEND_TXN_FINAL_SCREEN_SOLANA:
      delay_scr_init(ui_text_exported_signed_transaction_to_desktop,
                     DELAY_TIME);
      CY_Reset_Not_Allow(true);
      break;

    default:
      break;
  }
}
