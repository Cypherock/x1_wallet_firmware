/**
 * @file    abi_encode.c
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
uint8_t Abi_Encode(Abi_Type_e inputAbiType,
                   uint8_t additionalInput,
                   const uint8_t *pAbiTypeData,
                   uint8_t *pBuffer) {
  uint8_t returnCode = ABI_BAD_ARGUMENT;

  if ((NULL == pAbiTypeData) || (NULL == pBuffer)) {
    return returnCode;
  }

  switch (inputAbiType) {
    case Abi_uint256_e: {
      /**
       * Assuming Abi_uint256_e input is big-endian and additionalInput
       * holds number of bytes in the Abi_uint256_e
       */

      /**
       * additionalInput refers to the number of bytes to be encoded
       * Ensure argument holds valid value
       */
      if (32 < additionalInput) {
        returnCode = ABI_BAD_ARGUMENT;
        break;
      }

      /* Calculate padding and fill with leading 0's */
      uint32_t paddingLen = (ABI_ELEMENT_SZ_IN_BYTES - additionalInput);

      if (0 < paddingLen) {
        memzero(pBuffer, paddingLen);
      }

      /* Copy additionalInput bytes to the output pBuffer */
      memcpy((void *)(pBuffer + paddingLen), pAbiTypeData, additionalInput);

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_address_e: {
      /**
       * Abi_address_e is 20 bytes (uint160) long
       * So pad first 12 bytes with 0's followed by address data
       */
      memzero(pBuffer, Abi_address_e_OFFSET_BE);

      memcpy((void *)(pBuffer + Abi_address_e_OFFSET_BE),
             pAbiTypeData,
             Abi_address_e_SZ_IN_BYTES);

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_bytes_e: {
      /**
       * additionalInput refers to the number of bytes to be encoded
       * Ensure argument holds valid value
       */
      if (32 < additionalInput) {
        returnCode = ABI_BAD_ARGUMENT;
        break;
      }

      /* Copy additionalInput bytes to the output pBuffer */
      memcpy(pBuffer, pAbiTypeData, additionalInput);

      /* Calculate padding and fill with trailing 0's */
      uint32_t paddingLen = (ABI_ELEMENT_SZ_IN_BYTES - additionalInput);

      if (0 < paddingLen) {
        memzero((void *)(pBuffer + additionalInput), paddingLen);
      }

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_int256_e: {
      /* additionalInput contains the number of bytes of data present */
      if (32 < additionalInput) {
        returnCode = ABI_BAD_ARGUMENT;
        break;
      }

      /* If additionalInput is zero, then fill with 0's */
      if (0 == additionalInput) {
        memzero(pBuffer, Abi_uint256_e_SZ_IN_BYTES);

        returnCode = ABI_PROCESS_COMPLETE;
        break;
      }

      /**
       * Test the MSB of the pAbiTypeData to check if the integer value
       * is positive or negative. If the value is positive, we will
       * pad the value with leading 0's like Abi_uint256_e, otherwise,
       * we will pad the value with leading 1's
       */
      uint8_t msbValue = *((uint8_t *)pAbiTypeData);

      if (0 == (msbValue & 0x80)) {
        /* Calculate padding and fill with leading 0's */
        uint32_t paddingLen = (ABI_ELEMENT_SZ_IN_BYTES - additionalInput);

        if (0 < paddingLen) {
          memzero(pBuffer, paddingLen);
        }

        /* Copy additionalInput bytes to the output pBuffer */
        memcpy((void *)(pBuffer + paddingLen), pAbiTypeData, additionalInput);

        returnCode = ABI_PROCESS_COMPLETE;
        break;
      } else {
        /* Calculate padding and fill with leading 1's */
        uint32_t paddingLen = (ABI_ELEMENT_SZ_IN_BYTES - additionalInput);

        if (0 < paddingLen) {
          memset(pBuffer, ABI_PADDING_ONES, paddingLen);
        }

        /* Copy additionalInput bytes to the output pBuffer */
        memcpy((void *)(pBuffer + paddingLen), pAbiTypeData, additionalInput);

        returnCode = ABI_PROCESS_COMPLETE;
        break;
      }

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_bool_e: {
      /**
       * Abi_bool_e is 1 byte (uint8) long
       * So pad first 30 bytes with 0's followed by boolean data
       */
      memzero(pBuffer, Abi_bool_e_OFFSET_BE);

      memcpy((void *)(pBuffer + Abi_bool_e_OFFSET_BE),
             pAbiTypeData,
             Abi_bool_e_SZ_IN_BYTES);

      returnCode = ABI_PROCESS_COMPLETE;
      break;
    }
    case Abi_bytes_dynamic_e:
    case Abi_uint256_array_dynamic_e: {
      /* Just added to suppress compilation warning */
      break;
    }
    default: {
      break;
    }
  }

  return returnCode;
}