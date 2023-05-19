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
#include <manager/core.pb.h>
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
bool decode_manager_query(const uint8_t *data,
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
                           size_t *bytes_written_out);

/**
 * @brief This API checks if the `which_request` field of the query of type
 * `manager_query_t` matches against the expected tag.
 *
 * @param query The query of type `manager_query_t` to be checked
 * @param exp_query_tag The expected tag of the query
 * @return true If the query tag matches the expected tag
 * @return false If the query tag does not match the expected tag
 */
bool check_manager_request(const manager_query_t *query,
                           const pb_size_t exp_query_tag);

/**
 * @brief Returns zero initialized object of type
 * manager_result_t result_tag set in result.which_response field
 *
 * @param result_tag Result tag to be set in the manager_result_t result
 * @return manager_result_t Result object of type manager_result_t
 */
manager_result_t get_manager_result_template(const pb_size_t result_tag);

/**
 * @brief This API encodes manager_result_t in protobuf structure. If the
 * encoding is successful, then it sends the corresponding result to the host.
 *
 * @param result The result which needs to be sent to the host.
 * @param out_buffer Pointer to the buffer where protobuf encoder can output
 * it's result
 * @param size_of_buffer Size of output buffer
 * @return true If protobuf encoding was done and message was sent to the host
 * @return false If protobuf encoding failed.
 */
bool encode_and_send_manager_result(manager_result_t *result,
                                    uint8_t *out_buffer,
                                    const size_t size_of_buffer);

#endif
