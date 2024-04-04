/**
 * @file    proto_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for protobuf encoding/decoding
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

#include <pb_decode.h>
#include <pb_encode.h>

#include "btc/core.pb.h"
#include "lv_symbol_def.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(proto_tests);

TEST_SETUP(proto_tests) {
  return;
}

TEST_TEAR_DOWN(proto_tests) {
  return;
}

#define RAW_TXN_2151                                                           \
  "0200000000010ea1c29326173d6f23cfe5bdf5bebec7d5811e54063a4ccfece6966b6c7b8e" \
  "3008020000000000000000f089e773b0fb10d573e45cb2c2cac571b89c52582c7aabee8ba4" \
  "8fd31cc9e494040000000000000000996eec770b9621db3b94a40ced893df7cc3f1e281ff5" \
  "035cdcc6c2cc4ce0d5b7010000000000000000b3bbc22b24711efdd433d030d72ab7811fb9" \
  "ed6f70f2eade78b251518ccb9fe201000000000000000040d5f44017b394b339020878506c" \
  "11f572a4bf04964db350e842629665b173ab0100000000000000008d5fc17750698fd735ce" \
  "b9b11c110c01c84bb007d177b0c22bb966d2ae2acb71010000000000000000a2b8034fc47f" \
  "68ced20d811296eaf276a98b5b3f712d5d817caf5a8294d4cb2c01000000000000000076b0" \
  "1370051d5c7c7fa44697c1bd253bf80272b73e5f1c013cbe4cacbd39036101000000000000" \
  "000077507cef4e74e079a879ee71b95302fcb026b4a2fda8e409a4291e9d8f9e45c3010000" \
  "000000000000dd910002effdd9e14307de7a5853552ea0d7117ffacb09bca1c79f90ec16c7" \
  "be0300000000000000000029d9ab66735067e04ea0924fb579a3bd616e805ca1b23a09cc93" \
  "66be893dc811000000000000000014963acc7f2be843c2ce0f2204279a0915d48d163fac0c" \
  "dd104aa2107a6f48b006000000000000000079c3c7a88dfc373a4b12337eac19b982f07125" \
  "6f86dc540ba8266e4cb7c6bc930400000000000000005b63e7612672814a1a4248595ccf3c" \
  "8574cfc09840fccdcd0cd2a7eec6b7329411000000000000000002c05be80a000000001600" \
  "14dc3c8c6ea099c8495f02498c19387d2dcb2d392450c68b0000000000160014575fa23ed6" \
  "8490fbde2db8d992bf17d2037304f1024730440220729a6431d66d67d8ea948329afad3d7d" \
  "80a4e2cf2318f0d30df523a1756e6cf602204d2cecedf625eae44973cc466e600d72543035" \
  "6c035e83995fa87e16231ebfee012102db4732cf7aed49291087d732ac27a17a944553d2ea" \
  "1c10d585dfafe7082b8695024730440220758f988aa685ca3fe2c75f4290490bcf48e4162c" \
  "ff6691d438f976f1285a8402022013ea33949393660b6986219e08365b1311ccf1bf36052c" \
  "6795265e4a74e2dce10121027831e6490b3c1b43f0af50f38a18f552310540d53f3aaf21d9" \
  "66c29d2cfcffb00248304502210080b7b41aad72d29b56f7687445d6166cad4fe93ff83822" \
  "aa4568cff727c76c1002204c26756076b6342e181c7acb611772b3855257deb93ca776459e" \
  "2d1461a2885a012102c3982ffa13f472750f6456a44ddd4fe62d552ff4915a667639e141a7" \
  "78d4537d02483045022100d2e067dd8eec381d64b7e292a602af0366866c84fa79d9d46a36" \
  "b75d84d068ba02205e18ea1f98a80b3ee52d7d9c43057a70f61f6b27c1621d79f9618f7415" \
  "71799c012103e834acca57af858d85c97f0dbf10ed669bb65ef6b1c2bc46b144f638860bcb" \
  "7902473044022032a6aaa16254690df9b9e1da73399cddb9f89dc8fbaa7bbeb0d18e74e3db" \
  "2f3202207851d30a4e97887638ae387e2e928d43d8e1db3f39eac95608256d79e95459d901" \
  "2102883c759a11fa8b1f9cf8c02681bff0b7ef4d3e241ceca605f454268e34a6cc97024830" \
  "45022100f61f09429ca43603619fae00377c13dfc9659523a215b43f59247961cd55297b02" \
  "205fbdf95ab78ab87ad5464a6ed8e8cc48bdbf65a647e13b1302c486df845bf00901210278" \
  "d189c1bdb0a7f57e6cf818839ac4c786867da0ae151909a03dabf3ad91d18f024730440220" \
  "241c6eb43e924105bc749c2d557f2c002ee23305951f053ae459ff551fc98c920220044623" \
  "827b2a02c9bdeeeb61432148d9f9235965858af2cf7b765ae58758e212012103190f2d8025" \
  "43b3fa2442c72e79b52648e10a035762ceea0b209ee155debd6bd50247304402200a7fb9c8" \
  "5be747459ab0e4cde68c7dfa184c482da32cf941a206991c83af3e1802200ae6654f049642" \
  "d61c5551eae73969e9a203b79b0017ab79d902e607dd7f24e6012103c3c4d2b5f4717ba251" \
  "af40b3b6a104ce142bb6d2d6460c7359332920fcbb022a0248304502210084d2acc986164d" \
  "eca62211d1d8026d17d16efb5065afdb3f3cf558faaf38291c022001ca645fe6b6ca9df610" \
  "94d92e3cf0f6aa15ae5e43d8bcc91cf2998807ac93f5012103b0018835890134e9599fdc60" \
  "36adde89f54e1cd4ec7ae83e42332be9b674000802473044022079089d7e24eb88b3927929" \
  "d215873074c0d6f5942c07363576ce498939f849890220317d9f5065dc5f622bf90a580763" \
  "0eee97ff11934ff7d09e6829646fbd64ad0e01210261823a28f79e535a2795a07692807a36" \
  "65ce5b0f8e382c4e74f1a0f036e3491e0247304402206baeb52c4496bf966ff7e3860a0b14" \
  "f2e30a51ab8407a0c3ff752fffeb0d77b402203785edf042463fee2defafe92a8f148e2398" \
  "013cd3d9e8a286989e4f0a8f39ed01210386d2a0e862e738f9575de9bb69d6018a92505db0" \
  "f85f510e48c7f1dbaac4ed48024730440220275673b74ce66db92f41319c66a176167ffe82" \
  "17d3cb5d0ee730c62c2e79761a02207880d609c8367bfc2bb2cbc0541f999d209c38886df9" \
  "f7638a99e0fd2f99c784012102b08043fc4e72200a6b830b472d0c51ce661831a9adcf2ca4" \
  "45689bb16292083902483045022100ad3fef55de873ad38712479e7aacb01cc90502d58302" \
  "f458aa54cf39c855cfc002202e63138aced37ee40ad8ac16b446d97ca288e801d27e27abb9" \
  "fe63d6ac96db08012103dc136c26a61133e3afbef092dae10e02b3641d018f60affbaec8f7" \
  "54db18cb500247304402207a46d401a36344fc528ac85b969214468c43d7517cd997dddea9" \
  "3574cf350e6c022034cd8d7c3e343da803dd20c57a5552c31e2bc6f207399d9f3053da7885" \
  "40b15d0121025665dfb2c299ec50669987dfe4ea29c8e4188c5365bc40f6860df2de07242c" \
  "9900000000"

TEST(proto_tests, decoding_out_of_bounds) {
  const uint8_t encoded_msg[3072] = {0};
  const size_t msg_size = 4462 / 2;
  btc_query_t expected_query = {
      .which_request = BTC_QUERY_SIGN_TXN_TAG,
      .sign_txn =
          {
              .which_request = BTC_SIGN_TXN_REQUEST_INPUT_TAG,
              .input =
                  {
                      .address_index = 0,
                      .change_index = 0,
                      .prev_output_index = 0,
                      .prev_txn = {.bytes = {0}, .size = 2151},
                      .prev_txn_hash = {0},
                      .script_pub_key = {.bytes = {0}, .size = 22},
                      .sequence = 0,
                      .value = 183000000,
                  },
          },
  };
  btc_query_t query_out = BTC_QUERY_INIT_ZERO;

  hex_string_to_byte_array(
      "74956a32a9401b823e024979db2b4ef3f0a5917df664792a18c1153c781ff8d5",
      64,
      expected_query.sign_txn.input.prev_txn_hash);
  cy_reverse_byte_array(expected_query.sign_txn.input.prev_txn_hash, 32);

  hex_string_to_byte_array("0014dc3c8c6ea099c8495f02498c19387d2dcb2d3924",
                           44,
                           expected_query.sign_txn.input.script_pub_key.bytes);
  hex_string_to_byte_array(
      RAW_TXN_2151, 4302, expected_query.sign_txn.input.prev_txn.bytes);

  hex_string_to_byte_array(
      "1ab4111ab1110ae710" RAW_TXN_2151
      "1220d5f81f783c15c1182a7964f67d91a5f0f34e2bdb7949023e821b40a9326a957420c0"
      "b7a1572a160014dc3c8c6ea099c8495f02498c19387d2dcb2d392430ffffffff0f4001",
      msg_size * 2,
      encoded_msg);

  /* Create a stream that reads from the buffer. */
  pb_istream_t stream = pb_istream_from_buffer(encoded_msg, msg_size);
  /* Now we are ready to decode the message. */
  bool status = pb_decode(&stream, BTC_QUERY_FIELDS, &query_out);
  TEST_ASSERT_FALSE(status);
}
