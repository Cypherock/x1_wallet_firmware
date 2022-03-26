/**
 * @file    apdu.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
/**
 * @file
 * Functions to encode raw data in APDU. The APDU
 * can then be sent to card.
 * 
 * For the format of APDU refer Shield X document.
 * 
 * @note These functions are not called directly but indirectly from nfc.c
 */

#ifndef APDU_H
#define APDU_H

#include "wallet.h"
#include "aes.h"
#include <stdint.h>
#include <string.h>

#define SHA256_SIZE 32
#define POW_RAND_NUMBER_SIZE 32
#define POW_NONCE_SIZE 32

/// enum defined with expected lengths for different APDUs
typedef enum {
    ADD_WALLET_EXPECTED_LENGTH = 2,
    RETRIEVE_WALLET_EXPECTED_LENGTH = 152,
    DELETE_WALLET_EXPECTED_LENGTH = 2,
    ECDSA_EXPECTED_LENGTH = 68
} Apdu_expected_length;

/// enum defined with command type for different APDUs
typedef enum {
    //Template - APDU_FUNCTION = INS_CODE
    APDU_PAIR = 0x12,
    APDU_UNPAIR = 0x13,
    APDU_EST_SESSION = 0x14,
    APDU_ADD_WALLET = 0xC1,
    APDU_RETRIEVE_WALLET = 0xC2,
    APDU_DELETE_WALLET = 0xC3,
    APDU_LIST_ALL_WALLET = 0xC4,
    APDU_SIGN_DATA_ECDSA = 0xC6,
    APDU_INHERITANCE = 0xC9,
    APDU_PROOF_OF_WORK = 0xCB
} apdu_command_type;

/// enum for storing command for calling certian function in APDUs
typedef enum {
    // Template - P1_FUNCTION_SUBFUNCTION = P1_CODE
    P1_POW_GET_CHALLENGE = 0x00,
    P1_POW_VERIFY_CHALLENGE = 0x01,

    P1_INHERITANCE_DECRYPT_DATA = 0x00,
    P1_INHERITANCE_ENCRYPT_DATA = 0x01,
} p1_function_subfunction;

/// enum for tag values in APDUs
typedef enum {
    //Tag for Wallet Structure
    INS_NAME = 0xE0,
    INS_PASSWORD,
    INS_xCor,
    INS_NO_OF_MNEMONICS,
    INS_TOTAL_NO_OF_SHARE,
    INS_WALLET_SHARE,
    INS_STRUCTURE_CHECKSUM,
    INS_MIN_NO_OF_SHARES,
    INS_WALLET_INFO,
    INS_KEY, // Key for storing xPUB
    INS_BENEFICIARY_KEY,
    INS_IV_FOR_BENEFICIARY_KEY,
    INS_WALLET_ID,
    INS_ARBITRARY_DATA = 0xA0,
    INS_IS_ARBITRARY_DATA = 0xA1,

    // Tag for Select Applet Command
    TAG_VERSION = 0xB0,
    TAG_FAMILY_ID,
    TAG_CARD_NUMBER,
    TAG_CARD_KEYID,
    TAG_CARD_IV,

    TAG_SIGNED_DATA = 0xEB,

    // Tag for proof of work
    TAG_POW_RANDOM_NUM = 0xD1,
    TAG_POW_TARGET = 0xD2,
    TAG_POW_NONCE = 0xD3,

    // Tag for inheritance
    TAG_INHERITANCE_PLAIN_DATA = 0xD5,
    TAG_INHERITANCE_ENCRYPTED_DATA = 0xD6
} Tag_value;

/// ISO7816 values
typedef enum {
    CLA_ISO7816 = 0x00,
    INS_EXTERNAL_AUTHENTICATE = 0x82,
    INS_SELECT = 0xA4,
    OFFSET_CDATA = 5,
    OFFSET_CLA = 0,
    OFFSET_EXT_CDATA = 7,
    OFFSET_INS = 1,
    OFFSET_LC = 4,
    OFFSET_P1 = 2,
    OFFSET_P2 = 3,
    SW_INCOMPATIBLE_APPLET = 0x1000,
    SW_NO_ERROR = 0x9000,
    SW_FILE_INVALID = 0x6983,
    SW_RECORD_NOT_FOUND = 0x6A83,
    SW_CORRECT_LENGTH_00 = 0x6C00,
    SW_FILE_FULL = 0x6A84,
    SW_WRONG_DATA = 0x6A80,
    SW_NULL_POINTER_EXCEPTION = 0x6281,
    SW_OUT_OF_BOUNDARY = 0x91BE,
    SW_TRANSACTION_EXCEPTION = 0x6900,
    SW_CRYPTO_EXCEPTION = 0x7C00,
    SW_CONDITIONS_NOT_SATISFIED = 0x6985,
    SW_SECURITY_CONDITIONS_NOT_SATISFIED = 0x6982,
    SW_NOT_PAIRED = 0x7985,
    SW_WARNING_STATE_UNCHANGED = 0x6200,
    SW_FILE_NOT_FOUND = 0x6A82,
    SW_INVALID_INS = 0x6D00,
    POW_SW_WALLET_LOCKED = 0x7D00,
    POW_SW_CHALLENGE_FAILED = 0x6A88
} ISO7816;

/**
 * @brief Fills the APDU for the passed Tag value.
 * 
 * @details
 * Fills array in this format:
 * [TAG][Length][Value] where
 * Tag : Unique Identifier.
 * Length : Length of Value. (skipped if length of value = 1).
 * Value : The data to be sent.
 * Example for Name - [0xE0][16][Data from wallet->wallet_name].
 * 
 * @param [out] array               Byte array of apdu to store data.
 * @param [in, out] starting_index  Starting index or offset of apdu.
 * @param [in] tag                  Tag value
 * @param [in] length               Length of data to filled.
 * @param [in] wallet               Pointer to Wallet instance to retreive data.
 *
 * @return No return
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void fill_tlv(uint8_t* array, uint16_t* starting_index, Tag_value tag, uint8_t length, const struct Wallet* wallet);

/**
 * @brief Create a apdu for add wallet.
 * @details
 *
 * @param [in] wallet   Pointer to Wallet instance.
 * @param [in] apdu     Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_add_wallet(const struct Wallet* wallet, uint8_t apdu[]);

/**
 * @brief
 * @details
 *
 * @param [in] data
 * @param [in] length
 * @param [out] apdu
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_pair(const uint8_t *data, uint16_t length, uint8_t *apdu);

/**
 * @brief Create a apdu for add arbitrary data.
 * @details
 *
 * @param [in] wallet    Pointer to Wallet instance.
 * @param [in] apdu      Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_add_arbitrary_data(const struct Wallet* wallet, uint8_t apdu[]);

/**
 * @brief Create a apdu for retrieve wallet.
 * @details
 *
 * @param wallet    Pointer to Wallet instance.
 * @param apdu      Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_retrieve_wallet(const struct Wallet* wallet, uint8_t apdu[]);

/**
 * @brief Create a apdu for delete wallet.
 * @details
 *
 * @param wallet Pointer to Wallet instance.
 * @param apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_delete_wallet(const struct Wallet* wallet, uint8_t apdu[]);

/**
 * @brief Create a apdu for list all wallets.
 * @details
 *
 * @param apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint8_t create_apdu_list_wallet(uint8_t apdu[]);

/**
 * @brief Create a apdu for select applet.
 * @details
 *
 * @param apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint8_t create_apdu_select_applet(uint8_t apdu[]);

/**
 * @brief Create a apdu for ecdsa.
 * @details
 *
 * @param data Byte array of signature.
 * @param length Length of signature.
 * @param apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_ecdsa(const uint8_t data[], uint16_t length, uint8_t apdu[]);

/**
 * @brief Create a apdu for verify challenge.
 * @details
 * 
 * @param [in] name Byte array of name of wallet.
 * @param [in] nonce Byte array of nonce value.
 * @param [in] password Byte array of password.
 * @param [out] apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_verify_challenge(const uint8_t name[NAME_SIZE], const uint8_t nonce[POW_NONCE_SIZE], const uint8_t password[BLOCK_SIZE], uint8_t apdu[]);

/**
 * @brief Create a apdu for get challenge.
 * @details
 * 
 * @param [in] name Byte array of name of wallet.
 * @param [out] apdu Byte array to store apdu.
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_get_challenge(const uint8_t name[NAME_SIZE], uint8_t apdu[]);

/**
 * @brief Create a apdu for inheritance.
 * 
 * operation = P1_INHERITANCE_DECRYPT_DATA if data needs to decrypted
 * else operation = P1_INHERITANCE_ENCRYPT_DATA if data needs to be encrypted
 */

/**
 * @brief Create a apdu inheritance object
 * @details
 * 
 * @param [in] name Byte array for name of wallet.
 * @param [in] data Byte array for data to perform operation on.
 * @param [in] data_size Length of data byte array.
 * @param [out] apdu Byte array to store apdu.
 * @param [in] operation operation = P1_INHERITANCE_DECRYPT_DATA if data needs to decrypted
 *                  else operation = P1_INHERITANCE_ENCRYPT_DATA if data needs to be encrypted
 *
 * @return Length of stored bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint16_t create_apdu_inheritance(const uint8_t name[NAME_SIZE], const uint8_t* data, uint16_t data_size, uint8_t apdu[], uint8_t operation);

/**
 * @brief Deserialize challenge from the response apdu
 * @details
 * 
 * @param target Byte array to store target.
 * @param random_number Byte array to store random number
 * @param apdu Byte array to store apdu.
 * @param len Length of received apdu.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void extract_apdu_get_challenge(uint8_t target[SHA256_SIZE], uint8_t random_number[POW_RAND_NUMBER_SIZE], const uint8_t apdu[], uint16_t len);

/**
 * @brief Deserialize wallet info from the response apdu
 * @details
 *
 * @param[in,out]   wallet  Wallet structure
 * @param[in]  apdu Received APDU
 * @param[in]  len Length of received APDU
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void extract_from_apdu(struct Wallet* wallet, const uint8_t apdu[], uint16_t len);

/**
 * @brief Deserialize family-id and applet version of the card
 * @details
 *
 * @param[in]   apdu  Response apdu from the card
 * @param[in]   len   Response apdu length
 * @param[out]  family_id Family ID of the card
 * @param[out]  card_number Member number of the family-id
 *
 * @returns     ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 extract_card_detail_from_apdu(const uint8_t apdu[], uint8_t len, uint8_t family_id[], uint8_t *version, uint8_t* card_number, uint8_t *card_key_id);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void init_session_keys(const uint8_t enc_key[32], const uint8_t mac_key[32], const uint8_t iv[16]);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int apdu_encrypt_data(uint8_t *InOut_data, uint16_t *len);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int apdu_decrypt_data(uint8_t *InOut_data, uint8_t *len);

#endif
