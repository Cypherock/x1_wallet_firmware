/**
 * @file    solana_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper implementation for interpreting and signing Solana
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

#include "solana_txn_helpers.h"

#include <stdlib.h>
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
uint16_t get_compact_array_size(const uint8_t *data,
                                uint16_t *size,
                                int *error) {
  uint16_t offset = 0;
  uint32_t value = 0;
  *error = 0;

  while (offset < 3) {
    value |= (*(data + offset) & 0x7F) << offset * 7;
    if ((*(data + offset) & 0x80) == 0)
      break;
    offset++;
  }

  if (value > UINT16_MAX)
    *error = SOL_D_COMPACT_U16_OVERFLOW;    // overflow

  *size = value;
  return offset + 1;
}

int solana_byte_array_to_unsigned_txn(uint8_t *byte_array,
                                      uint16_t byte_array_size,
                                      solana_unsigned_txn *utxn,
                                      solana_txn_extra_data *extra_data) {
  if (byte_array == NULL || utxn == NULL)
    return SOL_ERROR;
  memzero(utxn, sizeof(solana_unsigned_txn));

  uint16_t offset = 0;
  int error = 0;
  bool is_versioned = false;

  if (byte_array_size > 0 && byte_array[0] == SOLANA_VERSIONED_MSG_PREFIX) {
    is_versioned = true;
    offset += 1;
  }

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
  offset += utxn->account_addresses_count * SOLANA_ACCOUNT_ADDRESS_LENGTH;

  // Blockhash
  utxn->blockhash = byte_array + offset;
  offset += SOLANA_BLOCKHASH_LENGTH;

  // Instructions
  offset += get_compact_array_size(
      byte_array + offset, &(utxn->instructions_count), &error);
  if (error != SOL_OK)
    return error;
  if (utxn->instructions_count == 0)
    return SOL_D_MIN_LENGTH;
  if (utxn->instructions_count > SOLANA_MAX_INSTRUCTION_COUNT)
    return SOL_V_UNSUPPORTED_INSTRUCTION_COUNT;

  utxn->instruction = (solana_instruction *)malloc(utxn->instructions_count *
                                                   sizeof(solana_instruction));
  if (utxn->instruction == NULL)
    return SOL_ALLOCATION_FAILED;
  memzero(utxn->instruction,
          utxn->instructions_count * sizeof(solana_instruction));

  // prepare list of supported program ids
  uint8_t system_program_id[SOLANA_PROGRAM_ID_COUNT]
                           [SOLANA_ACCOUNT_ADDRESS_LENGTH];
  // Set System instruction address for SOL transfer
  memzero(system_program_id[SOLANA_SOL_TRANSFER_PROGRAM_ID_INDEX],
          SOLANA_ACCOUNT_ADDRESS_LENGTH);
  // Set System instruction address for Token Program
  hex_string_to_byte_array(SOLANA_TOKEN_PROGRAM_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           system_program_id[SOLANA_TOKEN_PROGRAM_ID_INDEX]);
  // Set Associated Token Program address
  hex_string_to_byte_array(
      SOLANA_ASSOCIATED_TOKEN_PROGRAM_ADDRESS,
      SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
      system_program_id[SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID_INDEX]);
  // Set Compute Budget Program address
  hex_string_to_byte_array(
      SOLANA_COMPUTE_BUDGET_PROGRAM_ADDRESS,
      SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
      system_program_id[SOLANA_COMPUTE_BUDGET_PROGRAM_ID_INDEX]);
  // Set Stake Program address
  hex_string_to_byte_array(SOLANA_STAKE_PROGRAM_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX]);

  extra_data->compute_unit_limit =
      extra_data->compute_unit_price_micro_lamports = 0;
  extra_data->is_stake_operation = false;
  extra_data->is_deactivate_operation = false;
  extra_data->create_account_with_seed_instruction_index = -1;
  extra_data->stake_initialize_instruction_index = -1;
  extra_data->stake_delegate_instruction_index = -1;
  extra_data->split_instruction_index = -1;
  extra_data->deactivate_instruction_count = 0;
  bool allocate_with_seed_seen = false;

  for (int i = 0; i < utxn->instructions_count; i++) {
    utxn->instruction[i].program_id_index = *(byte_array + offset++);

    offset += get_compact_array_size(
        byte_array + offset,
        &(utxn->instruction[i].account_addresses_index_count),
        &error);
    if (error != SOL_OK)
      return error;

    utxn->instruction[i].account_addresses_index = byte_array + offset;
    offset += utxn->instruction[i].account_addresses_index_count;
    offset += get_compact_array_size(byte_array + offset,
                                     &(utxn->instruction[i].opaque_data_length),
                                     &error);
    if (error != SOL_OK)
      return error;

    utxn->instruction[i].opaque_data = byte_array + offset;
    offset += utxn->instruction[i].opaque_data_length;

    if (memcmp(utxn->account_addresses + utxn->instruction[i].program_id_index *
                                             SOLANA_ACCOUNT_ADDRESS_LENGTH,
               system_program_id[SOLANA_SOL_TRANSFER_PROGRAM_ID_INDEX],
               SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      if (utxn->instruction[i].account_addresses_index_count == 0 ||
          utxn->instruction[i].opaque_data_length == 0)
        return SOL_D_MIN_LENGTH;

      uint32_t instruction_enum =
          U32_READ_LE_ARRAY(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case SSI_TRANSFER:    // transfer instruction
          extra_data->transfer_instruction_index = i;
          utxn->instruction[i].program.transfer.funding_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 0) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer.recipient_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 1) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer.lamports =
              U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data + 4);
          break;

        case SSI_CREATE_ACCOUNT_WITH_SEED:
          if (extra_data->create_account_with_seed_instruction_index != -1)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          if (utxn->instruction[i].account_addresses_index_count < 2)
            return SOL_D_MIN_LENGTH;

          extra_data->transfer_instruction_index = i;
          extra_data->is_stake_operation = true;
          extra_data->create_account_with_seed_instruction_index = i;

          utxn->instruction[i].program.create_account_with_seed.from_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 0) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          // 0: from_account, 1: new_account, 2: base_account optional as per
          // spec when base == from_account, may or maynot be present
          utxn->instruction[i].program.create_account_with_seed.base_account =
              (utxn->instruction[i].account_addresses_index_count >= 3)
                  ? utxn->account_addresses +
                        (*(utxn->instruction[i].account_addresses_index + 2) *
                         SOLANA_ACCOUNT_ADDRESS_LENGTH)
                  : utxn->instruction[i]
                        .program.create_account_with_seed.from_account;

          utxn->instruction[i].program.create_account_with_seed.base =
              utxn->instruction[i].opaque_data + 4;
          {
            uint64_t seed_len =
                (uint64_t)U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data +
                                            4 + SOLANA_ACCOUNT_ADDRESS_LENGTH);
            if (seed_len > 32) {
              return SOL_D_READ_SIZE_MISMATCH;
            }
            uint16_t after_seed =
                4 + SOLANA_ACCOUNT_ADDRESS_LENGTH + 8 + seed_len;
            utxn->instruction[i].program.create_account_with_seed.lamports =
                U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data +
                                  after_seed);
            // then we have 8 bytes of "space"
            utxn->instruction[i].program.create_account_with_seed.owner =
                utxn->instruction[i].opaque_data + after_seed + 16;
          }
          break;

        case SSI_ALLOCATE_WITH_SEED:
          if (allocate_with_seed_seen)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          if (utxn->instruction[i].account_addresses_index_count < 2)
            return SOL_D_MIN_LENGTH;

          extra_data->is_stake_operation = true;
          extra_data->is_deactivate_operation = true;
          allocate_with_seed_seen = true;

          utxn->instruction[i].parsed_kind = SOLANA_PARSED_ALLOCATE_WITH_SEED;
          // 0: new_account, 1: base_account
          utxn->instruction[i].program.allocate_with_seed.new_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 0) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.allocate_with_seed.base_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 1) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);

          utxn->instruction[i].program.allocate_with_seed.base =
              utxn->instruction[i].opaque_data + 4;
          {
            uint64_t seed_len =
                (uint64_t)U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data +
                                            4 + SOLANA_ACCOUNT_ADDRESS_LENGTH);
            if (seed_len > 32) {
              return SOL_D_READ_SIZE_MISMATCH;
            }
            uint16_t after_seed =
                4 + SOLANA_ACCOUNT_ADDRESS_LENGTH + 8 + seed_len;
            // then we have 8 bytes of "space"
            utxn->instruction[i].program.allocate_with_seed.owner =
                utxn->instruction[i].opaque_data + after_seed + 8;
          }
          break;

        default:
          break;
      }
    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_TOKEN_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      if (utxn->instruction[i].account_addresses_index_count == 0 ||
          utxn->instruction[i].opaque_data_length == 0)
        return SOL_D_MIN_LENGTH;

      extra_data->transfer_instruction_index = i;

      uint8_t instruction_enum = *(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case STPI_TRANSFER_CHECKED:    // transfer checked instruction
          utxn->instruction[i].program.transfer_checked.source =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 0) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer_checked.token_mint =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 1) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer_checked.destination =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 2) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer_checked.owner =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 3) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.transfer_checked.amount =
              U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data + 1);
          utxn->instruction[i].program.transfer_checked.decimals =
              *(utxn->instruction[i].opaque_data + sizeof(uint64_t) +
                1);    // decimal value comes after amount(which is a u64)
          break;

        default:
          break;
      }
    } else if (memcmp(
                   utxn->account_addresses +
                       utxn->instruction[i].program_id_index *
                           SOLANA_ACCOUNT_ADDRESS_LENGTH,
                   system_program_id[SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID_INDEX],
                   SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      if (utxn->instruction[i].account_addresses_index_count == 0)
        return SOL_D_MIN_LENGTH;

    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_COMPUTE_BUDGET_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      if (utxn->instruction[i].opaque_data_length == 0)
        return SOL_D_MIN_LENGTH;

      uint8_t instruction_enum = *(utxn->instruction[i].opaque_data);
      switch (instruction_enum) {
        case SCBI_SET_COMPUTE_UNIT_LIMIT:
          extra_data->compute_unit_limit =
              utxn->instruction[i].program.compute_unit_limit_data.units =
                  U32_READ_LE_ARRAY(utxn->instruction[i].opaque_data + 1);
          break;

        case SCBI_SET_COMPUTE_UNIT_PRICE:
          extra_data->compute_unit_price_micro_lamports =
              utxn->instruction[i]
                  .program.compute_unit_price_data.micro_lamports =
                  U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data + 1);
          break;

        default:
          break;
      }
    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      if (utxn->instruction[i].opaque_data_length == 0)
        return SOL_D_MIN_LENGTH;

      extra_data->is_stake_operation = true;

      // See:
      // https://docs.rs/solana-sdk/1.10.8/solana_sdk/stake/instruction/enum.StakeInstruction.html
      uint32_t instruction_enum =
          U32_READ_LE_ARRAY(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case SPI_INITIALIZE:
          if (extra_data->stake_initialize_instruction_index != -1)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          // Opaque data: tag(4) + Authorized{staker:32, withdrawer:32} + Lockup
          // which is unused here
          if (utxn->instruction[i].account_addresses_index_count < 1)
            return SOL_D_MIN_LENGTH;

          extra_data->stake_initialize_instruction_index = i;
          utxn->instruction[i].program.stake_initialize.staker =
              utxn->instruction[i].opaque_data + 4;
          utxn->instruction[i].program.stake_initialize.withdrawer =
              utxn->instruction[i].opaque_data + 4 +
              SOLANA_ACCOUNT_ADDRESS_LENGTH;
          break;

        case SPI_DELEGATE_STAKE:
          if (extra_data->stake_delegate_instruction_index != -1)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          if (utxn->instruction[i].account_addresses_index_count < 6)
            return SOL_D_MIN_LENGTH;

          extra_data->stake_delegate_instruction_index = i;

          utxn->instruction[i].program.stake_delegate.vote_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 1) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.stake_delegate.authorized_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 5) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          break;

        case SPI_SPLIT:
          if (extra_data->split_instruction_index != -1)
            return SOL_V_UNSUPPORTED_SPLIT_COUNT;
          if (utxn->instruction[i].account_addresses_index_count < 3)
            return SOL_D_MIN_LENGTH;

          extra_data->split_instruction_index = i;
          utxn->instruction[i].parsed_kind = SOLANA_PARSED_STAKE_SPLIT;
          utxn->instruction[i].program.stake_split.destination_stake_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 1) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.stake_split.authorized_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 2) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.stake_split.lamports =
              U64_READ_LE_ARRAY(utxn->instruction[i].opaque_data + 4);
          break;

        case SPI_DEACTIVATE:
          if (utxn->instruction[i].account_addresses_index_count < 3)
            return SOL_D_MIN_LENGTH;

          extra_data->is_deactivate_operation = true;
          extra_data->deactivate_instruction_count++;

          utxn->instruction[i].parsed_kind = SOLANA_PARSED_STAKE_DEACTIVATE;
          utxn->instruction[i].program.stake_deactivate.stake_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 0) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          utxn->instruction[i].program.stake_deactivate.authorized_account =
              utxn->account_addresses +
              (*(utxn->instruction[i].account_addresses_index + 2) *
               SOLANA_ACCOUNT_ADDRESS_LENGTH);
          break;

        default:
          break;
      }
    }
  }

  // v0 txns appends an array of MessageAddressTableLookup after instructions
  // which never appears in account_addresses, and cant verifed in firmware
  if (is_versioned) {
    uint16_t lookup_count = 0;
    offset +=
        get_compact_array_size(byte_array + offset, &lookup_count, &error);
    if (error != SOL_OK)
      return error;
    if (lookup_count != 0)
      return SOL_V_UNSUPPORTED_VERSIONED_TXN;
  }

  return ((offset <= byte_array_size) && (offset > 0))
             ? SOL_OK
             : SOL_D_READ_SIZE_MISMATCH;
}

int solana_validate_unsigned_txn(const solana_unsigned_txn *utxn) {
  if (utxn->instructions_count > SOLANA_MAX_INSTRUCTION_COUNT)
    return SOL_V_UNSUPPORTED_INSTRUCTION_COUNT;

  // prepare list of supported program ids and validators
  uint8_t system_program_id[SOLANA_PROGRAM_ID_COUNT]
                           [SOLANA_ACCOUNT_ADDRESS_LENGTH];
  uint8_t mainnet_validator[SOLANA_ACCOUNT_ADDRESS_LENGTH];
  uint8_t devnet_validator[SOLANA_ACCOUNT_ADDRESS_LENGTH];
  // Set System instruction address for SOL transfer
  memzero(system_program_id[SOLANA_SOL_TRANSFER_PROGRAM_ID_INDEX],
          SOLANA_ACCOUNT_ADDRESS_LENGTH);
  // Set System instruction address for Token Program
  hex_string_to_byte_array(SOLANA_TOKEN_PROGRAM_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           system_program_id[SOLANA_TOKEN_PROGRAM_ID_INDEX]);
  // Set Associated Token Program address
  hex_string_to_byte_array(
      SOLANA_ASSOCIATED_TOKEN_PROGRAM_ADDRESS,
      SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
      system_program_id[SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID_INDEX]);
  // Set Compute Budget Program address
  hex_string_to_byte_array(
      SOLANA_COMPUTE_BUDGET_PROGRAM_ADDRESS,
      SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
      system_program_id[SOLANA_COMPUTE_BUDGET_PROGRAM_ID_INDEX]);
  // Set Stake Program address
  hex_string_to_byte_array(SOLANA_STAKE_PROGRAM_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX]);

  hex_string_to_byte_array(SOLANA_MAINNET_VALIDATOR_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           mainnet_validator);
  hex_string_to_byte_array(SOLANA_DEVNET_VALIDATOR_ADDRESS,
                           SOLANA_ACCOUNT_ADDRESS_LENGTH * 2,
                           devnet_validator);

  bool transfer_instruction_found = false;
  bool stake_initialize_found = false;
  int stake_initialize_index = -1;
  bool is_deactivate_operation = false;
  bool create_flow_found = false;
  int split_count = 0;
  int deactivate_instruction_count = 0;
  bool allocate_with_seed_found = false;
  const uint8_t *allocate_new_account = NULL;
  const uint8_t *transfer_recipient = NULL;
  const uint8_t *split_destination = NULL;

  for (int i = 0; i < utxn->instructions_count; i++) {
    if (!(0 < utxn->instruction[i].program_id_index &&
          utxn->instruction[i].program_id_index <
              utxn->account_addresses_count))
      return SOL_V_INDEX_OUT_OF_RANGE;

    if (memcmp(utxn->account_addresses + utxn->instruction[i].program_id_index *
                                             SOLANA_ACCOUNT_ADDRESS_LENGTH,
               system_program_id[SOLANA_SOL_TRANSFER_PROGRAM_ID_INDEX],
               SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      uint32_t instruction_enum =
          U32_READ_LE_ARRAY(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case SSI_TRANSFER:    // transfer instruction
          if (transfer_instruction_found)
            return SOL_ERROR;
          transfer_instruction_found = true;
          transfer_recipient =
              utxn->instruction[i].program.transfer.recipient_account;
          break;

        case SSI_CREATE_ACCOUNT_WITH_SEED: {
          if (transfer_instruction_found)
            return SOL_ERROR;
          transfer_instruction_found = true;
          create_flow_found = true;

          const solana_create_account_with_seed_data *cas =
              &utxn->instruction[i].program.create_account_with_seed;

          if (memcmp(cas->from_account,
                     cas->base_account,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_AUTHORITY_MISMATCH;
          if (memcmp(cas->base,
                     cas->base_account,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_AUTHORITY_MISMATCH;

          if (memcmp(cas->owner,
                     system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX],
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_PROGRAM_OWNER_MISMATCH;

          break;
        }

        case SSI_ALLOCATE_WITH_SEED: {
          if (allocate_with_seed_found)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          allocate_with_seed_found = true;
          is_deactivate_operation = true;
          const solana_allocate_with_seed_data *aws =
              &utxn->instruction[i].program.allocate_with_seed;
          allocate_new_account = aws->new_account;

          if (memcmp(aws->base,
                     aws->base_account,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_AUTHORITY_MISMATCH;

          if (memcmp(aws->owner,
                     system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX],
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_PROGRAM_OWNER_MISMATCH;
          break;
        }

        default:
          return SOL_V_UNSUPPORTED_INSTRUCTION;
          break;
      }
    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_TOKEN_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      uint8_t instruction_enum = *(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case STPI_TRANSFER_CHECKED:    // transfer checked instruction
          if (transfer_instruction_found)
            return SOL_ERROR;
          transfer_instruction_found = true;
          break;
        default:
          return SOL_V_UNSUPPORTED_INSTRUCTION;
          break;
      }
    } else if (memcmp(
                   utxn->account_addresses +
                       utxn->instruction[i].program_id_index *
                           SOLANA_ACCOUNT_ADDRESS_LENGTH,
                   system_program_id[SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID_INDEX],
                   SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      // no opaque data or instruction enum to validate
      // do nothing

    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_COMPUTE_BUDGET_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      uint8_t instruction_enum = *(utxn->instruction[i].opaque_data);
      switch (instruction_enum) {
        case SCBI_SET_COMPUTE_UNIT_LIMIT:
        case SCBI_SET_COMPUTE_UNIT_PRICE:
          break;

        default:
          return SOL_V_UNSUPPORTED_INSTRUCTION;
          break;
      }
    } else if (memcmp(utxn->account_addresses +
                          utxn->instruction[i].program_id_index *
                              SOLANA_ACCOUNT_ADDRESS_LENGTH,
                      system_program_id[SOLANA_STAKE_PROGRAM_ID_INDEX],
                      SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      uint32_t instruction_enum =
          U32_READ_LE_ARRAY(utxn->instruction[i].opaque_data);

      switch (instruction_enum) {
        case SPI_INITIALIZE:
          stake_initialize_found = true;
          stake_initialize_index = i;
          create_flow_found = true;
          break;

        case SPI_DELEGATE_STAKE: {
          // Must immediately follow Initialize
          if (!stake_initialize_found || stake_initialize_index != i - 1)
            return SOL_V_STAKE_SEQUENCE_MISMATCH;
          create_flow_found = true;

          const solana_stake_delegate_data *dg =
              &utxn->instruction[i].program.stake_delegate;

          if (memcmp(dg->vote_account,
                     mainnet_validator,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0 &&
              memcmp(dg->vote_account,
                     devnet_validator,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return SOL_V_STAKE_VALIDATOR_MISMATCH;
          break;
        }

        case SPI_SPLIT: {
          // At max 1 split per txn is enough
          split_count++;
          if (split_count > 1)
            return SOL_V_UNSUPPORTED_SPLIT_COUNT;
          split_destination =
              utxn->instruction[i]
                  .program.stake_split.destination_stake_account;
          break;
        }

        case SPI_DEACTIVATE:
          is_deactivate_operation = true;
          deactivate_instruction_count++;
          break;

        default:
          return SOL_V_UNSUPPORTED_INSTRUCTION;
          break;
      }
    } else {
      return SOL_V_UNSUPPORTED_PROGRAM;
    }
  }

  if (!transfer_instruction_found && !is_deactivate_operation)
    return SOL_ERROR;

  if (create_flow_found && is_deactivate_operation)
    return SOL_V_MIXED_STAKE_OPERATION;

  if (is_deactivate_operation == true && deactivate_instruction_count == 0)
    return SOL_V_MIXED_STAKE_OPERATION;

  if (is_deactivate_operation) {
    // A Split creates a new destination account must be paired with an
    // AllocateWithSeed and Transfer, reject a bare/mismatched Transfer or a
    // Split missing its funding pair
    if ((split_count > 0) != allocate_with_seed_found)
      return SOL_V_MIXED_STAKE_OPERATION;
    if (transfer_instruction_found != allocate_with_seed_found)
      return SOL_V_MIXED_STAKE_OPERATION;
    if (allocate_with_seed_found) {
      if (memcmp(transfer_recipient,
                 allocate_new_account,
                 SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
        return SOL_V_STAKE_AUTHORITY_MISMATCH;
      if (memcmp(split_destination,
                 allocate_new_account,
                 SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
        return SOL_V_STAKE_AUTHORITY_MISMATCH;
    }
  }

  return SOL_OK;
}

bool solana_verify_stake_authorities(const solana_unsigned_txn *utxn,
                                     const solana_txn_extra_data *extra_data,
                                     const uint8_t *derived_public_key) {
  if (!extra_data->is_stake_operation)
    return true;

  if (extra_data->is_deactivate_operation) {
    for (int i = 0; i < utxn->instructions_count; i++) {
      switch (utxn->instruction[i].parsed_kind) {
        case SOLANA_PARSED_ALLOCATE_WITH_SEED:
          if (memcmp(
                  utxn->instruction[i].program.allocate_with_seed.base_account,
                  derived_public_key,
                  SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return false;
          break;

        case SOLANA_PARSED_STAKE_SPLIT:
          if (memcmp(
                  utxn->instruction[i].program.stake_split.authorized_account,
                  derived_public_key,
                  SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return false;
          break;

        case SOLANA_PARSED_STAKE_DEACTIVATE:
          if (memcmp(utxn->instruction[i]
                         .program.stake_deactivate.authorized_account,
                     derived_public_key,
                     SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
            return false;
          break;

        default:
          break;
      }
    }
    return true;
  }

  const solana_create_account_with_seed_data *cas =
      &utxn->instruction[extra_data->create_account_with_seed_instruction_index]
           .program.create_account_with_seed;
  const solana_stake_initialize_data *init =
      &utxn->instruction[extra_data->stake_initialize_instruction_index]
           .program.stake_initialize;
  const solana_stake_delegate_data *dg =
      &utxn->instruction[extra_data->stake_delegate_instruction_index]
           .program.stake_delegate;

  if (memcmp(cas->from_account,
             derived_public_key,
             SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
    return false;
  if (memcmp(init->staker, derived_public_key, SOLANA_ACCOUNT_ADDRESS_LENGTH) !=
      0)
    return false;
  if (memcmp(init->withdrawer,
             derived_public_key,
             SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
    return false;
  if (memcmp(dg->authorized_account,
             derived_public_key,
             SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
    return false;

  return true;
}

uint8_t solana_get_decimal() {
  return SOLANA_DECIMAL;
}
