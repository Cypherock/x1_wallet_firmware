/**
 * @file    abi_decode.c
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

#include <stdint.h>
#include "abi_decode.h"
#include "assert_conf.h"
#include "utils.h"

/* Static function prototypes
 *****************************************************************************/
static void ABI_MemcpyBeDataInLeFmt(
                                    uint8_t *pConvertedBuffer,
                                    uint8_t *pAbiTypeData,
                                    uint8_t numBytes
                                   );

static void ABI_GetDynamicMetaData(
                                    ABI_Type_e inputAbiType,
                                    uint8_t optInput,
                                    uint8_t *pAbiTypeData,
                                    uint8_t *pConvertedBuffer
                                  );


/* Global functions
 *****************************************************************************/
void ABI_DecodeStaticValByType(
                                ABI_Type_e inputAbiType,
                                uint8_t optInput,
                                uint8_t *pAbiTypeData,
                                uint8_t *pConvertedBuffer
                              )
{
    if (
        (NULL == pAbiTypeData)              ||
        (NULL == pConvertedBuffer)
       )
    {
        return;
    }

    switch (inputAbiType)
    {
        case ABI_bytes_e:
        {
            memcpy(
                    pConvertedBuffer,
                    pAbiTypeData,
                    optInput
                  );
            break;
        }
        case ABI_uint256_e:
        {
            ABI_MemcpyBeDataInLeFmt(
                                    pConvertedBuffer,
                                    pAbiTypeData,
                                    32
                                   );
            break;
        }
        case ABI_address_e:
        {
            ABI_DecodeStaticValByType(
                                        ABI_bytes_e,
                                        20,
                                        (uint8_t *)(pAbiTypeData + 12),
                                        pConvertedBuffer
                                     );
            break;
        }
        case ABI_bytes_dynamic_e:
        case ABI_uint256_array_dynamic_e:
        default:
        {
            break;
        }
    }
}



void ABI_DecodeDynamicValByType(
                                ABI_Type_e inputAbiType,
                                uint8_t optInput,
                                uint8_t *pAbiTypeData,
                                uint8_t *pConvertedBuffer
                               )
{
    if (
        (NULL == pAbiTypeData)              ||
        (NULL == pConvertedBuffer)
       )
    {
        return;
    }

    switch (inputAbiType)
    {
        case ABI_uintx_e:
        case ABI_uint256_e:
        case ABI_address_e:
        case ABI_bytes_e:
        {
            break;
        }
        case ABI_bytes_dynamic_e:
        {
            /**
             * pAbiTypeData points to the 32-byte data denoting num of bytes
             * in the dynamic input
             */
            uint32_t numBytesInData;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    4,
                                    (uint8_t *)(pAbiTypeData),
                                    &numBytesInData
                                  );

            /* Increment pAbiTypeData pointer to point to the first 32 byte data */
            pAbiTypeData += 32;
            
            /* Copy bytes data in chunks of 32-byte into the buffer */
            uint32_t byteIndex;
            uint32_t bytesOverMultipleOf32;
            
            for (byteIndex = 0; byteIndex < (numBytesInData / 32); byteIndex++)
            {
                /**
                 * Calls into ABI_DecodeStaticValByType with base 
                 * ABI_Type_e ABI_bytes_e
                 */
                ABI_DecodeStaticValByType(
                                            ABI_bytes_e,
                                            32,
                                            pAbiTypeData,
                                            pConvertedBuffer
                                         );
                
                /**
                 * Increment pAbiTypeData pointer and pConvertedBuffer pointer 
                 * so that next they point to valid addresses 
                 */
                pAbiTypeData += 32;
                pConvertedBuffer += 32;
            }

            /* Copy remaining data bytes */
            bytesOverMultipleOf32 = (numBytesInData % 32);
            ABI_DecodeStaticValByType(
                                        ABI_bytes_e,
                                        bytesOverMultipleOf32,
                                        pAbiTypeData,
                                        pConvertedBuffer
                                     );  

            break;              
        }
        case ABI_uint256_array_dynamic_e:
        {
            /**
             * pAbiTypeData points to the 32-byte data denoting num of elements
             * in the uint256[] array
             */
            uint32_t numElementsInDataArr;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    4,
                                    (uint8_t *)(pAbiTypeData),
                                    &numElementsInDataArr
                                  );
            
            /* Increment pAbiTypeData pointer to point to the first 32 byte data */
            pAbiTypeData += 32;
            
            /* Copy bytes data in chunks of 32-byte into the buffer */
            uint32_t u256EleIndex;
            
            for (u256EleIndex = 0; u256EleIndex < numElementsInDataArr; u256EleIndex++)
            {
                /** 
                 * Calls into ABI_DecodeStaticValByType with base ABI_Type_e 
                 * ABI_uint256_e
                 */
                ABI_DecodeStaticValByType(
                                            ABI_uint256_e,
                                            0,
                                            pAbiTypeData,
                                            pConvertedBuffer
                                         );
                
                /**
                 * Increment pAbiTypeData pointer and pConvertedBuffer pointer so that next
                 * they point to valid addresses 
                 */
                pAbiTypeData += 32;
                pConvertedBuffer += 32;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/* Static functions
 *****************************************************************************/
static void ABI_MemcpyBeDataInLeFmt(
                                    uint8_t *pConvertedBuffer,
                                    uint8_t *pAbiTypeData,
                                    uint8_t numBytes
                                   )
{
    if (
        (NULL == pAbiTypeData)              ||
        (NULL == pConvertedBuffer)
       )
    {
        return;
    }

    uint8_t *pSrcPtr = (uint8_t *)(pAbiTypeData + 31);
    
    while (
            (NULL != pSrcPtr)                   &&
            (NULL != pConvertedBuffer)          &&
            (0 < numBytes)
          )
    {
        *pConvertedBuffer = *pSrcPtr;
        
        pConvertedBuffer += 1;
        pSrcPtr -= 1;
        numBytes -= 1;
    }  

    return;
}

static void ABI_GetDynamicMetaData(
                                    ABI_Type_e inputAbiType,
                                    uint8_t optInput,
                                    uint8_t *pAbiTypeData,
                                    uint8_t *pConvertedBuffer
                                  )
{
    if (
        (NULL == pAbiTypeData)              ||
        (NULL == pConvertedBuffer)
       )
    {
        return;
    }

    ABI_DecodeStaticValByType(
                                ABI_bytes_e,
                                4,
                                (uint8_t *)(pAbiTypeData + 28),
                                pConvertedBuffer
                             );
    *((uint32_t *)pConvertedBuffer) = U32_READ_BE_ARRAY(pConvertedBuffer);
    
    return;
}
