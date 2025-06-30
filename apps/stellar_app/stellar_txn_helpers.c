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

// Helper function 1: Parse memo data
static int parse_memo_data(const uint8_t *xdr, int *pos, int xdr_len, stellar_transaction_t *tx) {
    tx->memo_type = (stellar_memo_type_t)read_uint32_be_pos(xdr, pos);
    
    switch (tx->memo_type) {
        case MEMO_NONE:
            break;
            
        case MEMO_TEXT:
            {
                char temp_memo[64];
                int memo_len = read_string_pos(xdr, pos, temp_memo, sizeof(temp_memo), xdr_len);
                if (memo_len < 0) {
                    return -1;
                }
                strncpy(tx->memo.text, temp_memo, sizeof(tx->memo.text) - 1);
                tx->memo.text[sizeof(tx->memo.text) - 1] = '\0';
            }
            break;
            
        case MEMO_ID:
            tx->memo.id = read_uint64_be_pos(xdr, pos);
            break;
            
        case MEMO_HASH:
        case MEMO_RETURN:
            if (*pos + 32 > xdr_len) {
                return -1;
            }
            memcpy(tx->memo.hash, xdr + *pos, 32);
            *pos += 32;
            break;
            
        default:
            return -1;
    }
    
    return 0;
}

// Helper function 2: Parse operation data
static int parse_operation_data(const uint8_t *xdr, int *pos, int xdr_len, stellar_transaction_t *tx, stellar_payment_t *payment) {
    // Parse Operations count
    tx->operation_count = read_uint32_be_pos(xdr, pos);
    
    if (tx->operation_count == 0) {
        return -1;
    }
    
    // Parse First Operation
    uint32_t has_source_account = read_uint32_be_pos(xdr, pos);
    
    if (has_source_account == 1) {
        *pos += 36;
    } else if (has_source_account != 0) {
        return -1;
    }
    
    // Parse operation type
    uint32_t operation_type = read_uint32_be_pos(xdr, pos);
    
    if (operation_type != 1 && operation_type != 0) {
        return -1;
    }
    tx->operation_type = operation_type;

    if (operation_type == 0) {  // CREATE_ACCOUNT
        uint32_t dest_account_type = read_uint32_be_pos(xdr, pos);
        if (dest_account_type != 0) {
            return -1;
        }
        read_account_pos(xdr, pos, payment->destination);
        payment->amount = read_uint64_be_pos(xdr, pos);
        return 0;  // Early return for CREATE_ACCOUNT
    } 
    
    // PAYMENT Operation
    uint32_t dest_account_type = read_uint32_be_pos(xdr, pos);
    if (dest_account_type != 0) {
        return -1;
    }
    
    read_account_pos(xdr, pos, payment->destination);
    
    uint32_t asset_type = read_uint32_be_pos(xdr, pos);
    if (asset_type != 0) {
        return -1;
    }
    
    payment->amount = read_uint64_be_pos(xdr, pos);
    return 0;
}

//Parses transaction
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
    if (envelope_type != 2) {
        return -1;
    }
    
    // 2. Parse Source Account
    uint32_t source_account_type = read_uint32_be_pos(xdr, &pos);
    if (source_account_type != 0) {
        return -1;
    }
    
    read_account_pos(xdr, &pos, tx->source_account);
        
    // 3. Parse Fee (4 bytes)
    tx->fee = read_uint32_be_pos(xdr, &pos);
    
    // 4. Parse Sequence Number (8 bytes)
    tx->sequence_number = read_uint64_be_pos(xdr, &pos);
    
    // 5. Parse Preconditions
    uint32_t preconditions_type = read_uint32_be_pos(xdr, &pos);
    
    if (preconditions_type == 1) {
        pos += 16;
    } else if (preconditions_type != 0) {
        return -1;
    }
    
    // 6. Parse Memo
    if (parse_memo_data(xdr, &pos, xdr_len, tx) != 0) {
        return -1;
    }
    
    // 7. Parse Operations
    int result = parse_operation_data(xdr, &pos, xdr_len, tx, payment);
    if (result != 0) {
        return result;
    }
    
    // 8. Skip transaction extension
    if (pos + 4 <= xdr_len) {
        read_uint32_be_pos(xdr, &pos);
    }
    
    return 0; 
}