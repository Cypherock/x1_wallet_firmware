/**
 * @file    common_error.h
 * @author  Cypherock X1 Team
 * @brief   Get an initialized instance of common error.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <error.pb.h>

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
 * @brief Returns an initialized instance of error_common_error_t.
 * @details The error_code is uint32_t. The function works because size of union
 * is uint32_t.
 * NOTE: Any updates in error_common_error_t might effect the working of this
 * function.
 *
 * @param which_error The error type to initialize
 * @param error_code The error code to be filled
 * @return An instance of error_common_error_t filled with the provided values
 */
error_common_error_t init_common_error(pb_size_t which_error,
                                       uint32_t error_code);

#endif
