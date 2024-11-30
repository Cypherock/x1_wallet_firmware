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

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

btc_validation_error_e btc_validate_inputs(byte_stream_t *stream,
                                           const btc_sign_txn_input_t *input);

#endif    // BTC_INPUTS_VALIDATOR_H
