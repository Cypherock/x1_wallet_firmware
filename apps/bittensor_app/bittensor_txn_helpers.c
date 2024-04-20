/**
 * @file    bittensor_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper implementation for interpreting and signing Bittensor
 *          transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

#include "bittensor_txn_helpers.h"

#include <string.h>

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
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
// uint16_t get_compact_array_size(const uint8_t *data,
//                                 uint16_t *size,
//                                 int *error) {
//   uint16_t offset = 0;
//   uint32_t value = 0;
//   *error = 0;

//   while (offset < 3) {
//     value |= (*(data + offset) & 0x7F) << offset * 7;
//     if ((*(data + offset) & 0x80) == 0)
//       break;
//     offset++;
//   }bittensor_get_derivation_depth

//   if (value > UINT16_MAX)
//     *error = SOL_D_COMPACT_U16_OVERFLOW;    // overflow

//   *size = value;
//   return offset + 1;
// }

int bittensor_byte_array_to_unsigned_txn(uint8_t *byte_array,
                                         uint16_t byte_array_size,
                                         bittensor_unsigned_txn *utxn) {
  if (byte_array == NULL || utxn == NULL)
    return SOL_ERROR;
  memzero(utxn, sizeof(bittensor_unsigned_txn));

  uint16_t offset = 0;
  int error = 0;

  // Message header
  utxn->required_signatures_count = *(byte_array + offset++);
  utxn->read_only_accounts_require_signature_count = *(byte_array + offset++);
  utxn->read_only_accounts_not_require_signature_count =
      *(byte_array + offset++);

  // Account addresses
  offset += get_compact_array_size(
      byte_array + offset, &(utxn->account_addresses_count), &error);
  if (error != SOL_OK)
    return error;
  if (utxn->account_addresses_count == 0)
    return SOL_D_MIN_LENGTH;

  utxn->account_addresses = byte_array + offset;
  offset += utxn->account_addresses_count * BITTENSOR_ACCOUNT_ADDRESS_LENGTH;

  // Blockhash
  utxn->blockhash = byte_array + offset;
  offset += BITTENSOR_BLOCKHASH_LENGTH;

  // Instructions: Currently expecting count to be only 1. TODO: Handle batch
  // instructions
  offset += get_compact_array_size(
      byte_array + offset, &(utxn->instructions_count), &error);
  if (error != SOL_OK)
    return error;
  if (utxn->instructions_count == 0)
    return SOL_D_MIN_LENGTH;

  utxn->instruction.program_id_index = *(byte_array + offset++);

  offset +=
      get_compact_array_size(byte_array + offset,
                             &(utxn->instruction.account_addresses_index_count),
                             &error);
  if (error != SOL_OK)
    return error;
  if (utxn->instruction.account_addresses_index_count == 0)
    return SOL_D_MIN_LENGTH;

  utxn->instruction.account_addresses_index = byte_array + offset;
  offset += utxn->instruction.account_addresses_index_count;
  offset += get_compact_array_size(
      byte_array + offset, &(utxn->instruction.opaque_data_length), &error);
  if (error != SOL_OK)
    return error;
  if (utxn->instruction.opaque_data_length == 0)
    return SOL_D_MIN_LENGTH;

  utxn->instruction.opaque_data = byte_array + offset;
  offset += utxn->instruction.opaque_data_length;

  uint32_t instruction_enum = U32_READ_LE_ARRAY(utxn->instruction.opaque_data);

  uint8_t system_program_id[BITTENSOR_ACCOUNT_ADDRESS_LENGTH] = {
      0};    // System instruction address
  if (memcmp(utxn->account_addresses + utxn->instruction.program_id_index *
                                           BITTENSOR_ACCOUNT_ADDRESS_LENGTH,
             system_program_id,
             BITTENSOR_ACCOUNT_ADDRESS_LENGTH) == 0) {
    switch (instruction_enum) {
      case SSI_TRANSFER:    // transfer instruction
        utxn->instruction.program.transfer.funding_account =
            utxn->account_addresses +
            (*(utxn->instruction.account_addresses_index + 0) *
             BITTENSOR_ACCOUNT_ADDRESS_LENGTH);
        utxn->instruction.program.transfer.recipient_account =
            utxn->account_addresses +
            (*(utxn->instruction.account_addresses_index + 1) *
             BITTENSOR_ACCOUNT_ADDRESS_LENGTH);
        utxn->instruction.program.transfer.lamports =
            U64_READ_LE_ARRAY(utxn->instruction.opaque_data + 4);
        break;

      default:
        break;
    }
  }

  return ((offset <= byte_array_size) && (offset > 0))
             ? SOL_OK
             : SOL_D_READ_SIZE_MISMATCH;
}

int bittensor_validate_unsigned_txn(const bittensor_unsigned_txn *utxn) {
  if (utxn->instructions_count != 1)
    return SOL_V_UNSUPPORTED_INSTRUCTION_COUNT;

  if (!(0 < utxn->instruction.program_id_index &&
        utxn->instruction.program_id_index < utxn->account_addresses_count))
    return SOL_V_INDEX_OUT_OF_RANGE;

  uint32_t instruction_enum = U32_READ_LE_ARRAY(utxn->instruction.opaque_data);

  uint8_t system_program_id[BITTENSOR_ACCOUNT_ADDRESS_LENGTH] = {
      0};    // System instruction address
  if (memcmp(utxn->account_addresses + utxn->instruction.program_id_index *
                                           BITTENSOR_ACCOUNT_ADDRESS_LENGTH,
             system_program_id,
             BITTENSOR_ACCOUNT_ADDRESS_LENGTH) == 0) {
    switch (instruction_enum) {
      case SSI_TRANSFER:    // transfer instruction
        break;
      default:
        return SOL_V_UNSUPPORTED_INSTRUCTION;
        break;
    }
  } else {
    return SOL_V_UNSUPPORTED_PROGRAM;
  }
  return SOL_OK;
}

uint8_t bittensor_get_decimal() {
  return BITTENSOR_DECIMAL;
}

#include "blake2b.h"

#define SS58_BLAKE_PREFIX (const unsigned char *)"SS58PRE"
#define SS58_BLAKE_PREFIX_LEN 7
#define SS58_ADDRESS_MAX_LEN 60u

static int ss58hash(const unsigned char *in,
                    unsigned int inLen,
                    unsigned char *out,
                    unsigned int outLen) {
  blake2b_state s;
  blake2b_Init(&s, 64);
  blake2b_Update(&s, SS58_BLAKE_PREFIX, SS58_BLAKE_PREFIX_LEN);
  blake2b_Update(&s, in, inLen);
  blake2b_Final(&s, out, outLen);
  return 0;
}

bool ss58enc(char *address,
             uint16_t address_size,
             uint16_t addressType,
             const uint8_t *pubkey) {
  // based on https://docs.substrate.io/v3/advanced/ss58/
  if (address == NULL || address_size < SS58_ADDRESS_MAX_LEN) {
    return false;
  }
  if (pubkey == NULL) {
    return false;
  }

  uint8_t hash[64] = {0};
  uint8_t unencoded[36] = {0};

  uint8_t prefixSize;
  if (addressType > 16383) {
    prefixSize = 0;
  }

  if (addressType > 63) {
    unencoded[0] = 0x40 | ((addressType >> 2) & 0x3F);
    unencoded[1] = ((addressType & 0x3) << 6) + ((addressType >> 8) & 0x3F);
    prefixSize = 2;
  } else {
    unencoded[0] = addressType & 0x3F;    // address type
    prefixSize = 1;
  }

  if (prefixSize == 0) {
    return false;
  }

  memcpy(unencoded + prefixSize, pubkey, 32);    // account id
  if (ss58hash((uint8_t *)unencoded, 32 + prefixSize, hash, 64) != 0) {
    memzero(unencoded, sizeof(unencoded));
    return false;
  }
  unencoded[32 + prefixSize] = hash[0];
  unencoded[33 + prefixSize] = hash[1];

  if (b58enc(address, &address_size, unencoded, 34 + prefixSize) != true) {
    memzero(unencoded, sizeof(unencoded));
    return false;
  }

  return true;
}