// Test file for wallet-key-auth proto structs
#include "inheritance_app.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

void inheritance_get_wallet_sign(inheritance_query_t *query) {
  char *message = query->wallet_auth.initiate.challenge;
  if (!core_scroll_page("Test Challenge", message, inheritance_send_error)) {
    return;
  }

  inheritance_result_t pubkey = INHERITANCE_RESULT_INIT_DEFAULT;
  pubkey.which_response = INHERITANCE_WALLET_AUTH_RESPONSE_RESULT_TAG;
  uint8_t bytes[] = {0x11, 0x22, 0x12, 0xfe, 0xff};
  memcpy(pubkey.wallet_auth.result.public_key, bytes, 5);

  inheritance_send_result(&pubkey);
}