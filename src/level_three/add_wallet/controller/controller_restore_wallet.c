/**
 * @file    controller_restore_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Restore wallet next controller.
 *          Handles post event (only next events) operations for restore wallet flow.
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
#include "bip39.h"
#include "bip39_english.h"
#include "constant_texts.h"
#include "controller_add_wallet.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "ui_message.h"
#include "wallet_utilities.h"
#include "flash_if.h"
#include "card_action_controllers.h"


extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;

extern Flash_Wallet wallet_for_flash;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

#ifndef SKIP_ENTER_MNEMONICS_DEBUG
static void restore_wallet_enter_mnemonics_flow_controller()
{
    if (flow_level.level_four <= wallet.number_of_mnemonics) {
        snprintf(wallet_credential_data.mnemonics[flow_level.level_four - 1], sizeof(wallet_credential_data.mnemonics[flow_level.level_four - 1]), "%s", wordlist[flow_level.screen_input.list_choice]);
        flow_level.level_four++;
    } else {
        flow_level.level_three = RESTORE_WALLET_VERIFY_MNEMONICS_INSTRUCTION;
    }
}
#endif

void restore_wallet_controller()
{
    switch (flow_level.level_three) {
    case RESTORE_WALLET_NAME_INPUT: {
        flow_level.level_three = RESTORE_WALLET_NAME_CONFIRM;
    } break;
    case RESTORE_WALLET_NAME_CONFIRM: {
        uint8_t dummy;
        uint8_t status = get_index_by_name(flow_level.screen_input.input_text, &dummy);
        if (status == SUCCESS_) {
            LOG_ERROR("0xx# wallet %d %d %d %s",
                get_wallet_info(dummy),
                get_wallet_card_state(dummy),
                get_wallet_locked_status(dummy),
                get_wallet_name(dummy));
            mark_error_screen(ui_text_wallet_name_exists);
            flow_level.level_three = RESTORE_WALLET_NAME_INPUT;
        } else if (status == INVALID_ARGUMENT){ 
            mark_error_screen(ui_text_wallet_name_size_limit);
            flow_level.level_three = RESTORE_WALLET_NAME_INPUT;
        } else {
            CY_Reset_Not_Allow(true);
            memzero((void*)&wallet_for_flash, sizeof(wallet_for_flash));
            memzero((void*)&wallet, sizeof(wallet));
            set_wallet_init();
            flow_level.level_three = RESTORE_WALLET_PIN_INSTRUCTIONS_1;
            snprintf((char *) wallet_for_flash.wallet_name, sizeof(wallet_for_flash.wallet_name), "%s", flow_level.screen_input.input_text);
            snprintf((char *) wallet.wallet_name, sizeof(wallet.wallet_name), "%s", flow_level.screen_input.input_text);
        }
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_1: {
      
        flow_level.level_three = RESTORE_WALLET_PIN_INSTRUCTIONS_2;

    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_2: {
      
        flow_level.level_three = RESTORE_WALLET_SKIP_PASSWORD;
    
    } break;

    case RESTORE_WALLET_SKIP_PASSWORD: {
        flow_level.level_three = RESTORE_WALLET_PIN_INPUT;
        WALLET_SET_PIN(wallet_for_flash.wallet_info);
        WALLET_SET_PIN(wallet.wallet_info);
    } break;

    case RESTORE_WALLET_PIN_INPUT: {
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strnlen(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text)), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = RESTORE_WALLET_PIN_CONFIRM;
    } break;

    case RESTORE_WALLET_PIN_CONFIRM: {
        uint8_t CONFIDENTIAL temp[SHA256_DIGEST_LENGTH] = {0};
        ASSERT(temp != NULL);
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strnlen(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text)), temp);
        sha256_Raw(temp, SHA256_DIGEST_LENGTH, temp);
        if (memcmp(wallet.password_double_hash, temp, SHA256_DIGEST_LENGTH) == 0) {
            if(is_passphrase_enabled())
                flow_level.level_three = RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_1;
            else
                flow_level.level_three = RESTORE_WALLET_NUMBER_OF_WORDS_INPUT;
        } else {
            mark_error_screen(ui_text_pin_incorrect_re_enter);
            flow_level.level_three = RESTORE_WALLET_PIN_INPUT;
        }
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        memzero(temp, sizeof(temp));
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_1: {
        flow_level.level_three = RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_2;
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_2: {
        flow_level.level_three = RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_3;
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_3: {
        flow_level.level_three = RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_4;
    } break;

    case RESTORE_WALLET_PASSPHRASE_INSTRUCTIONS_4: {
        flow_level.level_three = RESTORE_WALLET_SKIP_PASSPHRASE;
    } break;

    case RESTORE_WALLET_SKIP_PASSPHRASE: {
        flow_level.level_three = RESTORE_WALLET_NUMBER_OF_WORDS_INPUT;
        WALLET_SET_PASSPHRASE(wallet_for_flash.wallet_info);
        WALLET_SET_PASSPHRASE(wallet.wallet_info);
    } break;


    case RESTORE_WALLET_NUMBER_OF_WORDS_INPUT: {
        flow_level.level_three = RESTORE_WALLET_ENTER_SEED_PHRASE_INSTRUCTION;
        if (flow_level.screen_input.list_choice == 1) {
            wallet.number_of_mnemonics = 12;
        } else if (flow_level.screen_input.list_choice == 2) {
            wallet.number_of_mnemonics
                = 18;
        } else if (flow_level.screen_input.list_choice == 3) {
            wallet.number_of_mnemonics
                = 24;
        } else
            ; //shouldn't come here
    } break;

    case RESTORE_WALLET_ENTER_SEED_PHRASE_INSTRUCTION: {
        flow_level.level_three = RESTORE_WALLET_ENTER_MNEMONICS;

    } break;

    case RESTORE_WALLET_ENTER_MNEMONICS: {
#ifndef SKIP_ENTER_MNEMONICS_DEBUG
        restore_wallet_enter_mnemonics_flow_controller();
#else
        __single_to_multi_line("--ENTER-MNEMONIC-HERE-FOR-TESTING--", 36, wallet_credential_data.mnemonics);
        flow_level.level_three = RESTORE_WALLET_VERIFY_MNEMONICS_INSTRUCTION;
#endif
    } break;

    case RESTORE_WALLET_CREATING_WAIT_SCREEN: {
        flow_level.level_three = RESTORE_WALLET_CREATE;
    } break;

    case RESTORE_WALLET_CREATE: {
        char single_line_mnemonics[MAX_NUMBER_OF_MNEMONIC_WORDS * MAX_MNEMONIC_WORD_LENGTH];

#ifdef SKIP_ENTER_MNEMONICS_DEBUG
        snprintf(single_line_mnemonics, 36, "--ENTER-MNEMONIC-HERE-FOR-TESTING--");
        __single_to_multi_line(single_line_mnemonics, 160, wallet_credential_data.mnemonics);
#endif

        uint8_t wallet_index, temp_wallet_id[WALLET_ID_SIZE];

        __multi_to_single_line(wallet_credential_data.mnemonics, wallet.number_of_mnemonics, single_line_mnemonics);
        calculate_wallet_id(temp_wallet_id, single_line_mnemonics);
        mnemonic_clear();
        int result = mnemonic_check(single_line_mnemonics);
        mnemonic_clear();
        if(!result) {
            mark_error_screen(ui_text_incorrect_mnemonics);
            flow_level.level_three = RESTORE_WALLET_NUMBER_OF_WORDS_INPUT;
            flow_level.level_four = 1;
            flow_level.level_five = 1;
        }
        else if ((get_first_matching_index_by_id(temp_wallet_id, &wallet_index) == DOESNT_EXIST) && result) {
            memcpy(wallet_for_flash.wallet_id, temp_wallet_id, WALLET_ID_SIZE);
            memcpy(wallet.wallet_id, wallet_for_flash.wallet_id, WALLET_ID_SIZE);
            uint8_t secret[BLOCK_SIZE + 1];
            memzero(secret, BLOCK_SIZE + 1);
            mnemonic_clear();
            mnemonic_to_entropy(single_line_mnemonics, secret);
            mnemonic_clear();
            convert_to_shares(
                BLOCK_SIZE, secret,
                wallet.total_number_of_shares,
                wallet.minimum_number_of_shares,
                wallet_shamir_data.mnemonic_shares);
            memzero(secret, sizeof(secret));
            if (WALLET_IS_PIN_SET(wallet.wallet_info))
                encrypt_shares();
            derive_beneficiary_key(wallet.beneficiary_key, wallet.iv_for_beneficiary_key, single_line_mnemonics);
            derive_wallet_key(wallet.key, single_line_mnemonics);
            flow_level.level_three = RESTORE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE;
        } else {
            mark_error_screen(ui_text_wallet_with_same_mnemo_exists);
            flow_level.level_three = RESTORE_WALLET_NUMBER_OF_WORDS_INPUT;
            flow_level.level_four = 1;
            flow_level.level_five = 1;
        }
        memzero(single_line_mnemonics, sizeof(single_line_mnemonics));
        memzero(wallet_credential_data.mnemonics, sizeof(wallet_credential_data.mnemonics));
    } break;

    case RESTORE_WALLET_VERIFY_MNEMONICS_INSTRUCTION: {
        flow_level.level_three = RESTORE_WALLET_VERIFY;
    } break;

    case RESTORE_WALLET_VERIFY: {
        flow_level.level_three = RESTORE_WALLET_CREATING_WAIT_SCREEN;
    } break;

    case RESTORE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE: {
        uint32_t index;
        wallet_for_flash.state = DEFAULT_VALUE_IN_FLASH;
        add_wallet_share_to_sec_flash(&wallet_for_flash, &index, wallet_shamir_data.mnemonic_shares[4]);
        flow_level.level_three = RESTORE_WALLET_TAP_CARDS;
        flow_level.level_four = 1;
        flow_level.level_five = 1;
    } break;

    case RESTORE_WALLET_TAP_CARDS:
        tap_cards_for_write_and_verify_flow_controller();
        break;

    case RESTORE_WALLET_VERIFY_SHARES:
        flow_level.level_three = verify_card_share_data() == 1 ? RESTORE_WALLET_SUCCESS_MESSAGE : RESTORE_WALLET_FAILED_MESSAGE;
        memzero(wallet.password_double_hash, sizeof(wallet.password_double_hash));
        memzero(wallet_credential_data.password_single_hash, sizeof(wallet_credential_data.password_single_hash));
        memzero(wallet.wallet_share_with_mac_and_nonce, sizeof(wallet.wallet_share_with_mac_and_nonce));
        memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
        memzero(wallet.checksum, sizeof(wallet.checksum));
        memzero(wallet.key, sizeof(wallet.key));
        memzero(wallet.beneficiary_key, sizeof(wallet.beneficiary_key));
        memzero(wallet.iv_for_beneficiary_key, sizeof(wallet.iv_for_beneficiary_key));
        break;

    case RESTORE_WALLET_SUCCESS_MESSAGE:
        reset_flow_level();
        break;

    case RESTORE_WALLET_FAILED_MESSAGE:
        flow_level.level_one = LEVEL_TWO_OLD_WALLET;
        flow_level.level_two = LEVEL_THREE_DELETE_WALLET;
        flow_level.level_three = 1;
        flow_level.level_four = 1;
        break;

    default:
        message_scr_init(ui_text_something_went_wrong);
        reset_flow_level();
        break;
    }
    return;
}
