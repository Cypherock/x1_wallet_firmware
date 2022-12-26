#include "controller_level_four.h"
#include "ui_instruction.h"

Swap_Transaction_Data
    swap_transaction_data;

void jump_to_swap() {
    counter.level = LEVEL_THREE;
    flow_level.level_two = LEVEL_THREE_SWAP_TRANSACTION;
    flow_level.level_three = 9; // SWAP_SIGN_RECEIVE_ADDRESS
    is_swap_txn = false;
}

static auth_data_t atecc_sign(uint8_t *hash) {
    uint8_t io_protection_key[32] = {0};
    uint8_t nonce[32] = {0};
    auth_data_t auth_challenge_packet = {0};
    uint8_t tempkey_hash[DEVICE_SERIAL_SIZE + POSTFIX2_SIZE] = {0};
    uint8_t final_hash[32] = {0};
    get_io_protection_key(io_protection_key);
    atca_sign_internal_in_out_t sign_internal_param = {0};

    atecc_data.retries = DEFAULT_ATECC_RETRIES;
    bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    do {
        OTG_FS_IRQHandler();

        if (atecc_data.status != ATCA_SUCCESS)
            LOG_CRITICAL("AERR CH: %04x, count:%d",
                         atecc_data.status,
                         DEFAULT_ATECC_RETRIES - atecc_data.retries);

        if ((atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface))
            != ATCA_SUCCESS) {
            continue;
        }

        atecc_data.status = atcab_write_enc(slot_5_challenge,
                                            0,
                                            hash,
                                            io_protection_key,
                                            slot_6_io_key);
        if (atecc_data.status != ATCA_SUCCESS) {
            continue;
        }

        atecc_data.status = atcab_nonce(nonce);
        if (atecc_data.status != ATCA_SUCCESS) {
            continue;
        }

        atecc_data.status =
            atcab_gendig(ATCA_ZONE_DATA, slot_5_challenge, NULL, 0);
        if (atecc_data.status != ATCA_SUCCESS) {
            continue;
        }

        atecc_data.status = atcab_sign_internal(slot_2_auth_key,
                                                false,
                                                false,
                                                auth_challenge_packet.signature);
        if (atecc_data.status != ATCA_SUCCESS) {
            continue;
        }

        helper_get_gendig_hash(slot_5_challenge,
                               hash,
                               tempkey_hash,
                               auth_challenge_packet.postfix1,
                               atecc_data);

        sign_internal_param.message = tempkey_hash;
        sign_internal_param.digest = final_hash;

        helper_sign_internal_msg(&sign_internal_param,
                                 SIGN_MODE_INTERNAL,
                                 slot_2_auth_key,
                                 slot_5_challenge,
                                 atecc_data);

    } while (--atecc_data.retries && atecc_data.status != ATCA_SUCCESS);
    if (usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

    return auth_challenge_packet;
}

static size_t append_signature(uint8_t *payload, uint8_t *address, size_t
address_length) {
    size_t payload_size = 0;

    memcpy(payload,
           address,
           address_length);
    payload_size += address_length;

    if (get_device_serial() == SUCCESS) {
        memcpy(payload + payload_size,
               atecc_data.device_serial,
               DEVICE_SERIAL_SIZE);
    } else {
        LOG_CRITICAL("err xx4: %d", atecc_data.status);
    }

    payload_size += DEVICE_SERIAL_SIZE;

    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_size, hash);

    auth_data_t auth_challenge_packet = atecc_sign(hash);

    memcpy(payload + payload_size,
           auth_challenge_packet.signature,
           sizeof(auth_challenge_packet.signature));
    payload_size += sizeof(auth_challenge_packet.signature);

    return payload_size;
}

void swap_transaction_controller() {
    switch (flow_level.level_three) {
        case SWAP_CONFIRM_SEND_COIN: {
            flow_level.level_three = SWAP_CONFIRM_RECEIVE_COIN;
        }
            break;

        case SWAP_CONFIRM_RECEIVE_COIN: {
            flow_level.level_three = SWAP_SELECT_RECEIVE_WALLET_ID;
        }
            break;

        case SWAP_SELECT_RECEIVE_WALLET_ID: {
            flow_level.level_three = SWAP_CONFIRM_SEND_AMOUNT;
        }
            break;
        case SWAP_CONFIRM_SEND_AMOUNT: {
            flow_level.level_three = SWAP_CONFIRM_RECEIVE_AMOUNT;
        }
            break;

        case SWAP_CONFIRM_RECEIVE_AMOUNT: {
            flow_level.level_three = SWAP_CONFIRM_NETWORK_FEES;
        }
            break;

        case SWAP_CONFIRM_NETWORK_FEES: {
            flow_level.level_three = SWAP_VERIFY_SESSION;
        }
            break;

        case SWAP_VERIFY_SESSION: {
            flow_level.level_three = SWAP_RECV_ADDR_DERIVATION;
        }
            break;

        case SWAP_RECV_ADDR_DERIVATION: {
            uint32_t coin_index = BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                                           .receive_txn_data.coin_index);
            counter.level = LEVEL_THREE;
            flow_level.level_three = 1;
            switch (coin_index) {
                case BITCOIN: {
                    flow_level.level_two = LEVEL_THREE_RECEIVE_TRANSACTION;
                }
                    break;

                case ETHEREUM: {
                    flow_level.level_two =
                        LEVEL_THREE_RECEIVE_TRANSACTION_ETH;
                }
                    break;

                case NEAR: {
                    flow_level.level_two =
                        LEVEL_THREE_RECEIVE_TRANSACTION_NEAR;
                }
                    break;

                case SOLANA: {
                    flow_level.level_two =
                        LEVEL_THREE_RECEIVE_TRANSACTION_SOLANA;
                }
                    break;

                default: break;
            }

        }
            break;

        case SWAP_SIGN_RECEIVE_ADDRESS: {
            uint32_t coin_index = BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                                           .receive_txn_data.coin_index);

            uint8_t recv_address[45] = {0};

            switch (coin_index) {
                case BITCOIN: {
                    memcpy(recv_address,
                           swap_transaction_data.receive_txn_data.address,
                           sizeof(swap_transaction_data.receive_txn_data.address));
                }
                    break;

                case ETHEREUM: {
                    memcpy(recv_address,
                           swap_transaction_data.receive_txn_data.eth_pubkeyhash,
                           sizeof(swap_transaction_data.receive_txn_data.eth_pubkeyhash));
                }
                    break;

                case NEAR: {
                    memcpy(recv_address,
                           swap_transaction_data.receive_txn_data.near_pubkey,
                           sizeof(swap_transaction_data.receive_txn_data.near_pubkey));
                }
                    break;

                case SOLANA: {
                    memcpy(recv_address,
                           swap_transaction_data.receive_txn_data.solana_address,
                           sizeof(swap_transaction_data.receive_txn_data.solana_address));
                }
                    break;

                default:break;
            }

            uint8_t payload[sizeof(recv_address) + DEVICE_SERIAL_SIZE + 64];

            size_t payload_size =
                append_signature(payload, recv_address, sizeof(recv_address));

            transmit_data_to_app(SWAP_TXN_SIGNED_RECV_ADDR,
                                 (uint8_t *) &payload,
                                 payload_size);

            flow_level.level_three = SWAP_TXN_UNSIGNED_TXN_WAIT_SCREEN;
        }
            break;

        case SWAP_SELECT_SEND_WALLET_ID: {
            flow_level.level_three = SWAP_TXN_UNSIGNED_TXN_WAIT_SCREEN;
        }
            break;

        case SWAP_TXN_UNSIGNED_TXN_WAIT_SCREEN: {
            memcpy(&var_send_transaction_data.transaction_metadata,
                   &swap_transaction_data.send_txn_metadata, sizeof
                       (swap_transaction_data.send_txn_metadata));

            swap_transaction_data.unsigned_txn_data_array = NULL;
            swap_transaction_data.unsigned_txn_data_array_size = 0;

            if (get_usb_msg_by_cmd_type(SWAP_TXN_UNSIGNED_TXN,
                                        &swap_transaction_data.unsigned_txn_data_array,
                                        &swap_transaction_data
                                        .unsigned_txn_data_array_size)) {

                uint32_t coin_index = BYTE_ARRAY_TO_UINT32
                (var_send_transaction_data.transaction_metadata.coin_index);

                counter.level = LEVEL_THREE;
                flow_level.level_three = 2;

                if (coin_index == ETHEREUM) {
                    flow_level.level_two = LEVEL_THREE_SEND_TRANSACTION_ETH;
                } else if (coin_index == NEAR_COIN_INDEX) {
                    flow_level.level_two = LEVEL_THREE_SEND_TRANSACTION_NEAR;
                } else if (coin_index == SOLANA_COIN_INDEX) {
                    flow_level.level_two = LEVEL_THREE_SEND_TRANSACTION_SOLANA;
                } else {
                    flow_level.level_two = LEVEL_THREE_SEND_TRANSACTION;
                }
            }

        }
            break;

        default:break;
    }
}