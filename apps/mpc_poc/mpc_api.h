#ifndef MPC_API_H
#define MPC_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <mpc_poc/core.pb.h>
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
 * @brief API to decode query from host with `SOLANA_QUERY_FIELDS`
 *
 * @param[in] data: PB encoded bytestream received from host
 * @param[in] data_size: size of pb encoded bytestream
 * @param[out] query_out: @ref solana_query_t obj to copy the decoded result to
 * @return bool True if decoding was successful, else false
 */
bool decode_mpc_query(const uint8_t *data,
                         uint16_t data_size,
                         mpc_poc_query_t *query_out);

/**
 * @brief Encodes the SOLANA result with `SOLANA_RESULT_FIELDS` to byte-stream
 *
 * @param[in] result: object of populated @ref solana_result_t to be encoded
 * @param[out] buffer: buffer to fill byte-stream into
 * @param[in] max_buffer_len: Max length allowed for writing bytestream to
 * buffer
 * @param[out] bytes_written_out: bytes written to bytestream
 * @return bool True if decoding was successful, else false
 */
bool encode_mpc_result(const mpc_poc_result_t *result,
                          uint8_t *buffer,
                          uint16_t max_buffer_len,
                          size_t *bytes_written_out);

/**
 * @brief This API checks if the `which_request` field of the query of type
 * `solana_query_t` matches against the expected tag.
 *
 * @param query The query of type `solana_query_t` to be checked
 * @param exp_query_tag The expected tag of the query
 * @return true If the query tag matches the expected tag
 * @return false If the query tag does not match the expected tag
 */
bool check_mpc_query(const mpc_poc_query_t *query, pb_size_t exp_query_tag);

/**
 * @brief Returns zero initialized object of type
 * solana_result_t result_tag set in result.which_response field
 *
 * @param result_tag Result tag to be set in the solana_result_t result
 * @return solana_result_t Result object of type solana_result_t
 */
mpc_poc_result_t init_mpc_result(pb_size_t result_tag);

/**
 * @brief Send the error to the host.
 *
 * @param which_error The error type to be sent
 * @param error_code The error code to sent to the host
 */
void mpc_send_error(pb_size_t which_error, uint32_t error_code);

/**
 * @brief This API encodes solana_result_t in protobuf structure.
 * @details If the encoding is successful, then it sends the corresponding
 * result to the host.
 *
 * The function ASSERTs the result of encode_solana_result internally.
 *
 * @param result The result which needs to be sent to the host.
 */
void mpc_send_result(const mpc_poc_result_t *result);

/**
 * @brief This API receives request of type solana_query_t of type
 * exp_query_tag from the host.
 *
 * @param query The reference to which the query needs to be populated
 * @param exp_query_tag The expected tag of the query
 * @return true If the query was recieved from the host matching the tag
 * @return false If the request timed out or the recieved request did not match
 * the tag
 */
bool mpc_get_query(mpc_poc_query_t *query, pb_size_t exp_query_tag);

#endif