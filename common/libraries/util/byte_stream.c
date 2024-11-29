#include "byte_stream.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BS_THROW_IF(x, y)                                                      \
  if (x)                                                                       \
  return y

static byte_stream_status_e process_byte_stream(
    byte_stream_t *byte_stream,
    uint8_t *destination,
    uint64_t total_number_of_bytes_to_process,
    bool is_copying_data) {
  BS_THROW_IF(byte_stream == NULL, BYTE_STREAM_INVALID_STREAM);
  BS_THROW_IF(byte_stream->stream_pointer == NULL, BYTE_STREAM_INVALID_STREAM);
  BS_THROW_IF(byte_stream->writer == NULL, BYTE_STREAM_INVALID_STREAM);
  BS_THROW_IF(byte_stream->capacity == 0, BYTE_STREAM_INVALID_STREAM);
  BS_THROW_IF(byte_stream->offset > byte_stream->capacity,
              BYTE_STREAM_INVALID_STREAM);
  BS_THROW_IF(total_number_of_bytes_to_process == 0,
              BYTE_STREAM_INVALID_STREAM);

  if (is_copying_data) {
    BS_THROW_IF(destination == NULL, BYTE_STREAM_INVALID_DESTINATION);
  } else {
    BS_THROW_IF(destination != NULL, BYTE_STREAM_INVALID_DESTINATION);
  }

  uint64_t processed_bytes = 0;

  do {
    uint64_t remaining_stream_size =
        byte_stream->capacity - byte_stream->offset;

    BS_THROW_IF(remaining_stream_size < 0, BYTE_STREAM_INVALID_STREAM);

    if (remaining_stream_size == 0) {
      byte_stream_writer_status_e writer_status =
          byte_stream->writer(byte_stream);
      BS_THROW_IF(writer_status != BYTE_STREAM_WRITER_SUCCESS,
                  BYTE_STREAM_FAILED_WRITER);
      remaining_stream_size = byte_stream->capacity - byte_stream->offset;
    }

    uint64_t remaining_bytes_to_process =
        total_number_of_bytes_to_process - processed_bytes;
    uint64_t number_of_bytes_to_process_this_time =
        remaining_bytes_to_process < remaining_stream_size
            ? remaining_bytes_to_process
            : remaining_stream_size;

    if (is_copying_data) {
      memcpy(destination + processed_bytes,
             byte_stream->stream_pointer + byte_stream->offset,
             number_of_bytes_to_process_this_time);
    }

    byte_stream->offset += number_of_bytes_to_process_this_time;
    processed_bytes += number_of_bytes_to_process_this_time;

  } while (processed_bytes < total_number_of_bytes_to_process);

  return BYTE_STREAM_SUCCESS;
}

byte_stream_status_e read_byte_stream(byte_stream_t *byte_stream,
                                      uint8_t *destination,
                                      uint64_t number_of_bytes_to_read) {
  return process_byte_stream(
      byte_stream, destination, number_of_bytes_to_read, true);
}

byte_stream_status_e skip_byte_stream(byte_stream_t *byte_stream,
                                      uint64_t number_of_bytes_to_skip) {
  return process_byte_stream(byte_stream, NULL, number_of_bytes_to_skip, false);
}
