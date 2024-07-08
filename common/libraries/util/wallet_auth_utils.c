/**
 * @file    wallet_utilities.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "wallet_auth_utils.h"

wallet_auth_t auth = {0};

bool verify_wallet_auth_inputs(){
    ASSERT(auth.challenge != NULL);
    ASSERT(auth.wallet_id != NULL);
    ASSERT(auth.challenge_size != NULL);
    ASSERT(auth.challenge_size <= CHALLENGE_SIZE_LIMIT);

    return true;
}

bool wallet_auth_get_entropy(){
    SecureMsg msgs[1] = {0};
    msgs[0].plain_data_size = WALLET_ID_SIZE;
    memcpy(msgs[0].plain_data, auth.wallet_id, WALLET_ID_SIZE);
    
    card_error_type_e status =
    card_fetch_encrypt_data(auth.wallet_id, msgs, 1);

    if (status != CARD_OPERATION_SUCCESS || msgs[0].encrypted_data_size > ENTROPY_SIZE_LIMIT) {
        printf("Secure msg encryption issue %d", status);
        return false;
    }
        
    memcpy(auth.entropy, msgs[0].encrypted_data, msgs[0].encrypted_data_size);
    auth.entropy_size = msgs[0].encrypted_data_size;

    return true;
}

bool wallet_auth_get_pairs(){
    mnemonic_to_seed(auth.entropy, "", auth.private_key, NULL);
    ed25519_publickey(auth.private_key, auth.public_key);

    memcpy(auth.public_key, auth.public_key, sizeof(ed25519_public_key));
}

bool wallet_auth_get_signature(){ 
    const uint16_t unsigned_txn_size = auth.challenge_size + WALLET_ID_SIZE;
    uint8_t unsigned_txn[unsigned_txn_size];

    memcpy(unsigned_txn, auth.challenge, auth.challenge_size);
    memcpy(unsigned_txn + auth.challenge_size, auth.wallet_id, WALLET_ID_SIZE);
    
    ed25519_sign(unsigned_txn, unsigned_txn_size, auth.private_key, auth.public_key, auth.signature);

    int valid = ed25519_sign_open(unsigned_txn, unsigned_txn_size, auth.public_key, auth.signature);
    if (0 != valid){
        printf("Signature generation err: %d", valid);
        return false;
    }

    return true;
}


wallet_auth_error_type_e wallet_login(){
    if (!verify_wallet_auth_inputs(auth)) {
        return WALLET_AUTH_INPUTS_INVALID;
    }

    switch (auth.auth_type) {
        case WALLET_AUTH_OWNER:
            if (!wallet_auth_get_entropy(auth) || wallet_auth_get_pairs(auth) || !wallet_auth_get_signature(auth))
                return WALLET_AUTH_ERR_OWNER;
            break;

        case WALLET_AUTH_NOMINEE:
            if (!wallet_auth_get_signature(auth))
                return WALLET_AUTH_ERR_NOMINEE;

    default: {
      return WALLET_AUTH_TYPE_INVALID;
    }
  }
  auth.status = WALLET_AUTH_OK;
  
  memset(&auth, 0, sizeof(auth));
}