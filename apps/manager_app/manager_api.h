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
#include "manager_app.h"

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
bool decode_manager_query(uint8_t *data,
                          uint16_t data_size,
                          manager_query_t *query_out);

/**
 * @brief Encodes the manager result with `MANAGER_RESULT_FIELDS` to byte-stream
 *
 * @param[in] result: object of populated @ref manager_result_t to be encoded
 * @param[out] buffer: buffer to fill byte-stream into
 * @param[in] max_buffer_len: Max length allowed for writing bytestream to
 * buffer
 * @param[out] bytes_written_out: bytes written to bytestream
 * @return bool True if decoding was successful, else false
 */
bool encode_manager_result(manager_result_t *result,
                           uint8_t *buffer,
                           uint16_t max_buffer_len,
                           uint32_t *bytes_written_out);
#endif
