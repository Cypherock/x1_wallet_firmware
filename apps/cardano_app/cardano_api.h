/**
 * @file    cardano_api.j
 * @author  Cypherock X1 Team
 * @brief   Internal usage functions for cardano
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARDANO_API_H
#define CARDANO_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cardano/core.pb.h"
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
 * @brief Returns zero initialized object of type @ref cardano_result_t with
 * result.which_response set to the given result_tag
 *
 * @param[in] result_tag Result tag to be set in @ref
 * cardano_result_t.which_response
 * @returns cardano_result_t result object
 */
cardano_result_t init_cardano_result(pb_size_t result_tag);

/**
 * @brief Check if `which_request` field of the given query matches with the
 * given `exp_query_tag` tag
 *
 * @param[in] query The query of type @ref cardano_query_t to be checked
 * @param[in] exp_query_tag The expected tag of the query
 * @return bool Indicating whether tag matches with the expected tag or not
 */
bool check_cardano_query(const cardano_query_t *query, pb_size_t exp_query_tag);

/**
 * @brief Decodes cardano query from host with `CARDANO_QUERY_FIELDS`
 *
 * @param[in] data PB encoded bytestream
 * @param[in] data_size Size of pb encoded bytestream
 * @param[out] query_out @ref cardano_query_t decoded data gets populated
 * @returns bool Indicating whether decoding succeeded or not
 */
bool decode_cardano_query(const uint8_t *data,
                          uint16_t data_size,
                          cardano_query_t *query_out);

/**
 * @brief Encodes the cardano result with `CARDANO_RESULT_FIELDS` to bytestream
 *
 * @param[in] result object of populated @ref cardano_result_t to be encoded
 * @param[out] buffer buffer to fill bytestream into
 * @param[in] max_buffer_len Upper limit allowed for writing bytestream to
 * buffer
 * @param[out] bytes_written_out Actual count of buffer written
 * @return bool Indicating whether decoding succeeded or not
 */
bool encode_cardano_result(const cardano_result_t *result,
                           uint8_t *buffer,
                           uint16_t max_buffer_len,
                           size_t *bytes_written_out);

/**
 * @brief Send a error to the host
 *
 * @param which_error The error type to be sent
 * @param error_code The error code to sent to the host
 */
void cardano_send_error(pb_size_t which_error, uint32_t error_code);

/**
 * @brief Send encodes and sends canton_result_t type obj to host
 * @note ASSERTs the result of encoder internally
 *
 * @param result The cardano_result_t to encode and send
 */
void cardano_send_result(const cardano_result_t *result);

/**
 * @brief Receives request of type @ref cardano_query_t of the given tag type.
 *
 * @param[out] query Reference to @ref cardano_query_t obj which would be
 * populated
 * @param [in] exp_query_tag The expected query type tag
 * @return bool Indicating whether recieved query is of the given expected type.
 */
bool cardano_get_query(cardano_query_t *query, pb_size_t exp_query_tag);

/**
 * @brief Decodes cardano serialized data from host with the given fields
 *
 * @param[in] data PB encoded bytestream
 * @param[in] data_size Size of pb encoded bytestream
 * @param[in] fields Fields to be decoded
 * @param[out] dest_struct Destination struct to store the decoded data
 * @returns bool Indicating whether decoding succeeded or not
 */
bool decode_cardano_serialized_data(const uint8_t *data,
                                    uint16_t data_size,
                                    const pb_msgdesc_t *fields,
                                    void *dest_struct);
#endif    // CARDANO_API_H
