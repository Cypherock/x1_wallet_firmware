/**
 * @file byte_stream.h
 * @brief Header file defining the byte stream operations and structures.
 *
 * This file defines structures and functions to handle byte streams, which
 * allow reading, writing, and skipping bytes in a buffered stream.
 */

#ifndef BYTE_STREAM_H
#define BYTE_STREAM_H

#include <stdint.h>

/**
 * @enum byte_stream_writer_status_e
 * @brief Enumeration of the possible writer statuses.
 *
 * These status codes are returned by the byte stream writer callback function.
 */
typedef enum {
  BYTE_STREAM_WRITER_UNKNOWN_ERROR = 0,
  BYTE_STREAM_WRITER_READING_OUT_OF_BOUNDS,
  BYTE_STREAM_WRITER_INVALID_REQUIRED_PARAMETERS,
  BYTE_STREAM_WRITER_SUCCESS,
} byte_stream_writer_status_e;

/**
 * @struct byte_stream_t
 * @brief Structure representing a byte stream.
 *
 * This structure encapsulates the current position (offset), the capacity of
 * the byte stream, and the pointer to the data stream. It also contains a
 * callback function (writer) for processing the stream.
 */
typedef struct byte_stream_t {
  uint8_t *stream_pointer;
  /**
   * The `writer` is responsible for reseting the `offset`, `stream_pointer`,
   * and the `capacity`; when called it should also throw appropriate erros
   * indicating error for writing to stream.
   */
  byte_stream_writer_status_e (*writer)(struct byte_stream_t *);
  uint32_t offset;
  uint32_t capacity;
} byte_stream_t;

/**
 * @enum byte_stream_status_e
 * @brief Enumeration of the possible byte stream operation statuses.
 *
 * These status codes are returned by functions performing byte stream
 * operations.
 */
typedef enum {
  BYTE_STREAM_UNKNOWN_ERROR = 0,
  BYTE_STREAM_INVALID_STREAM,
  BYTE_STREAM_INVALID_DESTINATION,
  BYTE_STREAM_FAILED_WRITER,
  BYTE_STREAM_SUCCESS,
} byte_stream_status_e;

/**
 * @brief Reads bytes from the byte stream into the destination buffer.
 *
 * This function reads `number_of_bytes_to_read` bytes from the byte stream and
 * copies them into the provided destination buffer.
 *
 * If the stream offset reaches its `capacity` this function automatically calls
 * the `writer` to refill the stream. It depends on the `writer` to manage the
 * new `offset` and `capacity` as well
 *
 * @param byte_stream Pointer to the byte stream object.
 * @param destination Pointer to the destination buffer where data will be
 * copied.
 * @param number_of_bytes_to_read Number of bytes to read from the byte stream.
 *
 * @return Status code indicating success or failure of the operation.
 */
byte_stream_status_e read_byte_stream(byte_stream_t *byte_stream,
                                      uint8_t *destination,
                                      uint64_t number_of_bytes_to_read);

/**
 * @brief Skips a specified number of bytes in the byte stream.
 *
 * This function advances the stream's offset by `number_of_bytes_to_skip`
 * without copying the data to a destination.
 *
 * If the stream offset reaches its `capacity` this function automatically calls
 * the `writer` to refill the stream. It depends on the `writer` to manage the
 * new `offset` and `capacity` as well
 *
 * @param byte_stream Pointer to the byte stream object.
 * @param number_of_bytes_to_skip Number of bytes to skip in the byte stream.
 *
 * @return Status code indicating success or failure of the operation.
 */
byte_stream_status_e skip_byte_stream(byte_stream_t *byte_stream,
                                      uint64_t number_of_bytes_to_skip);

#endif    // BYTE_STREAM_H
