/**
 * @file    hedera_api.c
 * @author  Cypherock X1 Team
 * @brief   Defines helper APIs for the Hedera app.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#include "hedera_api.h"
#include <pb_decode.h>
#include <pb_encode.h>
#include "common_error.h"
#include "core_api.h"
#include "events.h"

bool decode_hedera_query(const uint8_t *data, uint16_t data_size, hedera_query_t *query_out) {
    if (NULL == data || NULL == query_out || 0 == data_size) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_DECODING_FAILED);
        return false;
    }
    memzero(query_out, sizeof(hedera_query_t));
    pb_istream_t stream = pb_istream_from_buffer(data, data_size);
    bool status = pb_decode(&stream, HEDERA_QUERY_FIELDS, query_out);
    if (!status) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_DECODING_FAILED);
    }
    return status;
}

bool encode_hedera_result(const hedera_result_t *result, uint8_t *buffer, uint16_t max_buffer_len, size_t *bytes_written_out) {
    if (NULL == result || NULL == buffer || NULL == bytes_written_out) return false;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);
    bool status = pb_encode(&stream, HEDERA_RESULT_FIELDS, result);
    if (status) {
        *bytes_written_out = stream.bytes_written;
    }
    return status;
}

bool check_hedera_query(const hedera_query_t *query, pb_size_t exp_query_tag) {
    if ((NULL == query) || (exp_query_tag != query->which_request)) {
        hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_QUERY);
        return false;
    }
    return true;
}

hedera_result_t init_hedera_result(pb_size_t result_tag) {
    hedera_result_t result = HEDERA_RESULT_INIT_ZERO;
    result.which_response = result_tag;
    return result;
}

void hedera_send_error(pb_size_t which_error, uint32_t error_code) {
    hedera_result_t result = init_hedera_result(HEDERA_RESULT_COMMON_ERROR_TAG);
    result.common_error = init_common_error(which_error, error_code);
    hedera_send_result(&result);
}

void hedera_send_result(const hedera_result_t *result) {
    uint8_t buffer[1700] = {0};
    size_t bytes_encoded = 0;
    ASSERT(encode_hedera_result(result, buffer, sizeof(buffer), &bytes_encoded));
    send_response_to_host(buffer, bytes_encoded);
}

bool hedera_get_query(hedera_query_t *query, pb_size_t exp_query_tag) {
    evt_status_t event = get_events(EVENT_CONFIG_USB, MAX_INACTIVITY_TIMEOUT);
    if (event.p0_event.flag) {
        return false;
    }
    if (!decode_hedera_query(event.usb_event.p_msg, event.usb_event.msg_size, query)) {
        return false;
    }
    if (!check_hedera_query(query, exp_query_tag)) {
        return false;
    }
    return true;
}