/**
 * @file    btc.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include "btc.h"
#include "utils.h"
#include "string.h"
#include "sha2.h"
#include "base58.h"
#include "logger.h"
#include "segwit_addr.h"
#include "assert_conf.h"


/**
 * @brief Converts the byte array to lendian decimal.
 * @details
 *
 * @param [in] bytes Byte array.
 * @param [in] len Length of the byte array.
 *
 * @return Converted decimal in lendian format.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t lendian_to_dec(const uint8_t *bytes, uint8_t len)
{
    uint64_t result = 0;
    while (len--)
        result += ((uint64_t)bytes[len] << (8 * len));
    return result;
}

/**
 * @brief Copies the byte values from source to destination after offset.
 * @details
 *
 * @param [in] dst          Destination address.
 * @param [in] src          Source address.
 * @param [in, out] offset  Offset for destination.
 * @param [in] len          No of bytes to copy.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void m_memcpy(uint8_t *dst, const uint8_t *src, uint32_t *offset, const uint32_t len)
{
    memcpy(dst + *offset, src, len);
    *offset += len;
}

/**
 * @brief Check if unsigned transaction has segregated witnesses.
 * @details
 *
 * @param [in] utxn_ptr Pointer to unsigned_txn instance.
 *
 * @return If unsigned transaction has segwit
 * @retval 1 if unsigned transaction has segwit
 * @retval 0 if unsigned transaction does not have segwit
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static int unsigned_txn_has_segwit(const unsigned_txn *utxn_ptr)
{
    for (int i = 0; i < utxn_ptr->input_count[0]; i++) {
        if (utxn_ptr->input[i].segwit[0])
            return 1;
    }
    return 0;
}

/**
 * @brief Helper function to fill transaction preimage from unsigned transaction.
 * @details
 *
 * @param utxn_ptr Pointer to unsigned_txn instance to get all values.
 * @param preimage Pointer to txn_preimage instance to fill all values.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void fill_preimage(const unsigned_txn *utxn_ptr, txn_preimage *preimage)
{
    uint32_t len = 0, offset = 0;
    uint8_t *bytes;

    len = utxn_ptr->input_count[0] * (sizeof(utxn_ptr->input->previous_output_index)
          + sizeof(utxn_ptr->input->previous_txn_hash));
    bytes = (uint8_t *)malloc(len * sizeof(uint8_t));
    ASSERT(bytes != NULL);
    for (int i = 0; i < utxn_ptr->input_count[0]; i++) {
        m_memcpy(bytes, utxn_ptr->input[i].previous_txn_hash, &offset, sizeof(utxn_ptr->input->previous_txn_hash));
        m_memcpy(bytes, utxn_ptr->input[i].previous_output_index, &offset, sizeof(utxn_ptr->input->previous_output_index));
    }
    sha256_Raw(bytes, len, preimage->hash_prevouts);
    sha256_Raw(preimage->hash_prevouts, sizeof(preimage->hash_prevouts), preimage->hash_prevouts);
    free(bytes);

    offset = 0;
    len = utxn_ptr->input_count[0] * (sizeof(utxn_ptr->input->sequence));
    bytes = (uint8_t *)malloc(len * sizeof(uint8_t));
    ASSERT(bytes != NULL);
    for (int i = 0 ; i < utxn_ptr->input_count[0]; i++)
        m_memcpy(bytes, utxn_ptr->input[i].sequence, &offset, sizeof(utxn_ptr->input->sequence));
    sha256_Raw(bytes, len, preimage->hash_sequence);
    sha256_Raw(preimage->hash_sequence, sizeof(preimage->hash_sequence), preimage->hash_sequence);
    free(bytes);

    offset = 0;
    len = utxn_ptr->output_count[0] * (sizeof(utxn_ptr->output->value)
          + sizeof(utxn_ptr->output->script_length) + sizeof(utxn_ptr->output->script_public_key));
    bytes = (uint8_t *)malloc(len * sizeof(uint8_t));
    ASSERT(bytes != NULL);
    for (int i = 0; i < utxn_ptr->output_count[0]; i++) {
        m_memcpy(bytes, utxn_ptr->output[i].value, &offset, sizeof(utxn_ptr->output->value));
        m_memcpy(bytes, utxn_ptr->output[i].script_length, &offset, sizeof(utxn_ptr->output->script_length));
        m_memcpy(bytes, utxn_ptr->output[i].script_public_key, &offset, utxn_ptr->output[i].script_length[0]);
    }
    sha256_Raw(bytes, offset, preimage->hash_outputs);
    sha256_Raw(preimage->hash_outputs, sizeof(preimage->hash_outputs), preimage->hash_outputs);
    free(bytes);

    memcpy(preimage->locktime, utxn_ptr->locktime, sizeof(utxn_ptr->locktime));
    memcpy(preimage->sighash, utxn_ptr->sighash, sizeof(utxn_ptr->sighash));
    memcpy(preimage->network_version, utxn_ptr->network_version, sizeof(utxn_ptr->network_version));
    preimage->filled = 1;
}

/**
 * @brief Convert transaction preimage to byte array.
 * @details
 *
 * @param [in] src          Pointer to unsigned_txn_input instance.
 * @param [in] bytes        Byte array pointer to store bytes.
 * @param [out] preimage    Pointer to txn_preimage instance.
 *]
 * @return Offset used while conversion.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint32_t serialise_txn_preimage_to_sign(const unsigned_txn_input *src, const txn_preimage *preimage, uint8_t *bytes)
{
    uint32_t offset = 0;

    m_memcpy(bytes, preimage->network_version, &offset, sizeof(preimage->network_version));
    m_memcpy(bytes, preimage->hash_prevouts, &offset, sizeof(preimage->hash_prevouts));
    m_memcpy(bytes, preimage->hash_sequence, &offset, sizeof(preimage->hash_sequence));
    m_memcpy(bytes, src->previous_txn_hash, &offset, sizeof(src->previous_txn_hash));
    m_memcpy(bytes, src->previous_output_index, &offset, sizeof(src->previous_output_index));

    bytes[offset++] = src->script_length[0] - 1 + 2 + 2;
    hex_string_to_byte_array("76a9", 4, bytes + offset);
    offset += 2;
    m_memcpy(bytes, src->script_public_key + 1, &offset, src->script_length[0] - 1);
    hex_string_to_byte_array("88ac", 4, bytes + offset);
    offset += 2;

    m_memcpy(bytes, src->value, &offset, sizeof(src->value));
    m_memcpy(bytes, src->sequence, &offset, sizeof(src->sequence));
    m_memcpy(bytes, preimage->hash_outputs, &offset, sizeof(preimage->hash_outputs));
    m_memcpy(bytes, preimage->locktime, &offset, sizeof(preimage->locktime));
    m_memcpy(bytes, preimage->sighash, &offset, sizeof(preimage->sighash));

    return offset;
}

/**
 * @brief Convert transaction output to byte array.
 * @details
 *
 * @param [out] dst     Byte array pointer.
 * @param [in] src      Pointer to txn_output instance
 *
 * @return Size of converted byte array.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint32_t serialise_txn_output(uint8_t *dst, const txn_output *src)
{
    memcpy(dst, src->value, sizeof(src->value));
    memcpy((dst + sizeof(src->value)), src->script_length, sizeof(src->script_length));
    memcpy((dst + sizeof(src->value) + sizeof(src->script_length)),
           src->script_public_key, src->script_length[0]);
    return (sizeof(src->value) + sizeof(src->script_length) + src->script_length[0]);
}

/**
 * @brief Converts unsigned transaction to byte array.
 * @details
 *
 * @param [in] utxn_ptr             Pointer to unsigned_txn instance to convert.
 * @param [in] input_index          Input index.
 * @param [out] serialized_utxn     Converted byte array.
 *
 * @return Offset used while conversion.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint32_t serialize_unsigned_txn_to_sign(const unsigned_txn *utxn_ptr, const uint8_t input_index,
                                               uint8_t *serialized_utxn)
{
    uint32_t offset = 0;

    m_memcpy(serialized_utxn, utxn_ptr->network_version, &offset, sizeof(utxn_ptr->network_version));
    m_memcpy(serialized_utxn, utxn_ptr->input_count, &offset, sizeof(utxn_ptr->input_count));

    uint8_t inputIndex = 0;
    for (; inputIndex < *utxn_ptr->input_count; inputIndex++) {
        unsigned_txn_input *input = &utxn_ptr->input[inputIndex];

        m_memcpy(serialized_utxn, input->previous_txn_hash, &offset, sizeof(input->previous_txn_hash));
        m_memcpy(serialized_utxn, input->previous_output_index, &offset, sizeof(input->previous_output_index));
        if (inputIndex == input_index) {
            m_memcpy(serialized_utxn, input->script_length, &offset, sizeof(input->script_length));
            m_memcpy(serialized_utxn, input->script_public_key, &offset, input->script_length[0]);
        } else {
            serialized_utxn[offset++] = 0x00;
        }
        m_memcpy(serialized_utxn, input->sequence, &offset, sizeof(input->sequence));
    }

    m_memcpy(serialized_utxn, utxn_ptr->output_count, &offset, sizeof(utxn_ptr->output_count));

    uint8_t outputIndex = 0;
    for (; outputIndex < *utxn_ptr->output_count; outputIndex++)
        offset += serialise_txn_output(serialized_utxn + offset, &utxn_ptr->output[outputIndex]);

    m_memcpy(serialized_utxn, utxn_ptr->locktime, &offset, sizeof(utxn_ptr->locktime));
    m_memcpy(serialized_utxn, utxn_ptr->sighash, &offset, sizeof(utxn_ptr->sighash));

    return offset;
}

int32_t byte_array_to_unsigned_txn(const uint8_t *utxn_byte_array, const uint32_t size, unsigned_txn *utxn_ptr)
{
    int64_t offset = 0, len = 0;

    s_memcpy(utxn_ptr->network_version, utxn_byte_array,
             size, sizeof(utxn_ptr->network_version), &offset);
    s_memcpy(utxn_ptr->input_count, utxn_byte_array,
             size, sizeof(utxn_ptr->input_count), &offset);

    len = (*utxn_ptr->input_count) * sizeof(unsigned_txn_input);
    utxn_ptr->input = (unsigned_txn_input *) cy_malloc(len);

    uint8_t inputIndex = 0;
    for (; inputIndex < *utxn_ptr->input_count; inputIndex++) {
        unsigned_txn_input *input = &utxn_ptr->input[inputIndex];
        s_memcpy(input->previous_txn_hash, utxn_byte_array, size, sizeof(input->previous_txn_hash), &offset);
        s_memcpy(input->previous_output_index, utxn_byte_array, size, sizeof(input->previous_output_index), &offset);
        s_memcpy(input->value, utxn_byte_array, size, sizeof(input->value), &offset);
        s_memcpy(input->script_length, utxn_byte_array, size, sizeof(input->script_length), &offset);
        s_memcpy(input->script_public_key, utxn_byte_array, size, input->script_length[0], &offset);
        s_memcpy(input->sequence, utxn_byte_array, size, sizeof(input->sequence), &offset);

        if (input->script_public_key[0] == 0 && (input->script_length[0] == 22 || input->script_length[0] == 42))
            input->segwit[0] = 0x01;
        else
            input->segwit[0] = 0x00;
    }

    s_memcpy(utxn_ptr->output_count, utxn_byte_array, size, sizeof(utxn_ptr->output_count), &offset);

    len = (*utxn_ptr->output_count) * sizeof(txn_output);
    utxn_ptr->output = (txn_output *) cy_malloc(len);
    uint8_t outputIndex = 0;
    for (; outputIndex < *utxn_ptr->output_count; outputIndex++) {
        txn_output *output = &utxn_ptr->output[outputIndex];
        s_memcpy(output->value, utxn_byte_array, size, sizeof(output->value), &offset);
        s_memcpy(output->script_length, utxn_byte_array, size, sizeof(output->script_length), &offset);
        s_memcpy(output->script_public_key, utxn_byte_array, size, output->script_length[0], &offset);
    }

    s_memcpy(utxn_ptr->locktime, utxn_byte_array, size, sizeof(utxn_ptr->locktime), &offset);
    s_memcpy(utxn_ptr->sighash, utxn_byte_array, size, sizeof(utxn_ptr->sighash), &offset);
    return offset;
}

int sig_from_unsigned_txn(const unsigned_txn *utxn_ptr, txn_metadata *txn_metadata_ptr,
                          const uint32_t index, const char *mnemonic, const char *passphrase,
                          txn_preimage *preimage, uint8_t *script_sig)
{
    uint8_t *byte_array_to_sign;
    uint8_t digest[32];
    uint8_t sig[65];
    uint8_t der_sig[128];
    uint8_t der_sig_len;

    uint32_t len = 0;
    uint32_t byte_array_len = 0;
    HDNode hdnode;

    if (!script_sig)
        return -1;

    int witness = unsigned_txn_has_segwit(utxn_ptr);
    if (witness) {
        if (!preimage)
            return -1;
        if (!(preimage->filled))
            fill_preimage(utxn_ptr, preimage);
    }

    len = 39 + 41 * (*utxn_ptr->input_count) + 34 * (*utxn_ptr->output_count);
    len = len > sizeof(txn_preimage) ? len : sizeof(txn_preimage);
    byte_array_to_sign = (uint8_t *)malloc(len * sizeof(uint8_t));
    ASSERT(byte_array_to_sign != NULL);

    unsigned_txn_input *src = &utxn_ptr->input[index];
    witness = src->segwit[0];

    if (witness) {
        byte_array_len = serialise_txn_preimage_to_sign(src, preimage, byte_array_to_sign);
        dec_to_hex(0x80000054, txn_metadata_ptr->purpose_index, 4);
    } else {
        byte_array_len = serialize_unsigned_txn_to_sign(utxn_ptr, index, byte_array_to_sign);
        dec_to_hex(0x8000002c, txn_metadata_ptr->purpose_index, 4);
    }
    get_address_node(txn_metadata_ptr, index, mnemonic, passphrase, &hdnode);

    sha256_Raw(byte_array_to_sign, byte_array_len, digest);
    sha256_Raw(digest, SHA256_DIGEST_LENGTH, digest);
    ecdsa_sign_digest(&secp256k1, hdnode.private_key, digest, sig, NULL, NULL);
    der_sig_len = ecdsa_sig_to_der(sig, der_sig);

    uint8_t script_sig_len = der_sig_len + 36;      //PUSHDATA Opcode(1) + der_sig_len + SigHash Code(1) + PUSHDATA Opcode(1) + Public Key(33)
    uint8_t pushdata_opcode_der = der_sig_len + 1; //includes der_sig and SigHash Code(1)
    uint8_t sighash_code = 1;
    uint8_t pushdata_opcode_pub = 33;

    memcpy(script_sig, &pushdata_opcode_der, 1);
    memcpy(script_sig + 1, der_sig, der_sig_len);
    memcpy(script_sig + 1 + der_sig_len, &sighash_code, 1);
    memcpy(script_sig + 1 + der_sig_len + 1, &pushdata_opcode_pub, 1);
    memcpy(script_sig + 1 + der_sig_len + 1 + 1, hdnode.public_key, 33);

    free(byte_array_to_sign);
    memzero(&hdnode, sizeof(HDNode));
    return script_sig_len;
}

bool btc_verify_utxo(const uint8_t *raw_txn, const uint32_t size, const unsigned_txn_input *input)
{
    if (!input) return true;
    if (!raw_txn || !size) return false;

    uint8_t hash[SHA256_DIGEST_LENGTH];
    int32_t offset = 0, count, value_offset = 0, start_offset = 0, out_index = U32_READ_LE_ARRAY(input->previous_output_index);
    uint8_t txn_data[size];

    offset += (raw_txn[4] == 0 ? 6 : 4);
    start_offset = offset;
    count = raw_txn[offset++];

    for (int32_t input_index = 0; input_index < count; input_index++) {
        offset += 36;
        offset += (raw_txn[offset] + 1 + 4);
    }
    count = raw_txn[offset++];
    for (int32_t output_index = 0; output_index < count; output_index++) {
        if (output_index == out_index)
            value_offset = offset;
        offset += 8;
        offset += (raw_txn[offset] + 1);
    }

    if (value_offset == 0)
        return false;
    memcpy(txn_data, raw_txn, 4);                                           // network version (first 4 bytes)
    memcpy(txn_data + 4, raw_txn + start_offset, offset - start_offset);    // txin and txout (skip marker & flag)
    memcpy(txn_data + offset - start_offset + 4, raw_txn + size - 4, 4);    // locktime (last 4 bytes)
    sha256_Raw(txn_data, offset - start_offset + 4 + 4, hash);
    sha256_Raw(hash, sizeof(hash), hash);
    if (memcmp(hash, input->previous_txn_hash, sizeof(input->previous_txn_hash)) != 0)      // verify UTXO txn hash
        return false;
    return (memcmp(raw_txn + value_offset, input->value, sizeof(input->value)) == 0);
}

int get_segwit_address(const uint8_t *public_key, uint8_t key_len, const uint32_t coin_index, char *address)
{
    char *hrp;
    uint8_t digest[SHA256_DIGEST_LENGTH];
    uint8_t rip[RIPEMD160_DIGEST_LENGTH];
    if (!public_key || !address)
        return 1;

    if (key_len != 33 && key_len != 65)
        return 1;

    if (key_len == 65) {
        bignum256 y;
        bn_read_be(public_key + 33, &y);
        ((uint8_t*)public_key)[0] = bn_is_odd(&y) ? 0x03 : 0x02;
        key_len = 33;
    }

    if (coin_index == BITCOIN)
        hrp = "bc";
    else
        hrp = "tb";

    sha256_Raw(public_key, key_len, digest);
    ripemd160(digest, SHA256_DIGEST_LENGTH, rip);
    return segwit_addr_encode(address, hrp, 0x00, rip, 20);
}

int get_address(const char* hrp, const uint8_t* script_pub_key, char* address_output)
{
  if (script_pub_key == NULL || address_output == NULL) return 2;

  if (script_pub_key[0] == 0) {
    // Segwit address
    return segwit_addr_encode(address_output, hrp, 0x00, script_pub_key + 2, script_pub_key[1]);
  }

  uint8_t address[SHA3_256_DIGEST_LENGTH];
  uint8_t offset = 1, script_offset = 0, version = 0;

  if (script_pub_key[0] == 0x41) {
    // hash160 P2PK
    hasher_Raw(HASHER_SHA2_RIPEMD, script_pub_key + 1, 65, address + offset); // overwrite with RIPEMD160
    offset += RIPEMD160_DIGEST_LENGTH;
  } else if (script_pub_key[0] == 0x76) {
    script_offset = 3; // P2PKH
  } else if (script_pub_key[0] == 0xa9) {
    script_offset = 2;  // P2SH upto 15 pub keys
    version = 5;     // Version for P2SH: 5
  } else if (script_pub_key[1] > 0x50 && script_pub_key[1] <= 0x53) return -1; // P2MS upto 3 pub keys
  else return -2; // Unknown script type

  if (script_pub_key[0] != 0x41) {
    memcpy(address + offset, script_pub_key + script_offset, RIPEMD160_DIGEST_LENGTH);
    offset += RIPEMD160_DIGEST_LENGTH;
  }
  address[0] = version;

  return base58_encode_check(address, offset, HASHER_SHA2D, address_output, 35);
}

uint64_t btc_get_txn_fee(const unsigned_txn *utxn_ptr)
{
    uint64_t result = 0;

    for (int i = 0; i < utxn_ptr->input_count[0]; i++)
        result += lendian_to_dec(utxn_ptr->input[i].value, sizeof(utxn_ptr->input[i].value));

    for (int i = 0; i < utxn_ptr->output_count[0]; i++)
        result -= lendian_to_dec(utxn_ptr->output[i].value, sizeof(utxn_ptr->output[i].value));

    return result;
}

/**
 * @brief Used to calculate transaction fee threshold
 * @details
 *
 * @param [in] coin_index Coin index in HD-Key derivation path
 *
 * @return constant for fee threshold estimation for corresponding chain
 * @retval
 *
 * @see get_transaction_fee_threshold for usage
 * @since v1.0.0
 *
 * @note
 */
static uint64_t get_max_coin_fee(const uint32_t coin_index) {
        switch (coin_index) {
        case 0x80000000:
            return 2000000;
        case 0x80000001:
            return 10000000;
        case 0x80000002:
            return 67000000;
        case 0x80000003:
            return 1200000000000;
        case 0x80000005:
            return 45000000;
        default: {
            ASSERT(false);
            return 0;
        }
    }
}

/**
 * @brief Returns transaction weight for a given transaction
 * @details Weight is required for fee estimation and signifies how much time
 *          would it take to execute a txn in block
 *
 * @param [in] unsigned_txn_ptr Instance of unsigned_txn
 *
 * @return weight of the transaction
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint32_t get_transaction_weight(const unsigned_txn *unsigned_txn_ptr) {

    uint8_t segwit_count = 0;
    uint32_t weight = 0;

    //TODO:Replace multiple instructions with single macro for weight
    weight += 4; // network version size
    weight += 1; // input count size

    for (uint8_t input_index = 0; input_index < unsigned_txn_ptr->input_count[0]; input_index++) {
        weight += 32;                                                    // previous transaction hash
        weight += 4;                                                     // previous output index
        weight += 1;                                                     // script length size
        weight += unsigned_txn_ptr->input[input_index].script_length[0]; // Size of script
        weight += 4;                                                     // sequence

        if (unsigned_txn_ptr->input[input_index].segwit[0] == 1) // Check if current input is segwit or not
            segwit_count++;
    }

    weight += 1; // output count size

    for (uint8_t output_index = 0; output_index < unsigned_txn_ptr->output_count[0]; output_index++) {
        weight += 8;                                                       // value size
        weight += 1;                                                       //  script length size
        weight += unsigned_txn_ptr->output[output_index].script_length[0]; // Output script length
    }

    weight += 4;         // locktime
    weight = 4 * weight; // As per standard non segwit transaction size is multiplied by 4

    if (segwit_count > 0) {
        weight += 2;                    // Segwit headers
        weight += (106 * segwit_count); // Adding sizes of all witnesses for all inputs
    }

    return weight;
}

uint64_t get_transaction_fee_threshold(const unsigned_txn *unsigned_txn_ptr, const uint32_t coin_index) {
    return (get_max_coin_fee(coin_index) / 1000) * (get_transaction_weight(unsigned_txn_ptr) / 4);
}

bool btc_validate_unsigned_txn(const unsigned_txn *utxn_ptr) {
    if (utxn_ptr->input_count[0] == 0 || utxn_ptr->output_count[0] == 0) return false;
    uint8_t zero_value_transaction = 1;

    // P2PK 68, P2PKH 25 (21 excluding OP_CODES), P2WPKH 22, P2MS ~, P2SH 23 (21 excluding OP_CODES)
    // refer https://learnmeabitcoin.com/technical/script for explaination
    // Currently the device can spend P2PKH or P2WPKH UTXOs only
    for (int i = 0; i < utxn_ptr->input_count[0]; i++)
        if (utxn_ptr->input[i].script_length[0] != 22 && utxn_ptr->input[i].script_length[0] != 25)
            return false;

    for (int i = 0; i < utxn_ptr->output_count[0]; i++) {
        if (utxn_ptr->output[i].script_public_key[0] == OP_RETURN &&
            !is_zero(utxn_ptr->output[i].value, sizeof utxn_ptr->output[i].value))
            return false;
        if (!is_zero(utxn_ptr->output[i].value, sizeof utxn_ptr->output[i].value))
            zero_value_transaction = 0;
    }

    if (zero_value_transaction == 1) return false;

    // Only accept SIGHASH_ALL for sighash type
    // More info: https://wiki.bitcoinsv.io/index.php/SIGHASH_flags
    if (BYTE_ARRAY_TO_UINT32(utxn_ptr->sighash) != 0x01000000) return false;
    return true;
}

bool validate_change_address(const unsigned_txn *utxn_ptr, const txn_metadata *txn_metadata_ptr,
                             const char* mnemonic, const char* passphrase) {
    if (txn_metadata_ptr->change_count[0] == 0) return true;
    uint8_t index = utxn_ptr->output_count[0] - 1, *change_address;
    uint8_t digest[SHA256_DIGEST_LENGTH];
    uint8_t rip[RIPEMD160_DIGEST_LENGTH];
    HDNode hdnode;

    if (utxn_ptr->output[index].script_public_key[0] == OP_RETURN) return false;

    if (utxn_ptr->output[index].script_length[0] == 22)
        change_address = utxn_ptr->output[index].script_public_key + 2;
    else if (utxn_ptr->output[index].script_length[0] == 25)
        change_address = utxn_ptr->output[index].script_public_key + 3;
    else return false;

    if (U32_READ_BE_ARRAY(txn_metadata_ptr->coin_index) == BITCOIN || U32_READ_BE_ARRAY(txn_metadata_ptr->coin_index) == BTC_TEST)
        dec_to_hex(0x80000054, (uint8_t *) txn_metadata_ptr->purpose_index, 4);
    get_address_node(txn_metadata_ptr, -1, mnemonic, passphrase, &hdnode);
    memzero(hdnode.chain_code, sizeof(hdnode.chain_code));
    memzero(hdnode.private_key, sizeof(hdnode.private_key));
    memzero(hdnode.private_key_extension, sizeof(hdnode.private_key_extension));
    sha256_Raw(hdnode.public_key, sizeof(hdnode.public_key), digest);
    ripemd160(digest, SHA256_DIGEST_LENGTH, rip);
    return (memcmp(rip, change_address, sizeof(rip)) == 0);
}