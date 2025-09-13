/**
 * @file    canton_api.h
 * @author  Cypherock X1 Team
 * @brief   Header exports helper functions for internal canton usage
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CANTON_API_H
#define CANTON_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "canton/core.pb.h"
#include "pb.h"

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
 * @brief Returns zero initialized object of type @ref canton_result_t with
 * result.which_response set to the given result_tag
 *
 * @param[in] result_tag Result tag to be set in @ref
 * canton_result_t.which_response
 * @returns canton_result_t result object
 */
canton_result_t init_canton_result(pb_size_t result_tag);

/**
 * @brief Check if `which_request` field of the given query matches with the
 * given `exp_query_tag` tag
 *
 * @param[in] query The query of type @ref canton_query_t to be checked
 * @param[in] exp_query_tag The expected tag of the query
 * @return bool Indicating whether tag matches with the expected tag or not
 */
bool check_canton_query(const canton_query_t *query, pb_size_t exp_query_tag);

/**
 * @brief Decodes canton query from host with `CANTON_QUERY_FIELDS`
 *
 * @param[in] data PB encoded bytestream
 * @param[in] data_size Size of pb encoded bytestream
 * @param[out] query_out @ref canton_query_t decoded data gets populated
 * @returns bool Indicating whether decoding succeeded or not
 */
bool decode_canton_query(const uint8_t *data,
                         uint16_t data_size,
                         canton_query_t *query_out);

/**
 * @brief Encodes the canton result with `CANTON_RESULT_FIELDS` to bytestream
 *
 * @param[in] result object of populated @ref canton_result_t to be encoded
 * @param[out] buffer buffer to fill bytestream into
 * @param[in] max_buffer_len Upper limit allowed for writing bytestream to
 * buffer
 * @param[out] bytes_written_out Actual count of buffer written
 * @return bool Indicating whether decoding succeeded or not
 */
bool encode_canton_result(const canton_result_t *result,
                          uint8_t *buffer,
                          uint16_t max_buffer_len,
                          size_t *bytes_written_out);

/**
 * @brief Send a error to the host
 *
 * @param which_error The error type to be sent
 * @param error_code The error code to sent to the host
 */
void canton_send_error(pb_size_t which_error, uint32_t error_code);

/**
 * @brief Send encodes and sends canton_result_t type obj to host
 * @note ASSERTs the result of encoder internally
 *
 * @param result The canton_result_t to encode and send
 */
void canton_send_result(const canton_result_t *result);

/**
 * @brief Receives request of type @ref canton_query_t of the given tag type.
 *
 * @param[out] query Reference to @ref canton_query_t obj which would be
 * populated
 * @param [in] exp_query_tag The expected query type tag
 * @return bool Indicating whether recieved query is of the given expected type.
 */
bool canton_get_query(canton_query_t *query, pb_size_t exp_query_tag);

#endif
