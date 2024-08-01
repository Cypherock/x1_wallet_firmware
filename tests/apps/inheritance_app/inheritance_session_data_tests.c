/**
 * @file    inheritance_script_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bitcoin scripts
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

#include "inheritance_api.h"
#include "inheritance_main.h"
#include "unity_fixture.h"

TEST_GROUP(inheritance_session_data_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(inheritance_session_data_test) {
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(inheritance_session_data_test) {
}

TEST(inheritance_session_data_test, inheritance_setup_one) {
  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;

  query.setup.plain_data_count = 1;

  char message_1[PLAIN_DATA_SIZE] = "An important message.";
  memcpy(query.setup.plain_data[0].message.bytes, message_1, 21);
  query.setup.plain_data[0].message.size = 21;
  query.setup.plain_data[0].is_private = true;

  inheritance_result_t response = INHERITANCE_RESULT_INIT_ZERO;
  inheritance_setup(&query, &response);

  uint8_t encrpyted_result[ENCRYPTED_DATA_SIZE / 2];
  hex_string_to_byte_array(
      "eb3c8139d596dae909477a6e676499f23f24ed643496e1ca1c656533f02a852d",
      64,
      encrpyted_result);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      encrpyted_result, response.setup.encrypted_data.packet.bytes, 32);
}

TEST(inheritance_session_data_test, inheritance_recovery_one) {
  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;

  uint8_t encrpyted_data_packet[ENCRYPTED_DATA_SIZE];
  hex_string_to_byte_array(
      "eb3c8139d596dae909477a6e676499f23f24ed643496e1ca1c656533f02a852d",
      64,
      encrpyted_data_packet);

  query.recovery.has_encrypted_data = true;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(query.recovery.encrypted_data.packet.bytes, encrpyted_data_packet, 32);
  query.recovery.encrypted_data.packet.size = 32;

  inheritance_result_t response = INHERITANCE_RESULT_INIT_ZERO;
  inheritance_recovery(&query, &response);

  char plain_data_1_message[PLAIN_DATA_SIZE] = "An important message.";
  bool plain_data_1_is_private = true;

  TEST_ASSERT_EQUAL_CHAR_ARRAY(plain_data_1_message,
                               response.recovery.plain_data[0].message.bytes,
                               response.recovery.plain_data[0].message.size);
  TEST_ASSERT_EQUAL_FLOAT(plain_data_1_is_private,
                          response.recovery.plain_data[0].is_private);
}

const uint8_t msg_count = 5;
static const bool *sizes[] = {8, 79, 123, 445, 900};
static const bool *is_privates[] = {true, false, true, false, true};
static const char *msgs[] = {
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

TEST(inheritance_session_data_test, inheritance_setup_five) {
  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;
  query.setup.plain_data_count = msg_count;

  for (int i = 0; i < msg_count; i++) {
    memcpy(query.setup.plain_data[i].message.bytes, msgs[i], sizes[i]);
    query.setup.plain_data[i].message.size = sizes[i];
    query.setup.plain_data[i].is_private = is_privates[i];
  }

  inheritance_result_t response = INHERITANCE_RESULT_INIT_ZERO;
  inheritance_setup(&query, &response);

  uint8_t encrpyted_result[ENCRYPTED_DATA_SIZE * SESSION_MSG_MAX];
  hex_string_to_byte_array(
      "11ff0392dba65b5390b3558affad48a3538e4f8cd20d4d56c4a51cebf3b4b093975876a6"
      "fa987f2bee51036d382a9a48c7f38d8a4797a3f873979a51ec060940938788499ed01a4e"
      "183d2dcaff4f4d03ab903a46baa5cebbb31904ae76615613719a2f5253907a75a899e4f0"
      "23d4605102e5af2e8bba03b75c09261461e6f0c4fdef7300c5fb8fba62c753ce5915a349"
      "33ece0f6feccf52ec7d807a95c0fd624acaaae440c6054b5fcf83ca7315e64f7fb6d504a"
      "d9a15b4faa2bfa3c730ad5cc990f101fb59cfa20f65c73a9f7c40656acfada7d7b726a01"
      "69adad296b12690869c06f1f0faa506ba8ce0d0a7689a308490731f45ab38ea917725517"
      "0beae8018bb5d59086b1d02c65470a3c7af9d722150b9d0e65cee99df4b662f4896d6199"
      "6899942ada85b5f0d9b056a8460cdbe9387785b8aae5429e82e7390791db510b7146db37"
      "7fc373f736afa722db173f6adb38f29a3956db69646e012e4d0b69d2fc845007341f8159"
      "2adb5380c0480b5d138d0fff6e7224f436b8a157bae6d25e67a15b168bb66caa6738f555"
      "56464cedd962d4339a53716145f3746a57a3526c0c7e60851ed8efc8ec2f73248b40257d"
      "67b75f7e265dc028bbded7244fda3116c3cccc2bb2aabbb937d17c590b5c02ccdaf525cf"
      "d258622a46e19f78be8a2871ae71735b832ca67beadc03825b0819ef30e27b2320924c69"
      "1a18124115c0fe8213afb057aebde944ae2b08ebe5b8de6b0f1c440920e6749d528a368f"
      "3b8b90fe9733040472124125beca9057ec47de6e2237f5c278e3c0356166e6046fae8088"
      "40d7e12b409bd4080ce7ac7e6242f90d8c322cfe228e49a70b5457ca65716074b49c46d7"
      "3b6588753d2e5295a8402f1d7e3d3add59e981a42155b1bbfa941803a7c24ccce7884695"
      "aafb3e98c41e74cb48607f060060b069766a7757857050fa79410d8859a0993373670996"
      "eaab5e3ecc44e7fbed99b68dc69681816c7fd09679ad59695c9d662a00362d46185544fb"
      "58db5029a3f4ff4cd9d127bf44853a4a65242490433bcbe40180eaf03c0f5c56f3bb5cce"
      "0b02643d62e5d51c2f3829dd3161726fcf343438c3bfc1f0ca33873bbec0e734fde4e111"
      "88b2d5348938bd41e5fb411bfa862d6f712ce0e62dbd8c8b052013ad69a6e870d5ab6b2a"
      "ece9afd49ed3a9fe0e58728ed6c3ee631cf21551c3c62264a83a45febb6eb76709d7e8a1"
      "a262d871e9b47b2d2ab97922f0cee742639e0bdb05cb17b0bf7e59d634fa66d78831a681"
      "0adb3219ada4bc28a6aeeb123baea5ab52ec91fd194f3b9864ac4c7e67f4e81b16047928"
      "54dcac9494dff788e3362d3d56025a9c2d1511282a5d15ab2721d9884c46d3fa709c9da3"
      "2591f040b45cda8fca909336d893b5a7b6749921cc64bceb71a7b5b6a276835556db5c3f"
      "5835abaf29205e365e31c376997003c81a1c25be740041594f802c6f638155ee7f73f908"
      "fc876c882c866b30c8a77dadb232ddd7a7da242d0d15c1eb498d3a7afa252879584f8da7"
      "716410f448975c5c1e54919de8cd8798dbc9406d5cb706d9770deca7dad6f8b6cb9a72a6"
      "4a8124cfac00f75a8ca65c22cf2476179842d4258c6eaf08b5eb188bd8d9744f0f789927"
      "8c6428e08693bcc0d4dbe78b94255de444c399512e9d4e471802a18ec570668265561ee9"
      "0a773fbd60b12cb32d6410ec0a5b5adbce8e2ed7940bb6875a5a66d36113205308ec5d7d"
      "4222c2f1dbe8a4bccade4bba50a8202f6b62ec7e64a862b1aecf817714c65c423a9e0977"
      "8e10d46d1db5cb3df4d7685b57f465b24c116f2c17d8d3bedd94172f312fe0df636cd297"
      "7b2fda3aaad55abe847998590a6fe85201c97cfdbacace5f2939722102256973477f4c7b"
      "1454981453e08480ae58d2cea387354a1f68aad12faf40c07659b9c0f7f6528c87bc52a0"
      "2e9ca568b074a21d0df1d7665458c439731716d833fbd9c536fd0a5b65437f2ec38ec866"
      "b4e1088ada341a18b443d631eefc37f571d49de285a9904f258881cbdd74c67c4b6fa37c"
      "e85440159e43ea6000a932a71f1add03acd6b02dfaa358b3569701dee29111a96b2b465f"
      "96800eb59adb14da2fcf59336afba85e3edb03b79293c41eef0cff68e6cc5e5fc63ba0fd"
      "a84771890be5eaddebe85235a1617f2af1e3bd5a095f30e5b9d67aa959c187763db373ee"
      "442b90c4d9a2a29689f203bd7f3b9c44f29d56e50817259cbfe9be71d6dee838365be7b2"
      "4a635a1b1b217cdcffb17a1747697da4d28e077a0f6593490f1d09e8c1cc2ed752bc8545"
      "9f841c5d5fdb212d70b8f013a02857e4e0e4d5753581c2139fbe5b0d43a934aca4ee3633"
      "d734a67d648431cd",
      3328,
      encrpyted_result);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      encrpyted_result, response.setup.encrypted_data.packet.bytes, 1664);
}

TEST(inheritance_session_data_test, inheritance_recovery_five) {
  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;

  uint8_t encrpyted_data_packet[ENCRYPTED_DATA_SIZE * SESSION_MSG_MAX];
  hex_string_to_byte_array(
      "11ff0392dba65b5390b3558affad48a3538e4f8cd20d4d56c4a51cebf3b4b093975876a6"
      "fa987f2bee51036d382a9a48c7f38d8a4797a3f873979a51ec060940938788499ed01a4e"
      "183d2dcaff4f4d03ab903a46baa5cebbb31904ae76615613719a2f5253907a75a899e4f0"
      "23d4605102e5af2e8bba03b75c09261461e6f0c4fdef7300c5fb8fba62c753ce5915a349"
      "33ece0f6feccf52ec7d807a95c0fd624acaaae440c6054b5fcf83ca7315e64f7fb6d504a"
      "d9a15b4faa2bfa3c730ad5cc990f101fb59cfa20f65c73a9f7c40656acfada7d7b726a01"
      "69adad296b12690869c06f1f0faa506ba8ce0d0a7689a308490731f45ab38ea917725517"
      "0beae8018bb5d59086b1d02c65470a3c7af9d722150b9d0e65cee99df4b662f4896d6199"
      "6899942ada85b5f0d9b056a8460cdbe9387785b8aae5429e82e7390791db510b7146db37"
      "7fc373f736afa722db173f6adb38f29a3956db69646e012e4d0b69d2fc845007341f8159"
      "2adb5380c0480b5d138d0fff6e7224f436b8a157bae6d25e67a15b168bb66caa6738f555"
      "56464cedd962d4339a53716145f3746a57a3526c0c7e60851ed8efc8ec2f73248b40257d"
      "67b75f7e265dc028bbded7244fda3116c3cccc2bb2aabbb937d17c590b5c02ccdaf525cf"
      "d258622a46e19f78be8a2871ae71735b832ca67beadc03825b0819ef30e27b2320924c69"
      "1a18124115c0fe8213afb057aebde944ae2b08ebe5b8de6b0f1c440920e6749d528a368f"
      "3b8b90fe9733040472124125beca9057ec47de6e2237f5c278e3c0356166e6046fae8088"
      "40d7e12b409bd4080ce7ac7e6242f90d8c322cfe228e49a70b5457ca65716074b49c46d7"
      "3b6588753d2e5295a8402f1d7e3d3add59e981a42155b1bbfa941803a7c24ccce7884695"
      "aafb3e98c41e74cb48607f060060b069766a7757857050fa79410d8859a0993373670996"
      "eaab5e3ecc44e7fbed99b68dc69681816c7fd09679ad59695c9d662a00362d46185544fb"
      "58db5029a3f4ff4cd9d127bf44853a4a65242490433bcbe40180eaf03c0f5c56f3bb5cce"
      "0b02643d62e5d51c2f3829dd3161726fcf343438c3bfc1f0ca33873bbec0e734fde4e111"
      "88b2d5348938bd41e5fb411bfa862d6f712ce0e62dbd8c8b052013ad69a6e870d5ab6b2a"
      "ece9afd49ed3a9fe0e58728ed6c3ee631cf21551c3c62264a83a45febb6eb76709d7e8a1"
      "a262d871e9b47b2d2ab97922f0cee742639e0bdb05cb17b0bf7e59d634fa66d78831a681"
      "0adb3219ada4bc28a6aeeb123baea5ab52ec91fd194f3b9864ac4c7e67f4e81b16047928"
      "54dcac9494dff788e3362d3d56025a9c2d1511282a5d15ab2721d9884c46d3fa709c9da3"
      "2591f040b45cda8fca909336d893b5a7b6749921cc64bceb71a7b5b6a276835556db5c3f"
      "5835abaf29205e365e31c376997003c81a1c25be740041594f802c6f638155ee7f73f908"
      "fc876c882c866b30c8a77dadb232ddd7a7da242d0d15c1eb498d3a7afa252879584f8da7"
      "716410f448975c5c1e54919de8cd8798dbc9406d5cb706d9770deca7dad6f8b6cb9a72a6"
      "4a8124cfac00f75a8ca65c22cf2476179842d4258c6eaf08b5eb188bd8d9744f0f789927"
      "8c6428e08693bcc0d4dbe78b94255de444c399512e9d4e471802a18ec570668265561ee9"
      "0a773fbd60b12cb32d6410ec0a5b5adbce8e2ed7940bb6875a5a66d36113205308ec5d7d"
      "4222c2f1dbe8a4bccade4bba50a8202f6b62ec7e64a862b1aecf817714c65c423a9e0977"
      "8e10d46d1db5cb3df4d7685b57f465b24c116f2c17d8d3bedd94172f312fe0df636cd297"
      "7b2fda3aaad55abe847998590a6fe85201c97cfdbacace5f2939722102256973477f4c7b"
      "1454981453e08480ae58d2cea387354a1f68aad12faf40c07659b9c0f7f6528c87bc52a0"
      "2e9ca568b074a21d0df1d7665458c439731716d833fbd9c536fd0a5b65437f2ec38ec866"
      "b4e1088ada341a18b443d631eefc37f571d49de285a9904f258881cbdd74c67c4b6fa37c"
      "e85440159e43ea6000a932a71f1add03acd6b02dfaa358b3569701dee29111a96b2b465f"
      "96800eb59adb14da2fcf59336afba85e3edb03b79293c41eef0cff68e6cc5e5fc63ba0fd"
      "a84771890be5eaddebe85235a1617f2af1e3bd5a095f30e5b9d67aa959c187763db373ee"
      "442b90c4d9a2a29689f203bd7f3b9c44f29d56e50817259cbfe9be71d6dee838365be7b2"
      "4a635a1b1b217cdcffb17a1747697da4d28e077a0f6593490f1d09e8c1cc2ed752bc8545"
      "9f841c5d5fdb212d70b8f013a02857e4e0e4d5753581c2139fbe5b0d43a934aca4ee3633"
      "d734a67d648431cd",
      3328,
      encrpyted_data_packet);

  query.recovery.has_encrypted_data = true;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(
      query.recovery.encrypted_data.packet.bytes, encrpyted_data_packet, 1664);
  query.recovery.encrypted_data.packet.size = 1664;

  inheritance_result_t response = INHERITANCE_RESULT_INIT_ZERO;
  inheritance_recovery(&query, &response);

  for (int i = 0; i < msg_count; i++) {
    TEST_ASSERT_EQUAL_CHAR_ARRAY(msgs[i],
                                 response.recovery.plain_data[i].message.bytes,
                                 response.recovery.plain_data[i].message.size);
    TEST_ASSERT_EQUAL_INT(is_privates[i],
                          response.recovery.plain_data[i].is_private);
  }
}