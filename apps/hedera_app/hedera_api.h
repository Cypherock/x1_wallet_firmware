/**
 * @file    hedera_api.h
 * @author  Cypherock X1 Team
 * @brief   Header file for Hedera app helper functions.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#ifndef HEDERA_API_H
#define HEDERA_API_H

#include <hedera/core.pb.h>
#include <stdint.h>

// API to decode query from host with HEDERA_QUERY_FIELDS
bool decode_hedera_query(const uint8_t *data, uint16_t data_size, hedera_query_t *query_out);

// Encodes the Hedera result with HEDERA_RESULT_FIELDS to byte-stream
bool encode_hedera_result(const hedera_result_t *result, uint8_t *buffer, uint16_t max_buffer_len, size_t *bytes_written_out);

// Checks if the `which_request` field of the query matches the expected tag.
bool check_hedera_query(const hedera_query_t *query, pb_size_t exp_query_tag);

// Returns a zero-initialized hedera_result_t with the specified result tag.
hedera_result_t init_hedera_result(pb_size_t result_tag);

// Sends an error response to the host.
void hedera_send_error(pb_size_t which_error, uint32_t error_code);

// Encodes and sends a result to the host.
void hedera_send_result(const hedera_result_t *result);

// Waits for and decodes a query from the host, ensuring it matches the expected tag.
bool hedera_get_query(hedera_query_t *query, pb_size_t exp_query_tag);

#endif