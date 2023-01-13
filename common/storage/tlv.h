/**
 * @file    tlv.h
 * @author  Cypherock X1 Team
 * @brief   Header for coin specific data.
 *          Contains declarations for coin specific data functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef TLV
#define TLV
#include <stdint.h>
#include <flash_if.h>
#include <flash_api.h>
#include <coin_utils.h>

/* Local MACROS
 ******************************************************************************/
#define ALIGN_UP_8_BYTE_ALIGNED(x)  (((x) + 0x7) & ~(0x7))
#define ALIGN_UP(x)                 (ALIGN_UP_8_BYTE_ALIGNED(x))

#define TLV_TAG_SIZE_IN_BYTES       (2)
#define TLV_TAG_VIRGIN_INVALID      (0xFFFF)

#define TLV_LEN_SIZE_IN_BYTES       (2)
#define TLV_LEN_MAXVAL              (0xFFFF)
#define TLV_LEN_MAXSIZE             (0xFFFF + TLV_TAG_SIZE_IN_BYTES + TLV_LEN_SIZE_IN_BYTES)

#define TLV_SIZE_OF_HDR_IN_BYTES    (TLV_TAG_SIZE_IN_BYTES + TLV_LEN_SIZE_IN_BYTES)

#define TLV_RETURN_CODE_INVALID_ARGUMENT    (0x11)
#define TLV_RETURN_CODE_MEMORY_FULL         (0x22)
#define TLV_RETURN_CODE_TAG_NOT_FOUND       (0x33)
#define TLV_RETURN_CODE_TAG_FOUND           (0x44)
#define TLV_RETURN_CODE_PROCESS_COMPLETE    (0xAA)

/* Local typedefs
 ******************************************************************************/
typedef struct
{
    uint16_t TLV_Tag;
    uint16_t TLV_Length;
} TLV_Header_t;

typedef struct
{
    uint32_t pNVMPageBaseAddr;
    uint16_t noOfPagesInTLV;
} TLV_MemBlkInfo_t;

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
uint8_t TLV_InsertData(
                        TLV_MemBlkInfo_t memBlkInfoTlv,
                        const void *pInsertDataTlvFmtBuffer
                      );

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
uint8_t TLV_ReadLatestData(
                            TLV_MemBlkInfo_t memBlkInfoTlv,
                            const uint16_t readDataTag,
                            uint8_t *pReadDataByteBuffer
                          );

#ifdef TLV_UNIT_TESTS
/**
 * @brief: Runs unit tests which check TLV functionality. This function
 * issues a reset if any test case fails caught via assertions().
 */
void TLV_Tests(void);
#endif /* TLV_UNIT_TESTS */

#endif /* TLV */