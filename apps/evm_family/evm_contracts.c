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

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload,
                             const uint64_t sizeOfPayload,
                             ui_display_node **displayNode) {
  uint8_t returnCode = ETH_BAD_ARGUMENTS;

  /* Size of transaction payload must be atleast EVM_FUNC_SIGNATURE_LENGTH */
  if ((NULL == pAbiPayload) || (EVM_FUNC_SIGNATURE_LENGTH > sizeOfPayload) ||
      (NULL == displayNode)) {
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
