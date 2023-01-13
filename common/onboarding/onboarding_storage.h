/**
 * @file    onboarding_storage.h
 * @author  Cypherock X1 Team
 * @brief   Header for coin specific data.
 *          Contains declarations for coin specific data functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef ONBOARDING_STORAGE
#define ONBOARDING_STORAGE
#include <stdint.h>
#include "tlv.h"

#define ONBOARDING_STORAGE_UNIT_TESTS

/* Local MACROS
 ******************************************************************************/
#define ONBOARDING_TLV_TAG              (0x0001)
#define ONBOARDING_TLV_PAGE_BASE_ADDR   (0x080FF800)
#define ONBOARDING_TLV_NUM_PAGES        (0x1)
#define ONBOARDING_NOT_STARTED          (0x00000000)

#define ONBOARDING_DATA_STRUCT_SIZE_IN_BYTES    (0x4)

#define STAGE_0                 (0x0)
#define STAGE_1                 (0x1)
#define STAGE_2                 (0x2)
#define STAGE_3                 (0x3)
#define STAGE_4                 (0x4)
#define STAGE_5                 (0x5)
#define STAGE_6                 (0x6)
#define STAGE_MAX_VAL           (STAGE_6)
#define STAGE_NO_ENTRY          (0xFF)

#define ONBOARDING_RETURN_CODE_WRITE_STAGE_COMPLETE     (0xAA)
#define ONBOARDING_RETURN_CODE_WRITE_STAGE_INCOMPLETE   (0x5A)

#define STAGE_VALID_MASK        (0x0F)
#define STAGE_L_MASK            (0x0F)
#define STAGE_H_MASK            (0xF0)

#define STAGE_2_STEP_1          (0x1)
#define STAGE_2_STEP_2          (0x2)
#define STAGE_2_STEP_3          (0x3)
#define STAGE_n_COMPLETE        (0xA)
#define STAGE_n_NOT_STARTED     (0x0)

/* Local typedefs
 ******************************************************************************/
typedef struct
{
    uint8_t StageNum;
    uint8_t StageEncoding;
} Onboarding_StageData;

/* Global function prototypes
 ******************************************************************************/
/**
 * @brief: Writes data in the flash memory which is managed using TLV format
 * 
 * @details: The input data is written at the end of the flash memory block. 
 * It is to be noted that this function expects the input buffer 
 * (const void *pInsertDataTlvFmtBuffer) is already pre-pended with the TLV 
 * header and TLV length values
 * 
 * @param: TLV_MemBlkInfo_t memBlkInfoTlv: This data structure defines the 
 * boundary of the flash memory block managed by the TLV
 * @param: const void *pInsertDataTlvFmtBuffer: Pointer to the buffer which 
 * holds the data in TLV format
 * 
 * @returns:
 * @retval: TLV_RETURN_CODE_INVALID_ARGUMENT: In case of invalid arguments 
 * (eg: NULL pointer) or TLV_TAG_VIRGIN_INVALID tag
 * @retval: TLV_RETURN_CODE_MEMORY_FULL: In case the flash memory block is 
 * full and will not be able to write the input data
 * @retval: TLV_RETURN_CODE_PROCESS_COMPLETE: In case the write operation was complete
 *
 * @see
 */
uint8_t OnboardingStorage_SetStage(Onboarding_StageData newStageData);

/**
 * @brief: Reads back the last instance of data bytes in the flash memory which is 
 * managed using TLV format corresponding to a tag 
 * 
 * @details: Last instance of the data in TLV format is returned. User should allocate
 * appropriate memory buffer to store the data bytes 
 * 
 * @param: TLV_MemBlkInfo_t memBlkInfoTlv: This data structure defines the 
 * boundary of the flash memory block managed by the TLV
 * @param: const uint16_t readDataTag: This is the tag for which the search will take place.
 * @param: uint8_t *pReadDataByteBuffer: Pointer to the buffer which will be filled by this
 * function if the tag is found
 * 
 * @returns:
 * @retval: TLV_RETURN_CODE_INVALID_ARGUMENT: In case of invalid arguments 
 * (eg: NULL pointer) or TLV_TAG_VIRGIN_INVALID tag
 * @retval: TLV_RETURN_CODE_TAG_NOT_FOUND: In case the tag was NOT found in flash memory block.
 * @retval: TLV_RETURN_CODE_PROCESS_COMPLETE: In case the read operation completed successfully.
 *
 * @see
 */
Onboarding_StageData OnboardingStorage_GetLastStageData(void);

#ifdef ONBOARDING_STORAGE_UNIT_TESTS
/**
 * @brief: Runs unit tests which check TLV functionality. This function
 * issues a reset if any test case fails caught via assertions().
 */
void OnboardingStorage_Tests(void);
#endif /* ONBOARDING_STORAGE_UNIT_TESTS */

#endif /* ONBOARDING_STORAGE */