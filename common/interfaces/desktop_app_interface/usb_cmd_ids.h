/**
 * @file    usb_cmd_ids.h
 * @author  Cypherock X1 Team
 * @brief   Definition for supported command ids.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef USB_CMD_IDS_H
#define USB_CMD_IDS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/// enum for command types received from/sent to desktop.
typedef enum commandType {
  ACK_PACKET = 1,      ///< Acknowledge packet
  ERROR_PACKET = 7,    ///< Nak packet for when device is busy or error occurred

  /** Start Auth Command **/
  START_AUTH_PROCESS =
      12,    ///< deprecated enum NOTE:replace with START_CARD_AUTH
  SEND_SIGNATURE_TO_APP = 13,    ///< Send card auth signature to desktop
  APP_SEND_RAND_NUM = 16,        ///< Receive random challenge for signing
  SIGNED_CHALLENGE = 17,         ///< Send challenge signature to desktop

  APP_LOG_DATA_REJECT = 37,    ///< Request by desktop to receive logs

  READY_STATE_PACKET = 41,    ///< Request by desktop for device status
  STATUS_PACKET = 42,    ///< Command for device status request and response

  SEND_WALLET_TO_DESKTOP = 44,    ///< Response for fetch wallets request

  USER_CONFIRMED_PASSPHRASE =
      90,    ///< Prompt sent to desktop if user confirms passphrase option
  USER_REJECTED_PASSPHRASE_INPUT =
      91,    ///< Prompt sent to desktop if user rejects passphrase option

  USER_ENTERED_PIN = 47,     ///< Prompt sent to desktop for pin entered by user
  USER_TAPPED_CARDS = 48,    ///< Prompt sent to desktop for card tapping

  ADD_COIN_VERIFIED_BY_USER = 46,    ///< Prompt for add coin verified by user
  ADD_COIN_SENDING_XPUBS = 49,       ///< Response for add coin with xpubs

  SEND_TXN_START = 50,    ///< Request by desktop to start a send transaction
  SEND_TXN_REQ_UNSIGNED_TXN =
      51,    ///< Request by device to fetch unsigned transaction
  SEND_TXN_UNSIGNED_TXN =
      52,    ///< Response by desktop with unsigned transaction
  SEND_TXN_USER_VERIFIES_ADDRESS =
      53,    ///< Prompt for send transaction after user verifies send address
  SEND_TXN_SENDING_SIGNED_TXN = 54,    ///< Response by device for send
                                       ///< transaction with signed transaction

  RECV_TXN_START = 59,    ///< Request by desktop to start a receive transaction
  RECV_TXN_USER_VERIFIED_COINS =
      63,    ///< Prompt for user confirmation or rejection of receive coin
  RECV_TXN_USER_VERIFIED_ADDRESS =
      64,    ///< Prompt for user confirmation or rejection of receive address
  RECV_TXN_XPUBS_EXISTS_ON_DEVICE =
      65,    ///< Prompt for finding/deriving XPUB on device

#ifdef DEV_BUILD
  START_CARD_UPGRADE = 60,
  APDU_PACKET = 61,
  STOP_CARD_UPGRADE = 62,
#endif
  CARD_ERROR_FACED = 71,    ///< Response by device for card auth error faced
#ifdef DEV_BUILD
  EXPORT_ALL_SEND = 73,
#endif

  WALLET_IS_LOCKED = 75,          ///< Prompt by device when wallet is locked
  WALLET_DOES_NOT_EXISTS = 76,    ///< Prompt by device when wallet not found

  USER_REJECT_PIN_INPUT = 79,    ///< Prompt for pin input rejection by user

  WALLET_DOES_NOT_EXISTS_ON_CARD =
      81,    ///< Prompt by device when wallet is not found on card

  START_DEVICE_PROVISION =
      84,    ///< Request by desktop(provisioning tool) for provisioning device
  ADD_DEVICE_PROVISION =
      82,    ///< Response by device(initial firmware) if provisioning started
  CONFIRM_PROVISION = 80,    ///< Response by device(initial firmwaer) after
                             ///< provisioning complete/failed

  DEVICE_SERAIL_NO_SIGNED =
      85,    ///< Response by device with device serial number and signature
  DEVICE_CHALLENGE_SIGNED =
      86,    ///< Response by device with challenge signature

  COMM_SDK_VERSION_REQ = 88,    ///< Command for Communication SDK version

  RECV_TXN_USER_VERIFIED_ACCOUNT =
      96,    ///< Prompt for user for verification of account
  RECV_TXN_REPLACE_ACCOUNT =
      97,    ///< Request by desktop to start replace account

  COIN_SPECIFIC_DATA_ERROR = 98,    ///< Command for Coin Specific Data Error
  LIST_SUPPORTED_COINS =
      99,    ///< Exchange the list of supported coins on device

  SEND_TXN_PRE_SIGNING_DATA = 92,    ///< Command for data required just before
                                     ///< signing in send transaction

  SIGN_MSG_START = 93,      ///< Request by desktop to start a sign message flow
  SIGN_MSG_RAW_MSG = 94,    ///< Request by device to fetch raw message
  SIGN_MSG_SEND_SIG = 95,    ///< Response by device for send transaction with
                             ///< signed transaction

  DEVICE_FLOW_RESET_REQ = 0xFF    ///< unused enum

} En_command_type_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif