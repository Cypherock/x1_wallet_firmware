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
#include "utils.h"

static void ABI_DecodeIntX(
                            const void *pAbiTypeData,
                            void *pConvertedBuffer,
                            uint16_t numBits
                           )
{
    if (0 != (numBits % 8))
    {
        return;
    }

    memcpy(
            pConvertedBuffer,
            pAbiTypeData,
            (numBits / 8)
          );

    return;
}

void ABI_DecodeStaticValByType(
                                ABI_Type_e inputAbiType,
                                const void *pAbiTypeData,
                                void *pConvertedBuffer,
                                uint8_t additionalData
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
        case ABI_address_e:
        case ABI_uint256_e:
        {
            ABI_DecodeIntX(pAbiTypeData, pConvertedBuffer, 256);
            break;
        }
        case ABI_bytes_e:
        {
            uint16_t numBitsInAbiData = 0;
            
            if (32 >= additionalData)
            {
                numBitsInAbiData = (additionalData * 8);
                ABI_DecodeIntX(pAbiTypeData, pConvertedBuffer, numBitsInAbiData);
            }
            
            break;
        }
    }
}

void ABI_VerifyArguments(
                            const void *pInputPayload,
                            const uint64_t payloadLength,
                            const ABI_Type_e *pABIDataTypes,
                            const uint8_t numArgs
                        )
{
    if (
        (NULL == pInputPayload)         ||
        (NULL == pABIDataTypes)         ||
        (0 == numArgs)
       )
    {
        return;
    }

    uint8_t *pCurrentInputPayload = (uint8_t *)pInputPayload;
    uint32_t currentPayloadOffset = 0;
    uint8_t convertedBuffer[32];
    
    uint8_t currArgNum;
    
    for (currArgNum = 0; currArgNum < numArgs; currArgNum++)
    {
        memzero(&(convertedBuffer[0]), sizeof(convertedBuffer));

        if (currentPayloadOffset > payloadLength)
        {
            break;
        }
        

        /* Check if static data type, then call the Static API directly */
        if (ABI_bytes_dynamic_e > pABIDataTypes[currArgNum])
        {
            
            ABI_DecodeStaticValByType(
                                        pABIDataTypes[currArgNum],
                                        (const void *)pCurrentInputPayload,
                                        (void *)(&(convertedBuffer[0])),
                                        0
                                     );
        }

        /* Else decode the dynamic data type and then call the Static API */
        else
        {
            uint32_t dynamicBytesOffset = U32_READ_BE_ARRAY(pCurrentInputPayload + 28);
            uint32_t dynamicBytesOffsetData = 
                    U32_READ_BE_ARRAY((uint8_t *)(pInputPayload + dynamicBytesOffset + 28));

            /* Only bytes and uint256[] to be supported as of now */
            if (ABI_bytes_dynamic_e == pABIDataTypes[currArgNum])
            {
                uint32_t loopCounter = 1;
                
                for (loopCounter = 1; loopCounter <= (dynamicBytesOffsetData / 32); loopCounter++)
                {
                    memzero(&(convertedBuffer[0]), sizeof(convertedBuffer));
                    uint8_t *srcPtr = (uint8_t *)(pInputPayload + dynamicBytesOffset + 32 * (loopCounter));
                    ABI_DecodeStaticValByType(
                                                ABI_bytes_e,
                                                (const void *)(srcPtr),
                                                (void *)(&(convertedBuffer[0])),
                                                32
                                             );
                }

                uint8_t loopCounterRemaining = (dynamicBytesOffsetData % 32);
                
                if (0 != loopCounterRemaining)
                {
                    memzero(&(convertedBuffer[0]), sizeof(convertedBuffer));
                    uint8_t *srcPtr = (uint8_t *)(pInputPayload + dynamicBytesOffset + 32 * (loopCounter));
                    ABI_DecodeStaticValByType(
                                                ABI_bytes_e,
                                                (const void *)(srcPtr),
                                                (void *)(&(convertedBuffer[0])),
                                                loopCounterRemaining
                                             );
                }
            }
            else if (ABI_uint256_array_dynamic_e == pABIDataTypes[currArgNum])
            {
                uint32_t currentArrayIndex;
                
                for (currentArrayIndex = 1; currentArrayIndex <= dynamicBytesOffsetData; currentArrayIndex++)
                {
                    memzero(&(convertedBuffer[0]), sizeof(convertedBuffer));
                    uint8_t *srcPtr = (uint8_t *)(pInputPayload + dynamicBytesOffset + 32 * (currentArrayIndex));
                    ABI_DecodeStaticValByType(
                                                ABI_uint256_e,
                                                (const void *)(srcPtr),
                                                (void *)(&(convertedBuffer[0])),
                                                0
                                             );
                }
            }
        }

        currentPayloadOffset += 32;
        
        /* Increment pInputPayload by 32 bytes as each static type is stored in 32 bytes */
        pCurrentInputPayload = (uint8_t *)((uint32_t)pInputPayload + currentPayloadOffset);
    }
}
