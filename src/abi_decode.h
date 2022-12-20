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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void ABI_VerifyArguments(
                            const void *pInputPayload,
                            const uint64_t payloadLength,
                            const ABI_Type_e *pABIDataTypes,
                            const uint8_t numArgs
                        );

#endif /* ABI_DECODE */