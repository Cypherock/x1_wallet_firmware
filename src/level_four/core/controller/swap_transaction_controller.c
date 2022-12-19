#include "controller_level_four.h"

Swap_Transaction_Data
    swap_transaction_data;

void jump_to_swap() {
    counter.level = LEVEL_THREE;
    flow_level.level_two = LEVEL_THREE_SWAP_TRANSACTION;
    flow_level.level_three = 6; // SWAP_AFTER_RECV_FLOW
    is_swap_txn = false;
}

auth_data_t atecc_sign(uint8_t *hash) {
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

void swap_transaction_controller() {
    switch (flow_level.level_three) {
        case SWAP_CONFIRM_SEND_AMOUNT: {
            flow_level.level_three = SWAP_CONFIRM_RECV_AMOUNT;
        }
            break;

        case SWAP_CONFIRM_RECV_AMOUNT: {
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
                                                           .source_coin_index);
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

        case SWAP_AFTER_RECV_FLOW: {
            // Payload contains:
            // - Recv address (variable size, at most 45 bytes)
            // - Device ID (32 bytes)
            // - Signature (64 bytes)
            uint8_t payload[sizeof(swap_transaction_data.recv_address)
                + DEVICE_SERIAL_SIZE + 64];
            int payload_size = 0;

            memcpy(payload,
                   swap_transaction_data.recv_address,
                   sizeof(swap_transaction_data.recv_address));
            payload_size += sizeof(swap_transaction_data.recv_address);

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

            transmit_data_to_app(SWAP_TXN_SIGNED_RECV_ADDR,
                                 (uint8_t *) &payload,
                                 payload_size);
        }
            break;

        default:break;
    }
}