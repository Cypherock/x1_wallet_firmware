/**
 * @file    abi.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef ABI_DECODE
#define ABI_DECODE

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "coin_utils.h"
#include "sha3.h"

/* MACROS
 *****************************************************************************/
#define ABI_ELEMENT_SZ_IN_BYTES (32)
#define Abi_uint256_e_SZ_IN_BYTES (32)

#define Abi_address_e_SZ_IN_BYTES (20)
#define Abi_address_e_OFFSET_BE                                                \
  (ABI_ELEMENT_SZ_IN_BYTES - Abi_address_e_SZ_IN_BYTES)

#define Abi_bool_e_SZ_IN_BYTES (1)
#define Abi_bool_e_OFFSET_BE (ABI_ELEMENT_SZ_IN_BYTES - Abi_bool_e_SZ_IN_BYTES)

/**
 * Dynamic metadata contains information such as offsets, number of bytes of
 * data, etc. Our assumption is that the value if exceeds by 4 bytes, device
 * will not be able to decode it in anyway as it will become out of bounds for
 * the 32-bit MCU. Therefore keeping ABI_DYN_METADATA_SZ_IN_BYTES as 4.
 */
#define ABI_DYN_METADATA_SZ_IN_BYTES (4)
#define ABI_DYN_METADATA_OFFET_BE                                              \
  (ABI_ELEMENT_SZ_IN_BYTES - ABI_DYN_METADATA_SZ_IN_BYTES)

#define ABI_BAD_ARGUMENT (0x11)
#define ABI_PROCESS_INCOMPLETE (0x22)
#define ABI_PROCESS_COMPLETE (0xAA)

#define ABI_PADDING_ONES (0xFF)

/* Enums
 *****************************************************************************/
/**
 * https://docs.soliditylang.org/en/latest/abi-spec.html#formal-specification-of-the-encoding
 * https://docs.soliditylang.org/en/v0.8.17/abi-spec.html#types
 */
typedef enum {
  /**
   * Currently only 3 types of static ABI types are supported:
   * Abi_uint256_e
   * Abi_address_e
   * Abi_bytes_e
   * Abi_int256_e,
   * Abi_bool_e,
   * NOTE: New static ABI types must be inserted
   * after Abi_bytes_e but before Abi_bytes_dynamic_e
   */
  Abi_uint256_e = 1,
  Abi_address_e,
  Abi_bytes_e,
  Abi_int256_e,
  Abi_bool_e,

  /**
   * Currently only 2 types of dynamic ABI types are supported:
   * Abi_bytes_dynamic_e
   * Abi_uint256_array_dynamic_e
   * NOTE: New dynamic ABI types must be appended
   * after Abi_uint256_array_dynamic_e
   */
  Abi_bytes_dynamic_e,
  Abi_uint256_array_dynamic_e,
} Abi_Type_e;

/* Global function prototypes
 *****************************************************************************/
/**
 * @brief This function decodes a dynamic Abi metadata, checks if the metadata
 * is valid and informs caller about the metadata and pointer to the actual
 * data.
 *
 * @param inputAbiType Type of Abi data to be handled by the function
 * @param pAbiTypeData Pointer to Abi data
 * @param pAbiTypeDataBase Pointer to base address of Abi data
 * @param sizeOfAbiChunk Total size in bytes of the Abi data starting from
 * 						  pAbiTypeDataBase
 * @param pNumBytesReturned Pointer to a uint32_t variable. This function
 * modifies the value for the caller. The caller must only consider this value
 * valid if the function return valid return code.
 * ->if (inputAbiType == Abi_bytes_dynamic_e): This depicts number of bytes in
 * the dynamic bytes of Abi data
 * ->else if (inputAbiType == Abi_uint256_array_dynamic_e): This depicts the
 * number of uint256 elements in uint256 array.
 * @param dpAbiDataPtr Pointer to the base address of the static Abi data. This
 * function modifies the value for the caller. The caller must
 * only consider this value valid if the function returns ABI_PROCESS_COMPLETE
 * return code.
 * ->if (inputAbiType == Abi_bytes_dynamic_e): This depicts base address of the
 * Abi bytes of size pNumBytesReturned
 * ->else if (inputAbiType == Abi_uint256_array_dynamic_e): This depicts the
 * base address of the uint256[] elements in uint256 array.
 * @return uint8_t Depicts the status of operation for this function
 * ABI_BAD_ARGUMENT: If any argument is invalid
 * ABI_PROCESS_INCOMPLETE: If any issue is found during Abi decoding. It could
 * be due to out of bound access.
 * ABI_PROCESS_COMPLETE: If no issue is found during the Abi decoding
 */
uint8_t Abi_DynamicHelp(Abi_Type_e inputAbiType,
                        uint8_t *pAbiTypeData,
                        const uint8_t *pAbiTypeDataBase,
                        const uint32_t sizeOfAbiChunk,
                        uint32_t *pNumBytesReturned,
                        uint8_t **dpAbiDataPtr);

/**
 * @brief This function converts ABI value based on Abi_Type_e to UTF8 format
 * to be displayed on the LED display for user verification. This function
 * creates and returns a UI node of type ui_display_node.
 * Important thing to note here is that it only supports static Abi type data.
 * If the Abi data is dynamic, it must be broken down into static data and then
 * stringified.
 *
 * @param inputAbiType Type of Abi data to be stringified
 * @param pAbiTypeData Pointer to Abi data to be stringified
 * @param additionalData This argument is optional:
 * ->if (inputAbiType == Abi_bytes_e): This depicts number of bytes in the Abi
 * data
 * ->else it is not used
 * @return ui_display_node: This function returns pointer to the UI node which
 * can be displayed by the caller.
 */
ui_display_node *ABI_Stringify(Abi_Type_e inputAbiType,
                               uint8_t *pAbiTypeData,
                               uint32_t additionalData);

/**
 * @brief This function encodes data to Abi_Type_e inputAbiType. This function
 * only supports static Abi_Type_e as limited functionality is required
 * currently.
 *
 * @param inputAbiType Type of Abi data to be converted
 * @param additionalInput This argument is optional:
 * ->if (inputAbiType == Abi_uint256_e or Abi_int256_e or Abi_bytes_e): This
 * argument informs about the number of bytes of relevant data is present.
 * @param pAbiTypeData Pointer to data to be converted in inputAbiType
 * @param pBuffer Pointer to the buffer where this function writes the data in
 * inputAbiType
 * @return uint8_t Depicts the status of operation for this function
 * ABI_BAD_ARGUMENT: If any argument is invalid
 * ABI_PROCESS_COMPLETE: If no issue is found during the Abi encoding
 */
uint8_t Abi_Encode(Abi_Type_e inputAbiType,
                   uint8_t additionalInput,
                   const uint8_t *pAbiTypeData,
                   uint8_t *pBuffer);

#endif /* ABI_DECODE */
