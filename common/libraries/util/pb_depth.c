/**
 * @file pb_depth.c
 * @brief Bounded nesting-depth check for serialized protobuf messages.
 */

#include "pb_depth.h"

#include "pb_common.h"
#include "pb_decode.h"

static bool check_depth(pb_istream_t *stream,
                        const pb_msgdesc_t *fields,
                        uint8_t depth,
                        uint8_t max_depth) {
  if (depth > max_depth) {
    return false;
  }

  pb_field_iter_t iter;
  // NULL message: only the descriptor values are used
  bool has_fields = pb_field_iter_begin(&iter, fields, NULL);

  while (stream->bytes_left > 0) {
    pb_wire_type_t wire_type;
    uint32_t tag;
    bool eof;
    if (!pb_decode_tag(stream, &wire_type, &tag, &eof)) {
      return eof;
    }

    if (PB_WT_STRING == wire_type && has_fields &&
        pb_field_iter_find(&iter, tag) && PB_LTYPE_IS_SUBMSG(iter.type) &&
        NULL != iter.submsg_desc) {
      pb_istream_t substream;
      if (!pb_make_string_substream(stream, &substream)) {
        return false;
      }
      if (!check_depth(&substream, iter.submsg_desc, depth + 1, max_depth)) {
        return false;
      }
      if (!pb_close_string_substream(stream, &substream)) {
        return false;
      }
    } else if (!pb_skip_field(stream, wire_type)) {
      return false;
    }
  }

  return true;
}

bool pb_check_depth(const pb_byte_t *buf,
                    size_t size,
                    const pb_msgdesc_t *fields,
                    uint8_t max_depth) {
  if (NULL == buf || NULL == fields) {
    return false;
  }
  pb_istream_t stream = pb_istream_from_buffer(buf, size);
  return check_depth(&stream, fields, 0, max_depth);
}
