/**
 * @file    tasks_restore_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Restore wallet task.
 *          This file contains the implementation of the restore wallet task.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include "controller_main.h"
#include "shamir_wrapper.h"
#include "stdint.h"
#include "tasks.h"
#include "tasks_add_wallet.h"
#include "wallet.h"
#include "tasks_tap_cards.h"
#include "ui_screens.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;

extern Wallet_credential_data wallet_credential_data;

static void restore_wallet_enter_mnemonics_flow()
{
    if (flow_level.level_four <= wallet.number_of_mnemonics) {
        char heading[20];
        snprintf(heading, sizeof(heading), UI_TEXT_ENTER_WORD, flow_level.level_four);
        ui_mnem_init(heading);
    } else {
        // todo check if mnemonics is correct
        mark_event_over();
    }
}

void tasks_restore_wallet()
{
    if (flow_level.show_error_screen) {
        message_scr_init(flow_level.error_screen_text);
        return;
    }

    switch (flow_level.level_three) {
    case RESTORE_WALLET_NAME_INPUT: {
        input_text_init(
            ALPHABET,
            ui_text_enter_wallet_name,
            2,
            DATA_TYPE_TEXT,
            15);
    } break;

    case RESTORE_WALLET_NAME_CONFIRM: {
        char display[65];
        snprintf(display, sizeof(display), "%s", flow_level.screen_input.input_text);
        ui_scrollable_page(ui_text_confirm_wallet_name, display, MENU_SCROLL_HORIZONTAL, false);
        // address_scr_init(ui_text_confirm_wallet_name, display, false);
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_1: {
      char display[65];
      if(strnlen(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(display, sizeof(display), UI_TEXT_PIN_INS1, wallet.wallet_name);
      else
        snprintf(display, sizeof(display), UI_TEXT_PIN_INS1, "this wallet");
      delay_scr_init(display, DELAY_TIME);
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_2: {
      
      delay_scr_init(ui_wallet_pin_instruction_2,DELAY_TIME);
    
    } break;

    case RESTORE_WALLET_SKIP_PASSWORD: {
        ui_scrollabe_page(ui_heading_confirm_action, ui_text_do_you_want_to_set_pin);
        // confirm_scr_init(ui_text_do_you_want_to_set_pin);
        // confirm_scr_focus_cancel();
    } break;

    case RESTORE_WALLET_PIN_INPUT: {
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);
    } break;

    case RESTORE_WALLET_PIN_CONFIRM: {
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_confirm_pin,
            4,
            DATA_TYPE_PIN,
            8);
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_1: {
      char display[65];
      if(strnlen(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(display, sizeof(display), UI_TEXT_PASSPHRASE_INS1, wallet.wallet_name);
      else
        snprintf(display, sizeof(display), UI_TEXT_PASSPHRASE_INS1, "this wallet");
      delay_scr_init(display, DELAY_TIME);
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_2: {
      delay_scr_init(ui_wallet_passphrase_instruction_2, DELAY_TIME);
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_3: {
      delay_scr_init(ui_wallet_passphrase_instruction_3, DELAY_TIME);
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_4: {
      delay_scr_init(ui_wallet_passphrase_instruction_4, DELAY_TIME);
    } break;

    case RESTORE_WALLET_SKIP_PASSPHRASE: {
        ui_scrollabe_page(ui_heading_confirm_action, ui_text_use_passphrase_question);
        // confirm_scr_init(ui_text_use_passphrase_question);
        // confirm_scr_focus_cancel();
    } break;

    case RESTORE_WALLET_NUMBER_OF_WORDS_INPUT: {
        menu_init(
            ui_text_mnemonics_number_options,
            3,
            ui_text_number_of_words,
            true);
    } break;

    case RESTORE_WALLET_ENTER_SEED_PHRASE_INSTRUCTION: {
        message_scr_init(ui_text_now_enter_your_seed_phrase);

    } break;

    case RESTORE_WALLET_ENTER_MNEMONICS: {
#ifndef SKIP_ENTER_MNEMONICS_DEBUG
        restore_wallet_enter_mnemonics_flow();
#else
        mark_event_over();
#endif

    } break;

    case RESTORE_WALLET_CREATING_WAIT_SCREEN: {
        instruction_scr_init(ui_text_processing, NULL);
        mark_event_over();
    } break;

    case RESTORE_WALLET_CREATE: {
        instruction_scr_destructor();
        mark_event_over();
    } break;

    case RESTORE_WALLET_VERIFY_MNEMONICS_INSTRUCTION: {
        delay_scr_init(ui_text_verify_entered_words, DELAY_TIME);

    } break;

    case RESTORE_WALLET_VERIFY: {
        set_theme(LIGHT);
        list_init(
            wallet_credential_data.mnemonics,
            wallet.number_of_mnemonics,
            ui_text_verify_word_hash,
            true);
        reset_theme();
    } break;

    case RESTORE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE:
        mark_event_over();
        break;

    case RESTORE_WALLET_TAP_CARDS: {
        tap_cards_for_write_flow();
    } break;

    case RESTORE_WALLET_VERIFY_SHARES:
        instruction_scr_init(ui_text_processing, "");
        instruction_scr_change_text(ui_text_processing, true);
        BSP_DelayMs(DELAY_SHORT);
        mark_event_over();
        break;

    case RESTORE_WALLET_SUCCESS_MESSAGE: {
        instruction_scr_destructor();
        const char *messages[6] = {
            ui_text_verification_is_now_complete_messages[0], ui_text_verification_is_now_complete_messages[1],
            ui_text_verification_is_now_complete_messages[2], ui_text_verification_is_now_complete_messages[4],
            ui_text_verification_is_now_complete_messages[5], NULL};
        uint8_t count = 5;

        if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
            messages[3] = ui_text_verification_is_now_complete_messages[3];
            messages[4] = ui_text_verification_is_now_complete_messages[4];
            messages[5] = ui_text_verification_is_now_complete_messages[5];
            count = 6;
        }

        multi_instruction_init(messages, count, DELAY_LONG_STRING, true);
    } break;

    case RESTORE_WALLET_FAILED_MESSAGE: {
        instruction_scr_destructor();
        message_scr_init(ui_text_creation_failed_delete_wallet);
    } break;

    default: {
        message_scr_init(ui_text_something_went_wrong);
    } break;
    }
    return;
}
