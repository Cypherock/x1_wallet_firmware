/**
 * @file    tlv.c
 * @author  Cypherock X1 Team
 * @brief   Coin specific data.
 *          Contains functions for managing coin specific data.
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
#include "tlv.h"

/* Static functions prototypes
 ******************************************************************************/
static uint8_t TLV_GetLatestFlashAddrByTag(
                                            TLV_MemBlkInfo_t memBlkInfoTlv,
                                            const uint16_t searchDataTag,
                                            uint32_t *pSearchTagTlvFlashOffset
                                          );

/* Global functions
 ******************************************************************************/
uint8_t TLV_InsertData(
                        TLV_MemBlkInfo_t memBlkInfoTlv,
                        const void *pInsertDataTlvFmtBuffer
                      )
{
    uint8_t returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;

    do
    {
        if (NULL == pInsertDataTlvFmtBuffer)
        {
            returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
            break;
        }

        TLV_Header_t *headerTlvBuffer = (TLV_Header_t *)pInsertDataTlvFmtBuffer;

        /* Validate insertDataTag. It must not be TLV_TAG_VIRGIN_INVALID */
        uint16_t insertDataTlvTag = headerTlvBuffer->TLV_Tag;

        if (TLV_TAG_VIRGIN_INVALID == insertDataTlvTag)
        {
            returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
            break; 
        }

        /**
         * Read insertDataTlvFmtSize and add sizeof TLV header, as 
         * TLV_SIZE_OF_HDR_IN_BYTES will also be written in the flash
         */
        uint32_t insertDataTlvFmtSize = (uint32_t)(headerTlvBuffer->TLV_Length) + TLV_SIZE_OF_HDR_IN_BYTES;
        /* Make insertDataTlvFmtSize 8-byte aligned always */
        insertDataTlvFmtSize = ALIGN_UP(insertDataTlvFmtSize);

        /**
         * If the flash sector is virgin or erased, flash memory reads all 1's
         * Therefore, we can find free space by searching for searchDataTag 
         * TLV_TAG_VIRGIN_INVALID
         */
        uint32_t freeInsertFlashAddr;
        returnCode = TLV_GetLatestFlashAddrByTag(
                                                    memBlkInfoTlv, 
                                                    TLV_TAG_VIRGIN_INVALID,
                                                    &freeInsertFlashAddr
                                                );

        /**
         * If searchDataTag TLV_TAG_VIRGIN_INVALID was not found, it means that 
         * there is no space in mem block 
         */
        if (TLV_RETURN_CODE_TAG_FOUND != returnCode)
        {
            returnCode = TLV_RETURN_CODE_MEMORY_FULL;
            break;
        }

        /**
         * Handle if no space is free in the flash bank for insertDataTlvFmtSize
         * starting from freeInsertFlashAddr.
         * Valid flash memory range is [memBlkInfoTlv.pNVMPageBaseAddr, 
         * memBlkInfoTlv.pNVMPageBaseAddr+memBlkInfoTlv.noOfPagesInTLV * FLASH_PAGE_SIZE]
         */
        if (
            (freeInsertFlashAddr + insertDataTlvFmtSize - 1) <=
            (memBlkInfoTlv.pNVMPageBaseAddr + (memBlkInfoTlv.noOfPagesInTLV * FLASH_PAGE_SIZE))
           )
        {
            write_cmd(
                        freeInsertFlashAddr, 
                        (const uint32_t *)(pInsertDataTlvFmtBuffer),
                        insertDataTlvFmtSize
                     );

            returnCode = TLV_RETURN_CODE_PROCESS_COMPLETE;
            break;
        }
        else
        {
            returnCode = TLV_RETURN_CODE_MEMORY_FULL;
            break;
        }
    } while (0);

    return returnCode;
}

uint8_t TLV_ReadLatestData(
                            TLV_MemBlkInfo_t memBlkInfoTlv,
                            const uint16_t readDataTag,
                            uint8_t *pReadDataByteBuffer
                          )
{
    uint8_t returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
    
    do
    {
        /**
         * Validate arguments before proceeding further
         * readDataTag must not be TLV_TAG_VIRGIN_INVALID and pReadDataByteBuffer 
         * should not be NULL
         */
        if (
            (TLV_TAG_VIRGIN_INVALID == readDataTag)     ||
            (NULL == pReadDataByteBuffer)
           )
        {
            returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
            break;
        }

        /* Find the latest offset in the memory block with the readDataTag */
        uint32_t readDataTlvFmtFlashAddr;
        returnCode = TLV_GetLatestFlashAddrByTag(
                                                    memBlkInfoTlv, 
                                                    readDataTag,
                                                    &readDataTlvFmtFlashAddr
                                                );

        /**
         * If searchDataTag readDataTag was not found, it means that specific 
         * data type was not present 
         */
        if (TLV_RETURN_CODE_TAG_FOUND != returnCode)
        {
            returnCode = TLV_RETURN_CODE_TAG_NOT_FOUND;
            break;
        }

        /* Read TLV tag and value fields */
        TLV_Header_t headerTlvFlashValue;

        read_cmd(
                    readDataTlvFmtFlashAddr, 
                    (uint32_t *)(&headerTlvFlashValue),
                    sizeof(headerTlvFlashValue)
                );

        /* Re-check TLV header data (TL) and validate against readDataTag */
        if (readDataTag == headerTlvFlashValue.TLV_Tag)
        {
            /* Fill buffer with the data value in flash */
            read_cmd(
                        (readDataTlvFmtFlashAddr + sizeof(headerTlvFlashValue)), 
                        (uint32_t *)pReadDataByteBuffer,
                        headerTlvFlashValue.TLV_Length
                    );

            returnCode = TLV_RETURN_CODE_PROCESS_COMPLETE;
            break;
        }
        else
        {
            returnCode = TLV_RETURN_CODE_TAG_NOT_FOUND;
            break;
        }
    } while (0);

    return returnCode;
}

/* Static functions
 ******************************************************************************/
static uint8_t TLV_GetLatestFlashAddrByTag(
                                            TLV_MemBlkInfo_t memBlkInfoTlv,
                                            const uint16_t searchDataTag,
                                            uint32_t *pSearchTagTlvFlashOffset
                                          )
{
    uint8_t returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
    
    do
    {
        if (NULL == pSearchTagTlvFlashOffset)
        {
            returnCode = TLV_RETURN_CODE_INVALID_ARGUMENT;
            break;
        }
        
        /* Initialize last address with invalid value */
        uint32_t dataTlvFlashAddrLast = 0;
        uint32_t currFlashOffset = 0;

        returnCode = TLV_RETURN_CODE_TAG_NOT_FOUND;

        /**
         * Parse through the memory block to find the last TLV data with searchDataTag
         * Breaks if currFlashOffset exceeds the memory block size or an 
         * TLV_TAG_VIRGIN_INVALID tag match which signifies end of stored data.
         */
        while(currFlashOffset < (memBlkInfoTlv.noOfPagesInTLV * FLASH_PAGE_SIZE))
        {
            TLV_Header_t currHeaderTlvFlashValue;
            uint32_t currDataTlvFlashAddr = memBlkInfoTlv.pNVMPageBaseAddr + currFlashOffset;

            /* Read TLV tag and value fields */
            read_cmd(
                        currDataTlvFlashAddr, 
                        (uint32_t *)(&currHeaderTlvFlashValue),
                        sizeof(currHeaderTlvFlashValue)
                    );

            currHeaderTlvFlashValue.TLV_Length = currHeaderTlvFlashValue.TLV_Length;

            /* Check TLV header data (T, L) and validate against searchDataTag */
            if (searchDataTag == currHeaderTlvFlashValue.TLV_Tag)
            {
                /* If match is found, store the current flash address */
                dataTlvFlashAddrLast = currDataTlvFlashAddr;
                returnCode = TLV_RETURN_CODE_TAG_FOUND;
            }
            else
            {
                /* Do nothing */
            }

            /* Break if reached end of storage */
            if (TLV_TAG_VIRGIN_INVALID == currHeaderTlvFlashValue.TLV_Tag)
            {
                break;
            }

            /**
             * Increment offset by 
             * currHeaderTlvFlashValue.TLV_Length + sizeof(currHeaderTlvFlashValue) bytes
             */
            currFlashOffset =   currFlashOffset + 
                                currHeaderTlvFlashValue.TLV_Length + 
                                sizeof(currHeaderTlvFlashValue);
            currFlashOffset = ALIGN_UP(currFlashOffset);
        }

        /* Update the flash offset for the caller */
        *pSearchTagTlvFlashOffset = dataTlvFlashAddrLast;
    } while (0);

    return returnCode;
}
