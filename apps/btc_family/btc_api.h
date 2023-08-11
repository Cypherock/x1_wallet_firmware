/**
 * @file    btc_api.h
 * @author  Cypherock X1 Team
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_API_H
#define BTC_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <btc/core.pb.h>
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
 * @brief API to decode query from host with `BTC_QUERY_FIELDS`
 *
 * @param[in] data: PB encoded bytestream received from host
 * @param[in] data_size: size of pb encoded bytestream
 * @param[out] query_out: @ref btc_query_t obj to copy the decoded result to
 * @return bool True if decoding was successful, else false
 */
bool decode_btc_query(const uint8_t *data,
                      uint16_t data_size,
                      btc_query_t *query_out);

/**
 * @brief Encodes the bitcoin result with `BTC_RESULT_FIELDS` to byte-stream
 *
 * @param[in] result: object of populated @ref btc_result_t to be encoded
 * @param[out] buffer: buffer to fill byte-stream into
 * @param[in] max_buffer_len: Max length allowed for writing bytestream to
 * buffer
 * @param[out] bytes_written_out: bytes written to bytestream
 * @return bool True if decoding was successful, else false
 */
bool encode_btc_result(const btc_result_t *result,
                       uint8_t *buffer,
                       uint16_t max_buffer_len,
                       size_t *bytes_written_out);

/**
 * @brief This API checks if the `which_request` field of the query of type
 * `btc_query_t` matches against the expected tag.
 *
 * @param query The query of type `btc_query_t` to be checked
 * @param exp_query_tag The expected tag of the query
 * @return true If the query tag matches the expected tag
 * @return false If the query tag does not match the expected tag
 */
bool check_btc_query(const btc_query_t *query, pb_size_t exp_query_tag);

/**
 * @brief Returns zero initialized object of type
 * btc_result_t result_tag set in result.which_response field
 *
 * @param result_tag Result tag to be set in the btc_result_t result
 * @return btc_result_t Result object of type btc_result_t
 */
btc_result_t init_btc_result(pb_size_t result_tag);

/**
 * @brief Send the error to the host.
 *
 * @param which_error The error type to be sent
 * @param error_code The error code to sent to the host
 */
void btc_send_error(pb_size_t which_error, uint32_t error_code);

/**
 * @brief This API encodes btc_result_t in protobuf structure.
 * @details If the encoding is successful, then it sends the corresponding
 * result to the host.
 *
 * The function ASSERTs the result of encode_btc_result internally.
 *
 * @param result The result which needs to be sent to the host.
 */
void btc_send_result(const btc_result_t *result);

/**
 * @brief This API receives request of type btc_query_t of type
 * exp_query_tag from the host.
 *
 * @param query The reference to which the query needs to be populated
 * @param exp_query_tag The expected tag of the query
 * @return true If the query was recieved from the host matching the tag
 * @return false If the request timed out or the recieved request did not match
 * the tag
 */
bool btc_get_query(btc_query_t *query, pb_size_t exp_query_tag);

#endif
