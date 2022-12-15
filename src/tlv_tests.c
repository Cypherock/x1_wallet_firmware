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

/* Global functions
 ******************************************************************************/

#ifdef TLV_UNIT_TESTS

#define TLV_TEST_FLASH_BASE     (0x080FF800)
#define TLV_TEST_NUM_FLASH_PG   (1)
#define TLV_TEST_DUMMY_TAG      (0x00C0)

#define TLV_TEST_TAG_1          (0x0069)
#define TLV_TEST_LEN_1          (0x0004)

#define TLV_TEST_TAG_2          (0x00A5)
#define TLV_TEST_LEN_2          (0x000A)

#define TLV_TEST_TAG_3          (0x05A4)
#define TLV_TEST_LEN_3          (0x0004)

#define TLV_TEST_MAX_READ_BYTES (FLASH_PAGE_SIZE)

void TLV_Tests(void)
{
	erase_cmd(TLV_TEST_FLASH_BASE, FLASH_PAGE_SIZE);

    uint8_t pReadArray[TLV_TEST_MAX_READ_BYTES];
    TLV_MemBlkInfo_t memBlkInfoTlv;
    memBlkInfoTlv.pNVMPageBaseAddr = TLV_TEST_FLASH_BASE;
    memBlkInfoTlv.noOfPagesInTLV = TLV_TEST_NUM_FLASH_PG;

    uint32_t pFlashAddr;
    volatile uint8_t returnCode;
    returnCode = TLV_GetLatestFlashAddrByTag(memBlkInfoTlv, TLV_TEST_DUMMY_TAG, &pFlashAddr);
    ASSERT(TLV_RETURN_CODE_TAG_NOT_FOUND == returnCode);

    /* Enter data 1 */
    uint8_t pDataTlv1[8];
    TLV_Header_t headerDataTlv;
    headerDataTlv.TLV_Tag = TLV_TEST_TAG_1;
    headerDataTlv.TLV_Length = TLV_TEST_LEN_1;
    memcpy(&(pDataTlv1[0]), &headerDataTlv, sizeof(TLV_Header_t));
    pDataTlv1[4] = 0xAA;
    pDataTlv1[5] = 0xBB;
    pDataTlv1[6] = 0xCC;
    pDataTlv1[7] = 0xDD;
    returnCode = TLV_InsertData(memBlkInfoTlv, pDataTlv1);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    memzero(pReadArray, TLV_TEST_MAX_READ_BYTES);
    returnCode = TLV_ReadLatestData(memBlkInfoTlv, headerDataTlv.TLV_Tag, pReadArray);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    ASSERT(memcmp(&(pReadArray[0]), &(pDataTlv1[4]), headerDataTlv.TLV_Length) == 0);

    /* Enter data 2 */
    uint8_t pDataTlv2[14];
    headerDataTlv.TLV_Tag = TLV_TEST_TAG_2;
    headerDataTlv.TLV_Length = TLV_TEST_LEN_2;
    memcpy(&(pDataTlv2[0]), &headerDataTlv, sizeof(TLV_Header_t));
    pDataTlv2[4] = 0xAA;
    pDataTlv2[5] = 0xBB;
    pDataTlv2[6] = 0xCC;
    pDataTlv2[7] = 0xDD;
    pDataTlv2[8] = 0xAA;
    pDataTlv2[9] = 0xBB;
    pDataTlv2[10] = 0xCC;
    pDataTlv2[11] = 0xDD;
    pDataTlv2[12] = 0xDD;
    pDataTlv2[13] = 0xDD;
    returnCode = TLV_InsertData(memBlkInfoTlv, pDataTlv2);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    memzero(pReadArray, TLV_TEST_MAX_READ_BYTES);
    returnCode = TLV_ReadLatestData(memBlkInfoTlv, headerDataTlv.TLV_Tag, pReadArray);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    ASSERT(memcmp(&(pReadArray[0]), &(pDataTlv2[4]), headerDataTlv.TLV_Length) == 0);

    /* Update data 1 */
    headerDataTlv.TLV_Tag = TLV_TEST_TAG_1;
    headerDataTlv.TLV_Length = TLV_TEST_LEN_1;
    memcpy(&(pDataTlv1[0]), &headerDataTlv, sizeof(TLV_Header_t));
    pDataTlv1[4] = 0x11;
    pDataTlv1[5] = 0x22;
    pDataTlv1[6] = 0x33;
    pDataTlv1[7] = 0x44;
    returnCode = TLV_InsertData(memBlkInfoTlv, pDataTlv1);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    memzero(pReadArray, TLV_TEST_MAX_READ_BYTES);
    returnCode = TLV_ReadLatestData(memBlkInfoTlv, headerDataTlv.TLV_Tag, pReadArray);
    ASSERT(TLV_RETURN_CODE_PROCESS_COMPLETE == returnCode);
    ASSERT(memcmp(&(pReadArray[0]), &(pDataTlv1[4]), headerDataTlv.TLV_Length) == 0);
}

#endif /* TLV_UNIT_TESTS */