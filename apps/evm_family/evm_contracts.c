/**
 * @file    evm_contract_data_helper.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for interpreting contract data in EVM transactions
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

#include "evm_contracts.h"

#include "abi.h"
#include "address.h"
#include "constant_texts.h"
#include "evm_priv.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief Function pointer type for a semantic ABI parser.
 * @details A semantic parser decodes a specific known function's calldata into
 *          human-readable display nodes (token amounts with symbols, etc.).
 *          Returns ETH_UTXN_ABI_DECODE_OK on success, an error code otherwise.
 */
typedef uint8_t (*SemanticParser_fn)(uint32_t selector,
                                     const uint8_t *pAbiPayload,
                                     uint64_t sizeOfPayload,
                                     const uint8_t *toAddress,
                                     ui_display_node **displayNode);

/** Maps a 4-byte function selector to its semantic parser. */
typedef struct {
  uint32_t selector;
  SemanticParser_fn parser;
} SemanticParserEntry_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This function checks if an EVM function tag is supported by the
 * X1 wallet parser. If a known function is found, a UI element of type
 * ui_display_node is created. Also, dpAbiTypeArray is updated to point
 * the argument type list for that function.
 *
 * @param functionTag The function tag found in the EVM transaction payload
 * @param dpAbiTypeArray Pointer to start of the argument type array for the
 * identified function.
 * @param displayNode Pointer to storage for ui_display_node
 * @return uint8_t The number of arguments in an identified function.
 */
static uint8_t ETH_DetectFunction(const uint32_t functionTag,
                                  Abi_Type_e const **const dpAbiTypeArray,
                                  ui_display_node **displayNode);

static uint8_t hysp_parse_function(uint32_t selector,
                                   const uint8_t *pAbiPayload,
                                   uint64_t sizeOfPayload,
                                   const uint8_t *toAddress,
                                   ui_display_node **displayNode);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static const Abi_Type_e EVM_swapDataType[EVM_swap_NUM_ARGS] = {
    Abi_address_e,
    Abi_address_e,
    Abi_address_e,
    Abi_address_e,
    Abi_address_e,
    Abi_uint256_e,
    Abi_uint256_e,
    Abi_uint256_e,
    Abi_bytes_dynamic_e,
    Abi_bytes_dynamic_e,
};

static const char *EVM_swap_Title = "Function: swap";
static const char *EVM_swap_Signature =
    "swap(address,(address,address,address,address,uint256,uint256,uint256),"
    "bytes,bytes)";

static const Abi_Type_e EVM_uniswapV3SwapDataType[EVM_uniswapV3Swap_NUM_ARGS] =
    {Abi_uint256_e, Abi_uint256_e, Abi_uint256_array_dynamic_e};

static const char *EVM_uniswapV3Swap_Title = "Function: uniswapV3Swap";
static const char *EVM_uniswapV3Swap_Signature =
    "uniswapV3Swap(uint256,uint256,uint256[])";

static const Abi_Type_e
    EVM_safeTransferFromDataType[EVM_safeTransferFrom_NUM_ARGS] = {
        Abi_address_e,
        Abi_address_e,
        Abi_uint256_e,
};

static const char *EVM_safeTransferFrom_Title = "Function: safeTransferFrom";
static const char *EVM_safeTransferFrom_Signature =
    "safeTransferFrom(address,address,uint256)";

static const Abi_Type_e EVM_depositDataType[EVM_deposit_NUM_ARGS] = {};

static const char *EVM_deposit_Title = "Function: deposit";
static const char *EVM_deposit_Signature = "deposit()";

static const Abi_Type_e EVM_transferDataType[EVM_transfer_NUM_ARGS] = {
    Abi_address_e,
    Abi_uint256_e,
};
static const char *EVM_transfer_Title = "Function: transfer";
static const char *EVM_transfer_Signature = "transfer(address,uint256)";

static const char *EVM_approve_Title = "Function: approve";
static const char *EVM_approve_Signature = "approve(address,uint256)";

static const char *EVM_hysp_depositInstant_Title = "Function: depositInstant";
static const char *EVM_hysp_depositInstant_Signature =
    "depositInstant(address,uint256,uint256,bytes32)";

static const char *EVM_hysp_redeemInstant_Title = "Function: redeemInstant";
static const char *EVM_hysp_redeemInstant_Signature =
    "redeemInstant(address,uint256,uint256)";

static const char *EVM_hysp_redeemRequest_Title = "Function: redeemRequest";
static const char *EVM_hysp_redeemRequest_Signature =
    "redeemRequest(address,uint256)";

/* HYSP vault contract table:
 * Maps the two Midas vault addresses (ETH mainnet) to human-readable names. */
static const struct {
  const uint8_t address[EVM_ADDRESS_LENGTH];
  const char *name;
} hysp_known_vaults[] = {
    /* Midas issuance vault (ETH): 0x5455222CCDd32F85C1998f57DC6CF613B4498C2a
     * depositInstant is called on this address */
    {{0x54, 0x55, 0x22, 0x2c, 0xcd, 0xd3, 0x2f, 0x85, 0xc1, 0x99,
      0x8f, 0x57, 0xdc, 0x6c, 0xf6, 0x13, 0xb4, 0x49, 0x8c, 0x2a},
     "Midas Issuance Vault"},
    /* Midas redemption vault (ETH): 0x9C3743582e8b2d7cCb5e08caF3c9C33780ac446f
     * redeemInstant and redeemRequest are called on this address */
    {{0x9c, 0x37, 0x43, 0x58, 0x2e, 0x8b, 0x2d, 0x7c, 0xcb, 0x5e,
      0x08, 0xca, 0xf3, 0xc9, 0xc3, 0x37, 0x80, 0xac, 0x44, 0x6f},
     "Midas Redemption Vault"},
    /* Base - Issuance: 0x5f09Aff8B9b1f488B7d1bbaD4D89648579e55d61 */
    {{0x5f, 0x09, 0xAf, 0xf8, 0xB9, 0xb1, 0xf4, 0x88, 0xB7, 0xd1,
      0xbb, 0xaD, 0x4D, 0x89, 0x64, 0x85, 0x79, 0xe5, 0x5d, 0x61},
     "Midas Issuance Vault (Base)"},
    /* Base - Redemption: 0x9BF00b7CFC00D6A7a2e2C994DB8c8dCa467ee359 */
    {{0x9B, 0xF0, 0x0b, 0x7C, 0xFC, 0x00, 0xD6, 0xA7, 0xa2, 0xe2,
      0xC9, 0x94, 0xDB, 0x8c, 0x8d, 0xCa, 0x46, 0x7e, 0xe3, 0x59},
     "Midas Redemption Vault (Base)"},

};
#define HYSP_KNOWN_VAULT_COUNT                                                 \
  (sizeof(hysp_known_vaults) / sizeof(hysp_known_vaults[0]))

/* Semantic Parser Registry — Token-Aware Display
 *
 * Registered functions show "2.000000 USDC" instead of "0x001e8480".
 * ETH_ExtractArguments checks this registry first; unregistered selectors
 * fall back to ETH_DetectFunction (generic hex display).
 *
 * Currently: 4 HYSP functions (all share hysp_parse_function).
 * All other functions (swap, deposit, etc.) use hex display.
 */

static const SemanticParserEntry_t s_semantic_parsers[] = {
    {EVM_approve_TAG, hysp_parse_function},
    {EVM_hysp_depositInstant_TAG, hysp_parse_function},
    {EVM_hysp_redeemInstant_TAG, hysp_parse_function},
    {EVM_hysp_redeemRequest_TAG, hysp_parse_function},
};
#define SEMANTIC_PARSER_COUNT                                                  \
  (sizeof(s_semantic_parsers) / sizeof(s_semantic_parsers[0]))

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static uint8_t ETH_DetectFunction(const uint32_t functionTag,
                                  Abi_Type_e const **const dpAbiTypeArray,
                                  ui_display_node **displayNode) {
  if (NULL == dpAbiTypeArray) {
    return 0;
  }

  uint8_t numArgsInFunction = 0;
  const char *EvmFunctionTitle = NULL;
  const char *EvmFunctionSignature = NULL;

  switch (functionTag) {
    case EVM_swap_TAG: {
      numArgsInFunction = EVM_swap_NUM_ARGS;
      *(dpAbiTypeArray) = &(EVM_swapDataType[0]);
      EvmFunctionTitle = EVM_swap_Title;
      EvmFunctionSignature = EVM_swap_Signature;
      break;
    }
    case EVM_uniswapV3Swap_TAG: {
      numArgsInFunction = EVM_uniswapV3Swap_NUM_ARGS;
      *(dpAbiTypeArray) = &(EVM_uniswapV3SwapDataType[0]);
      EvmFunctionTitle = EVM_uniswapV3Swap_Title;
      EvmFunctionSignature = EVM_uniswapV3Swap_Signature;
      break;
    }
    case EVM_safeTransferFrom_TAG: {
      numArgsInFunction = EVM_safeTransferFrom_NUM_ARGS;
      *(dpAbiTypeArray) = &(EVM_safeTransferFromDataType[0]);
      EvmFunctionTitle = EVM_safeTransferFrom_Title;
      EvmFunctionSignature = EVM_safeTransferFrom_Signature;
      break;
    }
    case EVM_deposit_TAG: {
      numArgsInFunction = EVM_deposit_NUM_ARGS;
      *(dpAbiTypeArray) = &(EVM_depositDataType[0]);
      EvmFunctionTitle = EVM_deposit_Title;
      EvmFunctionSignature = EVM_deposit_Signature;
      break;
    }
    case EVM_transfer_TAG: {
      numArgsInFunction = EVM_transfer_NUM_ARGS;
      *(dpAbiTypeArray) = &(EVM_transferDataType[0]);
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

    if (*displayNode == NULL) {
      *displayNode = pAbiDispNode;
    } else {
      ui_display_node *temp = *displayNode;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = pAbiDispNode;
    }
  }

  return numArgsInFunction;
}

/**
 * @brief Look up the semantic parser registered for a given function selector.
 * @param selector  4-byte function selector (big-endian uint32).
 * @return Pointer to the matching parser, or NULL if none is registered.
 */
static SemanticParser_fn find_semantic_parser(uint32_t selector) {
  for (uint8_t i = 0; i < SEMANTIC_PARSER_COUNT; i++) {
    if (s_semantic_parsers[i].selector == selector) {
      return s_semantic_parsers[i].parser;
    }
  }
  return NULL;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload,
                             const uint64_t sizeOfPayload,
                             const uint8_t *toAddress,
                             ui_display_node **displayNode) {
  uint8_t returnCode = ETH_BAD_ARGUMENTS;

  /* Size of transaction payload must be atleast EVM_FUNC_SIGNATURE_LENGTH */
  if ((NULL == pAbiPayload) || (EVM_FUNC_SIGNATURE_LENGTH > sizeOfPayload) ||
      (NULL == displayNode)) {
    return returnCode;
  }

  uint8_t *pCurrHeadPtr = (uint8_t *)pAbiPayload;
  uint32_t functionTag = U32_READ_BE_ARRAY(pCurrHeadPtr);

  /* Try semantic parser first — token-aware display for registered protocols.
   * If found, delegate entirely and return its result.
   * If not found, fall through to the generic ABI parser below. */
  SemanticParser_fn semantic_parser = find_semantic_parser(functionTag);
  if (semantic_parser != NULL) {
    return semantic_parser(
        functionTag, pAbiPayload, sizeOfPayload, toAddress, displayNode);
  }

  uint8_t numArgsInFunction = 0;
  Abi_Type_e const *pArgumentAbiType = NULL;

  numArgsInFunction =
      ETH_DetectFunction(functionTag, &pArgumentAbiType, displayNode);

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

  // if argument count is zero (eg. `deposit`), it's an immediate success
  if (0 == numArgsInFunction) {
    returnCode = ETH_UTXN_ABI_DECODE_OK;
  }

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

    if (*displayNode == NULL) {
      *displayNode = pAbiDispNode;
    } else {
      ui_display_node *temp = *displayNode;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = pAbiDispNode;
    }
  }

  return returnCode;
}

/* HYSP — Token-Aware Display Helpers */
/**
 * @brief Append a display node (title/value) to the linked list.
 * @note  ui_create_display_node copies the strings internally, so stack
 *        buffers passed as title/value are safe to pass here.
 */
static void hysp_append_node(ui_display_node **head,
                             const char *title,
                             const char *value) {
  ui_display_node *node = ui_create_display_node(
      title, strnlen(title, 64), value, strnlen(value, 128));
  if (node == NULL)
    return;
  if (*head == NULL) {
    *head = node;
  } else {
    ui_display_node *tail = *head;
    while (tail->next != NULL)
      tail = tail->next;
    tail->next = node;
  }
}

/**
 * @brief Format a 32-byte ABI uint256 slot as a human-readable decimal amount
 *        followed by the token symbol (e.g. "6.000000 USDC").
 *
 * @param raw32     Pointer to the 32-byte ABI slot (big-endian uint256).
 * @param decimals  Number of decimal places for the token.
 * @param symbol    Token symbol string (e.g. "USDC").
 * @param out_buf   Output buffer — must be at least 64 bytes.
 * @param out_size  Size of out_buf.
 */
static void hysp_format_amount(const uint8_t *raw32,
                               uint8_t decimals,
                               const char *symbol,
                               char *out_buf,
                               size_t out_size) {
  char hex_str[65] = {0};
  char dec_str[50] = {0};
  byte_array_to_hex_string(raw32, 32, hex_str, sizeof(hex_str));
  convert_byte_array_to_decimal_string(
      64, decimals, hex_str, dec_str, sizeof(dec_str));
  snprintf(out_buf, out_size, "%s %s", dec_str, symbol);
}

/**
 * @brief Format a 32-byte ABI address slot as a checksummed "0x..." string.
 *
 * @param slot32   Pointer to the full 32-byte ABI slot (12-byte left-pad
 *                 + 20-byte address).
 * @param out_buf  Output buffer — must be at least 43 bytes.
 * @param out_size Size of out_buf.
 */
static void hysp_format_address(const uint8_t *slot32,
                                char *out_buf,
                                size_t out_size) {
  if (out_size < 43)
    return;
  out_buf[0] = '0';
  out_buf[1] = 'x';
  /* Address is right-aligned in the 32-byte slot; skip 12-byte left padding */
  ethereum_address_checksum(slot32 + Abi_address_e_OFFSET_BE,
                            out_buf + 2,
                            false,
                            g_evm_app->chain_id);
}

/**
 * @brief Return a human-readable name for a known HYSP contract address, or
 *        NULL if the address is not in the HYSP tables.
 *
 * @details Checks hysp_known_tokens first (returns symbol, e.g. "USDC" for
 *          approve() calls on the token contract), then hysp_known_vaults
 *          (returns descriptive name, e.g. "Midas Issuance Vault").
 *          Returns NULL for any address not in either table — callers must
 *          treat NULL as "unknown contract" and show the standard warning.
 *
 * @param addr20  20-byte contract address (raw bytes, not a 32-byte ABI slot).
 * @return const char * — static string, valid for lifetime of the firmware.
 *         NULL if address is not a known HYSP contract.
 */
const char *HYSP_FindContractName(const uint8_t *addr20) {
  /* Token contracts (USDC, USDT, mevUSD) — approve() is called on these */
  const erc20_contracts_t *token = NULL;
  g_evm_app->is_token_whitelisted(addr20, &token);
  if (token != NULL) {
    return token->symbol;
  }
  /* Vault contracts (issuance, redemption) */
  for (uint8_t i = 0; i < HYSP_KNOWN_VAULT_COUNT; i++) {
    if (memcmp(addr20, hysp_known_vaults[i].address, EVM_ADDRESS_LENGTH) == 0) {
      return hysp_known_vaults[i].name;
    }
  }
  return NULL;
}

/**
 * @brief Semantic parser for HYSP vault functions.
 * @details Produces token-aware display nodes (symbols, decimal amounts) for
 *          approve, depositInstant, redeemInstant, and redeemRequest.
 *          Registered in s_semantic_parsers and called via find_semantic_parser
 *          from ETH_ExtractArguments — not called directly.
 *
 * @param selector      4-byte function selector (big-endian uint32).
 * @param pAbiPayload   Full data field including the 4-byte selector.
 * @param sizeOfPayload Total size of pAbiPayload in bytes.
 * @param toAddress     Transaction's to_address (20 bytes). Required by
 *                      approve() to identify the token being approved.
 * @param displayNode   Output: pointer to linked list of display nodes.
 * @return ETH_UTXN_ABI_DECODE_OK on success, error code otherwise.
 */
static uint8_t hysp_parse_function(uint32_t selector,
                                   const uint8_t *pAbiPayload,
                                   uint64_t sizeOfPayload,
                                   const uint8_t *toAddress,
                                   ui_display_node **displayNode) {
  uint32_t functionTag = selector;
  if (NULL == pAbiPayload || NULL == displayNode || NULL == toAddress ||
      sizeOfPayload < EVM_FUNC_SIGNATURE_LENGTH) {
    return ETH_BAD_ARGUMENTS;
  }

  /* args points to the first 32-byte argument slot (after the 4-byte selector)
   */
  const uint8_t *args = pAbiPayload + EVM_FUNC_SIGNATURE_LENGTH;
  uint64_t args_size = sizeOfPayload - EVM_FUNC_SIGNATURE_LENGTH;

  /* Macro: return error if fewer than (n) full 32-byte slots are available */
#define HYSP_CHECK_SLOTS(n)                                                    \
  do {                                                                         \
    if ((uint64_t)(n) * ABI_ELEMENT_SZ_IN_BYTES > args_size)                   \
      return ETH_UTXN_BAD_PAYLOAD;                                             \
  } while (0)

  /* Working buffers — stack-allocated; ui_create_display_node copies strings */
  char addr_buf[43] = "0x"; /* "0x" + 40 hex chars + NUL */
  char amount_buf[72] = {
      '\0'}; /* dec_str(49) + space(1) + symbol(20) + NUL(1) = 71 max */

  const char *tok_sym = NULL;
  uint8_t tok_dec = 0;

  switch (functionTag) {
    /* approve(address spender, uint256 amount)
     * Called ON the token contract — to_address IS the token.
     *   arg0 (slot 0): spender address (the vault)
     *   arg1 (slot 1): amount in token's native units */
    case EVM_approve_TAG: {
      HYSP_CHECK_SLOTS(2);

      /* Function header node */
      hysp_append_node(displayNode, EVM_approve_Title, EVM_approve_Signature);

      /* Token: identified from the transaction's to_address */
      const erc20_contracts_t *token = NULL;
      g_evm_app->is_token_whitelisted(toAddress, &token);
      if (token == NULL) {
        return ETH_UTXN_FUNCTION_NOT_FOUND;
      }
      tok_sym = token->symbol;
      tok_dec = token->decimal;
      hysp_append_node(displayNode, ui_text_hysp_label_token, tok_sym);

      /* Spender address (arg0) */
      hysp_format_address(
          args + 0 * ABI_ELEMENT_SZ_IN_BYTES, addr_buf, sizeof(addr_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_spender, addr_buf);

      /* Amount (arg1) in token units */
      hysp_format_amount(args + 1 * ABI_ELEMENT_SZ_IN_BYTES,
                         tok_dec,
                         tok_sym,
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_amount, amount_buf);
      break;
    }

    /* depositInstant(address tokenIn, uint256 amount,
     *                uint256 minReceiveAmount, bytes32 referrerId)
     * Called ON the issuance vault.
     *   arg0 (slot 0): tokenIn address (USDC / USDT)
     *   arg1 (slot 1): amount of tokenIn to deposit
     *   arg2 (slot 2): minimum mevUSD to receive
     *   arg3 (slot 3): referrer ID (bytes32, human-readable ASCII) */
    case EVM_hysp_depositInstant_TAG: {
      HYSP_CHECK_SLOTS(4);

      hysp_append_node(displayNode,
                       EVM_hysp_depositInstant_Title,
                       EVM_hysp_depositInstant_Signature);

      /* arg0: tokenIn */
      const erc20_contracts_t *token = NULL;
      uint8_t tokenAddr[EVM_ADDRESS_LENGTH] = {0};
      memcpy(tokenAddr,
             args + 0 * ABI_ELEMENT_SZ_IN_BYTES + 12,
             EVM_ADDRESS_LENGTH);
      g_evm_app->is_token_whitelisted(tokenAddr, &token);
      if (token == NULL) {
        return ETH_UTXN_FUNCTION_NOT_FOUND;
      }
      tok_sym = token->symbol;
      tok_dec = token->decimal;
      hysp_append_node(displayNode, ui_text_hysp_label_token_in, tok_sym);

      /* arg1: deposit amount — always encoded at 18 decimals by the HYSP vault
       * regardless of tokenIn's native decimals (e.g. USDC is 1e6 natively).
       * Ref: IDepositVault.depositInstant NatSpec — "amountToken (decimals 18)"
       * https://etherscan.io/address/0x6a3996f840c5f62a27cff3a204a33f588d30e40e#code#F32#L158
       */
      hysp_format_amount(args + 1 * ABI_ELEMENT_SZ_IN_BYTES,
                         HYSP_MEVUSD_DECIMALS,
                         tok_sym,
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_amount, amount_buf);

      /* arg2: minimum mevUSD to receive (18 decimals) */
      hysp_format_amount(args + 2 * ABI_ELEMENT_SZ_IN_BYTES,
                         HYSP_MEVUSD_DECIMALS,
                         "mevUSD",
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_min_receive, amount_buf);
      break;
    }

    /* redeemInstant(address tokenOut, uint256 amount,
     *               uint256 minReceiveAmount)
     * Called ON the redemption vault.
     *   arg0 (slot 0): tokenOut address (USDC / USDT)
     *   arg1 (slot 1): mevUSD amount to redeem (18 decimals)
     *   arg2 (slot 2): minimum tokenOut to receive */
    case EVM_hysp_redeemInstant_TAG: {
      HYSP_CHECK_SLOTS(3);

      hysp_append_node(displayNode,
                       EVM_hysp_redeemInstant_Title,
                       EVM_hysp_redeemInstant_Signature);

      /* arg0: tokenOut */
      const erc20_contracts_t *token = NULL;
      uint8_t tokenAddr[EVM_ADDRESS_LENGTH] = {0};
      memcpy(tokenAddr,
             args + 0 * ABI_ELEMENT_SZ_IN_BYTES + 12,
             EVM_ADDRESS_LENGTH);
      g_evm_app->is_token_whitelisted(tokenAddr, &token);
      if (token == NULL) {
        return ETH_UTXN_FUNCTION_NOT_FOUND;
      }
      tok_sym = token->symbol;
      tok_dec = token->decimal;
      hysp_append_node(displayNode, ui_text_hysp_label_token_out, tok_sym);

      /* arg1: mevUSD input amount (18 decimals) */
      hysp_format_amount(args + 1 * ABI_ELEMENT_SZ_IN_BYTES,
                         HYSP_MEVUSD_DECIMALS,
                         "mevUSD",
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_amount_in, amount_buf);

      /* arg2: minimum tokenOut amount */
      hysp_format_amount(args + 2 * ABI_ELEMENT_SZ_IN_BYTES,
                         tok_dec,
                         tok_sym,
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_min_receive, amount_buf);
      break;
    }

    /* ----------------------------------------------------------------
     * redeemRequest(address tokenOut, uint256 amount)
     * Called ON the redemption vault.
     *   arg0 (slot 0): tokenOut address (USDC / USDT)
     *   arg1 (slot 1): mevUSD amount to redeem (18 decimals)
     * ---------------------------------------------------------------- */
    case EVM_hysp_redeemRequest_TAG: {
      HYSP_CHECK_SLOTS(2);

      hysp_append_node(displayNode,
                       EVM_hysp_redeemRequest_Title,
                       EVM_hysp_redeemRequest_Signature);

      /* arg0: tokenOut */
      const erc20_contracts_t *token = NULL;
      uint8_t tokenAddr[EVM_ADDRESS_LENGTH] = {0};
      memcpy(tokenAddr,
             args + 0 * ABI_ELEMENT_SZ_IN_BYTES + 12,
             EVM_ADDRESS_LENGTH);
      g_evm_app->is_token_whitelisted(tokenAddr, &token);
      if (token == NULL) {
        return ETH_UTXN_FUNCTION_NOT_FOUND;
      }
      tok_sym = token->symbol;
      hysp_append_node(displayNode, ui_text_hysp_label_token_out, tok_sym);

      /* arg1: mevUSD input amount (18 decimals) */
      hysp_format_amount(args + 1 * ABI_ELEMENT_SZ_IN_BYTES,
                         HYSP_MEVUSD_DECIMALS,
                         "mevUSD",
                         amount_buf,
                         sizeof(amount_buf));
      hysp_append_node(displayNode, ui_text_hysp_label_amount_in, amount_buf);
      break;
    }

    default:
      return ETH_UTXN_FUNCTION_NOT_FOUND;
  }

#undef HYSP_CHECK_SLOTS
  return ETH_UTXN_ABI_DECODE_OK;
}
