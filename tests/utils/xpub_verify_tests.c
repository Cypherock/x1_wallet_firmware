#include "coin_utils.h"
#include "near.h"
#include "eth.h"
#include "btc.h"
#include "solana.h"
#include "logger.h"

const uint32_t paths[][7] = {
    // depth_size, validity, purpose_id, coin_id, account_id, change_id, address_id
    // test for account 0
    { 5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x80000000},          // near valid
    // test for account 0xffffffff
    { 5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0xffffffff},          // near valid
    // wrong purpose-id
    { 5, 0, NATIVE_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x80000000},       // near invalid
    // wrong coin id
    { 5, 0, NON_SEGWIT, NEAR - 2, 0x80000000, 0x80000000, 0x80000000},      // near invalid
    // 1 non-hardened index
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x8fffffff},          // near invalid
    // 1 non-hardened index
    { 5, 0, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x7fffffff},          // near invalid
    // 2 non-hardened indices
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x00000000, 0x8fffffff},          // near invalid
    // 2 non-hardened indices
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x0fffffff},          // near invalid

    // sol-paper account
    { 2, 1, NON_SEGWIT, SOLANA },                                           // sol  valid
    // sol-ledger account 0
    { 3, 1, NON_SEGWIT, SOLANA, 0x80000000 },                               // sol  valid
    // sol-ledger account 0x7fffffff
    { 3, 1, NON_SEGWIT, SOLANA, 0xffffffff },                               // sol  valid
    // sol-phantom account 0
    { 4, 1, NON_SEGWIT, SOLANA, 0x80000000, 0x80000000 },                   // sol  valid
    // sol-phantom account 0x7fffffff
    { 4, 1, NON_SEGWIT, SOLANA, 0x8fffffff, 0x80000000 },                   // sol  valid
    // sol-paper wrong purpose-id
    { 2, 0, NATIVE_SEGWIT, SOLANA },                                        // sol  invalid
    // sol-ledger non-hardened account
    { 3, 0, NON_SEGWIT, SOLANA, 0x00000000 },                               // sol  invalid
    // sol-ledger wrong purpose-id and non-hardened account
    { 3, 0, NATIVE_SEGWIT, SOLANA, 0x00000000 },                            // sol  invalid
    // sol-phantom wrong purpose-id
    { 4, 0, NATIVE_SEGWIT, SOLANA, 0x80000000, 0x80000001 },                // sol  invalid
    // sol-phantom 
    { 4, 0, NON_SEGWIT, SOLANA, 0x80000000, 0x80000001 },                   // sol  invalid
    { 5, 0, NON_SEGWIT, SOLANA, 0x80000000, 0x80000000, 0x8fffffff},        // sol  invalid

    // account 0
    { 3, 1, NON_SEGWIT, ETHEREUM, 0x80000000 },                             // eth  valid
    // account 0x7fffffff
    { 3, 1, NON_SEGWIT, ETHEREUM, 0xffffffff },                             // eth  valid
    // non-hardened
    { 3, 0, NON_SEGWIT, ETHEREUM, 0x00000000 },                             // eth  invalid
    // wrong depth for xpub
    { 5, 0, NON_SEGWIT, ETHEREUM, 0x80000000, 0x80000000, 0xffffffff},      // eth  invalid
    // wrong purpose-id
    { 3, 0, NATIVE_SEGWIT, ETHEREUM, 0x80000000 },                          // eth  invalid
    // wrong depth and purpose-id
    { 5, 0, NATIVE_SEGWIT, ETHEREUM, 0x80000000, 0x80000000, 0xffffffff},   // eth  invalid
    // wrong depth, hardened and purpose-id
    { 5, 0, NATIVE_SEGWIT, ETHEREUM, 0x00000000, 0x80000000, 0xffffffff},   // eth  invalid

    { 3, 1, NON_SEGWIT, BITCOIN, 0x80000000 },                              // btc  valid
    { 3, 1, NATIVE_SEGWIT, BITCOIN, 0x80000000 },                           // btc  valid
    { 3, 1, NATIVE_SEGWIT, BITCOIN, 0x8fffffff },                           // btc  valid
    { 3, 1, NON_SEGWIT, BITCOIN, 0x80000000 },                              // btc  valid
    { 3, 1, NON_SEGWIT, BTC_TEST, 0x8fffffff },                             // btct valid
    { 3, 1, NON_SEGWIT, DOGE, 0x80000000 },                                 // doge valid
    { 3, 1, NON_SEGWIT, DASH, 0x8fffffff },                                 // dash valid
    { 5, 0, NON_SEGWIT, BITCOIN, 0x80000000, 0x80000000, 0x8fffffff},       // btc  invalid
    { 3, 0, NON_SEGWIT, BITCOIN, 0x00000000 },                              // btc  invalid
    { 5, 0, NATIVE_SEGWIT, BITCOIN, 0x80000000, 0x80000000, 0x8fffffff},    // btc  invalid
    { 3, 0, NATIVE_SEGWIT, DOGE, 0x80000000 },                              // doge invalid
    { 3, 0, NON_SEGWIT, DASH, 0x0fffffff },                                 // dash invalid
};

void xpub_derivation_path_tests() {
    for (int i = 0; i < sizeof(paths) / (7 * sizeof(uint32_t)); i++) {
        bool validity = paths[i][1] == 1;
        uint16_t depth = (uint16_t) paths[i][0];
        bool status = verify_xpub_derivation_path(&paths[i][2], depth);
        if (validity != status) LOG_CRITICAL("err: %d : (expected: %s:: actual: %s)", i, validity ? "true" : "false", status ? "true" : "false");
    }
}
