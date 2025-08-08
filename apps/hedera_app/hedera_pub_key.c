/**
 * @file    hedera_pub_key.c
 * @author  Cypherock X1 Team
 * @brief   Generates public key for Hedera derivations.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#include "hedera_priv.h"
#include "hedera_api.h"
#include "hedera_helpers.h"
#include "hedera_context.h"

#include "wallet_list.h"
#include "ui_screens.h"
#include "ui_core_confirm.h"
#include "reconstruct_wallet_flow.h"
#include "bip32.h"
#include "curves.h"
#include "ed25519.h"
#include "coin_utils.h"

static bool check_which_request(const hedera_query_t *query, pb_size_t which_request) {
    if (which_request != query->get_public_keys.which_request) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }
    return true;
}

static bool validate_request(const hedera_get_public_keys_intiate_request_t *req, const pb_size_t which_request) {
    if (0 == req->derivation_paths_count) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_DATA);
        return false;
    }

    if (HEDERA_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request && 1 < req->derivation_paths_count) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_DATA);
        return false;
    }

    for (pb_size_t i = 0; i < req->derivation_paths_count; i++) {
        if (!hedera_derivation_path_guard(req->derivation_paths[i].path, req->derivation_paths[i].path_count)) {
            hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_DATA);
            return false;
        }
    }
    return true;
}

static bool get_public_key(const uint8_t *seed, const uint32_t *path, uint32_t path_length, uint8_t *public_key) {
    HDNode node = {0};
    if (!derive_hdnode_from_path(path, path_length, ED25519_NAME, seed, &node)) {
        hedera_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
        memzero(&node, sizeof(HDNode));
        return false;
    }
    memcpy(public_key, node.public_key, HEDERA_PUB_KEY_SIZE);
    memzero(&node, sizeof(HDNode));
    return true;
}

static bool fill_public_keys(const hedera_get_public_keys_derivation_path_t *path, const uint8_t *seed, uint8_t public_key_list[][HEDERA_PUB_KEY_SIZE], pb_size_t count) {
    for (pb_size_t i = 0; i < count; i++) {
        if (!get_public_key(seed, path[i].path, path[i].path_count, public_key_list[i])) {
            return false;
        }
    }
    return true;
}

static bool send_public_keys(hedera_query_t *query, const uint8_t pubkey_list[][HEDERA_PUB_KEY_SIZE], const pb_size_t count, const pb_size_t which_request, const pb_size_t which_response) {
    hedera_result_t response = init_hedera_result(which_response);
    hedera_get_public_keys_result_response_t *result = &response.get_public_keys.result;
    size_t batch_limit = sizeof(result->public_keys) / HEDERA_PUB_KEY_SIZE;
    size_t remaining = count;

    response.get_public_keys.which_response = HEDERA_GET_PUBLIC_KEYS_RESPONSE_RESULT_TAG;
    while (true) {
        size_t batch_size = CY_MIN(batch_limit, remaining);
        result->public_keys_count = batch_size;
        memcpy(result->public_keys, &pubkey_list[count - remaining], batch_size * HEDERA_PUB_KEY_SIZE);
        hedera_send_result(&response);
        remaining -= batch_size;
        if (0 == remaining) break;
        if (!hedera_get_query(query, which_request) || !check_which_request(query, HEDERA_GET_PUBLIC_KEYS_REQUEST_FETCH_NEXT_TAG)) {
            return false;
        }
    }
    return true;
}

static bool get_user_consent(const pb_size_t which_request, const char *wallet_name) {
    char msg[100] = "";
    if (HEDERA_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
        snprintf(msg, sizeof(msg), UI_TEXT_ADD_ACCOUNT_PROMPT, HEDERA_NAME, wallet_name);
    } else {
        snprintf(msg, sizeof(msg), "Export public key for %s?", HEDERA_NAME);
    }
    return core_confirmation(msg, hedera_send_error);
}

void hedera_get_pub_keys(hedera_query_t *query) {
    char wallet_name[NAME_SIZE] = "";
    uint8_t seed[64] = {0};
    const pb_size_t which_request = query->which_request;
    const hedera_get_public_keys_intiate_request_t *init_req;
    pb_size_t which_response;

    if (HEDERA_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
        which_response = HEDERA_RESULT_GET_PUBLIC_KEYS_TAG;
        init_req = &query->get_public_keys.initiate;
    } else {
        which_response = HEDERA_RESULT_GET_USER_VERIFIED_PUBLIC_KEY_TAG;
        init_req = &query->get_user_verified_public_key.initiate;
    }

    const pb_size_t count = init_req->derivation_paths_count;
    uint8_t pubkey_list[sizeof(init_req->derivation_paths) / sizeof(hedera_get_public_keys_derivation_path_t)][HEDERA_PUB_KEY_SIZE] = {0};

    if (!check_which_request(query, HEDERA_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG) ||
        !validate_request(init_req, which_request) ||
        !get_wallet_name_by_id(init_req->wallet_id, (uint8_t *)wallet_name, hedera_send_error)) {
        return;
    }

    if (!get_user_consent(which_request, wallet_name)) {
        return;
    }

    set_app_flow_status(HEDERA_GET_PUBLIC_KEYS_STATUS_CONFIRM);

    if (!reconstruct_seed(init_req->wallet_id, &seed[0], hedera_send_error)) {
        memzero(seed, sizeof(seed));
        return;
    }

    set_app_flow_status(HEDERA_GET_PUBLIC_KEYS_STATUS_SEED_GENERATED);
    delay_scr_init(ui_text_processing, DELAY_SHORT);

    bool result = fill_public_keys(init_req->derivation_paths, seed, pubkey_list, count);
    memzero(seed, sizeof(seed));

    if (!result) {
        hedera_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
        return;
    }

    if (HEDERA_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request) {
        char pubkey_hex[HEDERA_ADDRESS_STRING_SIZE];
        hedera_format_pubkey(pubkey_list[0], pubkey_hex);
        if (!core_scroll_page("Verify Public Key", pubkey_hex, hedera_send_error)) {
            return;
        }
        set_app_flow_status(HEDERA_GET_PUBLIC_KEYS_STATUS_VERIFY);
    }

    if (!send_public_keys(query, pubkey_list, count, which_request, which_response)) {
        return;
    }

    delay_scr_init(ui_text_check_software_wallet_app, DELAY_TIME);
}