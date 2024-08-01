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
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <string.h>

#include "inheritance_api.h"
#include "inheritance_main.h"
#include "unity_fixture.h"

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
static inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
static inheritance_result_t response = INHERITANCE_RESULT_INIT_ZERO;
static SecureData *msgs = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

TEST_GROUP(inheritance_session_data_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(inheritance_session_data_test) {
  msgs = (SecureData *)calloc(SESSION_MSG_MAX, sizeof(SecureData));
  set_dummy_session();
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(inheritance_session_data_test) {
  memset(&query, 0, sizeof(query));
  memset(&response, 0, sizeof(response));
  memset(msgs, 0, sizeof(msgs));
}

TEST(inheritance_session_data_test, inheritance_setup_one) {
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;
  query.setup.plain_data_count = 1;

  char message_1[PLAIN_DATA_SIZE] = "An important message.";
  memcpy(query.setup.plain_data[0].message.bytes, message_1, 21);
  query.setup.plain_data[0].message.size = 21;
  query.setup.plain_data[0].is_private = true;

  inheritance_setup(&query, msgs, &response);

  uint8_t encrpyted_result[SESSION_PACKET_SIZE] = {0};
  hex_string_to_byte_array(
      "eb3c8139d596dae909477a6e676499f23f24ed643496e1ca1c656533f02a852d",
      64,
      encrpyted_result);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      encrpyted_result, response.setup.encrypted_data.packet.bytes, 32);
}

TEST(inheritance_session_data_test, inheritance_recovery_one) {
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;

  uint8_t encrpyted_data_packet[SESSION_PACKET_SIZE];
  hex_string_to_byte_array(
      "eb3c8139d596dae909477a6e676499f23f24ed643496e1ca1c656533f02a852d",
      64,
      encrpyted_data_packet);

  query.recovery.has_encrypted_data = true;
  memcpy(query.recovery.encrypted_data.packet.bytes, encrpyted_data_packet, 32);
  query.recovery.encrypted_data.packet.size = 32;

  inheritance_recovery(&query, msgs, &response);

  char plain_data_1_message[PLAIN_DATA_SIZE] = "An important message.";
  bool plain_data_1_is_private = true;

  TEST_ASSERT_EQUAL_CHAR_ARRAY(plain_data_1_message,
                               response.recovery.plain_data[0].message.bytes,
                               response.recovery.plain_data[0].message.size);
  TEST_ASSERT_EQUAL_INT8(plain_data_1_is_private,
                         response.recovery.plain_data[0].is_private);
}

const uint8_t msg_count = 5;
static const uint16_t *sizes[] = {8, 79, 123, 445, 900};
static const bool *is_privates[] = {true, false, true, false, true};
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

static const char *encrypted_packet_five =
    "9e8117143bdbc11c0fbed0383a6bc12f7e62b56bf77384ce0eddd1ee9e9193fb73c157c0"
    "5e8da3ecf2a0f88bd13149d423875c4ead17120acffa6e45d8115a033c47929a5cb0ba64"
    "b5da374803da22089c6f23c81b6226192d09bc9dc52a3f55e160e14d778c954eb19f7389"
    "a0f6430cd776d416f452c4b382f591c4d7f4f37234f60eea3b83deb45a126ed0156b3e14"
    "06b9affdae32438e38a3145cd9e782ed0cddc38aa3d6fb65c4fc8d36e100fb10434e2fd4"
    "fd5567c15bbb37736dca9a82e92f0bf99d04d8f089cbcb4045c08f03c43b4fcebe83f1db"
    "46b59c118ca3a658a8796ca0a03a6d9ae21acfb8720347753fec7843edeadbffa181c72b"
    "a05ff89c79dcee6d79fbb92ddbd5a6a0f7e621eddea9ef03113d3a7c64e8eb71a9316664"
    "80f7ef0087d3a632a5b5b23c73e58ae0f355498fe5a0153f5a84723c6d1287c12c99b0f1"
    "b2c7a077586b68cc7a5f783943680be9ccd733e030d6e7b6a30e2bc6bc9a575b9c80ff3a"
    "b1327c0607583e64324595d2025b5c623d9dc491b70fd21c63dfddd6d1855c3127bd165f"
    "d59c3a2308db90933444dc309dd7be813f1ccd121560623fbdcc389f8c2728c1ad3ebeb8"
    "c7414af2840ae4e4be7d3c35958cc87919a5bbd785d9f2d601e598a48a7d0a2cdcb1bb96"
    "5e19c886db4e3613595a98d728aa46f2d380712063d3b8ce75b9837d772934a40cf03d78"
    "801040bbad824c3ca7ee9f8e5f89c3cfa84b11d7aeb2535ea73111d974ffc89e3114b5f4"
    "672741ea1ab1091a1f9efce07bc9dcc2437ac52a0161da17beb443a82da5c37a1d9de228"
    "c4843798125e68875e5297d6d3e78446fef14d25cbaeb6276484bb3a7be4d8e374e0073a"
    "b2306e4f160866be0699ac569a4995ce5a0acc8090243aa350c191b4bb266bf6797db25f"
    "f791182e1100149bde3a1506b31e04dcf69cc43404a34deffc1b0d475a65f2f58743f2d0"
    "076f564ce31ba9d94a073162f50491d102228a903684c3bb99e48e7bbe1f8140de5dbca1"
    "4b5fd0ced5d90e18912bebe83647cb054cf6fa8945c88b8315b08a56f436a93cd3ebe05a"
    "952586e0cc89e7a033c83b752f5a18bb3a23531ed0cd7fb5b76a40091f811749696f10f3"
    "a604311e32aea0e354c3310612b84081d1527525a5cc1aaa365ff22fac7afcc3f6242345"
    "4f5b25f0ca36bfa23592a98c4b407dd9a373fc4362f54dc4a1e104c701bd90f7848ea6f0"
    "dff675c4256d43bda17c7e257e9b703049cb30d2fcdf5366bfb5013466fa1e9533b213ac"
    "8d5a6523d8b6fb86a4de20bfe7880427260174a49c08b3339c5f50ac6bf541b907458ec8"
    "2f335bd25f17ef11757bc688cf20454b5d715878d170ced247ce0e574b9427cb74c2325b"
    "1c462d41dcc59a7e7707a88dc62c6e13d56e5faf9e57fe8ee62bc87ddd38bd76fb1bb673"
    "b92277f3c403d78fd4cd2604733c293f1c289f8bd93d0dc83faea300ca39eaf60f435f9b"
    "caec177360d18f735aa426dceb90eb97204f57c810f0e4cd8a51cc7969e7cc129e7e424e"
    "03d0174aa9d09466b0586f7b8198f1b6be528e3d80a1d6ded9db705df8811c6cf3e53376"
    "fc32825e1bec5a14f246917958f408f202ba3db6e731054864a6271e86ca7fc5ddbdc743"
    "66d14785e7bc764b5e3b81441a2fdf486c0265781ad3f4dc99870d3020c5275477a193a1"
    "5b00731eaa9c0dd256f246da362cc28262778f32b9af123ec7c65c161792cd2758e0266e"
    "6cf2671a33e47ef108100c1a0f79c6a58d473a793c39ab1b8ff4f37ad709508ec633baa9"
    "c11f9039cb7636350227b3ded09e9af62341089702931fc588245d9a77318f9d10132a8e"
    "d04ce2f53395ca3edb0741f651feac571ed3613d069bf8369514332735018e474c9aa94b"
    "39f2e3001e6d2c938287d04cdef36534d1dffb0e3705decd3599d48e5b7f80d54ecd891e"
    "6765f38d9ef5d075f4ad9898918cd947599916fdb50fe4a250a7acf436321aa90a2c0672"
    "56768940dc1fce52d7ae08b48c3b0b8e4a53ef65609838405120b2aea392305f2d26a8fc"
    "7890463af5284386726d81424964efb8f996f8f3a5eb31c36070cc825eab23a2839d4f27"
    "3127ebd018d043704e41886c1613154827a77ea00081194ef6ee92ec85ae8439e06908f5"
    "97c8e67b524b5106a23f3d66b7ec6e9f1caa2c502a9313de89c2c97484e3d070640b79d2"
    "edfac86802673498ff886ea45e6eed68e364d8d527d6757f7bad1d733d1cbd8360881d0d"
    "6212030482c4cf893585ff65e8009a1b014a9e9c684473d09e5238f062fac1c7";

TEST(inheritance_session_data_test, inheritance_setup_five) {
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;
  query.setup.plain_data_count = msg_count;

  for (int i = 0; i < msg_count; i++) {
    memcpy(query.setup.plain_data[i].message.bytes, messages[i], sizes[i]);
    query.setup.plain_data[i].message.size = sizes[i];
    query.setup.plain_data[i].is_private = is_privates[i];
  }

  inheritance_setup(&query, msgs, &response);

  uint8_t encrpyted_result[SESSION_PACKET_SIZE] = {0};
  hex_string_to_byte_array(encrypted_packet_five, 3232, encrpyted_result);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      encrpyted_result, response.setup.encrypted_data.packet.bytes, 1616);
}

TEST(inheritance_session_data_test, inheritance_recovery_five) {
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;

  uint8_t encrpyted_data_packet[SESSION_PACKET_SIZE];
  hex_string_to_byte_array(encrypted_packet_five, 3232, encrpyted_data_packet);

  query.recovery.has_encrypted_data = true;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(
      query.recovery.encrypted_data.packet.bytes, encrpyted_data_packet, 1616);
  query.recovery.encrypted_data.packet.size = 1616;

  inheritance_recovery(&query, msgs, &response);

  for (int i = 0; i < msg_count; i++) {
    TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[i],
                                 response.recovery.plain_data[i].message.bytes,
                                 response.recovery.plain_data[i].message.size);
    TEST_ASSERT_EQUAL_INT8(is_privates[i],
                           response.recovery.plain_data[i].is_private);
  }
}

static const char *encrypted_packet_max =
    "9c532ccd33a6a3fe3607f70d08603a3dfebb5c445c6f2117a831b81df53b465700b11b30"
    "479513cf6f97442dfd50f2bd885ef1734df9de2921ec2af32876bb6eb1eae41f0ad11a81"
    "de9573bc8948a7020d39b19fd79d88aafbb2a2db9a56d0500be7075e04e2ddf0bc31f75b"
    "b1431fb84091f590e32cec1ab0017bf420d7be6938a69205cf2bf0025ea0d35cfe2a2c03"
    "543d9e834be7fea1a8b0a540c74bc87c40d48e0abc8cae3f9e0652c7205ec64571f40325"
    "0ce3f6426cd6621ab83463f328485e4869a67d62dbdbb94c74ff916460433fbb1afafc26"
    "7fe5543c6d4c71e1587b44587b58885f53b95bf4a8578c6bbefbcc9e6a1fd8b3f4e983ce"
    "4f830aebe72037addbdf976b5950d65a77bba8c29a6429864fc94dc4521cfe39197aa7ca"
    "5b3155311d454f9fd46d7318bae99a7670d90dc3bca24ccd3d5bfef76e75a10c39acf560"
    "5059d8eccf0865f32980275362dc6f02eec832640b0f370e125e5d5292629c75f19ad616"
    "df47a0e536f8fc09d106890c01645c756e20161da2a29c361b21332254bf52f9cc999e52"
    "f159f43733c31e4043fb4a73e84dd49b0cb518f4490a673b470ead65979d0bbe6d7708e2"
    "4f3833e84e47e8cd6bde55092468b376e0b1de81bb7a2302913afcf8d007e6ea35efa96c"
    "65ea78ff0323f18892f82b981d410fd594d2aa91e81bdbf15b677655f7eed6ced5038e81"
    "0b870fa4fd1b9d83adfe5b04bb622287763805e32c0d2c52faca0dd1e7b3a4af08215c57"
    "0412d4d4962dafb928fe06816e576d89e3059f40c941198ca432ce43a209d5c5973b9ae4"
    "e80aa66f9687839fa979b23d42589b8f3cb35162939d9cbce972dc6badddd95220a0fb75"
    "bbbbdbb6773e8c58a8363150562eb2be1f77bbc17180b981f92c831ac702be5c70fa22d2"
    "96654ca3b2a53ac856c06ca1e52e805da7286ac9fa815170af58ad24182a18b204b2711b"
    "da139580d9126e797b13ebe235a05f8c157903bc62903583315a3871fe1b08c35f64a7fc"
    "300f40447ffb81f8159119a4f1b098f48e6d6b7e44995e3d9415af2b202b60dfe5365076"
    "e7bf8915500dcf0f16ec90bc6e9f8784bb0ca05322cd92bfc35a0646e02c1442a95177c5"
    "ec66a9b88e0c4e47a3b2215f674f10d90abad640ac38fa17bb82df8e7d37d7fba9562bfd"
    "9e6d5b3d70b76690dd6953650b7dd7dabb7219584d5bc9bf36e8dc47963723ed473b81c1"
    "567660c773b1c7ab3856b40d71ee6fe2dbc77e81690925e4d8f2c68668236ae504f620a0"
    "a596f61e5742b32a9b67a3e9254af3b3534aa52fdbeeca3b389aa47f0f779390d6cfb6ab"
    "82ab134b1e641632faa559926fb6e6d48913dbdfd97f4ce8f1430a8cb55d2bc8df318966"
    "ac7d0bb80a215d6794fd6846691d51aaf86d9f1f22331390d00295bbce873439f65e80a6"
    "22063a22a0aba23b6527a170f71dfc13f4f6c1efa0d3d4d0c3b5b6195e1ca53550f16cf1"
    "fd133edd326ae7a983a31b0513c9520ca8af1fb6835b837933aaddc23da8375473f2386b"
    "157a46ba6a51d2e6afafd65bea485ab42da7034e5b39c43c3380c321b75c3b2ecd7a6911"
    "e594483f9656c718e0db99fe72afb5719aaea8f51fddd9515fdd094efee7c410d941a470"
    "8adfee181e1f21ef9735080fa5afc13c2d5be920ddc51b3102a46fd573e1e99dcd712c81"
    "f5804ca8129fabb9b07a0fba8a9d7486b04fd4ea5a4cdbd85603ac1c4b23a1729ff8f7d6"
    "2518faaf7736f7a7ac7913619a13716e1105be17b2e14382e3b05934f933ff60e2c6cb46"
    "61af38dce080af5ec4dda233e14162ebf2dbf1cb160deedad887056a52d67fa24c92b677"
    "8a20ed9635a2a2cbbb04c8b46b085a4d0749c6a601d167553affe819d310c18da4f39887"
    "53476f706942bf46c22f16913248d1d40c863e3c83d05fcf09722f386dc4a6d0163fad88"
    "08e0cb52c5a6f28450d9e2b9b9cfaa02afb9ce9a4564b05ab30ba71b93ce0e11808826fc"
    "8167f570b791f9da2a183e719d7104c2cbd077a3f6f2efefe657878e03c20686d38cf014"
    "0b99de2e38c47fe07835412839b10be11dcb2cdeebe0815a8068ee29327d6d9c89e4d05a"
    "87d4d3e4b6f6c27e7030400c80c7b424449fc08cae6fde337fec5fe485f4500310158845"
    "bdab63ac47e09a155b2ec0efb10e1fa94dd4c07192defa546543caed9a0558ddc60d0510"
    "378397d10271f0af477d0550066ba15be617004fd88d5fc71868607aa813ad57565be15f"
    "0754c4a1418b8b3753fceef9c2519e3e53d5b6688396db8dc7a4a62ad986020a78f65a1c"
    "456f049407cb94ebc023269af64f37b57ff9f58f40a8993dbf9db2a4c232c6bb8a5c0379"
    "0b8f0d8dd9878bb1b40343b1b365b18b2de44edec1e8d905749db98247126cd27b4f1eac"
    "e71c66cf4a9db4808f494f27b0cae81c3557886b7d5563dd87456a7744343235048b6944"
    "dac7c71f1e108743a16aa98fad3f93161bc70b1d76046c56c861e1f222a17591d1c65590"
    "2afff5e19ceb12972e5dd0b4d51896ea6c277ccad1b23c7906af71bab97b09521c835afa"
    "57b2e510ec52475409426b245efdaf5146acbff62757ecd507b2b17188214e0e4cfa8e0e"
    "0296cd45ccc1cf1e13cdf510e48b45b9c131c5f772aa57e9748ae2737ff1a605e43d745b"
    "76e4ac6d0cb898bd7fc275da2a2d494e8e48d2f97ea164c8b19531aecb4536ff4c860cec"
    "26ab96ea2266c3762877f742056f8de809826881a27766fe4439be686c9ce736ec58254e"
    "4a1043de92a8b9eb302d66e70b8347b469f20e6623668d77066da4602debea63cd842c29"
    "add429bf9382c053721e7ac8d52bd98cf2df3a0c5fdfacc951cd4ce873365247f8bc0500"
    "fe3da52ca6f0606c90f0ac7818e6f2a00606e791ed80643596745cf556fb1ac5239779d9"
    "09c25675defda2dfecd9360c87db22a40da24cb3248cb7694d37175fa8da5bf097ee9d8b"
    "e5503c60e0e2f6a8e23785882b1d49edee51cbeb5e9f26243d27c62f87a2a6cbb7f879fb"
    "34f0bf92f8a5e372e4f3442d12a4c30dc98591c4c7a6aabce5770433446c142e474cc818"
    "749ed7f3eeb1d043bed176acfb443b5ca301ad9cc1053854b9dd81ac6e8a01a1ecccc4dd"
    "8edf1a289ac125215b2a811da27005874abdd8d2188f4b2b592b6f84f2b866ea45518c6f"
    "cbb2d3836c19085ba4e9e1c9c9cf43a7c58e4ba1bce26b78fefa9d247e2bec65aff64883"
    "573b1de0e49345104c2f6fc667a0090eb3b27b201025d5f774347b38e546a92e8ec85e06"
    "3501112e0a1976e1f5c23135f7fde9c66517b61d579053cc7dbbb2a234b0f98f2216de20"
    "40f350623b98019b3c048b896caf3c2b43e36323b4c4c9388dedba0c79eb4a30cf039681"
    "1c27caf9f6fed307968f757340825e68752498e8ceacbfa48fe03b20553fe6ae66223742"
    "eba2f5911fc473a124861baadfd583ac98463192d8d9845ac48324c2baf21006a7aea81a"
    "ba6d5b2babd5c3cc53393e1b21d4dc3d7ff9d9256a5f1184cf37535717d613bcb5de1a82"
    "3a19b2f632eb4904e8bd455c36235a910dcc633507f89d958b89268342bcd8006c3c2626"
    "2ae3c87309ab0f163509762b02b6bce9eca67f6a35b63920d5eb3849a8af0bc21bca4aa5"
    "dd50c34acbce4bb62021ebab72697985af9d524289b2c92062e8190720ef2727e9e91440"
    "60267f386c62b02514cc7caa79d8dff493d8dd6aa835db330dab59a397fbe897f8bcd5a4"
    "1a6ec56af253845b0dba2a582e5e0409e8c9c3e6e7e39593abe860dca8e800bacd626133"
    "1749d7d866afd83cedcb15fa9aa5024bf520a4958176e0fa8c5e5b90ae140e32fc4d993c"
    "ab3cdcf3c17bbafe981f4bfc532085c705a635b36cbc931ced67d3060f196aac10ef4d29"
    "d0ec6f65520ab09c3d6a26c217bc147b5fde2f6871e40a96dc6ad9c97ad91794266cfc84"
    "d3bcbd6078f3a879985aff06382649cbb030221a547dae61546c9f369d3025260d187668"
    "03c10b1f51b7c2f170fb5a8b015f46391e40d80b29a4f470431defe91bf207b788533046"
    "3a9d2da486f789bdc10ed2ff99aeaf1c5c14bc107fd186b539277b2141c51a21f0e55e67"
    "473f498ee79f885e0f1eb5803c04521d5a6465ef63b6a8d6cb7ea6fb8ce8f471dacd99b0"
    "8b95f3af5640b7870a127a72b1fd5ca23f213a3d85d63b8e4d29b77476775774ce483adf"
    "95dcd294c51400d421ef2e47577e9b06c2df740f32ae3e1a2d3346c42812c12191ef7ea4"
    "7f85d822878df323734025f2dafa79cc45c3d925f33edac0ea8078c8674f48cd38349e41"
    "d10fdb2a4632d3f0c0f27f6146615f6f91f84801d8bf9d4d51951e7809b01128d8c31720"
    "b6d981866ea86ab91c5f319e0b8d650b12b6d68b6fd38e72e14b59f9b24a533ed394be30"
    "2dacd61e84973451494d9ae0d01ac863b2280619c20205027566f9ce9835a17afa8e8282"
    "38aee12337a8820ab2e87cbc0bb18d22e1e84d70e24ac65dd17f0551e7c23ee635f97e05"
    "5e907ee4806f9c3b4746881fa54ac70f977aed8aaa71c6a1f4bc1fce35575dc5fa8aea78"
    "2ebea3607f0b782717f50e60c67a39e6197f8d6ded77777ddbd5f2ac456cb514f24a25a0"
    "39c6f90fd9e44d2139145a337a1f8e78662f65ef4c2238aeb52e0ba09aaebc7e7c78b779"
    "50d6d4726dbeff7c8753199047c67519d1f19206e49eaae35a04528f449eb6afdc359500"
    "48ac0ea3f1b20d4e3acf5d241e9029a6d669f36b8e019349d347ad0b2cd549268111a288"
    "be38fd69c0b35da20881fbc507cbbc3a75140f7195e703018f2f118133f8a8c3270d4b3d"
    "aeb17412d96c67ce5b13815a1f02b89cabf9ab9fa5a9989197fd8538dcce0567a1eacfec"
    "ef26720b5b4439fa08e54d24a42ad42dd332f62da8af89d1a6251e5a5bd946874e424099"
    "6d00014e04c2eba76c3db1f3c17ef81e5d69e1746ff8a1cbb549cf55fe12db9b18683a4a"
    "cc657474ec11a38e0a483aecaf509360a010e4eadb2779ba461d20c0d015d978987598d7"
    "301dfce8f689403a73ea4d8361373e14c66405e8dfd01437671c59addb897408a3ad960a"
    "1aab2142163f88998d29fbcdb6f93719308c0a7b8a5763ad98b98a7ba1edb90a52788105"
    "3a8b1557cc29fd7037529d419cc2de7c6ea35b4621c62e0ed5ba3f38dcdf00377f5692e5"
    "0c307720068a042d3aad4d8beb3959a77095c62e647aebb246207c6198fe6149207327c4"
    "11ac0da14db035e09e5fc6acfd95840fec2e4bc4eaf9c6989e6b6f3b44f26cffb75ae326"
    "91a46a211579b08169e9a3e6337f7e9f7153e4957fa87a30f5c0c8636af8a56b3b0eb2c2"
    "f5376f54380ae8e73f666e6d3959f33fa87ab3719ca15be07d82e2c0dcd706646b3f9031"
    "9103d30661cfac24dc12aa99373b2164e82a77ce46698590baa4e65c623feb0bb222ccff"
    "be87e6010096d917087599332d791ea92ab8de055b3da9892455152e978db909bdb2a34d"
    "ca5726c7ff324cf96f524a22e88f6066713d272b645cf312ac6f0b7519f7ceec2274fe76"
    "960951eea4cd1098fef9a4a55af87192f9c12a3bdf4720c5eb2b8d5d5d30dc884f06c3d7"
    "3af1ea60b0fac0eca0f09f644d157a423efb24dd9557ff9959e02fcb3e4a9f9c7657339a"
    "648973044faf16a3215af989722e62d9fba1c83ab4900f87004b8d934f3607ab7f2eb14c"
    "05c1ffe12736e8deb29825904934b06b39f29445781b7c6a6be39d10b5789600939cd3db"
    "d38b1aa3925e0607d94c0aa4af5515106cd8452d447e0cbc85dd7a4802d36fc46cc36f71"
    "bbb45f415c47183a5597d50c1a2b2a7cd867209959e38e97d6b6aa7e95a7b8d40a1845c1"
    "a6682343cd955e9663da6008484e780440d0ed1e7b54061ad00232bc62e1790a199bd278"
    "557b88f515dfeb7e7c8b3295d8aeca45f97331653c47e1075c213464aac66bcea349c909"
    "fa2ef93f3d08298fdd2aee1e11a7c8a669274a3c2ce03f198cbcc21fe940551e4a06b38d"
    "2d5a6dfb868602863ffc826e816871d5115a20c9605487f16bb89662be8bfb095f1ce4d3"
    "2feaa031676bad57cbf49ff97c844f9c8d57c4051f823fca353f988946313f1fe7e4cab8"
    "9eb0eeb4ade07090c2d831ed34e3c58b2dbfeb66ece4a89e5faf3dab4265152cc0fb41a7"
    "f9b4705c57b84df1e4f2c5fc3eeba6c985cb8b40fc0214e9b67a1ad5f055b91427c8e060"
    "71a987042745a4dcdcbae30d5b09c04bdc651e48645557720d4632a1925d6486b4aef8a6"
    "b2f10e97c6f90160d33c2bc9b6907756d4f30110ab0db5dd7d7ebaaa550f86eeef2c2995"
    "6d0e71213954a4004e38a2d2ba0c998ad4993906446e5d1dc8a4d60b4448b9e8724dc3ab"
    "e607f273aff58aeac7e2ac5d72cbde79c919771f11ff9e8f56ef3703c2f33b0f2a067cd9"
    "689e621599682063ebf4ec416c806d8571f4bdfaed9e6cc152af527798fa2bac1b6f4094"
    "fd6799d8bb5d20e1fd856078b3963c7dbaf301badf1603a352c79f26d0e76b77ae45ef1c"
    "976b6fe66cd92a28ba28dfcc2fe44716ebffe25d9fa68e832600b9d96e174a750fe12933"
    "8ec4fcb6f30960e692458c2ba85bdb26";

TEST(inheritance_session_data_test, inheritance_setup_max) {
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;
  query.setup.plain_data_count = msg_count;

  int8_t n = 4;
  for (int i = 0; i < msg_count; i++) {
    memcpy(query.setup.plain_data[i].message.bytes, messages[n], sizes[n]);
    query.setup.plain_data[i].message.size = sizes[n];
    query.setup.plain_data[i].is_private = is_privates[n];
  }

  inheritance_setup(&query, msgs, &response);

  uint8_t encrpyted_result[SESSION_PACKET_SIZE] = {0};
  hex_string_to_byte_array(encrypted_packet_max, 9348, encrpyted_result);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      encrpyted_result, response.setup.encrypted_data.packet.bytes, 4624);
}

TEST(inheritance_session_data_test, inheritance_recovery_max) {
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;

  uint8_t encrpyted_data_packet[SESSION_PACKET_SIZE];
  hex_string_to_byte_array(encrypted_packet_max, 9348, encrpyted_data_packet);

  query.recovery.has_encrypted_data = true;
  query.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(
      query.recovery.encrypted_data.packet.bytes, encrpyted_data_packet, 4624);
  query.recovery.encrypted_data.packet.size = 4624;

  inheritance_recovery(&query, msgs, &response);

  int8_t n = 4;
  for (int i = 0; i < msg_count; i++) {
    TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[n],
                                 response.recovery.plain_data[i].message.bytes,
                                 response.recovery.plain_data[i].message.size);
    TEST_ASSERT_EQUAL_INT8(is_privates[n],
                           response.recovery.plain_data[i].is_private);
  }
}