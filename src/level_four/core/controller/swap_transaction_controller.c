#include "controller_level_four.h"

Swap_Transaction_Data
    swap_transaction_data;

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
      counter.level = LEVEL_THREE;
      flow_level.level_two = LEVEL_THREE_RECEIVE_TRANSACTION;
      flow_level.level_three = 1;
    }
      break;

    default:break;
  }
}