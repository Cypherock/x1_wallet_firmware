/**
 * @file    onboarding.c
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
#include "onboarding_storage.h"

/* Static functions prototypes
 ******************************************************************************/


/* Global functions
 ******************************************************************************/
uint8_t OnboardingStorage_SetStage(Onboarding_StageData newStageData)
{
    TLV_MemBlkInfo_t TLV_OnboardingMemBlk;
    TLV_Header_t TLV_OnboardingHdr;
    uint8_t statusCode = ONBOARDING_RETURN_CODE_WRITE_STAGE_INCOMPLETE;
    
    TLV_OnboardingMemBlk.pNVMPageBaseAddr = ONBOARDING_TLV_PAGE_BASE_ADDR;
    TLV_OnboardingMemBlk.noOfPagesInTLV = ONBOARDING_TLV_NUM_PAGES;    
    
    /* Allocate stack for data in onboarding data structure (including TLV header) */
    uint8_t dataOnboardingTlv[sizeof(TLV_OnboardingHdr) + ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES];
    
    /**
     * First sizeof(TLV_OnboardingHdr) bytes are reserved for the TLV header, actual data is 
     * to be stored after the offset
     */
    
    /**
     * Fill data struction with all 0's, if the device is virgin, then TLV_ReadLatestData()
     * will not modify the buffer
     */
    memset(
            (void *)(&(dataOnboardingTlv[sizeof(TLV_OnboardingHdr)])),
            ONBOARDING_NOT_STARTED,
            ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES
          );

    /* Get the latest value from the memory block */
    (void)TLV_ReadLatestData(
                                TLV_OnboardingMemBlk,
                                ONBOARDING_TLV_TAG,
                                &(dataOnboardingTlv[sizeof(TLV_OnboardingHdr)])
                            );
    
    /**
     * Prepare mask to be updated in the onboarding data structure
     * Even stages are stored in bottom 4-bits of the byte
     * Odd stages are stored in upper 4-bits of the byte
     */
    uint8_t stageEncodingMask;
    uint8_t currStageEncoding = 
            dataOnboardingTlv[sizeof(TLV_OnboardingHdr) + (newStageData.StageNum / 2)];
    switch ((newStageData.StageNum) % 2)
    {
        case 0:
        {
            stageEncodingMask = (newStageData.StageEncoding & STAGE_VALID_MASK);
            currStageEncoding = ((currStageEncoding & STAGE_H_MASK) | (stageEncodingMask));
            break;
        }
        case 1:
        {
            stageEncodingMask = ((newStageData.StageEncoding & STAGE_VALID_MASK) << 4);
            currStageEncoding = ((currStageEncoding & STAGE_L_MASK) | (stageEncodingMask));
            break;
        }
        default:
        {
            break;
        }
    }

    /**
     * Basically assume onboarding data structure as an array dataOnboardingTlv
     * First sizeof(TLV_OnboardingHdr) elements hold the header of the TLV structure so
     * add it as the offset
     * Update the stage encoding based on inputs
     */
    dataOnboardingTlv[sizeof(TLV_OnboardingHdr) + (newStageData.StageNum / 2)]
        = currStageEncoding; 

    /* Pre-pend TLV header before the updated data */
    TLV_OnboardingHdr.TLV_Tag = ONBOARDING_TLV_TAG;
    TLV_OnboardingHdr.TLV_Length = ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES;
    
    memcpy(
            (void *)(&(dataOnboardingTlv[0])),
            (const void *)(&(TLV_OnboardingHdr)),
            sizeof(TLV_OnboardingHdr)
          );

    /* Finally insert the data in TLV Format */
    statusCode = TLV_InsertData(
                                    TLV_OnboardingMemBlk,
                                    (const void *)(&(dataOnboardingTlv[0]))
                               );
    
    if (TLV_RETURN_CODE_PROCESS_COMPLETE == statusCode)
    {
        statusCode = ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE;
    }

    return statusCode;
}

Onboarding_StageData OnboardingStorage_GetLastStageData(void)
{
    uint8_t dataOnboarding[ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES];
    TLV_MemBlkInfo_t TLV_OnboardingMemBlk;

    TLV_OnboardingMemBlk.pNVMPageBaseAddr = ONBOARDING_TLV_PAGE_BASE_ADDR;
    TLV_OnboardingMemBlk.noOfPagesInTLV = ONBOARDING_TLV_NUM_PAGES;

    /* Pre-fill return variable with invalid stage entry */
    Onboarding_StageData lastStageData;
    lastStageData.StageNum = STAGE_NO_ENTRY;
    lastStageData.StageEncoding = STAGE_n_NOT_STARTED;

    /**
     * Fill data struction with all 0's, if the device is virgin, then TLV_ReadLatestData()
     * will not modify the buffer
     */
    memset(
            (void *)(&(dataOnboarding[0])),
            ONBOARDING_NOT_STARTED,
            ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES
          );

    (void)TLV_ReadLatestData(
                                TLV_OnboardingMemBlk,
                                ONBOARDING_TLV_TAG,
                                &(dataOnboarding[0])
                            );

    uint8_t currStageNum = STAGE_0;
    uint8_t currStageEncoding = STAGE_n_NOT_STARTED;
    
    while(STAGE_MAX_VAL >= currStageNum)
    {
        switch ((currStageNum) % 2)
        {
            case 0:
            {
                currStageEncoding = (dataOnboarding[currStageNum / 2] & 0x0F);
                break;
            }
            case 1:
            {
                currStageEncoding = ((dataOnboarding[currStageNum / 2] & 0xF0) >> 4);
                break;
            }
            default:
            {
                break;
            }
        }

        /* Check if this state is started or NOT. If it is not started, then break */
        if (STAGE_n_NOT_STARTED == currStageEncoding)
        {
            break;
        }
        
        /* Save stage data */
        lastStageData.StageNum = currStageNum;
        lastStageData.StageEncoding = currStageEncoding;
        
        /* Increment current stage number and check for next state */
        currStageNum = currStageNum + 1;
    }

    return lastStageData;
}

#ifdef ONBOARDING_STORAGE_UNIT_TESTS
void OnboardingStorage_Tests(void)
{
	erase_cmd(ONBOARDING_TLV_PAGE_BASE_ADDR, FLASH_PAGE_SIZE);

    TLV_MemBlkInfo_t memBlkInfoTlv;
    memBlkInfoTlv.pNVMPageBaseAddr = ONBOARDING_TLV_PAGE_BASE_ADDR;
    memBlkInfoTlv.noOfPagesInTLV = ONBOARDING_TLV_NUM_PAGES;

    /* Virgin device will detect STAGE_NO_ENTRY */
    Onboarding_StageData currentStage;
    uint8_t returnCode;
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_NO_ENTRY == currentStage.StageNum);

    currentStage.StageNum = STAGE_0;
    currentStage.StageEncoding = STAGE_n_COMPLETE;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_0 == currentStage.StageNum);
    ASSERT(STAGE_n_COMPLETE == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_1;
    currentStage.StageEncoding = STAGE_n_COMPLETE;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_1 == currentStage.StageNum);
    ASSERT(STAGE_n_COMPLETE == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_2;
    currentStage.StageEncoding = STAGE_2_STEP_1;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_2 == currentStage.StageNum);
    ASSERT(STAGE_2_STEP_1 == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_2;
    currentStage.StageEncoding = STAGE_2_STEP_2;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_2 == currentStage.StageNum);
    ASSERT(STAGE_2_STEP_2 == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_2;
    currentStage.StageEncoding = STAGE_2_STEP_3;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_2 == currentStage.StageNum);
    ASSERT(STAGE_2_STEP_3 == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_2;
    currentStage.StageEncoding = STAGE_n_COMPLETE;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_2 == currentStage.StageNum);
    ASSERT(STAGE_n_COMPLETE == currentStage.StageEncoding);

    currentStage.StageNum = STAGE_3;
    currentStage.StageEncoding = STAGE_n_COMPLETE;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_3 == currentStage.StageNum);
    ASSERT(STAGE_n_COMPLETE == currentStage.StageEncoding);

    /* Skip stage 4 to check if read logic breaks */
    currentStage.StageNum = STAGE_5;
    currentStage.StageEncoding = STAGE_n_COMPLETE;
    returnCode = OnboardingStorage_SetStage(currentStage);
    ASSERT(ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE == returnCode);
    memzero(&currentStage, sizeof(currentStage));
    currentStage = OnboardingStorage_GetLastStageData();
    ASSERT(STAGE_5 != currentStage.StageNum);
}
#endif /* ONBOARDING_STORAGE_TESTS */