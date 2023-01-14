#include "communication.h"
#include "tasks_level_four.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "stdio.h"
#include "btc.h"
#include "utils.h"
#include "near.h"
#include "solana.h"

extern Swap_Transaction_Data swap_transaction_data;
extern Receive_Transaction_Data receive_transaction_data;

static const uint8_t *get_wallet_name_from_wallet_id(uint8_t *wallet_id) {
    uint8_t walletIndex = 0;

    for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
        if (memcmp(wallet_id, get_wallet_id(walletIndex), WALLET_ID_SIZE)
            == 0) {
            if (get_wallet_state(walletIndex) == VALID_WALLET) {
                return get_wallet_name(walletIndex);
            } else {
                comm_reject_request(WALLET_DOES_NOT_EXISTS, 1);
                return false;
            }
        }
    }
    return NULL;
}

void swap_transaction_tasks() {
    switch (flow_level.level_three) {
        case SWAP_CONFIRM_SEND_COIN: {
            instruction_scr_destructor();
            uint32_t send_coin_index =
                BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                         .send_txn_metadata.coin_index);
            char display[200] = {0};

            snprintf(display, sizeof(display), UI_TEXT_SWAP_SEND_PROMPT,
                     swap_transaction_data.send_txn_metadata.token_name,
                     get_wallet_name_from_wallet_id(swap_transaction_data.send_txn_wallet_id),
                     get_coin_name(send_coin_index,
                                   swap_transaction_data.send_txn_metadata.network_chain_id));
            confirm_scr_init(display);

        }
            break;

        case SWAP_CONFIRM_RECEIVE_COIN: {
            instruction_scr_destructor();
            uint32_t receive_coin_index =
                BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                         .receive_txn_data.coin_index);

            char display[200] = {0};

            snprintf(display, sizeof(display), UI_TEXT_SWAP_RECEIVE_PROMPT,
                     swap_transaction_data.receive_txn_data.token_name,
                     get_wallet_name_from_wallet_id(swap_transaction_data.receive_txn_data.wallet_id),
                     get_coin_name(receive_coin_index,
                                   swap_transaction_data.receive_txn_data.network_chain_id));

            confirm_scr_init(display);
        }
            break;

        case SWAP_SELECT_RECEIVE_WALLET_ID: {
            if (wallet_selector(swap_transaction_data.receive_txn_data
                                    .wallet_id)) {
                mark_event_over();
            } else {
                reset_flow_level();
                counter.next_event_flag = true;
            }
        }
            break;

        case SWAP_CONFIRM_SEND_AMOUNT: {
            instruction_scr_destructor();
            char display[125] = {0};
            snprintf(display, sizeof(display), "Amount to Send\n%s\n%s",
                     swap_transaction_data.send_amount_str, get_coin_symbol
                         (BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                                   .send_txn_metadata.coin_index),
                          swap_transaction_data.send_txn_metadata.network_chain_id));
            confirm_scr_init(display);
        }
            break;

        case SWAP_CONFIRM_RECEIVE_AMOUNT: {
            instruction_scr_destructor();
            char display[125] = {0};
            snprintf(display,
                     sizeof(display),
                     "Approx. Amount to Receive\n%s\n%s",
                     swap_transaction_data.receive_amount_str,
                     get_coin_symbol
                         (BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                                   .receive_txn_data.coin_index),
                          swap_transaction_data.receive_txn_data.network_chain_id));
            confirm_scr_init(display);
        }
            break;

        case SWAP_CONFIRM_NETWORK_FEES: {
            instruction_scr_destructor();
            char display[125] = {0};
            snprintf(display, sizeof(display), "Transaction Fee\n%s\n%s",
                     swap_transaction_data.fee_str, get_coin_symbol
                         (BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                                   .send_txn_metadata.coin_index),
                          swap_transaction_data.send_txn_metadata.network_chain_id));
            confirm_scr_init(display);
        }
            break;

        case SWAP_VERIFY_SESSION: {
            //TODO: verify and validate session data
            mark_event_over();
        }
            break;

        case SWAP_RECV_ADDR_DERIVATION: {
            memcpy((uint8_t *) &receive_transaction_data,
                   (uint8_t *) &swap_transaction_data.receive_txn_data,
                   sizeof(Receive_Transaction_Data));

            mark_event_over();
        }
            break;

        case SWAP_SIGN_RECEIVE_ADDRESS: {
            is_swap_txn = true;
            // TODO: show message processing

            memcpy((uint8_t *) &swap_transaction_data.receive_txn_data,
                   (uint8_t *) &receive_transaction_data,
                   sizeof(Receive_Transaction_Data));
            mark_event_over();
        }
            break;

        case SWAP_VERIFY_SIGNATURE: {
            instruction_scr_init("", NULL);
            instruction_scr_change_text(ui_text_processing, true);
            mark_event_over();
        }
            break;

        case SWAP_SELECT_SEND_WALLET_ID: {
            if (wallet_selector(swap_transaction_data.send_txn_wallet_id)) {
                mark_event_over();
            } else {
                reset_flow_level();
                counter.next_event_flag = true;
            }
        }
            break;

        case SWAP_TXN_UNSIGNED_TXN_WAIT_SCREEN: {
            // TODO: show message processing
            mark_event_over();
        }
            break;
    }
}