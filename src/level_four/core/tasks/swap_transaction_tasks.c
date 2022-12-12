#include "communication.h"
#include "tasks_level_four.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "stdio.h"
#include "btc.h"

extern Swap_Transaction_Data swap_transaction_data;

void swap_transaction_tasks() {
  switch (flow_level.level_three) {
    case SWAP_CONFIRM_SEND_AMOUNT: {
      instruction_scr_destructor();
      double amount_in_btc = 1.0 * swap_transaction_data.send_amount /
          (SATOSHI_PER_BTC);
      char display[125] = {0};
      uint8_t precision =
          get_floating_precision(swap_transaction_data.send_amount,
                                 SATOSHI_PER_BTC);
      snprintf(display, sizeof(display), "Send\n%0.*f\n%s",
               precision, amount_in_btc, get_coin_symbol
                   (BYTE_ARRAY_TO_UINT32(swap_transaction_data.source_coin_index),
                    swap_transaction_data.source_network_chain_id));
      confirm_scr_init(display);
    }
      break;

    case SWAP_CONFIRM_RECV_AMOUNT: {
      instruction_scr_destructor();
      double amount_in_btc = 1.0 * swap_transaction_data.recv_amount /
          (SATOSHI_PER_BTC);
      char display[125] = {0};
      uint8_t precision =
          get_floating_precision(swap_transaction_data.recv_amount,
                                 SATOSHI_PER_BTC);
      snprintf(display, sizeof(display), "Receive \n%0.*f\n%s",
               precision, amount_in_btc, get_coin_symbol
                   (BYTE_ARRAY_TO_UINT32(swap_transaction_data.dest_coin_index),
                   swap_transaction_data.dest_network_chain_id));
      confirm_scr_init(display);
    }
      break;

    case SWAP_CONFIRM_NETWORK_FEES: {
      instruction_scr_destructor();
      double amount_in_btc = 1.0 * swap_transaction_data.network_fee /
          (SATOSHI_PER_BTC);
      char display[125] = {0};
      uint8_t precision =
          get_floating_precision(swap_transaction_data.network_fee,
                                 SATOSHI_PER_BTC);
      snprintf(display, sizeof(display), "Network Fees \n%0.*f\n%s",
               precision, amount_in_btc, get_coin_symbol
                   (BYTE_ARRAY_TO_UINT32(swap_transaction_data
                                             .source_coin_index),
                    swap_transaction_data.dest_network_chain_id));
      confirm_scr_init(display);
    }
      break;

    case SWAP_VERIFY_SESSION: {
      transmit_data_to_app(103, (uint8_t *) &swap_transaction_data, sizeof
          (swap_transaction_data));
    }
  }
}