/**
 * @file pb_depth.h
 * @brief Bounded nesting-depth check for serialized protobuf messages.
 *
 * nanopb decodes nested submessages recursively and has no depth limit, so a
 * deeply nested recursive message (e.g. a value containing a value) can
 * exhaust the stack while it is being decoded. The check below walks the wire
 * format with the message descriptors, recursing only up to a fixed bound, so
 * that such input can be rejected before calling pb_decode.
 */

#ifndef PB_DEPTH_H
#define PB_DEPTH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pb.h"

/**
 * @brief Checks that a serialized message does not nest submessages deeper
 * than max_depth.
 *
 * The top level message is depth 0. Only fields that the descriptors declare
 * as submessages are descended into; every other field is skipped.
 *
 * @param buf Serialized message
 * @param size Size of buf
 * @param fields Descriptor of the top level message
 * @param max_depth Maximum allowed nesting depth
 *
 * @return true if the message is well formed (at the wire level) and within
 * max_depth, false otherwise
 */
bool pb_check_depth(const pb_byte_t *buf,
                    size_t size,
                    const pb_msgdesc_t *fields,
                    uint8_t max_depth);

#endif /* PB_DEPTH_H */
