/**
 * @file    receive_transaction_controller_eth.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction next controller for ETH.
 *          Handles post event (only next events) operations for receive transaction flow initiated by desktop app.
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
#include "communication.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "atecc_utils.h"

extern Receive_Transaction_Data receive_transaction_data;
extern Receive_Transaction_Auth_Data receive_transaction_auth_data;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;
extern Signed_Receive_Address signed_receive_address;

auth_data_t atecc_sign_eth(uint8_t *hash){
    uint8_t io_protection_key[32] = {0};
    uint8_t nonce[32] = {0};
    auth_data_t auth_challenge_packet = {0};
    uint8_t tempkey_hash[DEVICE_SERIAL_SIZE+POSTFIX2_SIZE] = {0};
    uint8_t final_hash[32] = {0};
    get_io_protection_key(io_protection_key);
    atca_sign_internal_in_out_t sign_internal_param = {0};

    atecc_data.retries = DEFAULT_ATECC_RETRIES;
    bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    do{
        OTG_FS_IRQHandler();

        if (atecc_data.status != ATCA_SUCCESS)
            LOG_CRITICAL("AERR CH: %04x, count:%d", atecc_data.status, DEFAULT_ATECC_RETRIES - atecc_data.retries);

        if ((atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface)) != ATCA_SUCCESS) {
            continue;
        }

        atecc_data.status = atcab_write_enc(slot_5_challenge, 0, hash, io_protection_key, slot_6_io_key);
        if(atecc_data.status!=ATCA_SUCCESS){
            continue;
        }

        atecc_data.status = atcab_nonce(nonce);
        if(atecc_data.status != ATCA_SUCCESS){
            continue;
        }

        atecc_data.status = atcab_gendig(ATCA_ZONE_DATA, slot_5_challenge, NULL, 0);
        if(atecc_data.status != ATCA_SUCCESS){
            continue;
        }

        atecc_data.status = atcab_sign_internal(slot_2_auth_key, false, false, auth_challenge_packet.signature);
        if(atecc_data.status != ATCA_SUCCESS){
            continue;
        }

        helper_get_gendig_hash(slot_5_challenge, hash, tempkey_hash, auth_challenge_packet.postfix1, atecc_data);

        sign_internal_param.message=tempkey_hash;
        sign_internal_param.digest=final_hash;

        helper_sign_internal_msg(&sign_internal_param, SIGN_MODE_INTERNAL, slot_2_auth_key, slot_5_challenge, atecc_data);

    }while(--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

    return auth_challenge_packet;
}

void receive_transaction_controller_eth()
{

    switch (flow_level.level_three) {

    case RECV_TXN_FIND_XPUB_ETH: {
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = RECV_TXN_ENTER_PASSPHRASE_ETH;
        } else {
            flow_level.level_three = RECV_TXN_CHECK_PIN_ETH;
        }
    } break;

    case RECV_TXN_ENTER_PASSPHRASE_ETH: {
        flow_level.level_three = RECV_TXN_CONFIRM_PASSPHRASE_ETH;
    } break;

    case RECV_TXN_CONFIRM_PASSPHRASE_ETH: {
        snprintf(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase), "%s", flow_level.screen_input.input_text);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = RECV_TXN_CHECK_PIN_ETH;
        flow_level.level_one = 1;
    } break;

    case RECV_TXN_CHECK_PIN_ETH:{
        if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = RECV_TXN_ENTER_PIN_ETH;
        } else {
            flow_level.level_three = RECV_TXN_TAP_CARD_ETH;
        }
    }break;

    case RECV_TXN_ENTER_PIN_ETH: {
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = RECV_TXN_TAP_CARD_ETH;
    } break;

    case RECV_TXN_TAP_CARD_ETH: {
        tap_card_data.desktop_control = true;
        tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD_ETH: {
        flow_level.level_three = RECV_TXN_READ_DEVICE_SHARE_ETH;
    } break;


    case RECV_TXN_READ_DEVICE_SHARE_ETH:
        wallet_shamir_data.share_x_coords[1] = 5;
        get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[1]);
        memcpy(wallet_shamir_data.share_encryption_data[1], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
        flow_level.level_three = RECV_TXN_DERIVE_ADD_SCREEN_ETH;
        break;

    case RECV_TXN_DERIVE_ADD_SCREEN_ETH: {

        flow_level.level_three = RECV_TXN_DERIVE_ADD_ETH;
    } break;

    case RECV_TXN_DERIVE_ADD_ETH: {
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
        HDNode node;
        uint8_t seed[64];

        memzero(seed, sizeof(seed));
        mnemonic_to_seed(mnemo, wallet_credential_data.passphrase, seed, NULL);
        mnemonic_clear();
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        hdnode_from_seed(seed, sizeof(seed), SECP256K1_NAME, &node);

        hdnode_private_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose));
        hdnode_private_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index));
        hdnode_private_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.account_index));

        uint32_t version;
        uint8_t address_version;
        get_version(BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose), BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index), &address_version, &version);

        hdnode_fill_public_key(&node);

        hdnode_public_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.chain_index));
        hdnode_fill_public_key(&node);
        hdnode_public_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.address_index));
        hdnode_fill_public_key(&node);
        hdnode_get_ethereum_pubkeyhash(&node, receive_transaction_data.eth_pubkeyhash);

       
        flow_level.level_three = RECV_TXN_DISPLAY_ADDR_ETH;
    } break;

    case RECV_TXN_DISPLAY_ADDR_ETH: {
        if(receive_transaction_auth_data.is_auth) {
            uint8_t payload[20 + SESSION_ID_SIZE];
            uint8_t payload_size = 0;
            memcpy(payload, receive_transaction_data.address, sizeof(receive_transaction_data.address));
            memcpy(payload + payload_size, receive_transaction_auth_data.session_id, sizeof(receive_transaction_auth_data.session_id));
            payload_size += sizeof(receive_transaction_auth_data.session_id);

            uint8_t hash[32];
            sha256_Raw(payload, payload_size, hash);

            auth_data_t auth_challenge_packet = atecc_sign_eth(hash);
                
            memcpy(signed_receive_address.signature, auth_challenge_packet.signature, sizeof(auth_challenge_packet.signature));
            memcpy(signed_receive_address.address, receive_transaction_data.eth_pubkeyhash, sizeof(receive_transaction_data.eth_pubkeyhash));
            signed_receive_address.addr_size = sizeof(receive_transaction_data.eth_pubkeyhash);

            uint8_t data[SESSION_ID_SIZE + DEVICE_ID_SIZE + sizeof(signed_receive_address.signature) + sizeof(signed_receive_address.addr_max_size) + sizeof(signed_receive_address.addr_max_size) + signed_receive_address.addr_max_size];
            uint64_t size = signed_recv_addr_to_byte_array(&signed_receive_address, data);
            transmit_data_to_app(RECV_TXN_SEND_SIGNED, data, size);
        } else {
            uint8_t data[1 + sizeof(receive_transaction_data.eth_pubkeyhash)];
            data[0] = 1;  // confirmation byte
            memcpy(data + 1, receive_transaction_data.eth_pubkeyhash, sizeof(receive_transaction_data.eth_pubkeyhash));
            transmit_data_to_app(RECV_TXN_USER_VERIFIED_ADDRESS, data, sizeof(data));
        } 
        reset_flow_level();
    } break;
    default:
        break;
    }

    return;

}
