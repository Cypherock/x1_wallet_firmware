/**
 * @file    eth.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef ABI_DECODE
#define ABI_DECODE

#include <assert.h>
#include <stdio.h>

/* MACROS
 *****************************************************************************/
#define ABI_uint256_e_SIZE_IN_BYTES                 (32)

#define ABI_address_e_SIZE_IN_BYTES                 (20)
#define ABI_address_e_OFFSET_BE                     (ABI_uint256_e_SIZE_IN_BYTES - ABI_address_e_SIZE_IN_BYTES)

#define ABI_DYNAMIC_METADATA_SIZE_IN_BYTES          (4)
#define ABI_DYNAMIC_METADATA_OFFET_BE               (ABI_uint256_e_SIZE_IN_BYTES - ABI_DYNAMIC_METADATA_SIZE_IN_BYTES)

#define ABI_DECODE_BAD_ARGUMENT                     (0x11)
#define ABI_DECODE_PROCESS_COMPLETE                 (0xAA)
/* Enums
 *****************************************************************************/
typedef enum
{
    /* Static types */
    ABI_uintx_e,
    ABI_uint256_e,
    ABI_address_e,
    ABI_bytes_e,

    /* Dynamic types */
    ABI_bytes_dynamic_e,
    ABI_uint256_array_dynamic_e,
} ABI_Type_e;

/* Global function prototypes
 *****************************************************************************/
uint8_t ABI_DecodeDynamicValByType(
                                    ABI_Type_e inputAbiType,
                                    uint8_t *pAbiTypeData,
                                    uint8_t *pAbiTypeDataBase,
                                    uint32_t *pNumBytesReturned,
                                    uint8_t **dpAbiDataPtr
                                  );

void ABI_Stringify(
                    ABI_Type_e inputAbiType,
                    uint8_t *pAbiTypeData,
                    uint32_t additionalData
                  );
                  
#ifdef ABI_DECODE_UNIT_TESTS
void ABIDecode_Test_Cases(void);
#endif /* ABI_DECODE_UNIT_TESTS */

#endif /* ABI_DECODE */