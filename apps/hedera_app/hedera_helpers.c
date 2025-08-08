/**
 * @file    hedera_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to the Hedera app.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#include "hedera_helpers.h"
#include "coin_utils.h"
#include "hedera_context.h"
#include <stdio.h>
#include <string.h>

bool hedera_derivation_path_guard(const uint32_t *path, uint8_t levels) {
    if (levels != HEDERA_COIN_DEPTH) {
        return false;
    }

    // Path must be m/44'/3030'/0'/0'/i'
    return (path[0] == HEDERA_PURPOSE_INDEX &&
            path[1] == HEDERA_COIN_INDEX &&
            path[2] == HEDERA_ACCOUNT_INDEX &&
            path[3] == HEDERA_CHANGE_INDEX &&
            is_non_hardened(path[4]));
}

void hedera_format_pubkey(const uint8_t *pubkey, char *out_str) {
    byte_array_to_hex_string(pubkey, HEDERA_PUB_KEY_SIZE, out_str, HEDERA_ADDRESS_STRING_SIZE);
}

void hedera_format_account_id(const Hedera_AccountID *account_id, char *out_str) {
    snprintf(out_str, 40, "%lld.%lld.%lld",
             (long long)account_id->shardNum,
             (long long)account_id->realmNum,
             (long long)account_id->account.accountNum);
}

void hedera_format_tinybars_to_hbar_string(int64_t tinybars, char *out_str) {
    char temp_str[30];
    const int64_t hbar_div = 100000000;
    
    int sign = (tinybars < 0) ? -1 : 1;
    if (tinybars < 0) tinybars = -tinybars;

    int64_t whole_part = tinybars / hbar_div;
    int64_t frac_part = tinybars % hbar_div;
    
    // Format fractional part with leading zeros
    snprintf(temp_str, sizeof(temp_str), "%lld.%08lld", (long long)whole_part, (long long)frac_part);
    
    // Trim trailing zeros
    char *end = temp_str + strlen(temp_str) - 1;
    while (end > temp_str && *end == '0') {
        *end-- = '\0';
    }
    if (end > temp_str && *end == '.') {
        *end = '\0';
    }
    
    snprintf(out_str, 40, "%s%s %s", (sign == -1) ? "-" : "", temp_str, HEDERA_LUNIT);
}