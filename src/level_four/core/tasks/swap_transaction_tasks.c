#include "communication.h"
#include "tasks_level_four.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "stdio.h"
#include "btc.h"

extern Swap_Transaction_Data swap_transaction_data;
extern Receive_Transaction_Data receive_transaction_data;

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
      //TODO: verify and validate session data
      mark_event_over();
    }
      break;

    case SWAP_RECV_ADDR_DERIVATION: {
      // copy the metadata from swap transaction to receive transaction
      memcpy(receive_transaction_data.wallet_id,
             swap_transaction_data.wallet_id,
             sizeof(receive_transaction_data.wallet_id));
      memcpy(receive_transaction_data.purpose, swap_transaction_data
          .purpose, sizeof(receive_transaction_data.purpose));
      memcpy(receive_transaction_data.coin_index, swap_transaction_data
          .source_coin_index, sizeof(receive_transaction_data.coin_index));
      memcpy(receive_transaction_data.account_index, swap_transaction_data
          .account_index, sizeof(receive_transaction_data.account_index));
      memcpy(receive_transaction_data.chain_index, swap_transaction_data
          .chain_index, sizeof(receive_transaction_data.chain_index));
      memcpy(receive_transaction_data.address_index, swap_transaction_data
          .address_index, sizeof(receive_transaction_data.address_index));
      //TODO: copy token name
      //TODO: copy chain id
      mark_event_over();
    }
      break;

    case SWAP_AFTER_RECV_FLOW: {
      transmit_data_to_app(100,
                           (uint8_t *) &receive_transaction_data.address,
                           sizeof(receive_transaction_data.address));
    }
      break;
  }
}