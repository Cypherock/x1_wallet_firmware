/**
 * @file    receive_transaction_controller.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction next controller (for BTC).
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
#include "sha2.h"
#include "controller_tap_cards.h"
#include "shamir_wrapper.h"

#include "stm32l4xx_it.h"
#include "atca_host.h"

#define SIGNATURE_SIZE          64
#define POSTFIX1_SIZE           7
#define POSTFIX2_SIZE           23
#define AUTH_DATA_SERIAL_SIGN_MSG_SIZE      (POSTFIX1_SIZE + POSTFIX2_SIZE + SIGNATURE_SIZE + DEVICE_SERIAL_SIZE)
#define AUTH_DATA_CHALLENGE_SIGN_MSG_SIZE   (POSTFIX1_SIZE + POSTFIX2_SIZE + SIGNATURE_SIZE)

typedef struct auth_data_struct{
    uint8_t postfix1[POSTFIX1_SIZE],
            postfix2[POSTFIX2_SIZE],
            signature[SIGNATURE_SIZE],
            serial[DEVICE_SERIAL_SIZE];
} auth_data_t;

static ATCA_STATUS helper_config_to_sign_internal(ATCADeviceType device_type, struct atca_sign_internal_in_out *param, const uint8_t* config);
static ATCA_STATUS helper_sign_internal_msg(struct atca_sign_internal_in_out *param, uint8_t mode, uint8_t priv_key_id, uint8_t data_key_id);
static void helper_get_gendig_hash(atecc_slot_define_t slot, uint8_t *data, uint8_t *digest, uint8_t *postfix);



extern Wallet_credential_data wallet_credential_data;

Receive_Transaction_Data receive_transaction_data;
Receive_Transaction_Auth_Data receive_transaction_auth_data;
Signed_Receive_Address signed_receive_address;
uint8_t segwit;

void receive_transaction_controller()
{
    switch (flow_level.level_three) {

    case RECV_TXN_FIND_XPUB: {
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        if (BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose) == NATIVE_SEGWIT)
            segwit = 1;
        else
            segwit = 0;
        if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = RECV_TXN_ENTER_PASSPHRASE;
        } else {
            flow_level.level_three = RECV_TXN_CHECK_PIN;
        }
    } break;

    case RECV_TXN_ENTER_PASSPHRASE: {
        flow_level.level_three = RECV_TXN_CONFIRM_PASSPHRASE;
    } break;

    case RECV_TXN_CONFIRM_PASSPHRASE: {
        snprintf(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase), "%s", flow_level.screen_input.input_text);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = RECV_TXN_CHECK_PIN;
        flow_level.level_one = 1;
    } break;

    case RECV_TXN_CHECK_PIN:{
        if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = RECV_TXN_ENTER_PIN;
        } else {
            flow_level.level_three = RECV_TXN_TAP_CARD;
        }
    }break;

    case RECV_TXN_ENTER_PIN: {
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
      
        flow_level.level_three = RECV_TXN_TAP_CARD;
    } break;

    case RECV_TXN_TAP_CARD: {
        tap_card_data.desktop_control = true;
        tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD: {
        flow_level.level_three = RECV_TXN_READ_DEVICE_SHARE;
    } break;

    case RECV_TXN_READ_DEVICE_SHARE:
        wallet_shamir_data.share_x_coords[1] = 5;
        get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[1]);
        memcpy(wallet_shamir_data.share_encryption_data[1], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
        flow_level.level_three = RECV_TXN_DERIVE_ADD_SCREEN;
        break;

    case RECV_TXN_DERIVE_ADD_SCREEN: {

        flow_level.level_three = RECV_TXN_DERIVE_ADD;
    } break;

    case RECV_TXN_DERIVE_ADD: {
        uint8_t secret[BLOCK_SIZE];
        if (WALLET_IS_PIN_SET(wallet.wallet_info))
            decrypt_shares();
        recover_secret_from_shares(
            BLOCK_SIZE,
            MINIMUM_NO_OF_SHARES,
            wallet_shamir_data.mnemonic_shares,
            wallet_shamir_data.share_x_coords,
            secret);
        memzero(wallet_shamir_data.share_encryption_data, sizeof(wallet_shamir_data.share_encryption_data));
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
        get_version(BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose),
                    BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index),
                    &address_version, &version);

        hdnode_fill_public_key(&node);

        hdnode_public_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.chain_index));
        hdnode_fill_public_key(&node);
        hdnode_public_ckd(&node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.address_index));
        hdnode_fill_public_key(&node);

        memzero(receive_transaction_data.address, sizeof(receive_transaction_data.address));
        if (segwit)
            get_segwit_address(node.public_key, sizeof(node.public_key),
                               BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index),
                               receive_transaction_data.address);
        else
            hdnode_get_address(&node, address_version, receive_transaction_data.address, 35);

        flow_level.level_three = RECV_TXN_DISPLAY_ADDR;
    } break;

    case RECV_TXN_DISPLAY_ADDR: {
        if(receive_transaction_auth_data.is_auth) {
            uint8_t payload[43 + SESSION_ID_SIZE];
            uint8_t payload_size = 0;
            memcpy(payload, receive_transaction_data.address, sizeof(receive_transaction_data.address));
            memcpy(payload + payload_size, receive_transaction_auth_data.session_id, sizeof(receive_transaction_auth_data.session_id));
            payload_size += sizeof(receive_transaction_auth_data.session_id);

            uint8_t hash[32];
            sha256_Raw(payload, payload_size, hash);

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

                helper_get_gendig_hash(slot_5_challenge, hash, tempkey_hash, auth_challenge_packet.postfix1);

                sign_internal_param.message=tempkey_hash;
                sign_internal_param.digest=final_hash;

                helper_sign_internal_msg(&sign_internal_param, SIGN_MODE_INTERNAL, slot_2_auth_key, slot_5_challenge);

                memset(hash, 0, sizeof(hash));
                atecc_data.status = atcab_write_enc(slot_5_challenge, 0, hash, io_protection_key, slot_6_io_key);
                if(atecc_data.status!=ATCA_SUCCESS){
                    continue;
                }

            }while(--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
            if(usb_irq_enable_on_entry == true)
                NVIC_EnableIRQ(OTG_FS_IRQn);
            
            
            memcpy(signed_receive_address.signature, auth_challenge_packet.signature, sizeof(auth_challenge_packet.signature));
            memcpy(signed_receive_address.address, receive_transaction_data.address, sizeof(receive_transaction_data.address));
            signed_receive_address.addr_size = sizeof(receive_transaction_data.address);

            uint8_t data[SESSION_ID_SIZE + DEVICE_ID_SIZE + sizeof(signed_receive_address.signature) + sizeof(signed_receive_address.addr_max_size) + sizeof(signed_receive_address.addr_max_size) + signed_receive_address.addr_max_size];
            uint64_t size = signed_recv_addr_to_byte_array(&signed_receive_address, data);
            transmit_data_to_app(RECV_TXN_SEND_SIGNED, data, size);
        } else {
            uint8_t data[1 + sizeof(receive_transaction_data.address)];
            data[0] = 1;  // confirmation byte
            memcpy(data + 1, receive_transaction_data.address, sizeof(receive_transaction_data.address));
            transmit_data_to_app(RECV_TXN_USER_VERIFIED_ADDRESS, data, strlen((const char *)data));
        }
        reset_flow_level();
    } break;

    default:
        break;
    }

    return;
}


void helper_get_gendig_hash(atecc_slot_define_t slot, uint8_t *data, uint8_t *digest, uint8_t *postfix)
{
    if(digest == NULL || data == NULL || postfix == NULL){
        return;
    }

    uint8_t tempkey_init[96] = {0};
    uint8_t atecc_serial[9];
    atecc_data.status = atcab_read_serial_number(atecc_serial);
    memcpy(tempkey_init, data, 32);
    postfix[0] = tempkey_init[32] = 0x15;
    postfix[1] = tempkey_init[33] = 0x02;
    postfix[2] = tempkey_init[34] = slot;
    postfix[3] = tempkey_init[35] = 0x00;
    postfix[4] = tempkey_init[36] = atecc_serial[8];
    postfix[5] = tempkey_init[37] = atecc_serial[0];
    postfix[6] = tempkey_init[38] = atecc_serial[1];

    sha256_Raw(tempkey_init, 96, digest);
}

ATCA_STATUS helper_sign_internal_msg(struct atca_sign_internal_in_out *param, uint8_t mode, uint8_t priv_key_id, uint8_t data_key_id)
{
    uint8_t msg[55];
    uint8_t cfg[128]={0}, sn[9]={0};
    atca_temp_key_t temp_key={0};
    param->key_id=priv_key_id;
    temp_key.key_id=data_key_id;
    temp_key.gen_dig_data=1;
    temp_key.valid=1;
    temp_key.source_flag=1;

    atecc_data.status = atcab_read_config_zone(cfg);
    memcpy(temp_key.value, param->message, 32);
    param->temp_key=&temp_key;
    helper_config_to_sign_internal(ATECC608A, param, cfg);
    atecc_data.status = atcab_read_serial_number(sn);

    if (param == NULL || param->temp_key == NULL)
    {
        return ATCA_BAD_PARAM;
    }

    memset(msg, 0, sizeof(msg));
    memcpy(&msg[0], param->temp_key->value, 32);
    msg[32] = ATCA_SIGN;                          // Sign OpCode
    msg[33] = param->mode;                        // Sign Mode
    msg[34] = (uint8_t)(param->key_id >> 0);      // Sign KeyID
    msg[35] = (uint8_t)(param->key_id >> 8);
    msg[36] = (uint8_t)(param->slot_config >> 0); // SlotConfig[TempKeyFlags.keyId]
    msg[37] = (uint8_t)(param->slot_config >> 8);
    msg[38] = (uint8_t)(param->key_config >> 0);  // KeyConfig[TempKeyFlags.keyId]
    msg[39] = (uint8_t)(param->key_config >> 8);

    //TempKeyFlags (b0-3: keyId, b4: sourceFlag, b5: GenDigData, b6: GenKeyData, b7: NoMacFlag)
    msg[40] |= ((param->temp_key->key_id & 0x0F) << 0);
    msg[40] |= ((param->temp_key->source_flag & 0x01) << 4);
    msg[40] |= ((param->temp_key->gen_dig_data & 0x01) << 5);
    msg[40] |= ((param->temp_key->gen_key_data & 0x01) << 6);
    msg[40] |= ((param->temp_key->no_mac_flag & 0x01) << 7);
    msg[41] = 0x00;
    msg[42] = 0x00;

    // Serial Number
    msg[43] = sn[8];
    memcpy(&msg[48], &sn[0], 2);
    if (param->mode & SIGN_MODE_INCLUDE_SN)
    {
        memcpy(&msg[44], &sn[4], 4);
        memcpy(&msg[50], &sn[2], 2);
    }

    // The bit within the SlotLocked field corresponding to the last key used in the TempKey computation is in the LSB
    msg[52] = param->is_slot_locked ? 0x00 : 0x01;

    // If the slot contains a public key corresponding to a supported curve, and if PubInfo indicates this key must be
    // validated before being used by Verify, and if the validity bits have a value of 0x05, then the PubKey Valid byte
    // will be 0x01.In all other cases, it will be 0.
    msg[53] = param->for_invalidate ? 0x01 : 0x00;

    msg[54] = 0x00;

    if (param->message)
    {
        memcpy(param->message, msg, sizeof(msg));
    }
    if (param->verify_other_data)
    {
        memcpy(&param->verify_other_data[0],  &msg[33], 10);
        memcpy(&param->verify_other_data[10], &msg[44], 4);
        memcpy(&param->verify_other_data[14], &msg[50], 5);
    }
    if (param->digest)
    {
        return atcac_sw_sha2_256(msg, sizeof(msg), param->digest);
    }
    else
    {
        return ATCA_SUCCESS;
    }
}

ATCA_STATUS helper_config_to_sign_internal(ATCADeviceType device_type, struct atca_sign_internal_in_out *param, const uint8_t* config)
{
    const uint8_t* value = NULL;
    uint16_t slot_locked = 0;

    if (param == NULL || config == NULL || param->temp_key == NULL)
    {
        return ATCA_BAD_PARAM;
    }

    // SlotConfig[TempKeyFlags.keyId]
    value = &config[20 + param->temp_key->key_id * 2];
    param->slot_config = (uint16_t)value[0] | ((uint16_t)value[1] << 8);

    // KeyConfig[TempKeyFlags.keyId]
    value = &config[96 + param->temp_key->key_id * 2];
    param->key_config = (uint16_t)value[0] | ((uint16_t)value[1] << 8);

    if (device_type == ATECC108A && param->temp_key->key_id < 8)
    {
        value = &config[52 + param->temp_key->key_id * 2];
        param->use_flag = value[0];
        param->update_count = value[0];
    }
    else
    {
        param->use_flag = 0x00;
        param->update_count = 0x00;
    }

    //SlotLocked:TempKeyFlags.keyId
    slot_locked = (uint16_t)config[88] | ((uint16_t)config[89] << 8);
    param->is_slot_locked = (slot_locked & (1 << param->temp_key->key_id)) ? false : true;

    return ATCA_SUCCESS;
}