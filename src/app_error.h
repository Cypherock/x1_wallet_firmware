/**
 * @file    app_error.h
 * @author  Cypherock X1 Team
 * @brief   Application error class.
 *          AppError to classify different errors in the application.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef APP_ERROR_H
#define APP_ERROR_H

/**
 * @file
 * @brief AppError to classify different errors in the application.
 *
 * @defgroup app_error Application wide error class
 * @{
 */
#define NFC_ERROR_BASE          0x10000UL
#define NFC_APP_ERROR_BASE      (NFC_ERROR_BASE + 0x1000)   ///< Base error code for errors thrown by NFC module
#define PN532_ERROR_BASE        (NFC_ERROR_BASE + 0x2000)   ///< Base error code for errors thrown by PN532 chip

/**
 * @defgroup nfc_error_codes NFC Error codes
 * @brief Macros related to the error codes returned by NFC module.
 * @ingroup app_error
 *
 * @{
 */
#define NFC_TIME_OUT            (NFC_APP_ERROR_BASE +  0x01)    ///< Time Out, the target has not answered
#define NFC_RSP_VERSION_ERROR   (NFC_APP_ERROR_BASE +  0x02)    ///< Error, version not as expected according to m_pn532_rsp_firmware_ver
#define NFC_INVALID_PREAMBLE    (NFC_APP_ERROR_BASE +  0x03)    ///< Packet preamble is invalid
#define NFC_INVALID_PARAM       (NFC_APP_ERROR_BASE +  0x04)    ///< Some invalid param for PN532 command
#define NFC_INVALID_RESPONSE    (NFC_APP_ERROR_BASE +  0x05)    ///< Response did not match expected command
#define NFC_INVALID_LENGTH      (NFC_APP_ERROR_BASE +  0x06)    ///< Invalid length provided for the command
#define NFC_DATA_LENGTH_EXCEED  (NFC_APP_ERROR_BASE +  0x07)    ///< Data length exceeds library buffer
#define NFC_CRC_ERROR           (NFC_APP_ERROR_BASE +  0x08)    ///< A CRC error has been detected
#define NFC_LENGTH_CS_ERROR     (NFC_APP_ERROR_BASE +  0x09)    ///< Checksum verification on length field failed
#define NFC_EXCHANGE_DIR_ERROR  (NFC_APP_ERROR_BASE +  0x0A)    ///< Invalid data exchange direction in packet
#define NFC_INVALID_SW          (NFC_APP_ERROR_BASE +  0x0B)    ///< Invalid status word received from applet
#define NFC_IO_ERROR_BASE       (NFC_APP_ERROR_BASE +  0x0C)    ///< Used as base for IO error in NFC module operations (read/write)
                                                         ///< 13, 14, 15 are reserved for HAL_StatusTypeDef members
#define NFC_INTERNAL_ERROR      (NFC_APP_ERROR_BASE +  0x10)    ///< Internal error due to wrong interface access or ack packet mismatch
#define NFC_CARD_ABSENT         (NFC_APP_ERROR_BASE +  0x11)    ///< Internal error due to wrong interface access or ack packet mismatch
#define NFC_SC_ENC_KEY_ERROR    (NFC_APP_ERROR_BASE +  0x12)    ///< NFC Secure Channel AES encryption key initialization error
#define NFC_SC_ENC_ERROR        (NFC_APP_ERROR_BASE +  0x13)    ///< NFC Secure Channel AES encryption error
#define NFC_SC_MAC_KEY_ERROR    (NFC_APP_ERROR_BASE +  0x14)    ///< NFC Secure Channel AES mac key initialization error
#define NFC_SC_MAC_ERROR        (NFC_APP_ERROR_BASE +  0x15)    ///< NFC Secure Channel AES mac calculation error
#define NFC_SC_MAC_MISMATCH     (NFC_APP_ERROR_BASE +  0x16)    ///< NFC Secure Channel AES mac mismatch
#define NFC_SC_DEC_KEY_ERROR    (NFC_APP_ERROR_BASE +  0x17)    ///< NFC Secure Channel AES decryption key initialization error
#define NFC_SC_DEC_ERROR        (NFC_APP_ERROR_BASE +  0x18)    ///< NFC Secure Channel AES decryption error
#define NFC_NULL_PTR_ERROR      (NFC_APP_ERROR_BASE +  0x19)    ///< NFC null pointer exception had occured in this card and is in recovery mode
/** @}
 *
 * @defgroup pn532_error_codes PN532 Error codes
 * @brief Macros related to the error codes returned by PN532. Refer:
 * <a href="https://www.nxp.com/docs/en/nxp/application-notes/AN133910.pdf" target="_blank">AN133910</a>
 *
 * @ingroup app_error
 *
 * @{
 */
#define PN532_TIME_OUT                  (PN532_ERROR_BASE + 0x01)    ///< Time Out, the target has not answered
#define PN532_CRC_ERROR                 (PN532_ERROR_BASE + 0x02)    ///< A CRC error has been detected by the contactless UART
                                                                     ///< From application context, this is indicator of weak/unstable connection
#define PN532_PARITY_ERROR              (PN532_ERROR_BASE + 0x03)    ///< A Parity error has been detected by the contactless UART
#define PN532_MIFARE_BIT_COUNT_ERROR    (PN532_ERROR_BASE + 0x04)    ///< During a MIFARE anticollision/select operation, an erroneous Bit Count has been detected
#define PN532_MIFARE_FRAMING_ERROR      (PN532_ERROR_BASE + 0x05)    ///< Framing error during MIFARE operation
#define PN532_ANTI_COLLISION_ERROR      (PN532_ERROR_BASE + 0x06)    ///< An abnormal bit-collision has been detected during bit wise anticollision at 106 kbps
#define PN532_BUFFER_SIZE_ERROR         (PN532_ERROR_BASE + 0x07)    ///< Communication buffer size insufficient
#define PN532_BUFFER_OVERFLOW           (PN532_ERROR_BASE + 0x09)    ///< RF Buffer overflow has been detected by the contactless UART (bit BufferOvfl of the register CL_ERROR)
#define PN532_TIME_MISMATCH             (PN532_ERROR_BASE + 0x0A)    ///< In active communication mode, the RF field has not been switched on in time by the counterpart (as defined in NFCIP-1 standard)
#define PN532_RF_PROTOCOL_ERROR         (PN532_ERROR_BASE + 0x0B)    ///< RF Protocol error (cf. reference [4], description of the CL_ERROR register)
                                                                     ///< This is an indication of some hardware level inconsistency
#define PN532_TEMPERATURE_ERROR         (PN532_ERROR_BASE + 0x0D)    ///< The internal temperature sensor has detected overheating, and therefore has automatically switched off the antenna drivers
#define PN532_INTERNAL_BUFFER_OF        (PN532_ERROR_BASE + 0x0E)    ///< Internal buffer overflow
#define PN532_INVALID_PARAM             (PN532_ERROR_BASE + 0x10)    ///< Invalid parameter (range, format, ...)
#define PN532_DEP_PROTOCOL_ERROR        (PN532_ERROR_BASE + 0x12)    ///< The the PN532 configured in target mode does not support the command received from the initiator
                                                                     ///< (the command received is not one of the following: ATR_REQ, WUP_REQ, PSL_REQ, DEP_REQ, DSL_REQ, RLS_REQ)
#define PN532_DATA_FORMAT_ERROR         (PN532_ERROR_BASE + 0x13)    ///< The data format does not match to the specification.
#define PN532_MIFARE_AUTH_ERROR         (PN532_ERROR_BASE + 0x14)    ///< Mifare: Authentication error
#define PN532_UID_CHECK_BYTE_WRONG      (PN532_ERROR_BASE + 0x23)    ///< ISO/IEC 14443-3: UID Check byte is wrong
#define PN532_DEP_INVALID_STATE         (PN532_ERROR_BASE + 0x25)    ///< DEP Protocol: Invalid device state, the system is in a state which does not allow the operation
#define PN532_OP_NA                     (PN532_ERROR_BASE + 0x26)    ///< Operation not allowed in this configuration (host controller interface)
#define PN532_CMD_UNACCEPTABLE          (PN532_ERROR_BASE + 0x27)    ///< This command is not acceptable due to the current context of the the PN532 (Initiator vs. Target, unknown target number, Target not in the good state, ..)
#define PN532_TG_RELEASED               (PN532_ERROR_BASE + 0x29)    ///< The the PN532 configured as target has been released by its initiator
#define PN532_CARD_ID_MISMATCH          (PN532_ERROR_BASE + 0x2A)    ///< The PN5321 and ISO/IEC 14443-3B only: the ID of the card does not match, meaning that the expected card has been exchanged with another one.
#define PN532_CARD_DISAPPEARED          (PN532_ERROR_BASE + 0x2B)    ///< The PN5321 and ISO/IEC 14443-3B only: the card previously activated has disappeared.
#define PN532_TG_IN_MISMATCH            (PN532_ERROR_BASE + 0x2C)    ///< Mismatch between the NFCID3 initiator and the NFCID3 target in DEP 212/424 kbps passive.
#define PN532_OVER_CURRENT              (PN532_ERROR_BASE + 0x2D)    ///< An over-current event has been detected
#define PN532_NAD_MISSING               (PN532_ERROR_BASE + 0x2E)    ///< NAD missing in DEP frame
/** @} */

/**
 * @}
 */

#endif //APP_ERROR_H