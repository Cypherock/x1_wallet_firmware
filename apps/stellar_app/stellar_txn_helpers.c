/**
 * @file    stellar_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the STELLAR app for txn signing flow
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2025 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "stellar_txn_helpers.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stellar_context.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static uint32_t read_uint32_be_pos(const uint8_t *data, int *pos) {
    uint32_t value = (data[*pos] << 24) | (data[*pos + 1] << 16) | 
                     (data[*pos + 2] << 8) | data[*pos + 3];
    *pos += 4;
    return value;
}

static uint64_t read_uint64_be_pos(const uint8_t *data, int *pos) {
    uint64_t high = read_uint32_be_pos(data, pos);
    uint64_t low = read_uint32_be_pos(data, pos);
    return (high << 32) | low;
}

static void read_account_pos(const uint8_t *data, int *pos, uint8_t *account) {
    memcpy(account, data + *pos, 32);
    *pos += 32;
}

static int read_string_pos(const uint8_t *data, int *pos, char *str, int max_len, int data_len) {
    if (*pos + 4 > data_len) return -1;
    
    uint32_t len = read_uint32_be_pos(data, pos);
    if (len >= max_len || *pos + len > data_len) return -1;
    
    // Calculate padded length (round up to 4-byte boundary)
    int padded_len = ((len + 3) / 4) * 4;
    if (*pos + padded_len > data_len) return -1;
    
    memcpy(str, data + *pos, len);
    str[len] = '\0';
    *pos += padded_len;  // Skip data + padding in one go
    
    return len;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

int stellar_parse_transaction(const uint8_t *xdr, int xdr_len, stellar_transaction_t *tx, stellar_payment_t *payment) {
    if (!xdr || !tx || !payment || xdr_len < 60) {
        return -1;
    }
    
    int pos = 0;
    
    // Clear structures
    memset(tx, 0, sizeof(stellar_transaction_t));
    memset(payment, 0, sizeof(stellar_payment_t));
    
    // 1. Parse Envelope Type (4 bytes)
    uint32_t envelope_type = read_uint32_be_pos(xdr, &pos);
    
    if (envelope_type != 2) {  // ENVELOPE_TYPE_TX (TransactionV1Envelope)
        return -1;
    }
    
    // 2. Parse Source Account
    // First check account type (should be 0 for KEY_TYPE_ED25519)
    uint32_t source_account_type = read_uint32_be_pos(xdr, &pos);
    
    if (source_account_type != 0) {
        return -1;
    }
    
    // Read 32-byte public key
    read_account_pos(xdr, &pos, tx->source_account);
        
    // 3. Parse Fee (4 bytes)
    tx->fee = read_uint32_be_pos(xdr, &pos);
    
    // 4. Parse Sequence Number (8 bytes)
    tx->sequence_number = read_uint64_be_pos(xdr, &pos);
    
    // 5. Parse Preconditions
    uint32_t preconditions_type = read_uint32_be_pos(xdr, &pos);
    
    if (preconditions_type == 1) {
        // TimeBounds present - skip 16 bytes (min_time + max_time)
        pos += 16;
    } else if (preconditions_type != 0) {
        return -1;
    }
    
    // 6. Parse Memo
    tx->memo_type = (stellar_memo_type_t)read_uint32_be_pos(xdr, &pos);
    
    switch (tx->memo_type) {
        case MEMO_NONE:
            break;
            
        case MEMO_TEXT:
            // Increase buffer size for memo text
            {
                char temp_memo[64];  // Temporary larger buffer
                int memo_len = read_string_pos(xdr, &pos, temp_memo, sizeof(temp_memo), xdr_len);
                if (memo_len < 0) {
                    return -1;
                }
                // Copy to transaction structure (truncate if needed)
                strncpy(tx->memo.text, temp_memo, sizeof(tx->memo.text) - 1);
                tx->memo.text[sizeof(tx->memo.text) - 1] = '\0';
            }
            break;
            
        case MEMO_ID:
            tx->memo.id = read_uint64_be_pos(xdr, &pos);
            break;
            
        case MEMO_HASH:
        case MEMO_RETURN:
            if (pos + 32 > xdr_len) {
                return -1;
            }
            memcpy(tx->memo.hash, xdr + pos, 32);
            pos += 32;
            break;
            
        default:
            return -1;
    }
    
    // 7. Parse Operations
    tx->operation_count = read_uint32_be_pos(xdr, &pos);
    
    if (tx->operation_count == 0) {
        return -1;
    }
    
    if (tx->operation_count > 1) {
        return -1;  // Currently only support single operation
    }
    
    // 8. Parse First Operation
    // Check if operation has source account
    uint32_t has_source_account = read_uint32_be_pos(xdr, &pos);
    
    if (has_source_account == 1) {
        // Skip operation source account (4 bytes type + 32 bytes account)
        pos += 36;
    } else if (has_source_account != 0) {
        return -1;
    }
    
    // Parse operation type
    uint32_t operation_type = read_uint32_be_pos(xdr, &pos);
    
    if (operation_type != 1 && operation_type != 0) {  // Only support PAYMENT and CREATE_ACCOUNT operations for now
        return -1;
    }
    tx->operation_type = operation_type;

    if (operation_type == 0) {  // CREATE_ACCOUNT
        // Parse CreateAccount operation
        // Destination account type (4 bytes)
        uint32_t dest_account_type = read_uint32_be_pos(xdr, &pos);
    
        if (dest_account_type != 0) {
            return -1;
        }
    
        // Destination account (32 bytes)  
         read_account_pos(xdr, &pos, payment->destination);
        // Starting balance (8 bytes in stroops) - reuse amount field
        payment->amount = read_uint64_be_pos(xdr, &pos);
    
        return 0;  // Early return for CREATE_ACCOUNT
    } 
    
    // 9. Parse Payment Operation
    // Destination account type
    uint32_t dest_account_type = read_uint32_be_pos(xdr, &pos);
    
    if (dest_account_type != 0) {
        return -1;
    }
    
    // Destination account (32 bytes)
    read_account_pos(xdr, &pos, payment->destination);
    
    // Asset type
    uint32_t asset_type = read_uint32_be_pos(xdr, &pos);
    
    if (asset_type != 0) {  // Only support native XLM for now
        return -1;
    }
    
    // Amount (8 bytes in stroops)
    payment->amount = read_uint64_be_pos(xdr, &pos);
    
    // 10. Skip transaction extension (should be 4 bytes of zeros)
    if (pos + 4 <= xdr_len) {
        read_uint32_be_pos(xdr, &pos);
       // Extension should be 0, but continue processing regardless
    }
    
    return 0; 
}