// Test file for wallet-key-auth proto structs
#include "inheritance_app.h"
#include "libraries/util/wallet_auth_utils.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

wallet_auth_t auth = {0};

bool wallet_login(inheritance_query_t *query) {
  // if (!verify_wallet_auth_inputs(auth)) {
  //     return WALLET_AUTH_INPUTS_INVALID;
  // }

  memcpy(auth.wallet_id, query->wallet_auth.initiate.wallet_id, 32);
  memcpy(auth.challenge, query->wallet_auth.initiate.challenge, 32);
  auth.challenge_size = 32;
  auth.is_publickey = query->wallet_auth.initiate.is_publickey;

  // size_t size = query->wallet_auth.initiate.challenge;
  // char *message = (char *)malloc(size + 1);
  // memcpy(message, query->wallet_auth.initiate.challenge.bytes, size );
  // message[size] = '\0';
  char *message = query->wallet_auth.initiate.challenge;

  printf("\nMESSAGE: %d \n", 32);
  for (int i = 0; i < 32; i++) {
    printf("%02x", message[i]);
    fflush(stdout);
  }
  fflush(stdout);

  if (!core_scroll_page("Test Challenge", message, inheritance_send_error)) {
    return;
  }

  switch (auth.is_publickey) {
    case true: {
      if (!wallet_auth_get_entropy(auth) || wallet_auth_get_pairs(auth) ||
          !wallet_auth_get_signature(auth))
        // return WALLET_AUTH_ERR_OWNER;
        break;
    }

    default: {
      if (!wallet_auth_get_signature(auth))
        // return WALLET_AUTH_ERR_NOMINEE;
        break;
    }
  }
  uint8_t publickey[32] = {144, 213, 122, 213, 228, 193, 104, 222, 201, 19, 75,
                           27,  117, 55,  36,  46,  87,  72,  90,  53,  41, 121,
                           48,  132, 163, 239, 183, 51,  61,  251, 171, 30};
  uint8_t signature[64] = {
      144, 213, 122, 213, 228, 193, 104, 222, 201, 19,  75,  27,  117,
      55,  36,  46,  87,  72,  90,  53,  41,  121, 48,  132, 163, 239,
      183, 51,  61,  251, 171, 30,  144, 213, 122, 213, 228, 193, 104,
      222, 201, 19,  75,  27,  117, 55,  36,  46,  87,  72,  90,  53,
      41,  121, 48,  132, 163, 239, 183, 51,  61,  251, 171, 30};
  printf("SENDING RESULT");
  inheritance_result_t result = INHERITANCE_RESULT_INIT_ZERO;
  result.which_response = INHERITANCE_RESULT_WALLET_AUTH_TAG;
  result.wallet_auth.which_response =
      INHERITANCE_WALLET_AUTH_RESPONSE_RESULT_TAG;
  memcpy(result.wallet_auth.result.public_key, publickey, 32);
  memcpy(result.wallet_auth.result.signature, signature, 64);
  for (int i = 0; i < 64; i++) {
    printf("%02x", result.wallet_auth.result.public_key[i]);
  }

  inheritance_send_result(&result);
  return true;
}
