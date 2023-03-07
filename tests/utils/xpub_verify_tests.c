#include "coin_utils.h"
#include "near.h"
#include "eth.h"
#include "btc.h"
#include "solana.h"
#include "logger.h"

const uint32_t paths[][7] = {
    // depth_size, validity, purpose_id, coin_id, account_id, change_id, address_id
    { 5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x80000000},          // near valid
    { 5, 1, NON_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x8fffffff},          // near valid
    { 5, 0, NATIVE_SEGWIT, NEAR, 0x80000000, 0x80000000, 0x80000000},       // near invalid
    { 5, 0, NON_SEGWIT, NEAR - 2, 0x80000000, 0x80000000, 0x80000000},      // near invalid
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x8fffffff},          // near invalid
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x00000000, 0x8fffffff},          // near invalid
    { 5, 0, NON_SEGWIT, NEAR, 0x00000000, 0x80000000, 0x0fffffff},          // near invalid

    { 2, 1, NON_SEGWIT, SOLANA },                                           // sol  valid
    { 3, 1, NON_SEGWIT, SOLANA, 0x80000000 },                               // sol  valid
    { 3, 1, NON_SEGWIT, SOLANA, 0x8fffffff },                               // sol  valid
    { 4, 1, NON_SEGWIT, SOLANA, 0x80000000, 0x80000000 },                   // sol  valid
    { 4, 1, NON_SEGWIT, SOLANA, 0x8fffffff, 0x80000000 },                   // sol  valid
    { 2, 0, NATIVE_SEGWIT, SOLANA },                                        // sol  invalid
    { 3, 0, NON_SEGWIT, SOLANA, 0x00000000 },                               // sol  invalid
    { 3, 0, NATIVE_SEGWIT, SOLANA, 0x00000000 },                            // sol  invalid
    { 4, 0, NATIVE_SEGWIT, SOLANA, 0x80000000, 0x80000001 },                // sol  invalid
    { 4, 0, NON_SEGWIT, SOLANA, 0x80000000, 0x80000001 },                   // sol  invalid
    { 5, 0, NON_SEGWIT, SOLANA, 0x80000000, 0x80000000, 0x8fffffff},        // sol  invalid

    { 3, 1, NON_SEGWIT, ETHEREUM, 0x80000000 },                             // eth  valid
    { 3, 1, NON_SEGWIT, ETHEREUM, 0x8fffffff },                             // eth  valid
    { 5, 0, NON_SEGWIT, ETHEREUM, 0x80000000, 0x80000000, 0x8fffffff},      // eth  invalid
    { 3, 0, NON_SEGWIT, ETHEREUM, 0x00000000 },                             // eth  invalid
    { 3, 0, NATIVE_SEGWIT, ETHEREUM, 0x80000000 },                          // eth  invalid
    { 5, 0, NATIVE_SEGWIT, ETHEREUM, 0x80000000, 0x80000000, 0x8fffffff},   // eth  invalid

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
