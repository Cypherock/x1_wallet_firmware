/**
 * @file    near_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Header file to export some helper functions for the NEAR app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_HELPERS_H
#define NEAR_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "near.h"

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
 * @brief Verifies the derivation path for any inconsistent/unsupported values.
 * The derivation depth is fixed at level 5. So if the depth level != 5, then
 * this function return false indicating invalid derivation path. Also, the path
 * indices should be hardened (as the EC curve only supports hadened derivation)
 * otherwise it is considered invalid. The supported derivation path is
 * `m/44'/397'/0'/0'/i'`.
 *
 * @param path      The address derivation path to be checked
 * @param levels    The number of levels in the derivation path
 *
 * @return true If the derivation path is as correct
 * @return false If the derivation path is NOT as expected
 */
bool near_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Extracts `account_id` field from the arguments of `create_account`
 * method call in case of NEAR_FUNCTION_CALL actions.
 *
 * @param args Constant reference to buffer containing the argument
 * @param args_len Length of the total argument
 * @param account_id Reference to char buffer where the `account_id` string will
 * be populated
 */
void near_get_new_account_id_from_fn_args(const char *args,
                                          uint32_t args_len,
                                          char *account_id);

/**
 * @brief Get the amount string from hex bytes of the NEAR unsigned transaction.
 * The lunit_name is also concatinated with the amount string
 * @note It is expected that the bytes are in LE order
 *
 * @param amount Constant reference to buffer containing hex bytes of amount
 * @param string Reference to char buffer where the amount string can be
 * populated
 * @param size_of_string Maximum size of the char buffer
 */
void get_amount_string(const uint8_t *amount,
                       char *string,
                       size_t size_of_string);

#endif /* NEAR_HELPERS_H */
