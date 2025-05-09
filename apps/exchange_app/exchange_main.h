/**
 * @file    exchange_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various Exchange actions
 supported.
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef EXCHANGE_MAIN_H
#define EXCHANGE_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"
#include "composable_app_queue.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef enum EXCHANGE_FLOW_TAGS {
  EXCHANGE_FLOW_TAG_RECEIVE = 0x1,
  EXCHANGE_FLOW_TAG_FETCH_SIGNATURE,
  EXCHANGE_FLOW_TAG_STORE_SIGNATURE,
  EXCHANGE_FLOW_TAG_SEND,
} exchange_flow_tag_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Returns the config for Exchange chain app descriptors
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_exchange_app_desc();

/**
 * @brief Validates the data against the data at the front of the composable app
 * queue.
 *
 * It compares the provided data with the data at the front of the queue. If
 * they match, it removes the data from the queue. If they do not match, it
 * triggers a reset. Does nothing if no data is present.
 *
 * @param data The data to validate against the queue.
 * @return true If the validation is successful and data is popped.
 * @return false If the validation fails.
 */
bool exchange_app_validate_caq(caq_node_data_t data);

/**
 * @brief Validates a stored signature against a server public key.
 *
 * Derives the server's public key and verifies the signature against the
 * provided receiver address.
 *
 * @param receiver Pointer to the receiver address string.
 * @param receiver_max_size Maximum size of the receiver buffer.
 * @return true If the signature is valid.
 * @return false If the signature is invalid.
 */
bool exchange_validate_stored_signature(char *receiver,
                                        size_t receiver_max_size);

/**
 * @brief Signs an address using the ATECC608A and stores the signature in the
 * shared context.
 *
 * This function calculates the SHA256 hash of the address, signs it using the
 * ATECC608A, and stores the resulting signature and postfix data in the shared
 * context.
 *
 * @param address Pointer to the address string to be signed.
 * @param address_max_size Maximum size of the address buffer.
 */
void exchange_sign_address(char *address, size_t address_max_size);
#endif /* EXCHANGE_MAIN_H */
