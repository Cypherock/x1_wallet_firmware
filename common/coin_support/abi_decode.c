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
#include "eth.h"

/* Static function prototypes
 *****************************************************************************/
static void ABI_GetDynamicMetaData(
                                    ABI_Type_e inputAbiType,
                                    uint8_t *pAbiTypeData,
                                    uint8_t *pConvertedBuffer
                                  );

/* Global functions
 *****************************************************************************/
uint8_t ABI_DecodeDynamicValByType(
                                    ABI_Type_e inputAbiType,
                                    uint8_t *pAbiTypeData,
                                    uint8_t *pAbiTypeDataBase,
                                    uint32_t *pNumBytesReturned,
                                    uint8_t **dpAbiDataPtr
                                  )
{
    uint8_t returnCode = ABI_DECODE_BAD_ARGUMENT;
    
    if (
        (NULL == pAbiTypeData)              ||
        (NULL == pAbiTypeDataBase)          ||
        (NULL == pNumBytesReturned)
       )
    {
        return returnCode;
    }

    switch (inputAbiType)
    {
        case ABI_bytes_dynamic_e:
        {
            uint32_t offsetDynamicData;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    pAbiTypeData,
                                    ((uint8_t *)&(offsetDynamicData))
                                  );
            
            /* Goto offset of the dynamic input */
            pAbiTypeData = pAbiTypeDataBase + offsetDynamicData;

            /**
             * pAbiTypeData points now to the 32-byte data denoting num of bytes
             * in the dynamic input
             */
            uint32_t numBytesInData;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    pAbiTypeData,
                                    ((uint8_t *)&(numBytesInData))
                                  );
            pAbiTypeData += 32;

            /* Finally inform the caller about number of bytes returned */
            *pNumBytesReturned = numBytesInData;
            *dpAbiDataPtr = pAbiTypeData;

            returnCode = ABI_DECODE_PROCESS_COMPLETE;
            break;              
        }
        case ABI_uint256_array_dynamic_e:
        {
            uint32_t offsetDynamicData;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    pAbiTypeData,
                                    ((uint8_t *)&(offsetDynamicData))
                                  );
            
            /* Goto offset of the dynamic input */
            pAbiTypeData = pAbiTypeDataBase + offsetDynamicData;

            /**
             * pAbiTypeData now points to the 32-byte data denoting num of elements
             * in the uint256[] array
             */
            uint32_t numElementsInDataArr;
            ABI_GetDynamicMetaData(
                                    ABI_bytes_e,
                                    pAbiTypeData,
                                    ((uint8_t *)&(numElementsInDataArr))
                                  );
            pAbiTypeData += 32;

            /* Finally inform the caller about number of bytes returned */
            *pNumBytesReturned = numElementsInDataArr;
            *dpAbiDataPtr = pAbiTypeData;

            returnCode = ABI_DECODE_PROCESS_COMPLETE;
            break;
        }
        default:
        {
            break;
        }
    }

    return returnCode;
}

/* Static functions
 *****************************************************************************/
static void ABI_GetDynamicMetaData(
                                    ABI_Type_e inputAbiType,
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

    memcpy(
            pConvertedBuffer,
            pAbiTypeData + ABI_DYNAMIC_METADATA_OFFET_BE,
            ABI_DYNAMIC_METADATA_SIZE_IN_BYTES
          );
    
    *((uint32_t *)pConvertedBuffer) = U32_READ_BE_ARRAY(pConvertedBuffer);
    
    return;
}

void ABI_Stringify(
                    ABI_Type_e inputAbiType,
                    uint8_t *pAbiTypeData,
                    uint32_t additionalData
                  )
{
    ui_display_node *result;

    switch (inputAbiType)
    {
        case ABI_uint256_e:
        {
            uint8_t staticBufferInUTF8[200];
            memzero(staticBufferInUTF8, sizeof(staticBufferInUTF8));

            byte_array_to_hex_string(
                                        pAbiTypeData,
                                        32,
                                        &(staticBufferInUTF8[0]),
                                        65
                                    );

            convert_byte_array_to_decimal_string(
                                                    64,
                                                    0,
                                                    &(staticBufferInUTF8[0]),
                                                    &(staticBufferInUTF8[100]),
                                                    100
                                                );
            
            result = eth_create_display_node(
                                                "Datatype:uint256\0",
                                                25,
                                                &(staticBufferInUTF8[100]),
                                                100
                                            );
            break;
        }
        case ABI_address_e:
        {
            uint8_t staticBufferInUTF8[41];
            memzero(staticBufferInUTF8, sizeof(staticBufferInUTF8));
            
            byte_array_to_hex_string(
                                        pAbiTypeData + ABI_address_e_OFFSET_BE,
                                        20,
                                        &(staticBufferInUTF8[0]),
                                        41
                                    );
            
            result = eth_create_display_node(
                                                "Datatype:address\0",
                                                25,
                                                &(staticBufferInUTF8[0]),
                                                41
                                            );
            break;
        }
        case ABI_bytes_e:
        {
            uint32_t numBytesBufferReq = (2 * additionalData) + 1;
            
            uint8_t *staticBufferInUTF8 = (uint8_t *)malloc(numBytesBufferReq);
            memzero(staticBufferInUTF8, numBytesBufferReq);

            byte_array_to_hex_string(
                                        pAbiTypeData,
                                        additionalData,
                                        staticBufferInUTF8,
                                        numBytesBufferReq
                                    );
            
            result = eth_create_display_node(
                                                "Datatype:bytes\0",
                                                25,
                                                staticBufferInUTF8,
                                                numBytesBufferReq
                                            );
            free(staticBufferInUTF8);
        }

        default:
        {
            break;
        }
    }

    /* TODO: Add result to global linked list */
}