/**
 * @file    add_coin_controller.c
 * @author  Cypherock X1 Team
 * @brief   Add coin next controller.
 *          Handles post event (only next events) operations for add coin flow initiated by desktop app.
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
#include "bip32.h"
#include "bip39_english.h"
#include "bip39.h"
#include "btc.h"
#include "communication.h"
#include "controller_level_four.h"
#include "curves.h"
#include "board.h"
#include "sha2.h"
#include "controller_tap_cards.h"
#include "shamir_wrapper.h"
#include "ui_instruction.h"
#include "controller_main.h"

Add_Coin_Data add_coin_data;
Cmd_Add_Coin_t cmd_add_coin;


extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

void add_coin_controller()
{
    switch (flow_level.level_three) {

    case ADD_COINS_VERIFY: {
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = ADD_COINS_ENTER_PASSPHRASE;
        } else {
            flow_level.level_three = ADD_COINS_CHECK_PIN;
        }
    } break;

    case ADD_COINS_ENTER_PASSPHRASE: {
        flow_level.level_three = ADD_COINS_CONFIRM_PASSPHRASE;
    } break;

    case ADD_COINS_CONFIRM_PASSPHRASE: {
        snprintf(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase), "%s", flow_level.screen_input.input_text);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = ADD_COINS_CHECK_PIN;
        flow_level.level_one = 1;
    } break;

    case ADD_COINS_CHECK_PIN:{
        if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = ADD_COINS_ENTER_PIN;
        } else {
            flow_level.level_three = ADD_COINS_TAP_CARD;
        }
    }break;

    case ADD_COINS_ENTER_PIN: {
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = ADD_COINS_TAP_CARD;
    } break;

    case ADD_COINS_TAP_CARD: {
        tap_card_data.desktop_control = true;
        tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case ADD_COINS_TAP_CARD_SEND_CMD: {
        flow_level.level_three = ADD_COINS_READ_DEVICE_SHARE;
    } break;

    case ADD_COINS_READ_DEVICE_SHARE:
        wallet_shamir_data.share_x_coords[1] = 5;
        get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[1]);
        memcpy(wallet_shamir_data.share_encryption_data[1], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
        flow_level.level_three = ADD_COIN_GENERATING_XPUBS;
        break;

    case ADD_COIN_GENERATING_XPUBS: {
        uint8_t secret[BLOCK_SIZE];
        if (WALLET_IS_PIN_SET(wallet.wallet_info))
            decrypt_shares();
        recover_secret_from_shares(
            BLOCK_SIZE,
            MINIMUM_NO_OF_SHARES,
            wallet_shamir_data.mnemonic_shares,
            wallet_shamir_data.share_x_coords,
            secret);
        memzero(wallet_shamir_data.mnemonic_shares, sizeof(wallet_shamir_data.mnemonic_shares));
        mnemonic_clear();
        const char* mnemo = mnemonic_from_data(secret,wallet.number_of_mnemonics * 4 / 3);

        ASSERT(mnemo != NULL);
        uint8_t seed[64]={0};

        mnemonic_to_seed(mnemo, wallet_credential_data.passphrase, seed, NULL);
        mnemonic_clear();

        uint8_t i = 0x0, j = 0x0;
        for (; i < add_coin_data.number_of_coins; i++, j++) {
            if(add_coin_data.coin_indexes[i] == SOLANA_COIN_INDEX){ 
                uint32_t path[] = {
                    SOLANA_PURPOSE_INDEX, add_coin_data.coin_indexes[i],
                    SOLANA_ACCOUNT_INDEX};
                generate_xpub(path,3, ED25519_NAME, seed, (char *) cmd_add_coin.xpubs[j]);
                memzero(path, sizeof(path));
            }else if(add_coin_data.coin_indexes[i] == NEAR_COIN_INDEX){ 
                uint32_t path[] = {
                    NEAR_PURPOSE_INDEX, add_coin_data.coin_indexes[i],
                    NEAR_ACCOUNT_INDEX, NEAR_CHAIN_INDEX, NEAR_ADDRESS_INDEX};
                generate_xpub(path, 5, ED25519_NAME, seed, (char *) cmd_add_coin.xpubs[j]);
                memzero(path, sizeof(path));
            }else{
                uint32_t path[] = { NON_SEGWIT, add_coin_data.coin_indexes[i], 0x80000000 };
                generate_xpub(path, 3, SECP256K1_NAME, seed, (char *) cmd_add_coin.xpubs[j]);
                memzero(path, sizeof(path));
            }
            if (add_coin_data.coin_indexes[i] == 0x80000000 || add_coin_data.coin_indexes[i] == 0x80000001) {
                uint32_t path[] = { NATIVE_SEGWIT, add_coin_data.coin_indexes[i], 0x80000000 };
                generate_xpub(path, 3, SECP256K1_NAME, seed, (char *) cmd_add_coin.xpubs[++j]);
                memzero(path, sizeof(path));
            }
        }
        memzero(secret, sizeof(secret));
        memzero(seed, sizeof(seed));
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        uint8_t out_arr[sizeof(Cmd_Add_Coin_t)];
        memcpy(out_arr, cmd_add_coin.xpubs, XPUB_SIZE * j);
        transmit_data_to_app(ADD_COIN_SENDING_XPUBS, out_arr, XPUB_SIZE * j);
        flow_level.level_three = ADD_COINS_FINAL_SCREEN;
    } break;

    case ADD_COINS_FINAL_SCREEN:
        reset_flow_level();
        break;

    default:
        break;
    }
}
