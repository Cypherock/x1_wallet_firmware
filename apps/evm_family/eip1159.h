/**
 * @file    eip1559.h
 * @author  Cypherock X1 Team
 * @brief   EVM eip1559 transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EIP1559_H
#define EIP1559_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "evm_txn_helpers.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Decodes transaction buffer based on the structure defined in EIP-2930
 * @details The function ensures that entire buffer is processed. If buffer
 * contains extra data (which is left unprocessed after decoding all the rlp
 * elements defined in the EIP), it will return false indicating failure in
 * decoding. Refer: https://eips.ethereum.org/EIPS/eip-2930
 *
 * @param data          The buffer containing encoded ethereum transaction as
 * defined in EIP-2930
 * @param data_size     The size of the data buffer
 * @param txn_context   Pointer to an instance of evm_txn_context_t which will
 * hold the decoded fields
 *
 * @return bool Indicating if parsing succeeded
 * @retval true If transaction is parsed successfully
 * @retval false If transaction parsing fails
 */
bool evm_parse_eip2930(const uint8_t *data,
                       size_t data_size,
                       evm_txn_context_t *txn_context);

/**
 * @brief Decodes transaction buffer based on the structure defined in EIP-1559
 * @details The function ensures that entire buffer is processed. If buffer
 * contains extra data (which is left unprocessed after decoding all the rlp
 * elements defined in the EIP), it will return false indicating failure in
 * decoding. Refer: https://eips.ethereum.org/EIPS/eip-1559
 *
 * @param data          The buffer containing encoded ethereum transaction as
 * defined in EIP-2930
 * @param data_size     The size of the data buffer
 * @param txn_context   Pointer to an instance of evm_txn_context_t which will
 * hold the decoded fields
 *
 * @return bool Indicating if parsing succeeded
 * @retval true If transaction is parsed successfully
 * @retval false If transaction parsing fails
 */
bool evm_parse_eip1559(const uint8_t *data,
                       size_t data_size,
                       evm_txn_context_t *txn_context);

#endif
