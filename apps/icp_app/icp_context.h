/**
 * @file    icp_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the ICP app
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef ICP_CONTEXT_H
#define ICP_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pb.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define ICP_NAME "ICP"
#define ICP_LUNIT "ICP"
#define ICP_PUB_KEY_SIZE 33
#define ICP_PRINCIPAL_LENGTH 29
#define ICP_ACCOUNT_ID_LENGTH 32

#define ICP_SELF_AUTH_ID_TAG 0x02
#define SECP256K1_UNCOMPRESSED_PK_LEN 65

#define ICP_SUBACCOUNT_ID_LEN 32

// macros for read_state_request
#define MAX_PATHS 1    // Max number of paths (only 1 needed)
#define MAX_SEGMENTS                                                           \
  2    // Maximum segments per path (only 2 needed: "request_status" and
       // requestId)
#define MAX_SEGMENT_SIZE 32    // Maximum byte length of a segment

#define MAX_INGRESS_EXPIRY_SIZE 10

#define SHA256_DIGEST_LENGTH 32

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// TODO: Populate structure for ICP
typedef struct {
} icp_config_t;

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/candid/src/idl.ts#L27
// https://github.com/dfinity/agent-js/blob/main/packages/candid/src/idl.ts#L1703
typedef enum {
  Null = -1,
  Bool = -2,
  Nat = -3,
  Int = -4,

  Nat8 = -5,
  Nat16 = -6,
  Nat32 = -7,
  Nat64 = -8,

  Int8 = -9,
  Int16 = -10,
  Int32 = -11,
  Int64 = -12,

  Float32 = -13,
  Float64 = -14,
  Text = -15,
  Reserved = -16,
  Empty = -17,
  Opt = -18,
  Vector = -19,
  Record = -20,
  Variant = -21,
  Func = -22,
  Service = -23,
  Principal = -24,
} IDLTypes_e;

// Calculated hashes of transfer method fields for easy comparison and avoiding
// re-computing again and again
/// Reference:
// https://github.com/Zondax/ledger-icp/blob/main/app/src/candid/candid_types.h#L104
typedef enum {
  transfer_hash_to = 25979,
  transfer_hash_fee = 5094982,
  transfer_hash_memo = 1213809850,
  transfer_hash_from_subaccount = 1835347746,    // optional
  transfer_hash_created_at_time = 3258775938,    // optional
  transfer_hash_amount = 3573748184,
} transfer_hash_fields;

/// Reference:
// https://github.com/dfinity/ic-js/blob/main/packages/ledger-icp/candid/ledger.certified.idl.js#L13
typedef struct {
  uint64_t e8s;
} token_t;

/// Reference:
// https://github.com/dfinity/ic-js/blob/main/packages/ledger-icp/candid/ledger.certified.idl.js#L198
typedef struct {
  uint64_t timestamp_nanos;
} timestamp_t;

/// Reference:
// https://github.com/dfinity/ic-js/blob/main/packages/ledger-icp/candid/ledger.certified.idl.js#L291
typedef struct {
  uint8_t to[ICP_ACCOUNT_ID_LENGTH];
  token_t amount;
  token_t fee;
  uint64_t memo;

  // optional fields
  bool has_from_subaccount;
  uint8_t from_subaccount[ICP_SUBACCOUNT_ID_LEN];
  bool has_created_at_time;
  timestamp_t created_at_time;
} icp_transfer_t;

typedef PB_BYTES_ARRAY_T(MAX_SEGMENT_SIZE) icp_path_segment_t;
typedef PB_BYTES_ARRAY_T(MAX_INGRESS_EXPIRY_SIZE)
    icp_read_state_request_ingress_expiry_t;

// Represents a path, which is an array of icp_path_segment_t
typedef struct {
  icp_path_segment_t segments[MAX_SEGMENTS];    // Path consists of segments
  size_t segment_count;    // Number of segments in this path
} icp_read_state_request_path_t;

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/agent/src/agent/http/types.ts#L93
typedef struct {
  char *request_type;
  icp_read_state_request_path_t paths[MAX_PATHS];
  size_t path_count;    // Number of paths
  icp_read_state_request_ingress_expiry_t ingress_expiry;
  uint8_t sender[ICP_PRINCIPAL_LENGTH];
} icp_read_state_request_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* ICP_CONTEXT_H */
