/**
 * @file    abi_decode.c
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

#include <stdint.h>

#include "abi.h"
#include "assert_conf.h"
#include "utils.h"

/* Global functions
 *****************************************************************************/
uint8_t Abi_DynamicHelp(Abi_Type_e inputAbiType,
                        uint8_t *pAbiTypeData,
                        const uint8_t *pAbiTypeDataBase,
                        const uint32_t sizeOfAbiChunk,
                        uint32_t *pNumBytesReturned,
                        uint8_t **dpAbiDataPtr) {
  uint8_t returnCode = ABI_BAD_ARGUMENT;

  if ((NULL == pAbiTypeData) || (NULL == pAbiTypeDataBase) ||
      (NULL == pNumBytesReturned) || (NULL == dpAbiDataPtr)) {
    return returnCode;
  }

  switch (inputAbiType) {
    /* Handle all Abi_Type_e to supress compilation warning */
    case Abi_uint256_e:
    case Abi_address_e:
    case Abi_bytes_e:
    case Abi_int256_e:
    case Abi_bool_e: {
      break;
    }
    case Abi_bytes_dynamic_e: {
      uint32_t offsetDynamicData;
      offsetDynamicData =
          U32_READ_BE_ARRAY(pAbiTypeData + ABI_DYN_METADATA_OFFET_BE);

      /* Goto offset of the dynamic input */
      pAbiTypeData = ((uint8_t *)pAbiTypeDataBase) + offsetDynamicData;

      /* Ensure if reading from pAbiTypeData is safe */
      if (UTIL_IN_BOUNDS != UTIL_CheckBound(pAbiTypeDataBase,
                                            sizeOfAbiChunk,
                                            pAbiTypeData,
                                            ABI_ELEMENT_SZ_IN_BYTES)) {
        returnCode = ABI_PROCESS_INCOMPLETE;
        break;
      }

      /**
       * pAbiTypeData points now to the 32-byte data denoting num of bytes
       * in the dynamic input
       */
      uint32_t numBytesInData;
      numBytesInData =
          U32_READ_BE_ARRAY(pAbiTypeData + ABI_DYN_METADATA_OFFET_BE);

      /* Increment pAbiTypeData by ABI_ELEMENT_SZ_IN_BYTES so that it points to
       * the bytes */
      pAbiTypeData += ABI_ELEMENT_SZ_IN_BYTES;

      /* Ensure if reading numBytesInData bytes from pAbiTypeData is safe */
      if (UTIL_IN_BOUNDS !=
          UTIL_CheckBound(
              pAbiTypeDataBase, sizeOfAbiChunk, pAbiTypeData, numBytesInData)) {
        returnCode = ABI_PROCESS_INCOMPLETE;
        break;
      }

      /**
       * Finally inform the caller about number of bytes held by dynamic bytes
       * (*pNumBytesReturned) & pointer to start of bytes (*dpAbiDataPtr)
       */
      *pNumBytesReturned = numBytesInData;
      *dpAbiDataPtr = pAbiTypeData;

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_uint256_array_dynamic_e: {
      uint32_t offsetDynamicData;
      offsetDynamicData =
          U32_READ_BE_ARRAY(pAbiTypeData + ABI_DYN_METADATA_OFFET_BE);

      /* Goto offset of the dynamic input */
      pAbiTypeData = ((uint8_t *)pAbiTypeDataBase) + offsetDynamicData;

      /* Ensure if reading from pAbiTypeData is safe */
      if (UTIL_IN_BOUNDS != UTIL_CheckBound(pAbiTypeDataBase,
                                            sizeOfAbiChunk,
                                            pAbiTypeData,
                                            ABI_ELEMENT_SZ_IN_BYTES)) {
        returnCode = ABI_PROCESS_INCOMPLETE;
        break;
      }

      /**
       * pAbiTypeData now points to the 32-byte data denoting num of elements
       * in the uint256[] array
       */
      uint32_t numElementsInDataArr;
      numElementsInDataArr =
          U32_READ_BE_ARRAY(pAbiTypeData + ABI_DYN_METADATA_OFFET_BE);

      /* Increment pAbiTypeData by ABI_ELEMENT_SZ_IN_BYTES so that it points to
       * the array */
      pAbiTypeData += ABI_ELEMENT_SZ_IN_BYTES;

      /* Ensure if reading numElementsInDataArr uint256 from pAbiTypeData is
       * safe */
      if (UTIL_IN_BOUNDS !=
          UTIL_CheckBound(pAbiTypeDataBase,
                          sizeOfAbiChunk,
                          pAbiTypeData,
                          (ABI_ELEMENT_SZ_IN_BYTES * numElementsInDataArr))) {
        returnCode = ABI_PROCESS_INCOMPLETE;
        break;
      }

      /**
       * Finally inform the caller about number of elements held by
       * uint256[] (*pNumBytesReturned) & pointer to start of array
       * of uint256 (*dpAbiDataPtr)
       */
      *pNumBytesReturned = numElementsInDataArr;
      *dpAbiDataPtr = pAbiTypeData;

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    default: {
      break;
    }
  }

  return returnCode;
}

ui_display_node *ABI_Stringify(Abi_Type_e inputAbiType,
                               uint8_t *pAbiTypeData,
                               uint32_t additionalData) {
  ui_display_node *ui_node = NULL;

  switch (inputAbiType) {
    case Abi_uint256_e: {
      char staticBufferInUTF8[200];
      memzero(staticBufferInUTF8, sizeof(staticBufferInUTF8));

      byte_array_to_hex_string(pAbiTypeData, 32, &(staticBufferInUTF8[0]), 65);

      convert_byte_array_to_decimal_string(
          64, 0, &(staticBufferInUTF8[0]), &(staticBufferInUTF8[100]), 100);

      ui_node = ui_create_display_node(
          "Datatype:uint256\0", 25, &(staticBufferInUTF8[100]), 100);
      break;
    }
    case Abi_address_e: {
      char staticBufferInUTF8[43] = "0x";

      byte_array_to_hex_string(pAbiTypeData + Abi_address_e_OFFSET_BE,
                               20,
                               &(staticBufferInUTF8[2]),
                               41);

      ui_node = ui_create_display_node("Datatype:address\0",
                                       25,
                                       &(staticBufferInUTF8[0]),
                                       sizeof(staticBufferInUTF8));
      break;
    }
    case Abi_bytes_e: {
      uint32_t numBytesBuffer = (2 * additionalData) + 1;

      char *dynamicBufferInUTF8 = (char *)malloc(numBytesBuffer);

      /* Assert to ensure that malloc did not fail */
      ASSERT(NULL != dynamicBufferInUTF8);
      memzero(dynamicBufferInUTF8, numBytesBuffer);

      byte_array_to_hex_string(
          pAbiTypeData, additionalData, dynamicBufferInUTF8, numBytesBuffer);

      ui_node = ui_create_display_node(
          "Datatype:bytes\0", 25, dynamicBufferInUTF8, numBytesBuffer);
      free(dynamicBufferInUTF8);
    }
    /* Handle all Abi_Type_e to suppress compilation warning */
    case Abi_bytes_dynamic_e:
    case Abi_uint256_array_dynamic_e:
    default: {
      break;
    }
  }

  return ui_node;
}