/**
 * @file    hedera_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for Hedera app internal operations.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#ifndef HEDERA_PRIV_H
#define HEDERA_PRIV_H

#include "hedera/core.pb.h"
#include "hedera/sign_txn.pb.h"
#include "proto/transaction_body.pb.h" // Nanopb generated header
#include "hedera_context.h"

// Context for the transaction signing flow
typedef struct {
    // The structure holds the wallet information of the transaction.
    hedera_sign_txn_initiate_request_t init_info;

    // Decoded protobuf transaction body for UI display
    Hedera_TransactionBody txn;

    // Raw serialized transaction bytes received from host. This is what we sign.
    uint8_t raw_txn_bytes[MAX_TXN_SIZE];
    size_t raw_txn_len;

} hedera_txn_context_t;

/* --- FUNCTION PROTOTYPES --- */

// Handler for public key derivation flows
void hedera_get_pub_keys(hedera_query_t *query);

// Handler for transaction signing flows
void hedera_sign_transaction(hedera_query_t *query);

#endif /* HEDERA_PRIV_H */