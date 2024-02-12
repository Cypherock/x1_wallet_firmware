#include "btc_app.h"
#include "btc_helpers.h"
#include "btc_priv.h"
#include "coin_utils.h"
#include "dash_app.h"
#include "doge_app.h"
#include "evm_helpers.h"
#include "ltc_app.h"
#include "near.h"
#include "near_helpers.h"
#include "solana.h"
#include "solana_helpers.h"
#include "unity_fixture.h"

const uint32_t paths[][7] = {
    // depth_size, validity, purpose_id, coin_id, account_id, change_id,
    // address_id
    // test for account 0
    {5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x80000000},    // near
                                                                     // valid
    // test for account 0xffffffff
    {5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0xffffffff},    // near
                                                                     // valid
    // wrong purpose-id
    {5,
     0,
     NATIVE_SEGWIT,
     NEAR,
     0x80000000,
     0x80000000,
     0x80000000},    // near invalid
    // wrong coin id
    {5,
     0,
     NON_SEGWIT,
     NEAR - 2,
     0x80000000,
     0x80000000,
     0x80000000},    // near invalid
    // 1 non-hardened index
    {5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x8fffffff},    // near
                                                                     // invalid
    // 1 non-hardened index
    {5, 0, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x7fffffff},    // near
                                                                     // invalid
    // 2 non-hardened indices
    {5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x00000000, 0x8fffffff},    // near
                                                                     // invalid
    // 2 non-hardened indices
    {5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x0fffffff},    // near
                                                                     // invalid

    // sol-paper account
    {2, 1, NON_SEGWIT, SOLANA},    // sol  valid
    // sol-ledger account 0
    {3, 1, NON_SEGWIT, SOLANA, 0x80000000},    // sol  valid
    // sol-ledger account 0x7fffffff
    {3, 1, NON_SEGWIT, SOLANA, 0xffffffff},    // sol  valid
    // sol-phantom account 0
    {4, 1, NON_SEGWIT, SOLANA, 0x80000000, 0x80000000},    // sol  valid
    // sol-phantom account 0x7fffffff
    {4, 1, NON_SEGWIT, SOLANA, 0x8fffffff, 0x80000000},    // sol  valid
    // sol-paper wrong purpose-id
    {2, 0, NATIVE_SEGWIT, SOLANA},    // sol  invalid
    // sol-ledger non-hardened account
    {3, 0, NON_SEGWIT, SOLANA, 0x00000000},    // sol  invalid
    // sol-ledger wrong purpose-id and non-hardened account
    {3, 0, NATIVE_SEGWIT, SOLANA, 0x00000000},    // sol  invalid
    // sol-phantom wrong purpose-id
    {4, 0, NATIVE_SEGWIT, SOLANA, 0x80000000, 0x80000001},    // sol  invalid
    // sol-phantom
    {4, 0, NON_SEGWIT, SOLANA, 0x80000000, 0x80000001},    // sol  invalid
    {5,
     0,
     NON_SEGWIT,
     SOLANA,
     0x80000000,
     0x80000000,
     0x8fffffff},    // sol  invalid

    // wrong depth for xpub
    {5,
     0,
     NON_SEGWIT,
     ETHEREUM,
     0x80000000,
     0x80000000,
     0xffffffff},    // eth  invalid
    // wrong depth and purpose-id
    {5,
     0,
     NATIVE_SEGWIT,
     ETHEREUM,
     0x80000000,
     0x80000000,
     0xffffffff},    // eth  invalid
    // wrong depth, hardened and purpose-id
    {5,
     0,
     NATIVE_SEGWIT,
     ETHEREUM,
     0x00000000,
     0x80000000,
     0xffffffff},    // eth  invalid

    {3, 1, NON_SEGWIT, BITCOIN, 0x80000000},       // btc  valid
    {3, 1, NATIVE_SEGWIT, BITCOIN, 0x80000000},    // btc  valid
    {3, 1, NATIVE_SEGWIT, BITCOIN, 0x8fffffff},    // btc  valid
    {3, 1, NON_SEGWIT, BITCOIN, 0x80000000},       // btc  valid
    {3, 1, NON_SEGWIT, DOGE, 0x80000000},          // doge valid
    {3, 1, NON_SEGWIT, DOGE, 0x8fffffff},          // doge  valid
    {3, 1, NON_SEGWIT, DASH, 0x8fffffff},          // dash valid
    {3, 1, NATIVE_SEGWIT, LITCOIN, 0x80000000},    // ltc  valid
    {3, 1, NATIVE_SEGWIT, LITCOIN, 0x8fffffff},    // ltc  valid
    {3, 1, NON_SEGWIT, LITCOIN, 0x8fffffff},       // ltc  valid
    {3, 1, NON_SEGWIT, LITCOIN, 0x80000000},       // ltc  valid
    {5,
     0,
     NON_SEGWIT,
     BITCOIN,
     0x80000000,
     0x80000000,
     0x8fffffff},                               // btc  invalid
    {3, 0, NON_SEGWIT, BITCOIN, 0x00000000},    // btc  invalid
    {5,
     0,
     NATIVE_SEGWIT,
     BITCOIN,
     0x80000000,
     0x80000000,
     0x8fffffff},                                           // btc  invalid
    {3, 0, NATIVE_SEGWIT, DOGE, 0x80000000},                // doge invalid
    {4, 0, NON_SEGWIT, DOGE, 0x80000000, 0x8fffffff},       // doge  invalid
    {3, 0, NON_SEGWIT, DASH, 0x0fffffff},                   // dash invalid
    {3, 0, NON_SEGWIT, LITCOIN, 0x00000000},                // ltc  invalid
    {4, 0, NON_SEGWIT, LITCOIN, 0x80000000, 0x8fffffff},    // ltc  invalid
    {5,
     0,
     NATIVE_SEGWIT,
     LITCOIN,
     0x80000000,
     0x80000000,
     0x8fffffff},          // ltc  invalid
    {1, 0, NON_SEGWIT},    // invalid
};

TEST_GROUP(xpub);

TEST_SETUP(xpub) {
  return;
}

TEST_TEAR_DOWN(xpub) {
  return;
}

TEST(xpub, derivation_path_tests) {
  for (int i = 0; i < sizeof(paths) / (7 * sizeof(uint32_t)); i++) {
    bool validity = paths[i][1] == 1;
    uint16_t depth = (uint16_t)paths[i][0];
    bool status = false;
    switch (paths[i][3]) {
      case BITCOIN:
        g_btc_app = get_btc_app_desc()->app_config;
        status = btc_derivation_path_guard(&paths[i][2], depth);
        break;
      case DASH:
        g_btc_app = get_dash_app_desc()->app_config;
        status = btc_derivation_path_guard(&paths[i][2], depth);
        break;
      case LITCOIN:
        g_btc_app = get_ltc_app_desc()->app_config;
        status = btc_derivation_path_guard(&paths[i][2], depth);
        break;
      case DOGE:
        g_btc_app = get_doge_app_desc()->app_config;
        status = btc_derivation_path_guard(&paths[i][2], depth);
        break;
      case NEAR:
        status = near_derivation_path_guard(&paths[i][2], depth);
        break;
      case SOLANA:
        status = solana_derivation_path_guard(&paths[i][2], depth);
        break;
      case ETHEREUM:
        status = evm_derivation_path_guard(&paths[i][2], depth);
        break;
      default:
        status = false;
        break;
    }

    TEST_ASSERT(validity == status);
  }
}
