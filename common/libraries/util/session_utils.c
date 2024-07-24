/**
 * @file    session_utils.c
 * @author  Cypherock X1 Team
 * @brief   Definition of the session utility functions
 *          This file defines the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "session_utils.h"

#include "inheritance_app.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
const ecdsa_curve *curve;
const uint32_t session_key_rotation[2] = {6, 7};
Session session = {0};

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void session_curve_init() {
  curve = get_curve_by_name(SECP256K1_NAME)->params;
}

static bool derive_server_public_key() {
  HDNode node;
  char xpub[112] = {'\0'};

  base58_encode_check(get_card_root_xpub(),
                      FS_KEYSTORE_XPUB_LEN,
                      nist256p1_info.hasher_base58,
                      xpub,
                      112);
  hdnode_deserialize_public(
      (char *)xpub, 0x0488b21e, NIST256P1_NAME, &node, NULL);

  uint8_t index = 0;
  hdnode_public_ckd(&node, session_key_rotation[index]);

  index += 1;
  hdnode_public_ckd(&node, session_key_rotation[index]);

  memcpy(
      session.derived_server_public_key, node.public_key, SESSION_PUB_KEY_SIZE);

  return true;
}

bool verify_session_signature(uint8_t *payload,
                              size_t payload_size,
                              uint8_t *signature) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);

  // TODO: uncomment while integrating the sdk
  // uint8_t status = ecdsa_verify_digest(
  // &nist256p1, session.derived_server_public_key, signature, hash);
  uint8_t status =
      ecdsa_verify_digest(&nist256p1,
                          session.device_random_public,
                          signature,
                          hash);    // since signing with device_random
  return (status == 0);
};

bool derive_session_id() {
  curve_point random_point;
  memcpy(
      &random_point, &session.server_random_public_point, sizeof(curve_point));
  point_add(curve, &session.device_random_public_point, &random_point);

  // printf("\nsession id random_point: ");
  // bn_print(&random_point.x);
  // bn_print(&random_point.y);

  uint8_t temp[2 * SESSION_PRIV_KEY_SIZE];
  bn_write_be(&random_point.x, temp);
  bn_write_be(&random_point.y, temp + SESSION_PRIV_KEY_SIZE);

  print_arr("session id temp", temp, 2 * SESSION_PRIV_KEY_SIZE);

  sha256_Raw(temp, 2 * SESSION_PUB_KEY_SIZE, session.session_id);

  print_arr("session.session_id", session.session_id, SESSION_ID_SIZE);

  return true;
}

bool derive_session_key() {
  if (ecdh_multiply(curve,
                    session.device_random,
                    session.server_random_public,
                    session.session_key) != 0) {
    printf("ERROR: Session key not generated");
    return false;
  }
  print_arr("session.session_key", session.session_key, SESSION_KEY_SIZE);
  return true;
}

void session_append_signature(uint8_t *payload, size_t payload_size) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);
  auth_data_t signed_data = atecc_sign(hash);

  uint8_t offset = payload_size;
  memcpy(payload + offset, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;
  print_arr("sig detail", signed_data.signature, SIGNATURE_SIZE);
  memcpy(payload + offset, signed_data.postfix1, POSTFIX1_SIZE);
  offset += POSTFIX1_SIZE;
  print_arr("sig detail", signed_data.postfix1, POSTFIX1_SIZE);
  memcpy(payload + offset, signed_data.postfix2, POSTFIX2_SIZE);
  offset += POSTFIX2_SIZE;
  print_arr("sig detail", signed_data.postfix2, POSTFIX2_SIZE);
}

bool session_get_random_keys(uint8_t *random,
                             uint8_t *random_public,
                             curve_point random_public_point) {
  memzero(random, SESSION_PRIV_KEY_SIZE);
  memzero(random_public, SESSION_PUB_KEY_SIZE);
  memset(&random_public_point, 0, sizeof(random_public_point));

#if USE_SIMULATOR == 0
  random_generate(random, SESSION_PRIV_KEY_SIZE);
#else
  uint8_t get_ec_random[32] = {0x0b, 0x78, 0x9a, 0x1e, 0xb8, 0x0b, 0x7a, 0xac,
                               0x97, 0xa1, 0x54, 0xd7, 0x0c, 0x5a, 0x53, 0x95,
                               0x6f, 0x9c, 0xed, 0x97, 0x6f, 0xc7, 0xed, 0x7f,
                               0xf9, 0x10, 0x01, 0xc1, 0xa8, 0x30, 0xde, 0xb1};
  memcpy(random, get_ec_random, SESSION_PRIV_KEY_SIZE);
#endif
  ecdsa_get_public_key33(curve, random, random_public);
  print_arr("private key", random, SESSION_PRIV_KEY_SIZE);
  print_arr("public key", random_public, SESSION_PUB_KEY_SIZE);

  if (!ecdsa_read_pubkey(curve, random_public, &random_public_point)) {
    printf("\nERROR: Random public key point not read");
    return false;
  }

  // bn_print(&random_public_point.x);
  // bn_print(&random_public_point.y);

  return true;
}

bool session_is_valid(uint8_t *pass_key, uint8_t *pass_id) {
  return (memcmp(pass_key, session.session_key, SESSION_KEY_SIZE) == 0 &&
          memcmp(pass_id, session.session_id, SESSION_ID_SIZE) == 0);
}

bool session_send_device_key(uint8_t *payload) {
  if (!session_get_random_keys(session.device_random,
                               session.device_random_public,
                               session.device_random_public_point)) {
    printf("\nERROR: Device Random keys not generated");
    return false;
  }

  // Get device_id
#if USE_SIMULATOR == 0
  if (get_device_serial() != 0) {
    printf("\nERROR: Device Serial fetch failed");
    return false;
  }
  memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
#else
  memcpy(session.device_id, session.device_random, DEVICE_SERIAL_SIZE);
#endif

  uint8_t offset = 0;
  memcpy(payload + offset, session.device_random_public, SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(payload + offset, session.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  session_append_signature(payload, offset);
  print_arr("payload after signature", payload, offset + 64 + 7 + 23);

  return true;
}

bool session_receive_server_key(uint8_t *server_message) {
  // Input Payload: Server_Random_public + Session Age + Device Id
  ASSERT(server_message != NULL);

  if (!derive_server_public_key()) {
    printf("\nERROR: Server Randoms not read");
    return false;
  }

  size_t server_message_size =
      SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE;

  // TODO: uncomment when server test starts
  /* if (!verify_session_signature(server_message,
                                 server_message_size,
                                 server_message + server_message_size)) {
     printf("\nERROR: Message from server invalid");
     return false;
   }
  */

  uint8_t offset = 0;
  memcpy(session.server_random_public,
         server_message + offset,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(session.session_age, server_message + offset, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;

  if (memcmp(session.device_id, server_message + offset, DEVICE_SERIAL_SIZE) !=
      0) {
#if USE_SIMULATOR == 0
    print_arr("session.device_id", session.device_id, DEVICE_SERIAL_SIZE);
    print_arr(
        "server_message + offset", server_message + offset, DEVICE_SERIAL_SIZE);
    return false;
#endif
  }
  offset += DEVICE_SERIAL_SIZE;

  if (!ecdsa_read_pubkey(curve,
                         session.server_random_public,
                         &session.server_random_public_point)) {
    printf("\nERROR: Server random public key point not read");
    return false;
  }
  print_arr("session.server_random_public",
            session.server_random_public,
            SESSION_PUB_KEY_SIZE);

  if (!derive_session_id() || !derive_session_key()) {
    printf("\nERROR: Generation session keys");
    return false;
  }

  return true;
}

void session_serialise_packet(SecureData *msgs,
                              size_t msg_count,
                              uint8_t *data,
                              size_t *len) {
  size_t size = *len;

  ASSERT(msgs != NULL);
  ASSERT(msg_count != 0);

  size = 0;
  for (int i = 0; i < msg_count; i++) {
    data[size] += (msgs[i].encrypted_data_size >> 8) & 0xFF;
    size += 1;
    data[size] += msgs[i].encrypted_data_size & 0xFF;
    size += 1;

    memcpy(data + size, msgs[i].encrypted_data, msgs[i].encrypted_data_size);
    size += msgs[i].encrypted_data_size;
  }

  *len = size;
}

session_error_type_e session_aes_encrypt_packet(uint8_t *InOut_data,
                                                size_t *len,
                                                uint8_t *key,
                                                uint8_t *iv_immut) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);

  size_t size = *len;
  ASSERT(size <= SESSION_PACKET_SIZE);

  uint8_t payload[size];
  memcpy(payload, InOut_data, size);
  memzero(InOut_data, size);

  uint8_t last_block[AES_BLOCK_SIZE] = {0};
  uint8_t remainder = size % AES_BLOCK_SIZE;

  // Round down to last whole block
  size -= remainder;
  // Copy old last block
  memcpy(last_block, payload + size, remainder);
  // Pad new last block with number of missing bytes
  memset(last_block + remainder,
         AES_BLOCK_SIZE - remainder,
         AES_BLOCK_SIZE - remainder);

  // the IV gets mutated, so we make a copy not to touch the original
  uint8_t iv[AES_BLOCK_SIZE] = {0};
  memcpy(iv, iv_immut, AES_BLOCK_SIZE);

  aes_encrypt_ctx ctx = {0};

  if (aes_encrypt_key256(key, &ctx) != EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_KEY_ERR;
  }

  if (aes_cbc_encrypt(payload, InOut_data, size, iv, &ctx) != EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_ERR;
  }

  if (aes_cbc_encrypt(
          last_block, InOut_data + size, sizeof(last_block), iv, &ctx) !=
      EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_ERR;
  }

  size += sizeof(last_block);
  *len = size;

  memset(&ctx, 0, sizeof(ctx));

  return SESSION_ENCRYPT_PACKET_SUCCESS;
}

bool session_encrypt_packet(SecureData *msgs,
                            uint8_t msg_count,
                            uint8_t *key,
                            uint8_t *iv,
                            uint8_t *packet,
                            size_t *packet_size) {
  session_serialise_packet(msgs, msg_count, packet, packet_size);
  memcpy(session.packet, packet, *packet_size);
  if (SESSION_ENCRYPT_PACKET_SUCCESS !=
      session_aes_encrypt_packet(packet, packet_size, key, iv))
    return false;

  memcpy(session.packet, packet, *packet_size);

  return true;
}

void session_deserialise_packet(SecureData *msgs,
                                size_t msg_count,
                                uint8_t *data,
                                size_t *len) {
  ASSERT(msgs != NULL);

  size_t index = 0;
  while (index <= *len) {
    msgs[msg_count].encrypted_data_size = (uint16_t)data[index] << 8;
    index += 1;
    msgs[msg_count].encrypted_data_size |= (uint16_t)data[index];
    index += 1;

    memcpy(msgs[msg_count].encrypted_data,
           data + index,
           msgs[msg_count].encrypted_data_size);
    index += msgs[msg_count].encrypted_data_size;

    msg_count += 1;
  }
}

session_error_type_e session_aes_decrypt_packet(uint8_t *InOut_data,
                                                uint16_t *len,
                                                uint8_t *key,
                                                uint8_t *iv) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);

  size_t size = *len;

  uint8_t payload[size];
  memcpy(payload, InOut_data, size);
  memzero(InOut_data, size);

  aes_decrypt_ctx ctx = {0};

  if (EXIT_SUCCESS != aes_decrypt_key256(key, &ctx))
    return SESSION_DECRYPT_PACKET_KEY_ERR;

  if (aes_cbc_decrypt(payload, InOut_data, size, iv, &ctx) != EXIT_SUCCESS ||
      size > SESSION_PACKET_SIZE)
    return SESSION_DECRYPT_PACKET_ERR;

  *len = size;
  memset(&ctx, 0, sizeof(ctx));

  return SESSION_DECRYPT_PACKET_SUCCESS;
}

bool session_decrypt_packet(SecureData *msgs,
                            uint8_t msg_count,
                            uint8_t *key,
                            uint8_t *iv,
                            uint8_t *packet,
                            size_t *packet_size) {
  memcpy(session.packet, packet, *packet_size);

  if (SESSION_DECRYPT_PACKET_SUCCESS !=
      session_aes_decrypt_packet(packet, packet_size, key, iv))
    return false;

  memcpy(session.packet, packet, *packet_size);

  session_deserialise_packet(msgs, msg_count, packet, packet_size);

  memcpy(session.packet, packet, *packet_size);

  return true;
}

bool session_encrypt_secure_data(uint8_t *wallet_id,
                                 SecureData *msgs,
                                 size_t msg_count) {
  ASSERT(wallet_id != NULL);
  ASSERT(msgs != NULL);
  ASSERT(msg_count != 0);
  ASSERT(msg_count <= SESSION_MSG_MAX);

  card_error_type_e status =
      card_fetch_encrypt_data(wallet_id, msgs, msg_count);
  if (status != CARD_OPERATION_SUCCESS) {
    printf("Card encrypt err: %x", status);
    return false;
  }

  memcpy(session.SessionMsgs, msgs, sizeof(SecureData) * msg_count);
  session.msg_count = msg_count;

  // TODO: remove after testing session core
  // if (!session_id_is_valid(session.session_id)) {
  //   printf("ERROR: Session is invalid");
  //   return false;
  // }
  // uint8_t packet[SESSION_PACKET_SIZE] = {0};
  // size_t packet_size = 0;

  // session_encrypt_packet(session.SessionMsgs,
  //                        session.msg_count,
  //                        session.session_key,
  //                        session.session_id,
  //                        packet,
  //                        &packet_size);
  // session_reset_secure_data();
  // session_decrypt_packet(session.SessionMsgs,
  //                        session.msg_count,
  //                        session.session_key,
  //                        session.session_id,
  //                        packet,
  //                        &packet_size);

  return true;
}

bool session_decrypt_secure_data(uint8_t *wallet_id,
                                 SecureData *msgs,
                                 size_t msg_count) {
  ASSERT(wallet_id != NULL);
  ASSERT(msgs != NULL);
  ASSERT(msg_count != 0);
  ASSERT(msg_count <= SESSION_MSG_MAX);

  card_error_type_e status =
      card_fetch_decrypt_data(wallet_id, msgs, msg_count);

  if (status != CARD_OPERATION_SUCCESS) {
    printf("ERROR: Card is invalid: %x", status);
    return false;
  }

  memcpy(session.SessionMsgs, msgs, sizeof(SecureData) * msg_count);
  session.msg_count = msg_count;

  return true;
}

// TODO: add is private in setup
bool session_plaindata_to_msg(uint8_t *plain_data[],
                              SecureData *msgs,
                              size_t *msg_count) {
  if (*msg_count > SESSION_MSG_MAX)
    return false;

  for (uint8_t i = 0; i < msg_count; i++) {
    msgs[i].plain_data_size = sizeof(plain_data[i]);
    memcpy(msgs[i].plain_data, plain_data[i], msgs[i].plain_data_size);
  }

  return true;
}

// TODO: add is private in recovery
bool session_msg_to_plaindata(uint8_t *plain_data[],
                              SecureData *msgs,
                              size_t *msg_count) {
  if (*msg_count > SESSION_MSG_MAX)
    return false;

  for (uint8_t i = 0; i < msg_count; i++) {
    memcpy(plain_data[i], msgs[i].plain_data, msgs[i].plain_data_size);
  }

  return true;
}

void session_reset() {
  memset(&session, 0, sizeof(session));
}

void session_reset_secure_data() {
  memset(&session.SessionMsgs, 0, sizeof(session.SessionMsgs));
  session.msg_count = 0;
}

void session_send_error() {
  LOG_ERROR("\nSESSION invalid session query err: %d", session.status);
}

session_error_type_e session_main(dummy_inheritance_query_t *query) {
  char buffer[SESSION_BUFFER_SIZE] = {0};
  size_t size;

  switch (query->type) {
    case SESSION_MSG_SEND_DEVICE_KEY:
      size = SESSION_PUB_KEY_SIZE + DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
             POSTFIX1_SIZE + POSTFIX2_SIZE;
      if (!session_send_device_key(query->device_message)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_DEVICE_KEY;
      }
      byte_array_to_hex_string(
          query->device_message, size, buffer, size * 2 + 1);
      printf("Device Message: %s", buffer, size * 2 + 1);
      break;

    case SESSION_MSG_RECEIVE_SERVER_KEY:
      if (!session_receive_server_key(query->server_message)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_SERVER_KEY;
      }

      size = SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE +
             SIGNATURE_SIZE;
      byte_array_to_hex_string(
          query->server_message, size, buffer, size * 2 + 1);
      printf("Server Message: %s", buffer, size * 2 + 1);
      break;

    case SESSION_MSG_ENCRYPT:
      // TODO: Remove after testing
      for (int i = 0; i < query->msg_count; i++) {
        memzero(query->SessionMsgs[i].encrypted_data, ENCRYPTED_DATA_SIZE);
        query->SessionMsgs[i].encrypted_data_size = 0;
      }
      if (!session_encrypt_secure_data(
              query->wallet_id, query->SessionMsgs, query->msg_count)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_ENCRYPT;
      }
      session_reset_secure_data();
      break;

    case SESSION_MSG_DECRYPT:
      // TODO: Remove after testing
      for (int i = 0; i < query->msg_count; i++) {
        memzero(query->SessionMsgs[i].plain_data, PLAIN_DATA_SIZE);
        query->SessionMsgs[i].plain_data_size = 0;
      }
      if (!session_decrypt_secure_data(
              query->wallet_id, query->SessionMsgs, query->msg_count)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_DECRYPT;
      }
      session_reset_secure_data();
      break;

    case SESSION_CLOSE:
      session_reset();
      break;

    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      session_send_error();
      session.status = SESSION_ERR_INVALID;
      return;
    }
  }

  session.status = SESSION_OK;
}

void test_session_main(session_msg_type_e type) {
  dummy_inheritance_query_t *query = malloc(sizeof(dummy_inheritance_query_t));
  query->type = type;

  session.status = SESSION_ERR_INVALID;
  session_curve_init();

  switch (query->type) {
    case SESSION_MSG_SEND_DEVICE_KEY:
      break;
    case SESSION_MSG_RECEIVE_SERVER_KEY:
      test_generate_server_data(query);
      break;
    case SESSION_MSG_ENCRYPT:
      test_generate_server_encrypt_data(query);
      break;
    case SESSION_MSG_DECRYPT:
      break;
  }

  session_main(query);
  free(query);

  if (session.status != SESSION_OK) {
    printf("\nERORR: SESSION COMMAND FAILED");
    return;
  }
  printf("\nSESSION MSG STATUS: %d", session.status);
}

// TODO: functions to remove after testing

void uint8ToHexString(const uint8_t *data, size_t size, char *hexstring) {
  for (size_t i = 0; i < size; ++i) {
    for (size_t i = 0; i < size; ++i) {
      sprintf(hexstring + 2 * i,
              "%02x",
              data[i]);    // Each byte represented by 2 characters + '\0'
    }
  }
  hexstring[size * 2] = '\0';    // Null-terminate the string
}

void print_msg(SecureData msg, uint8_t index) {
  char hex[200];
  byte_array_to_hex_string(
      msg.plain_data, msg.plain_data_size, hex, msg.plain_data_size * 2 + 1);
  printf("\nData [%d] plain_data: %s\n", index + 1, msg.plain_data);
  printf("Data [%d] plain_size: %d\n", index + 1, msg.plain_data_size);
  byte_array_to_hex_string(msg.encrypted_data,
                           msg.encrypted_data_size,
                           hex,
                           msg.encrypted_data_size * 2 + 1);
  printf("Data [%d] encrypted_data: %s\n", index + 1, msg.encrypted_data);
  printf("Data [%d] encrypted_size: %d\n", index + 1, msg.encrypted_data_size);
}

bool debug = true;
// TODO cleanup: delete after testing
char *print_arr(char *name, uint8_t *bytearray, size_t size) {
  char bytearray_hex[size * 2 + 1];
  if (debug == true) {
    uint8ToHexString(bytearray, size, bytearray_hex);
    printf("\n%s[%d bytes]: %s\n",
           name,
           (strlen(bytearray_hex) / 2),
           bytearray_hex);
  }
  return bytearray_hex;
}

void test_uint32_to_uint8_array(uint32_t value, uint8_t arr[4]) {
  arr[0] = (value >> 24) & 0xFF;    // Extract the highest byte
  arr[1] = (value >> 16) & 0xFF;    // Extract the second highest byte
  arr[2] = (value >> 8) & 0xFF;     // Extract the second lowest byte
  arr[3] = value & 0xFF;            // Extract the lowest byte
}

void test_generate_server_data(dummy_inheritance_query_t *query) {
  uint8_t server_message[SESSION_BUFFER_SIZE];

  uint8_t server_random[SESSION_PRIV_KEY_SIZE];
  uint8_t server_random_public[SESSION_PUB_KEY_SIZE];
  curve_point server_random_public_point;
  if (!session_get_random_keys(
          server_random, server_random_public, server_random_public_point)) {
    printf("\nERROR: Server Random keys not generated");
    return false;
  }

  uint32_t session_age_int = 1234;
  uint8_t session_age[SESSION_AGE_SIZE];
  test_uint32_to_uint8_array(session_age_int, session_age);

  uint8_t offset = 0;
  memcpy(server_message + offset, server_random_public, SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(server_message + offset, session_age, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message + offset, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;

  uint8_t hash[32] = {0};
  uint8_t signature[SIGNATURE_SIZE] = {0};
  sha256_Raw(server_message, offset, hash);
  // NOTE: MSG would be signed by server_key (added from master device in
  // server)
  int res =
      ecdsa_sign_digest(curve, session.device_random, hash, signature, 0, 0);
  memcpy(server_message + offset, signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;

  print_arr("server data", server_message, offset);

  memcpy(query->server_message, server_message, offset);
}

void test_generate_server_encrypt_data(dummy_inheritance_query_t *query) {
  const uint8_t msg_count = 5;
  static const char *messages[] = {
      "Shortest",    // 8 chars

      "This is a slightly longer message to test the 50 characters "
      "length requirement.",    // 79 chars

      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua.",    // 123 chars

      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
      "mollit anim id est laborum.",    // 445 chars

      "Embarking on a journey to write a 900-character piece necessitates "
      "precision and clarity, ensuring each word contributes to the overall "
      "message. Begin by defining the central theme or purpose, whether it's "
      "to inform, persuade, or entertain. Structure is crucial: start with an "
      "engaging introduction to hook the reader, followed by the main content "
      "divided into concise paragraphs, and conclude with a memorable closing "
      "statement. Use active voice and vary sentence lengths to maintain "
      "reader interest. Edit ruthlessly to eliminate redundant words and "
      "ensure each sentence flows seamlessly into the next. Pay attention to "
      "grammar and punctuation, as these details enhance readability and "
      "professionalism. Finally, read the piece aloud to catch any awkward "
      "phrasing or overlooked errors, ensuring the final draft is polished and "
      "impactful. This approach not only adheres to the character limit of "
      "msgs.",    // 900 char
  };

  query->msg_count = msg_count;
  for (int i = 0; i < msg_count; i++) {
    memcpy(query->SessionMsgs[i].plain_data, messages[i], strlen(messages[i]));
    query->SessionMsgs[i].plain_data_size = strlen(messages[i]);
    // print_msg(query->SessionMsgs[i], i);
  }

#if USE_SIMULATOR == 0
  memcpy(query->wallet_id, get_wallet_id(0), WALLET_ID_SIZE);
#else
  memcpy(query->wallet_id,
         session.device_id,
         WALLET_ID_SIZE);    // will get wallet id from cysync
#endif
  print_arr("query->wallet_id", query->wallet_id, WALLET_ID_SIZE);

  memcpy(query->pass_key, session.session_id, SESSION_ID_SIZE);
  print_arr("query->pass_key", query->pass_key, SESSION_ID_SIZE);
}

bool plain_data_to_array_obj(inheritance_plain_data_t *plain_data,
                             SecureData *msgs,
                             size_t msgs_count) {
  for (uint8_t i = 0; i < msgs_count; i++) {
    memcpy(msgs[i].plain_data,
           plain_data[i].message.bytes,
           plain_data[i].message.size);
    msgs[i].plain_data_size = plain_data[i].message.size;
  }

  return true;
}
