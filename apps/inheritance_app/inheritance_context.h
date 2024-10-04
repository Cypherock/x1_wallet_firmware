/**
 * @file    inheritance_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the INHERITANCE app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INHERITANCE_CONTEXT_H
#define INHERITANCE_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define INHERITANCE_MESSAGES_MAX_COUNT 5
#define INHERITANCE_PACKET_MAX_SIZE 6000
#define INHERITANCE_PIN_TAG 0xff

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum inheritance_message_type {
  INHERITANCE_DEFAULT_MESSAGE = 0x00,
  INHERITANCE_ONLY_SHOW_ON_DEVICE = 0xff,
} inheritance_message_type_e;

typedef enum {
  ENCRYPTION_ERROR_DEFAULT = 0,
  ENCRYPTION_INVALID_REQUEST_ERROR,
  ENCRYPTION_INVALID_WALLET_ID_ERROR,
  ENCRYPTION_PIN_NOT_SET_ERROR,
  ENCRYPTION_USER_ABORT_FAILURE,
  ENCRYPTION_PB_DECODE_FAIL_ERROR,
  ENCRYPTION_INVALID_DATA_ERROR,
  ENCRYPTION_VERIFICATION_FAIL_ERROR,
  ENCRYPTION_MESSAGE_MAX_COUNT_EXCEED_ERROR,
  ENCRYPTION_CARD_ENCRYPTION_FAIL_ERROR,
  ENCRYPTION_SESSION_ENCRYPTION_FAIL_ERROR,
  ENCRYPTION_PB_ENCODE_FAIL_ERROR,
  ENCRYPTION_CHUNK_DATA_INVALID_ERROR,
  ENCRYPTION_ASSERT_MALLOC_ERROR,
  ENCRYPTION_OK,
} encryption_error_type_e;

typedef enum {
  ENCRYPTION_DEFAULT_START_FLOW = 0,
  ENCRYPTION_QUERY_HANDLE_FLOW,
  ENCRYPTION_PLAIN_DATA_GET_FLOW,
  ENCRYPTION_PIN_VERIFY_FLOW,
  ENCRYPTION_USER_VERIFY_FLOW,
  ENCRYPTION_MESSAGE_SERIALIZE_FLOW,
  ENCRYPTION_MESSAGE_ENCRYPT_FLOW,
  ENCRYPTION_PACKET_SERIALIZE_FLOW,
  ENCRYPTION_PACKET_ENCRYPT_FLOW,
  ENCRYPTION_SEND_RESULT_FLOW,
} encryption_flow_t;
/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* INHERITANCE_CONTEXT_H */
