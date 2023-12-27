/**
 * @file    nfc.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
/**
 * @file
 * Interface between PN532 NFC reader and Application Layer
 * Controller module has the main use for these functions.
 *
 * Before using any function two steps need to be followed
 * 1. nfc_select_card()
 * 2. nfc_select_applet()
 *
 * This file uses apdu.c for creating raw APDU's
 *
 * ISO7816 are return codes defined by the card. @ref apdu.h
 * The last two bytes of APDU are the returns code / status word.
 * The status word is extracted by the nfc functions and returned separately.
 *
 * @note The cards belonging to the same set have the same family ID
 */

#ifndef NFC_H
#define NFC_H

#define NFC_COMM_LINE_STATUS_BIT 0
#define NFC_CARD_PRESENCE_BIT 1
#define NFC_ANTENNA_STATUS_BIT 2

#define NFC_ANTENNA_CURRENT_TH                                                 \
  0x28    ///< Defines the preferred threshold range (25-105 mA) of current in
          ///< antenna. @see adafruit_diagnose_self_antenna

#include <stdbool.h>
#include <stdint.h>

#include "adafruit_pn532.h"
#include "apdu.h"
#include "board.h"
#include "controller_main.h"
#include "flash_if.h"
#include "memzero.h"
#include "wallet.h"
#include "wallet_list.h"

#define SHA256_SIZE 32
#define ECDSA_SIGNATURE_SIZE 64
#define SHA256_SIZE 32
#define POW_RAND_NUMBER_SIZE 32
#define POW_NONCE_SIZE 32

#define DEFAULT_NFC_TG_INIT_TIME 25

/**
 * @brief Initialize PN532 module
 * @details
 *
 * @param
 *
 * @returns ret_code_t STM_ERROR_CODE
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ret_code_t nfc_init();

/**
 * @brief Diagnose nfc antenna and PN532 module
 */
uint32_t nfc_diagnose_antenna_hw();

/**
 * @brief Diagnose if card present in feild or not
 *
 * @return uint32_t 0 for card presence detected
 */
uint32_t nfc_diagnose_card_presence();

/**
 * @brief Wait for card being deselected
 */
void nfc_deselect_card();

/**
 * @brief Wait for card being removed
 */
void nfc_detect_card_removal();

/**
 * @brief Wait for card indefinitely
 * @details
 *
 * @param
 *
 * @returns ret_code_t STM_ERROR_CODE
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ret_code_t nfc_select_card();

/**
 * @brief Wait for card for the given amount of time
 * @details
 *
 * @param wait_time Max time to wait for card
 *
 * @returns ret_code_t STM_ERROR_CODE
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ret_code_t nfc_wait_for_card(const uint16_t wait_time);

/**
 * @brief Operation to pair card
 * @details
 * Card pairing is performed via mutual authentication b/w device and a card,
 * device initiates pairing by sending pairing data including signature and card
 * returns its pairing data, nfc_pair is used to exchange pairing data,
 * data_inOut and length_inOut should contain pairing data and its size, pairing
 * data returned by card is copied to the buffer if pairing was successful on
 * card
 *
 * @param [in/out] data_inOut pointer to buffer with input and output data
 * @param [in,out] length_inOut pointer to variable with length of send and
 * receive data
 *
 * @return ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_pair(uint8_t *data_inOut, uint8_t *length_inOut);

/**
 * @brief Operation to unpair card
 * @details Unpair is performed by deleting the shared secret on card
 *
 * @param
 *
 * @return ISO7816 Status Word
 * @retval
 *
 * @see nfc_pair
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_unpair();

/**
 * @brief List all wallet saved in card
 * @details
 *
 * @param wallet_list Reference to buffer which will be populated by the NFC
 * operation
 *
 * @returns ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_list_all_wallet(wallet_list_t *wallet_list);

/**
 * @brief Select Applet
 * @details
 * If uint8_t id[FAMILY_ID_SIZE] = {DEFAULT_VALUE_IN_FLASH} is provided
 * then family id of tapped card is not checked. In this case the returned id
 * will contain the family id of the card tapped. This can be saved in flash.
 *
 * If the ith card is tapped then (i-1)th bit is unset in acceptable_cards.
 *
 * For example if acceptable_cards is (1111)b which means accept any card
 * and the 1st card is tapped then the returned value will be (1110)b
 *
 * @param[in,out]   expected_family_id  Any card with family ID other than the
 * provided ID will not be accepted
 * @param[in,out]   acceptable_cards    If the ith card is tapped then it is
 * accepted iff (i-1)th bit is set in acceptable_cards
 * @param[out]      version             Data byte array for storing getting
 * applet version
 * @param[out]      card_key_id         Data byte array for storing output key
 * id of card
 * @param[out]      recovery_mode       Records whether the card is in recovery
 * mode
 *
 * @returns  ISO7816                     Status Word
 * @retval  SW_NO_ERROR                 No error
 * @retval  SW_FILE_INVALID             Invalid Family ID
 * @retval  SW_CONDITIONS_NOT_SATISFIED Invalid Card Number
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_select_applet(uint8_t expected_family_id[],
                          uint8_t *acceptable_cards,
                          uint8_t *version,
                          uint8_t *card_key_id,
                          uint8_t *recovery_mode);

/**
 * @brief Adds wallet in card
 * @details
 *
 * @param[in]   wallet  Wallet structure which will be stored in card
 *
 * @returns     ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_add_wallet(const struct Wallet *wallet);

/**
 * @brief Delete wallet in card
 * @details
 * To delete a wallet, card requires name and password.
 * Both of these fields are extracted from struct Wallet *wallet
 *
 * @param [in]   wallet  Wallet structure
 *
 * @returns     ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_delete_wallet(const struct Wallet *wallet);

/**
 * @brief Retrieve wallet in card
 * @details
 * To retrieve a wallet, card requires name and password.
 * Both of these fields are extracted from struct Wallet *wallet
 * After data is retrieved from card it is filled back in the
 * wallet structure.
 *
 * @param[in,out]   wallet  Wallet structure
 *
 * @returns         ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_retrieve_wallet(struct Wallet *wallet);

/**
 * @brief Sign data using card's private key
 * @details
 * Each card has a private key stored inside. This
 * function is used to sign data using that key.
 *
 * @param[in,out]   data_inOut  Data to be signed as input and Signed data as
 * output
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_ecdsa(uint8_t data_inOut[ECDSA_SIGNATURE_SIZE],
                  uint16_t *length_inOut);

/**
 * @brief Verify Proof of Work challenge
 * @details
 * If the challenge is accepted by the card it then validates the
 * input password.
 *
 * @param [in] name      Name of wallet
 * @param [in] nonce     Nonce found by device
 * @param [in] password  Password of wallet
 *
 * @returns ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_verify_challenge(const uint8_t name[NAME_SIZE],
                             const uint8_t nonce[POW_NONCE_SIZE],
                             const uint8_t password[BLOCK_SIZE]);

/**
 * @brief Get challenge from card
 * @details
 *
 * @param[in]   name            Name of wallet
 * @param[out]  target          Target provided by card
 * @param[out]  random_number   Random number provided by card
 *
 * @returns ISO7816 Status Word
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_get_challenge(const uint8_t name[NAME_SIZE],
                          uint8_t target[SHA256_SIZE],
                          uint8_t random_number[POW_RAND_NUMBER_SIZE]);

/**
 * @brief Encrypt Data required for Inheritance flow
 * @details
 * For inheritance flow the card has to decrypt and encrypt any data.
 * The algorithm followed by the card is AES 128 in CBC mode.
 * The IV vector and key is supplied by the device during add wallet.
 * It is different for every wallet.
 *
 * @param [in] name                     Wallet Name
 * @param [in] plain_data               Data to be encrypted
 * @param [in] plain_data_size          Size of data to be encrypted
 * @param [out] encrypted_data          Encrypted data received by the card
 * @param [out] encrypted_data_size     Size of encrypted data
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_encrypt_data(const uint8_t name[NAME_SIZE],
                         const uint8_t *plain_data,
                         uint16_t plain_data_size,
                         uint8_t *encrypted_data,
                         uint16_t *encrypted_data_size);

/**
 * @brief Decrypt Data required for Inheritance flow
 * @details
 * For inheritance flow the card has to decrypt and encrypt any data.
 * The algorithm followed by the card is AES 128 in CBC mode.
 * The IV vector and key is supplied by the device during add wallet.
 * It is different for every wallet.
 *
 * @param name Wallet Name
 * @param plain_data Data to be encrypted
 * @param plain_data_size Size of data to be encrypted
 * @param encrypted_data Encrypted data received by the card
 * @param encrypted_data_size Size of encrypted data
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
ISO7816 nfc_decrypt_data(const uint8_t name[NAME_SIZE],
                         uint8_t *plain_data,
                         uint16_t *plain_data_size,
                         const uint8_t *encrypted_data,
                         uint16_t encrypted_data_size);

/**
 * @brief Exchange raw apdu
 * @details
 *
 * @param[in] send_apdu APDU to be sent
 * @param[in] send_len Length of APDU to be sent
 * @param[out] recv_apdu received APDU
 * @param[in,out] recv_len Expected length of received data and Length of
 * received data
 *
 * @returns ret_code_t STM_ERROR_CODE
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note The length of reponse APDU will be truncated to recv_len by the PN532
 * library
 */
ret_code_t nfc_exchange_apdu(uint8_t send_apdu[],
                             uint16_t send_len,
                             uint8_t recv_apdu[],
                             uint16_t *recv_len);

/**
 * @brief Set the abort callback function.
 * Aborts the ongoing flow and resets the Flow_level.
 * @details
 *
 * @param handler Abort callback function.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void nfc_set_early_exit_handler(void (*handler)());

/**
 * @brief Set device key id for nfc secure comm
 * @details
 *
 * @param [in] device_key_id device id for secure communication, size - 4 bytes
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void nfc_set_device_key_id(const uint8_t *device_key_id);

/**
 * @brief Used to set or reset `nfc_secure_comm` variable
 * @details
 *
 * @param [in] state defines if the NFC exchange is encrypted or not
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void nfc_set_secure_comm(bool state);

#endif
