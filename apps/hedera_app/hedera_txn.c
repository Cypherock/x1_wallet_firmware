/**
 * @file    hedera_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing for the Hedera app.
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

#include <pb_decode.h>
#include <pb_encode.h>
#include "proto/transaction_body.pb.h"

static hedera_txn_context_t *hedera_txn_context = NULL;

static bool check_which_request(const hedera_query_t *query, pb_size_t which_request) {
    if (which_request != query->sign_txn.which_request) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
    }
    return true;
}

static void send_response(const pb_size_t which_response) {
    hedera_result_t result = init_hedera_result(HEDERA_RESULT_SIGN_TXN_TAG);
    result.sign_txn.which_response = which_response;
    hedera_send_result(&result);
}

static bool validate_request_data(const hedera_sign_txn_request_t *request) {
    if (!hedera_derivation_path_guard(request->initiate.derivation_path, request->initiate.derivation_path_count)) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_DATA);
        return false;
    }
    return true;
}

static bool handle_initiate_query(const hedera_query_t *query) {
    char wallet_name[NAME_SIZE] = "";
    char msg[100] = "";

    if (!check_which_request(query, HEDERA_SIGN_TXN_REQUEST_INITIATE_TAG) ||
        !validate_request_data(&query->sign_txn) ||
        !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id, (uint8_t *)wallet_name, hedera_send_error)) {
        return false;
    }

    snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, HEDERA_NAME, wallet_name);
    if (!core_confirmation(msg, hedera_send_error)) {
        return false;
    }

    set_app_flow_status(HEDERA_SIGN_TXN_STATUS_CONFIRM);
    memcpy(&hedera_txn_context->init_info, &query->sign_txn.initiate, sizeof(hedera_sign_txn_initiate_request_t));

    send_response(HEDERA_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
    delay_scr_init(ui_text_processing, DELAY_SHORT);
    return true;
}

static bool fetch_valid_input(hedera_query_t *query) {
    if (!hedera_get_query(query, HEDERA_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, HEDERA_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
        return false;
    }

    const hedera_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
    if (!txn_data->has_txn_bytes || txn_data->txn_bytes.size > MAX_TXN_SIZE) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_DATA);
        return false;
    }

    // Store raw transaction bytes for signing
    hedera_txn_context->raw_txn_len = txn_data->txn_bytes.size;
    memcpy(hedera_txn_context->raw_txn_bytes, txn_data->txn_bytes.bytes, hedera_txn_context->raw_txn_len);

    // Decode protobuf message for UI display
    pb_istream_t stream = pb_istream_from_buffer(txn_data->txn_bytes.bytes, txn_data->txn_bytes.size);
    if (!pb_decode(&stream, Hedera_TransactionBody_fields, &hedera_txn_context->txn)) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_DECODING_FAILED);
        return false;
    }

    send_response(HEDERA_SIGN_TXN_RESPONSE_UNSIGNED_TXN_ACCEPTED_TAG);
    return true;
}

static bool get_user_verification() {
    const Hedera_TransactionBody *txn = &hedera_txn_context->txn;
    char display_str[128];
    char temp_str[64];

    // Verify Operator Account
    hedera_format_account_id(&txn->transactionID.accountID, temp_str);
    snprintf(display_str, sizeof(display_str), "Operator: %s", temp_str);
    if (!core_scroll_page("Verify Transaction", display_str, hedera_send_error)) return false;

    // Verify Transaction Details based on type
    switch (txn->which_data) {
        case Hedera_TransactionBody_cryptoTransfer_tag: {
            const Hedera_CryptoTransferTransactionBody *transfer = &txn->data.cryptoTransfer;
            for (int i = 0; i < transfer->transfers.accountAmounts_count; i++) {
                const Hedera_AccountAmount *aa = &transfer->transfers.accountAmounts[i];
                if (aa->amount == 0) continue; // Skip operator fee record
                
                hedera_format_account_id(&aa->accountID, temp_str);
                const char *action = (aa->amount > 0) ? "Recipient" : "Sender";
                snprintf(display_str, sizeof(display_str), "%s: %s", action, temp_str);
                if (!core_scroll_page("Verify Transfer", display_str, hedera_send_error)) return false;
                
                hedera_format_tinybars_to_hbar_string(aa->amount, temp_str);
                snprintf(display_str, sizeof(display_str), "Amount: %s", temp_str);
                if (!core_confirmation(display_str, hedera_send_error)) return false;
            }
            break;
        }
        // Add cases for other transaction types (TokenMint, TokenBurn, etc.) here
        default:
            if (!core_confirmation("Verify transaction details", hedera_send_error)) return false;
            break;
    }

    // Verify Fee
    hedera_format_tinybars_to_hbar_string(txn->transactionFee, temp_str);
    snprintf(display_str, sizeof(display_str), "Fee: %s", temp_str);
    if (!core_confirmation(display_str, hedera_send_error)) return false;

    // Verify Memo (if present)
    if (strlen(txn->memo) > 0) {
        if (!core_scroll_page("Memo", txn->memo, hedera_send_error)) return false;
    }

    set_app_flow_status(HEDERA_SIGN_TXN_STATUS_VERIFY);
    return true;
}

static bool sign_txn(hedera_sign_txn_signature_response_signature_t *signature) {
    uint8_t seed[64] = {0};
    if (!reconstruct_seed(hedera_txn_context->init_info.wallet_id, seed, hedera_send_error)) {
        memzero(seed, sizeof(seed));
        return false;
    }

    set_app_flow_status(HEDERA_SIGN_TXN_STATUS_SEED_GENERATED);

    HDNode hdnode = {0};
    if (!derive_hdnode_from_path(
            hedera_txn_context->init_info.derivation_path,
            hedera_txn_context->init_info.derivation_path_count,
            ED25519_NAME,
            seed,
            &hdnode)) {
        memzero(seed, sizeof(seed));
        return false;
    }
    memzero(seed, sizeof(seed));

    // Ed25519 signs the raw message, not a hash of it.
    ed25519_sign(hdnode.private_key,
                 hedera_txn_context->raw_txn_bytes,
                 hedera_txn_context->raw_txn_len,
                 signature->bytes);
    signature->size = HEDERA_SIGNATURE_SIZE;

    memzero(&hdnode, sizeof(hdnode));
    return true;
}

static bool send_signature(hedera_query_t *query, const hedera_sign_txn_signature_response_signature_t *signature) {
    hedera_result_t result = init_hedera_result(HEDERA_RESULT_SIGN_TXN_TAG);
    result.sign_txn.which_response = HEDERA_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

    if (!hedera_get_query(query, HEDERA_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, HEDERA_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
        return false;
    }

    memcpy(&result.sign_txn.signature.signature, signature, sizeof(hedera_sign_txn_signature_response_signature_t));
    hedera_send_result(&result);
    return true;
}

void hedera_sign_transaction(hedera_query_t *query) {
    hedera_txn_context = (hedera_txn_context_t *)malloc(sizeof(hedera_txn_context_t));
    memzero(hedera_txn_context, sizeof(hedera_txn_context_t));
    hedera_sign_txn_signature_response_signature_t signature = {0};

    if (handle_initiate_query(query) &&
        fetch_valid_input(query) &&
        get_user_verification() &&
        sign_txn(&signature) &&
        send_signature(query, &signature)) {
        delay_scr_init(ui_text_check_software_wallet_app, DELAY_TIME);
    }

    if (hedera_txn_context) {
        free(hedera_txn_context);
        hedera_txn_context = NULL;
    }
}