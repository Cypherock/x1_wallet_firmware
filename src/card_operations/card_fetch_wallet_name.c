#include "card_fetch_wallet_list.h"
#include "constant_texts.h"

void card_get_wallet_name(uint8_t *wallet_id, const char *wallet_name) {
  wallet_list_t wallets_in_card = {0};

  card_fetch_wallet_list_config_t configuration = {0};
  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.skip_card_removal = true;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = NULL;
  configuration.frontend.msg = ui_text_card_get_wallet_info;

  card_fetch_wallet_list_response_t response = {0};
  response.wallet_list = &wallets_in_card;
  response.card_info.tapped_card = 0;
  response.card_info.recovery_mode = 0;
  response.card_info.status = 0;
  response.card_info.tapped_family_id = NULL;

  // P0 abort is the only condition we want to exit the flow
  // Card abort error will be explicitly shown here as error codes
  card_error_type_e status = card_fetch_wallet_list(&configuration, &response);
  if (CARD_OPERATION_P0_OCCURED == status) {
    return;
  }

  // If the tapped card is not paired, it is a terminal case in the flow
  if (true == response.card_info.pairing_error) {
    return;
  }

  // At this stage, either there is no core error message set, or it is set but
  // we want to overwrite the error message using user facing messages in this
  // flow
  clear_core_error_screen();

  uint32_t card_fault_status = 0;
  if (1 == response.card_info.recovery_mode) {
    card_fault_status = NFC_NULL_PTR_ERROR;
  } else if (CARD_OPERATION_SUCCESS != status) {
    card_fault_status = response.card_info.status;
  }

  // uint8_t screens = 3;
  char display_msg[100] = "";
  // const char *msg[3];

  // if (0 == card_fault_status) {
  //   screens = 2;
  //   msg[0] = ui_text_card_seems_healthy;
  //   if (0 < wallets_in_card.count) {
  //     msg[1] = ui_text_click_to_view_wallets;
  //   } else {
  //     msg[1] = ui_text_no_wallets_fetched;
  //   }
  // } else {
  //   snprintf(display_msg,
  //            sizeof(display_msg),
  //            "%s: C%04lX",
  //            ui_text_card_health_check_error[0],
  //            card_fault_status);
  //   msg[0] = (const char *)display_msg;
  //   msg[1] = ui_text_card_health_check_error[1];
  //   if (0 < wallets_in_card.count) {
  //     msg[2] = ui_text_click_to_view_wallets;
  //   } else {
  //     msg[2] = ui_text_no_wallets_fetched;
  //   }
  // }

  // typedef enum {
  //   CARD_HC_SHOW_WALLETS,
  //   CARD_HC_EXIT_FLOW,
  // } card_health_check_states_e;

  // card_health_check_states_e state_on_confirm = CARD_HC_EXIT_FLOW;
  // if (0 < wallets_in_card.count) {
  //   state_on_confirm = CARD_HC_SHOW_WALLETS;
  // }

  // multi_instruction_init(msg, screens, DELAY_TIME, true);

  // if (CARD_HC_SHOW_WALLETS == get_state_on_confirm_scr(state_on_confirm,
  //                                                      CARD_HC_EXIT_FLOW,
  //                                                      CARD_HC_EXIT_FLOW)) {
  // memzero(display_msg, sizeof(display_msg));
  // snprintf(display_msg,
  //           sizeof(display_msg),
  //           UI_TEXT_CARD_FETCH_WALLETS,
  //           decode_card_number(response.card_info.tapped_card));

  // char wallet_list[MAX_WALLETS_ALLOWED][NAME_SIZE] = {"", "", "", ""};
  for (uint8_t i = 0; i < wallets_in_card.count; i++) {
    if (memcmp(wallet_id, wallets_in_card.wallet->id, WALLET_ID_SIZE) == 0) {
      memcpy(
          wallet_name, (const char *)wallets_in_card.wallet[i].name, NAME_SIZE);
      snprintf(display_msg,
               sizeof(display_msg),
               UI_TEXT_CARD_FETCH_WALLET,
               wallet_name);
      break;
    }
  }
  // list_init(wallet_list, wallets_in_card.count, display_msg, false);

  if (!core_scroll_page(NULL, display_msg, NULL)) {
    return;
  }

  // Do not care about the return value from confirmation screen
  (void)get_state_on_confirm_scr(0, 0, 0);
  // }

  return;
}
