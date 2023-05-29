/**
 * @file    manager_api.h
 * @author  Cypherock X1 Team
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef MANAGER_API_H
#define MANAGER_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <btc/core.pb.h>
#include <stddef.h>
#include <stdint.h>

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
 * @brief API to decode query from host with `MANAGER_QUERY_FIELDS`
 *
 * @param[in] data: PB encoded bytestream received from host
 * @param[in] data_size: size of pb encoded bytestream
 * @param[out] query_out: @ref manager_query_t obj to copy the decoded result to
 * @return bool True if decoding was successful, else false
 */
bool decode_btc_query(const uint8_t *data,
                      uint16_t data_size,
                      manager_query_t *query_out);

#endif
