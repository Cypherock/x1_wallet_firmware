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
void ABI_DecodeDynamicValByType(
                                ABI_Type_e inputAbiType,
                                uint8_t optInput,
                                uint8_t *pAbiTypeData,
                                uint8_t *pConvertedBuffer
                               );

void ABI_DecodeStaticValByType(
                                ABI_Type_e inputAbiType,
                                uint8_t optInput,
                                uint8_t *pAbiTypeData,
                                uint8_t *pConvertedBuffer
                              );

#ifdef ABI_DECODE_UNIT_TESTS
void ABIDecode_Test_Cases(void);
#endif /* ABI_DECODE_UNIT_TESTS */

#endif /* ABI_DECODE */