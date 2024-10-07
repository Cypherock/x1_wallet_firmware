/**
 * @file    inheritance_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for inheritance app internal operations
 *          This file is defined to separate INHERITANCE's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INHERITANCE_PRIV_H
#define INHERITANCE_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "card_fetch_data.h"
#include "ed25519.h"
#include "inheritance/core.pb.h"
#include "inheritance_context.h"
#include "ui_input_text.h"
#include "wallet.h"

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define CHALLENGE_SIZE_MAX 32
#define CHALLENGE_SIZE_MIN 16
#define ENTROPY_SIZE_LIMIT 100
#define SIZE_SEED 64
/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
#pragma pack(push, 1)
typedef struct {
  ed25519_signature signature;
  ed25519_public_key public_key;
} auth_wallet_result_t;

typedef struct {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t challenge[CHALLENGE_SIZE_MAX];
  size_t challenge_size;
} auth_wallet_data_t;

typedef struct {
  const uint8_t entropy[ENTROPY_SIZE_LIMIT];
  uint8_t entropy_size;
  ed25519_secret_key private_key;
  auth_wallet_result_t result;
  bool has_data;
} auth_wallet_private_t;

typedef struct {
  auth_wallet_data_t data;
  auth_wallet_private_t wallet_based_data;
  auth_wallet_private_t seed_based_data;
  bool with_public_key;
  bool do_seed_based;
  bool do_wallet_based;
} auth_wallet_config_t;
#pragma pack(pop)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  secure_data_t data[INHERITANCE_MESSAGES_MAX_COUNT];
  uint8_t data_count;
  uint8_t wallet_id[WALLET_ID_SIZE];
  inheritance_encrypt_data_with_pin_plain_data_structure_t plain_data;
  uint8_t pin_value[MAX_PIN_SIZE];
  inheritance_encrypt_data_with_pin_encrypted_data_structure_t payload;
} inheritance_encryption_context_t;

typedef struct {
  secure_data_t data[INHERITANCE_MESSAGES_MAX_COUNT];
  uint8_t data_count;
  uint8_t wallet_id[WALLET_ID_SIZE];
  inheritance_decrypt_data_with_pin_encrypted_data_structure_t encrypted_data;
  inheritance_decrypt_data_with_pin_decrypted_data_structure_t response_payload;
} inheritance_decryption_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Entry point to auth wallet flow
 *
 */
void inheritance_auth_wallet(inheritance_query_t *query);

/**
 * @brief Handler for inheritance messages encrytion flow.
 * @details The expected request type is INHERITANCE_QUERY_ENCRYPT_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for encrytion of inheritance messages.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void inheritance_encrypt_data(inheritance_query_t *query);

/**
 * @brief Handler for inheritance message decryption
 * @details This flow expects INHERITANCE_QUERY_DECRYPT_TAG as initial query.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for decryption of inheritance data.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void inheritance_decrypt_data(inheritance_query_t *query);
#endif /* INHERITANCE_PRIV_H */
