/**
 * @file    eth.c
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
#include "eth.h"

#include "assert_conf.h"
#include "constant_texts.h"
#include "evm_contracts.h"
#include "int-util.h"
#include "logger.h"
#include "pb_decode.h"
#include "utils.h"

extern const erc20_contracts_t *eth_contracts;

static uint8_t rlp_encode_decimal(uint64_t dec,
                                  uint8_t offset,
                                  uint8_t *metadata);

/* Refer https://www.4byte.directory/signatures/?bytes4_signature=0x7c025200 */
#define EVM_swap_TAG (0x12aa3caf)
#define EVM_swap_NUM_ARGS 10
const Abi_Type_e EVM_swapDataType[EVM_swap_NUM_ARGS] = {Abi_address_e,
                                                        Abi_address_e,
                                                        Abi_address_e,
                                                        Abi_address_e,
                                                        Abi_address_e,
                                                        Abi_uint256_e,
                                                        Abi_uint256_e,
                                                        Abi_uint256_e,
                                                        Abi_bytes_dynamic_e,
                                                        Abi_bytes_dynamic_e};

const char *EVM_swap_Title = "Function: swap";
const char *EVM_swap_Signature =
    "swap(address,(address,address,address,address,uint256,uint256,uint256),"
    "bytes,bytes)";

/* Refer https://www.4byte.directory/signatures/?bytes4_signature=0xe449022e */
#define EVM_uniswapV3Swap_TAG (0xe449022e)
#define EVM_uniswapV3Swap_NUM_ARGS 3
const Abi_Type_e EVM_uniswapV3SwapDataType[EVM_uniswapV3Swap_NUM_ARGS] = {
    Abi_uint256_e,
    Abi_uint256_e,
    Abi_uint256_array_dynamic_e};

const char *EVM_uniswapV3Swap_Title = "Function: uniswapV3Swap";
const char *EVM_uniswapV3Swap_Signature =
    "uniswapV3Swap(uint256,uint256,uint256[])";

/* Refer https://www.4byte.directory/signatures/?bytes4_signature=0x42842e0e */
#define EVM_safeTransferFrom_TAG (0x42842e0e)
#define EVM_safeTransferFrom_NUM_ARGS 3
const Abi_Type_e EVM_safeTransferFromDataType[EVM_safeTransferFrom_NUM_ARGS] = {
    Abi_address_e,
    Abi_address_e,
    Abi_uint256_e};

const char *EVM_safeTransferFrom_Title = "Function: safeTransferFrom";
const char *EVM_safeTransferFrom_Signature =
    "safeTransferFrom(address,address,uint256)";

/* Refer https://www.4byte.directory/signatures/?bytes4_signature=0xd0e30db0 */
#define EVM_deposit_TAG (0xd0e30db0)
#define EVM_deposit_NUM_ARGS 0
const Abi_Type_e EVM_depositDataType[EVM_deposit_NUM_ARGS] = {};

const char *EVM_deposit_Title = "Function: deposit";
const char *EVM_deposit_Signature = "deposit()";

/* Refer https://www.4byte.directory/signatures/?bytes4_signature=0xa9059cbb */
#define EVM_transfer_TAG (0xa9059cbb)
#define EVM_transfer_NUM_ARGS 2
const Abi_Type_e EVM_transferDataType[EVM_transfer_NUM_ARGS] = {Abi_address_e,
                                                                Abi_uint256_e};
const char *EVM_transfer_Title = "Function: transfer";
const char *EVM_transfer_Signature = "transfer(address,uint256)";

extern bool eth_is_token_whitelisted;

/**
 * @brief
 * @details
 *
 * @param [in] ch
 *
 * @return return index of hex-chars
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t get_index_of_signs(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  }
  return -1;
}

/**
 * @brief Converts byte array to RLP encoded byte array.
 * @details
 *
 * @param [in] byte Byte array to convert.
 * @param [in] len Length of byte array.
 * @param [in] type Type of data (STRING or LIST)
 * @param [out] encoded_byte Converted byte array.
 *
 * @return Size of length.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t rlp_encode_byte(const uint8_t *byte,
                                const uint64_t len,
                                const seq_type type,
                                uint8_t *encoded_byte) {
  uint64_t meta_size = 0;

  if (type == STRING) {
    if (len == 1 && byte[0] < 0x80) {
      if (encoded_byte)
        encoded_byte[0] = byte[0];
    } else if ((len + 0x80) < 0xb8) {
      if (encoded_byte)
        encoded_byte[0] = 0x80 + len;
      meta_size = 1;
    } else {
      meta_size = rlp_encode_decimal(len, 0xb7, encoded_byte);
    }
  } else if (type == LIST) {
    if ((len + 0xc0) < 0xf8) {
      if (encoded_byte)
        encoded_byte[0] = 0xc0 + len;
      meta_size = 1;
    } else {
      meta_size = rlp_encode_decimal(len, 0xf7, encoded_byte);
    }
  }

  if (byte && encoded_byte)
    memcpy(&encoded_byte[meta_size], byte, len);
  return meta_size;
}

/**
 * @brief Converts decimal to RLP encoded byte array.
 * @details
 *
 * @param [] dec        Decimal value.
 * @param [] offset     Offset for the metadata array.
 * @param [] metadata   Byte array to store the converted decimal.
 *
 * @return Size of converted encoded decimal.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint8_t rlp_encode_decimal(const uint64_t dec,
                                  const uint8_t offset,
                                  uint8_t *metadata) {
  uint32_t meta_size = 0;
  uint8_t len_hex[8];
  uint64_t s = dec_to_hex(dec, len_hex, 8);

  if (offset == 0) {
    meta_size = rlp_encode_byte(len_hex, s, STRING, metadata);
  } else {
    if (metadata) {
      metadata[0] = offset + s;
      memcpy(&metadata[1], len_hex + 8 - s, s);
    }
    meta_size = 1 + s;
  }
  return meta_size;
}

uint64_t bendian_byte_to_dec(const uint8_t *bytes, uint8_t len) {
  uint64_t result = 0;
  uint8_t i = 0;
  while (i < len) {
    int shift = 8 * (len - i - 1);
    uint64_t byte = bytes[i++];
    result = result | (byte << shift);
  }
  return result;
}

uint64_t hex2dec(const char *source) {
  uint64_t sum = 0;
  uint64_t t = 1;
  int len = 0;

  len = strnlen(source, 16);
  for (int i = len - 1; i >= 0; i--) {
    uint64_t j = get_index_of_signs(*(source + i));
    sum += (t * j);
    t *= 16;
  }

  return sum;
}

const char *eth_get_asset_symbol(const txn_metadata *metadata_ptr) {
  if (eth_is_token_whitelisted)
    return metadata_ptr->token_name;
  else
    return get_coin_symbol(ETHEREUM, metadata_ptr->network_chain_id);
}

void eth_sign_msg_data(const MessageData *msg_data,
                       const txn_metadata *transaction_metadata,
                       const char *mnemonics,
                       const char *passphrase,
                       uint8_t *sig) {
}

void eth_init_display_nodes(ui_display_node **node, MessageData *msg_data) {
}

int eth_byte_array_to_msg(const uint8_t *eth_msg,
                          size_t byte_array_len,
                          MessageData *msg_data) {
  pb_istream_t stream = pb_istream_from_buffer(eth_msg, byte_array_len);

  bool status = pb_decode(&stream, MessageData_fields, msg_data);

  if (!status) {
    return -1;
  }

  return 0;
}

void eth_init_msg_data(MessageData *msg_data) {
  MessageData zero = MessageData_init_zero;
  memcpy(msg_data, &zero, sizeof(zero));
}

void sig_unsigned_byte_array(const uint8_t *eth_unsigned_txn_byte_array,
                             uint64_t eth_unsigned_txn_len,
                             const txn_metadata *transaction_metadata,
                             const char *mnemonics,
                             const char *passphrase,
                             uint8_t *sig) {
  uint8_t digest[32];
  keccak_256(eth_unsigned_txn_byte_array, eth_unsigned_txn_len, digest);
  uint8_t recid;
  HDNode hdnode;
  get_address_node(transaction_metadata, 0, mnemonics, passphrase, &hdnode);
  ecdsa_sign_digest(&secp256k1, hdnode.private_key, digest, sig, &recid, NULL);
  memcpy(sig + 64, &recid, 1);
}

/**
 * @brief This function checks if an EVM function tag is supported by the
 * X1 wallet parser. If a known function is found, a UI element of type
 * ui_display_node is created. Also, dpAbiTypeArray is updated to point
 * the argument type list for that function.
 *
 * @param functionTag The function tag found in the EVM transaction payload
 * @param dpAbiTypeArray Pointer to start of the argument type array for the
 * identified function.
 * @return uint8_t The number of arguments in an identified function.
 */
static uint8_t ETH_DetectFunction(const uint32_t functionTag,
                                  Abi_Type_e const **const dpAbiTypeArray) {
  if (NULL == dpAbiTypeArray) {
    return 0;
  }

  uint8_t numArgsInFunction = 0;
  const char *EvmFunctionTitle = NULL;
  const char *EvmFunctionSignature = NULL;

  switch (functionTag) {
    case EVM_swap_TAG: {
      numArgsInFunction = EVM_swap_NUM_ARGS;
      *(dpAbiTypeArray) = (Abi_Type_e *)(&(EVM_swapDataType[0]));
      EvmFunctionTitle = EVM_swap_Title;
      EvmFunctionSignature = EVM_swap_Signature;
      break;
    }
    case EVM_uniswapV3Swap_TAG: {
      numArgsInFunction = EVM_uniswapV3Swap_NUM_ARGS;
      *(dpAbiTypeArray) = (Abi_Type_e *)(&(EVM_uniswapV3SwapDataType[0]));
      EvmFunctionTitle = EVM_uniswapV3Swap_Title;
      EvmFunctionSignature = EVM_uniswapV3Swap_Signature;
      break;
    }
    case EVM_safeTransferFrom_TAG: {
      numArgsInFunction = EVM_safeTransferFrom_NUM_ARGS;
      *(dpAbiTypeArray) = (Abi_Type_e *)(&(EVM_safeTransferFromDataType[0]));
      EvmFunctionTitle = EVM_safeTransferFrom_Title;
      EvmFunctionSignature = EVM_safeTransferFrom_Signature;
      break;
    }
    case EVM_deposit_TAG: {
      numArgsInFunction = EVM_deposit_NUM_ARGS;
      *(dpAbiTypeArray) = (Abi_Type_e *)(&(EVM_depositDataType[0]));
      EvmFunctionTitle = EVM_deposit_Title;
      EvmFunctionSignature = EVM_deposit_Signature;
      break;
    }
    case EVM_transfer_TAG: {
      numArgsInFunction = EVM_transfer_NUM_ARGS;
      *(dpAbiTypeArray) = (Abi_Type_e *)(&(EVM_transferDataType[0]));
      EvmFunctionTitle = EVM_transfer_Title;
      EvmFunctionSignature = EVM_transfer_Signature;
      break;
    }
    default: {
      break;
    }
  }

  /* Add the detected function as part of verification in the UI */
  if ((NULL != EvmFunctionTitle) && (NULL != EvmFunctionSignature)) {
    ui_display_node *pAbiDispNode;
    pAbiDispNode = ui_create_display_node(EvmFunctionTitle,
                                          strnlen(EvmFunctionTitle, 100),
                                          EvmFunctionSignature,
                                          strnlen(EvmFunctionSignature, 100));

    // if (current_display_node == NULL) {
    if (true) {
      // current_display_node = pAbiDispNode;
    } else {
      ui_display_node *temp;    // = current_display_node;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = pAbiDispNode;
    }
  }

  return numArgsInFunction;
}

uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload,
                             const uint64_t sizeOfPayload) {
  uint8_t returnCode = ETH_BAD_ARGUMENTS;

  /* Size of transaction payload must be atleast EVM_FUNC_SIGNATURE_LENGTH */
  if ((NULL == pAbiPayload) || (EVM_FUNC_SIGNATURE_LENGTH > sizeOfPayload)) {
    return returnCode;
  }

  uint8_t *pCurrHeadPtr = (uint8_t *)pAbiPayload;

  /**
   * Detect if the ethereum unsigned txn payload includes a function that
   * we can decode
   * pArgumentAbiType will hold pointer to array with information regarding
   * the types of argument corresponding to a function signature
   */
  uint32_t functionTag = U32_READ_BE_ARRAY(pCurrHeadPtr);
  uint8_t numArgsInFunction = 0;
  Abi_Type_e const *pArgumentAbiType = NULL;

  numArgsInFunction = ETH_DetectFunction(functionTag, &pArgumentAbiType);

  /**
   * If pArgumentAbiType is NULL, that means ETH_DetectFunction did not
   * detect a supported function
   * Therefore we should return from here
   */
  if (NULL == pArgumentAbiType) {
    returnCode = ETH_UTXN_FUNCTION_NOT_FOUND;
    return returnCode;
  }

  /* Increment pCurrHeadPtr to point to first argument */
  pCurrHeadPtr += EVM_FUNC_SIGNATURE_LENGTH;

  /**
   * Save the base address of the first argument; it will be required in case
   * of any dynamic element encoded in ABI format as the offset is calculated
   * from the base of the first argument.
   */
  const uint8_t *pPayloadBasePtr = pCurrHeadPtr;
  uint8_t currArgument;

  for (currArgument = 0; currArgument < numArgsInFunction; currArgument++) {
    /* Ensure that we are reading from within the bounds */
    if (UTIL_IN_BOUNDS != UTIL_CheckBound(pAbiPayload,
                                          sizeOfPayload,
                                          pCurrHeadPtr,
                                          ABI_ELEMENT_SZ_IN_BYTES)) {
      returnCode = ETH_UTXN_BAD_PAYLOAD;
      break;
    }

    ui_display_node *pAbiDispNode = NULL;

    /* Check if we are reading a dynamic or static element */
    if (Abi_bytes_dynamic_e <= pArgumentAbiType[currArgument]) {
      uint8_t *pDynamicDataPtr = NULL;
      uint8_t abiReturnCode = ABI_PROCESS_INCOMPLETE;
      uint32_t numBytesReturned = 0;

      /* Get the information regarding dynamic data types */
      abiReturnCode =
          Abi_DynamicHelp(pArgumentAbiType[currArgument],
                          pCurrHeadPtr,
                          pPayloadBasePtr,
                          (sizeOfPayload - EVM_FUNC_SIGNATURE_LENGTH),
                          &numBytesReturned,
                          &pDynamicDataPtr);

      /**
       * If abiReturnCode is not ABI_PROCESS_COMPLETE, that means
       * the function spotted an invalid argument during the call, or
       * the payload was not good as bounds check failed internally
       */
      if ((ABI_PROCESS_COMPLETE != abiReturnCode) ||
          (NULL == pDynamicDataPtr)) {
        returnCode = ETH_UTXN_BAD_PAYLOAD;
        break;
      }

      /**
       * Handle stringify based on dynamic data type
       * Abi_bytes_dynamic_e can be handled directly using number of bytes
       * Abi_uint256_array_dynamic_e needs to be handled in a loop, for each
       * uint256 bit data
       */
      if (Abi_bytes_dynamic_e == pArgumentAbiType[currArgument]) {
        pAbiDispNode =
            ABI_Stringify(Abi_bytes_e, pDynamicDataPtr, numBytesReturned);
      } else if (Abi_uint256_array_dynamic_e ==
                 pArgumentAbiType[currArgument]) {
        uint32_t item;
        for (item = 0; item < numBytesReturned; item++) {
          uint8_t *pStaticData =
              (uint8_t *)(pDynamicDataPtr + (ABI_ELEMENT_SZ_IN_BYTES * item));

          pAbiDispNode = ABI_Stringify(Abi_uint256_e, pStaticData, 0);
        }
      }
    } else /* Static elements can be stringified straight away */
    {
      pAbiDispNode =
          ABI_Stringify(pArgumentAbiType[currArgument], pCurrHeadPtr, 0);
    }

    pCurrHeadPtr += ABI_ELEMENT_SZ_IN_BYTES;
    returnCode = ETH_UTXN_ABI_DECODE_OK;

    // if (current_display_node == NULL) {
    if (true) {
      // current_display_node = pAbiDispNode;
    } else {
      ui_display_node *temp;    // = current_display_node;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = pAbiDispNode;
    }
  }

  return returnCode;
}
void eth_derivation_path_to_string(const txn_metadata *txn_metadata_ptr,
                                   char *output,
                                   const size_t out_len) {
  const uint32_t path[] = {
      BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->purpose_index),
      BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->coin_index),
      BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->account_index),
      BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->input[0].change_index),
      BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->input[0].address_index)};
  FUNC_RETURN_CODES code =
      hd_path_array_to_string(path, 5, false, output, out_len);
  if (code != FRC_SUCCESS) {
    LOG_ERROR("DP-ETH:%d", code);
    ASSERT(false);
  }
}

uint8_t eth_get_decimal(const txn_metadata *txn_metadata_ptr) {
  return txn_metadata_ptr->is_token_transfer
             ? txn_metadata_ptr->eth_val_decimal[0]
             : ETH_DECIMAL;
}

const char *eth_get_address_title(
    const evm_unsigned_txn *eth_unsigned_txn_ptr) {
  return ((eth_unsigned_txn_ptr->payload_status != PAYLOAD_ABSENT &&
           !eth_is_token_whitelisted)
              ? ui_text_verify_contract
              : ui_text_verify_address);
}
