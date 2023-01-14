#include "controller_level_four.h"

extern Swap_Transaction_Data swap_transaction_data;
void swap_transaction_controller_b() {
    switch (flow_level.level_three) {
        case SWAP_CONFIRM_SEND_COIN: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_CONFIRM_RECEIVE_COIN: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_SELECT_RECEIVE_WALLET_ID: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;
        case SWAP_CONFIRM_SEND_AMOUNT: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_CONFIRM_RECEIVE_AMOUNT: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_CONFIRM_NETWORK_FEES: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_VERIFY_SESSION: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_RECV_ADDR_DERIVATION: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_SELECT_SEND_WALLET_ID: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;

        case SWAP_TXN_UNSIGNED_TXN_WAIT_SCREEN: {
            reset_flow_level();
            counter.next_event_flag = true;
        }
            break;
    }
}