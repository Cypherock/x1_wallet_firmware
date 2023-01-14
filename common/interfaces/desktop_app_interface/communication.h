/**
 * @file    communication.h
 * @author  Cypherock X1 Team
 * @brief   USB communication interface APIs.
 *          Exposes the APIs for enabling USB communication at application layer.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 */
#ifndef _COMMUNICATION
#define _COMMUNICATION
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ALLOW_LOG_EXPORT

#define START_OF_FRAME 0xAA
#define START_OF_FRAME_U16 0x5A5A
#define MAXIMUM_DATA_SIZE 40
#define PKT_HEAD_SIZE 4
#define BYTE_STUFFED_DATA_SIZE ((MAXIMUM_DATA_SIZE + PKT_HEAD_SIZE) * 2)
#define CRC16_SIZE sizeof(uint16_t)
#define COMM_HEADER_SIZE 16
#define DATA_SIZE_INDEX (COMM_HEADER_SIZE - 1)
#define COMM_HEADER_SIZE_V0 (sizeof(comm_header_v0_t))
#define DATA_SIZE_INDEX_V0 (COMM_HEADER_SIZE_V0 - 1)

typedef enum cy_app_task {
    CY_UNUSED_TASK = 0x00,
    CY_APP_IDLE_TASK = 0x01,
    CY_APP_USB_TASK = 0x02,
    CY_APP_DEVICE_TASK = 0x03,
} cy_app_task_t;

typedef enum cy_app_status {
    CY_UNUSED_STATE = 0x00,
    CY_APP_IDLE = 0x10,
    CY_APP_WAIT_FOR_CARD = 0x20,
    CY_APP_WAIT_USER_INPUT = 0x30,
    CY_APP_BUSY = 0x40,
} cy_app_status_t;

/// enum for command types received from/sent to desktop.
typedef enum commandType {
    ACK_PACKET = 1,                         ///< Acknowledge packet
    ERROR_PACKET = 7,                       ///< Nak packet for when device is busy or error occurred

    /** Start Auth Command **/
    START_AUTH_PROCESS = 12,                ///< deprecated enum NOTE:replace with START_CARD_AUTH
    SEND_SIGNATURE_TO_APP = 13,             ///< Send card auth signature to desktop
    APP_SEND_RAND_NUM = 16,                 ///< Receive random challenge for signing
    SIGNED_CHALLENGE = 17,                  ///< Send challenge signature to desktop

    APP_LOG_DATA_REJECT = 37,               ///< Request by desktop to receive logs
    APP_LOG_DATA_SEND = 38,                 ///< Response to send log to desktop

    READY_STATE_PACKET = 41,                ///< Request by desktop for device status
    STATUS_PACKET = 42,                     ///< Command for device status request and response

    START_EXPORT_WALLET = 43,               ///< Request by desktop app to fetch wallets
    SEND_WALLET_TO_DESKTOP = 44,            ///< Response for fetch wallets request

    USER_CONFIRMED_PASSPHRASE = 90,         ///< Prompt sent to desktop if user confirms passphrase option
    USER_REJECTED_PASSPHRASE_INPUT = 91,    ///< Prompt sent to desktop if user rejects passphrase option

    USER_ENTERED_PIN = 47,                  ///< Prompt sent to desktop for pin entered by user
    USER_TAPPED_CARDS = 48,                 ///< Prompt sent to desktop for card tapping

    ADD_COIN_START = 45,                    ///< Request by desktop to fetch coin xpubs
    ADD_COIN_VERIFIED_BY_USER = 46,         ///< Prompt for add coin verified by user
    ADD_COIN_SENDING_XPUBS = 49,            ///< Response for add coin with xpubs

    SEND_TXN_START = 50,                    ///< Request by desktop to start a send transaction
    SEND_TXN_REQ_UNSIGNED_TXN = 51,         ///< Request by device to fetch unsigned transaction
    SEND_TXN_UNSIGNED_TXN = 52,             ///< Response by desktop with unsigned transaction
    SEND_TXN_USER_VERIFIES_ADDRESS = 53,    ///< Prompt for send transaction after user verifies send address
    SEND_TXN_SENDING_SIGNED_TXN = 54,       ///< Response by device for send transaction with signed transaction

    RECV_TXN_START = 59,                    ///< Request by desktop to start a receive transaction
    RECV_TXN_USER_VERIFIED_COINS = 63,      ///< Prompt for user confirmation or rejection of receive coin
    RECV_TXN_USER_VERIFIED_ADDRESS = 64,    ///< Prompt for user confirmation or rejection of receive address
    RECV_TXN_XPUBS_EXISTS_ON_DEVICE = 65,   ///< Prompt for finding/deriving XPUB on device

    SWAP_TXN_START = 66,                    ///< Request by desktop to start a swap transaction
    SWAP_TXN_SIGNED_RECV_ADDR = 67,         ///< Send desktop the receive address for swap along with signature

    #ifdef DEV_BUILD
    START_CARD_UPGRADE = 60,
    APDU_PACKET = 61,
    STOP_CARD_UPGRADE = 62,
    #endif
    START_CARD_AUTH = 70,                   ///< Request by desktop for start card auth
    CARD_ERROR_FACED = 71,                  ///< Response by device for card auth error faced
    #ifdef DEV_BUILD
    EXPORT_ALL = 72,
    EXPORT_ALL_SEND = 73,
    #endif

    WALLET_IS_LOCKED = 75,                  ///< Prompt by device when wallet is locked
    WALLET_DOES_NOT_EXISTS = 76,            ///< Prompt by device when wallet not found
     

    START_FIRMWARE_UPGRADE = 77,            ///< Request by desktop to initiate firmware update
    USER_FIRMWARE_UPGRADE_CHOICE = 78,      ///< Response by device firmware update
    USER_REJECT_PIN_INPUT = 79,             ///< Prompt for pin input rejection by user


    WALLET_DOES_NOT_EXISTS_ON_CARD = 81,    ///< Prompt by device when wallet is not found on card
    
    START_DEVICE_PROVISION = 84,            ///< Request by desktop(provisioning tool) for provisioning device
    ADD_DEVICE_PROVISION = 82,              ///< Response by device(initial firmware) if provisioning started
    CONFIRM_PROVISION = 80,                 ///< Response by device(initial firmware) after provisioning complete/failed

    START_DEVICE_AUTHENTICATION = 83,       ///< Request by device to start device authentication
 
    DEVICE_SERAIL_NO_SIGNED = 85,           ///< Response by device with device serial number and signature
    DEVICE_CHALLENGE_SIGNED = 86,           ///< Response by device with challenge signature

    DEVICE_INFO = 87,                       ///< Command for device information
    COMM_SDK_VERSION_REQ = 88,              ///< Command for Communication SDK version

    RECV_TXN_USER_VERIFIED_ACCOUNT= 96,         ///< Prompt for user for verification of account
    RECV_TXN_REPLACE_ACCOUNT= 97,               ///< Request by desktop to start replace account
    
    COIN_SPECIFIC_DATA_ERROR = 98,              ///< Command for Coin Specific Data Error
    LIST_SUPPORTED_COINS = 99,              ///< Exchange the list of supported coins on device

    SEND_TXN_PRE_SIGNING_DATA = 92,                    ///< Command for data required just before signing in send transaction

    DEVICE_FLOW_RESET_REQ = 0xFF            ///< unused enum

} En_command_type_t;

/**
 * @brief Initialize communication by registering USB receive function in libusb
 * 
 * @since v1.0.0
 */
void comm_init();

/**
 * @brief Checks if there is any message received from desktop.
 * @details
 *
 * @param
 *
 * @return Returns message receive status.
 * @retval 1 for any message
 * @retval 0 for no messages.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool is_there_any_msg_from_app();

/**
 * @brief This function allows to fetch any available transaction. The caller should provide storage to get the 
 * command-type, any accompanying data and length of byte array.
 * @details
 * 
 * @param[out] command_type  En_command_type_t to fetch the type of operation
 * @param[out] msg_data      Pointer to store message array location
 * @param[out] msg_len       Length of message contained in the command received
 *
 * @return true, false
 * @retval false             If exactly one out of msg_data and msg_len is NULL
 * @retval true              If the message is available
 * @retval false             If the message in not available
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool get_usb_msg(En_command_type_t *command_type, uint8_t **msg_data, uint16_t *msg_len);

/**
 * @brief This function allows to fetch a specific command type. The caller should specify the command type,
 * and length of expected data.
 * @details
 * 
 * @param command_type  En_command_type_t for which to look for
 * @param msg_data      Pointer to store message array location
 * @param msg_len       Length of message contained in the command received
 *
 * @return true, false
 * @retval false        If exactly one out of msg_data and msg_len is NULL
 * @retval true         If the requested message is available
 * @retval false        If the requested message in not available
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool get_usb_msg_by_cmd_type(En_command_type_t command_type, uint8_t **msg_data, uint16_t *msg_len);

/**
 * @brief Clear message from desktop.
 * Sets 0 to variable storing message from desktop.
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
void clear_message_received_data();

void comm_process_complete();

/**
 * @brief Set device state.
 * @details
 *
 * @param ready_state device state.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void mark_device_state(cy_app_status_t state, uint8_t flow_status);

/**
 * @brief Check if device is ready.
 * @details
 *
 * @param
 *
 * @return device ready state.
 * @return true for device is ready.
 * @return false for device is not ready.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool is_device_ready();

/**
* @brief Reject any ongoing flow.
* @details This will internally mark the comm_cmd_state to REJECTED. The point of rejection is conveyed
* to the desktop based on the value of curr_flow_status.
*
* @see
* @since v1.0.0
*/
void comm_reject_request(En_command_type_t command_type, uint8_t byte);

/**
 * @brief Reject an invalid command id request from the desktop application.
 *
 * @param command_type
 * @param byte
 */
void comm_reject_invalid_cmd();

/**
 * @brief Send confirmation to desktop with command type.
 * @details
 * 
 * @param command_type Command type to send with confirmation byte.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void transmit_one_byte_confirm(uint32_t command_type);

/**
 * @brief Send one byte data to desktop.
 * @details
 * 
 * @param command_type Command type.
 * @param byte Byte data.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void transmit_one_byte(uint32_t command_type, uint8_t byte);

/**
 * @brief Send a array of bytes to desktop.
 * @details
 *
 * @param command_type Command Type.
 * @param transmit_data Data.
 * @param size Size of data.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void transmit_data_to_app(uint32_t command_type, const uint8_t* transmit_data, uint32_t size);

#endif //_COMMUNICATION
