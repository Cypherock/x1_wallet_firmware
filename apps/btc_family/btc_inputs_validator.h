/**
 * @author  Cypherock X1 Team
 * @brief   Bitcoin inputs validator, uses a stream to read data
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_INPUTS_VALIDATOR_H
#define BTC_INPUTS_VALIDATOR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "btc/sign_txn.pb.h"
#include "byte_stream.h"

typedef enum {
  BTC_VALIDATE_ERR_UNKNOWN = 0,
  BTC_VALIDATE_ERR_INVALID_PARAMS,
  BTC_VALIDATE_ERR_READ_STREAM,
  BTC_VALIDATE_ERR_INVALID_VARINT,
  BTC_VALIDATE_ERR_INVALID_TX_HASH,
  BTC_VALIDATE_ERR_INVALID_OUTPUT_VALUE,
  BTC_VALIDATE_SUCCESS,
} btc_validation_error_e;

#define VERSION_NO_SIZE 4
#define TX_OUT_VALUE_SIZE 8
#define TX_IN_SEQ_NO_SIZE 4
#define MAX_VARINT_SIZE 9
#define SHA256_DIGEST_LENGTH 32
#define SLICE_SIZE 1024    // Adjust as needed based on system requirements

#define TX_IN_INDEX_OFFSET 1
#define TX_OUT_INDEX_OFFSET 1

btc_validation_error_e btc_validate_inputs(byte_stream_t *stream,
                                           const btc_sign_txn_input_t *input);

#endif    // BTC_INPUTS_VALIDATOR_H
