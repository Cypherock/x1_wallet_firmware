#ifndef BYTE_STREAM_H
#define BYTE_STREAM_H

#include <stdint.h>

typedef enum {
  BYTE_STREAM_WRITER_UNKNOWN_ERROR = 0,
  BYTE_STREAM_WRITER_SUCCESS,
} byte_stream_writer_status_e;

typedef struct byte_stream_t {
  uint8_t *stream_pointer;
  byte_stream_writer_status_e (*writer)(struct byte_stream_t *);
  uint32_t offset;
  uint32_t capacity;
} byte_stream_t;

typedef enum {
  BYTE_STREAM_UNKNOWN_ERROR = 0,
  BYTE_STREAM_INVALID_STREAM,
  BYTE_STREAM_INVALID_DESTINATION,
  BYTE_STREAM_FAILED_WRITER,
  BYTE_STREAM_SUCCESS,
} byte_stream_status_e;

byte_stream_status_e read_byte_stream(byte_stream_t *byte_stream,
                                      uint8_t *destination,
                                      uint64_t number_of_bytes_to_read);

byte_stream_status_e skip_byte_stream(byte_stream_t *byte_stream,
                                      uint64_t number_of_bytes_to_skip);

#endif    // BYTE_STREAM_H
