/**
 * @file eip712_validate.h
 * @brief Validation of host supplied EIP-712 typed data before it is shown or
 * signed.
 *
 * The host sends typed data as a tree of nodes. Every value that is displayed
 * must be exactly the value that is hashed, so each node is checked for a
 * known type, a consistent size and a type name that matches its encoding.
 * The type hash of every struct is recomputed on the device from the field
 * names and types that are displayed, instead of trusting the host's hash.
 */

#ifndef EIP712_VALIDATE_H
#define EIP712_VALIDATE_H

#include <stdbool.h>
#include <stdint.h>

#include "evm/sign_msg.pb.h"

/**
 * @brief Validates decoded EIP-712 typed data.
 *
 * @param typed_data Decoded typed data
 *
 * @return true if every node is well formed and every struct's type hash
 * matches the one computed from the displayed field names and types
 */
bool eip712_validate_typed_data(const evm_sign_typed_data_struct_t *typed_data);

/**
 * @brief Value of a validated bool node; empty data is treated as false so
 * that the displayed and the signed value are the same.
 */
uint8_t eip712_bool_value(const evm_sign_typed_data_node_t *node);

#endif /* EIP712_VALIDATE_H */
