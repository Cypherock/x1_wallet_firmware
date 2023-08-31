/**
 * @file    controller_level_four.h
 * @author  Cypherock X1 Team
 * @brief   Header for level four controllers.
 *          Houses the declarations of controllers for level one tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONTROLLER_LEVEL_FOUR_H
#define CONTROLLER_LEVEL_FOUR_H

#pragma once

#include "controller_main.h"
#include "cryptoauthlib.h"
#include "eth.h"
#include "flash_config.h"
#include "near.h"
#include "solana.h"
#include "tasks.h"

#define DEFAULT_ATECC_RETRIES 5

#define DEVICE_SERIAL_SIZE 32
#define MAXIMUM_COIN_SUPPORTED (14 + 5)    // 5 for segwit support

typedef enum {
  slot_0_unused = 0U,
  slot_1_unused = 1U,
  slot_2_auth_key = 2U,
  slot_3_nfc_pair_key = 3U,
  slot_4_unused = 4U,
  slot_5_challenge = 5U,
  slot_6_io_key = 6U,
  slot_7_unused = 7U,
  slot_8_serial = 8U,
  slot_9_unused = 9U,
  slot_10_unused = 10U,
  slot_11_unused = 11U,
  slot_12_unused = 12U,
  slot_13_unused = 13U,
  slot_14_unused = 14U,
  slot_15_unused = 15U
} atecc_slot_define_t;

typedef struct {
  uint8_t device_serial[DEVICE_SERIAL_SIZE], retries;
  ATCA_STATUS status;
  ATCAIfaceCfg *cfg_atecc608a_iface;
} atecc_data_t;

extern atecc_data_t atecc_data;

typedef enum {
  provision_empty = 0,
  provision_incomplete = 1,
  provision_complete = 2,
  provision_v1_complete = 3
} provision_status_t;

extern uint8_t provision_date[4];
extern uint8_t auth_card_number;
extern solana_unsigned_txn solana_unsigned_txn_ptr;

/**
 * @brief Stores the device specific information during provisioning process.
 * @details The device specific information is stored temporarily in the RAM
 * during the 2nd phase of device provisioning. These are received from the
 * in-house provisioning tool before a device is ready for use.
 *
 * @see Perm_Ext_Keys_Struct, Perm_Key_Data_Struct, Flash_Perm_Struct,
 * device_provision_controller(), FIREWALL_NVDATA_APP_KEYS_ADDR,
 * START_DEVICE_PROVISION, desktop_listener_task()
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Provision_Data_Struct {
  uint8_t device_private_key[32];
  uint8_t device_public_key[ECDSA_PUB_KEY_SIZE];
  uint8_t self_key_path[FS_KEYSTORE_KEYPATH_LEN];
  uint8_t priv_key[FS_KEYSTORE_PRIVKEY_LEN];
  uint8_t card_root_xpub[FS_KEYSTORE_XPUB_LEN];
} Provision_Data_struct;
#pragma pack(pop)

extern Provision_Data_struct provision_keys_data;

/**
 * @brief Stores the generated xpubs list for the add coin process.
 * @details The generated xpubs list is stored temporarily in the RAM during the
 * add coin process before it is sent to the desktop app.
 *
 * @see MAXIMUM_COIN_SUPPORTED, Add_Coin_Data, add_coin_controller(),
 * add_coin_task(), desktop_listener_task(), ADD_COIN_START
 * @since v1.0.0
 *
 * @note The number of entries in the list should not be greater than the
 * maximum number of coins supported (ref MAXIMUM_COIN_SUPPORTED).
 */
#pragma pack(push, 1)
typedef struct Cmd_Add_Coin_t {
  uint8_t xpub[XPUB_SIZE];
} Cmd_Add_Coin_t;
#pragma pack(pop)

/**
 * @brief send transaction data
 * @details The transaction data is stored temporarily in the RAM during the
 * intermediate phases of send transaction process.
 *
 * @see send_transaction_controller(), send_transaction_task(),
 * desktop_listener_task(), SEND_TXN_START
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Send_Transaction_Data {
  uint8_t transaction_confirmation_list_index;
  txn_metadata transaction_metadata;

} Send_Transaction_Data;
#pragma pack(pop)

extern Coin_Specific_Data_Struct coin_specific_data;
extern uint8_t *eth_unsigned_txn_byte_array;
extern Send_Transaction_Data var_send_transaction_data;

/**
 * @brief Card upgrade controller available for development purposes.
 * @details This controller is used to upgrade the card using a desktop app.
 *
 * @see card_upgrade_task(), desktop_listener_task(), START_CARD_UPGRADE
 * @since v1.0.0
 */
void card_upgrade_controller();

/**
 * @brief Device provision controller
 * @details This controller is used to provision the device using the desktop
 * app.
 *
 * @see desktop_listener_task(),
 * START_DEVICE_PROVISION
 * @since v1.0.0
 */
void device_provision_controller();

/**
 * @brief This controller is executed for verifying card in main application.
 * @details This controller is used to verify the card with the servers using
 * the desktop app.
 *
 * @see task_card_verification(), desktop_listener_task(), START_CARD_AUTH
 * @since v1.0.0
 */
void verify_card_controller();

/**
 * @brief This controller is executed for verifying card in initial application.
 * @details This controller is used to verify the card using the desktop app in
 * initial application.
 *
 * @see initial_verify_card_task(), desktop_listener_task(), START_CARD_AUTH
 * @since v1.0.0
 */
void initial_verify_card_controller();

/**
 * @brief This controller is executed when the name of a wallet is clicked and
 * it is locked.
 * @details This controller is used to handle next event for unlocking the
 * wallet using the proof of work mechanism where the X1Wallet does the proof of
 * work and sends the result to the card.
 *
 * @see wallet_locked_task(), wallet_locked_controller_b()
 * @since v1.0.0
 */
void wallet_locked_controller();

/**
 * @brief Back button controller for wallet locked flow
 * @details This controller is used to handle back button event for unlocking
 * the wallet using the proof of work mechanism.
 *
 * @see wallet_locked_controller(), wallet_locked_task()
 * @since v1.0.0
 */
void wallet_locked_controller_b();

/**
 * @brief Next button controller is executed for processing and signing unsigned
 * transaction of Ethereum.
 * @details This controller is used to process and sign the unsigned transaction
 * for ETH that is requested from the desktop app.
 *
 * @see send_transaction_controller_b_eth(), send_transaction_tasks_eth(),
 * send_transaction_controller(), send_transaction_tasks(),
 * send_transaction_controller_b()
 * @since v1.0.0
 */
void send_transaction_controller_eth();

/**
 * @brief Back button controller for send transaction Ethereum flow.
 * @details This controller is used to handle back button events during
 * processing and signing unsigned transaction for ETH.
 *
 * @see send_transaction_controller_eth(), send_transaction_tasks_eth(),
 * send_transaction_controller(), send_transaction_tasks()
 * @since v1.0.0
 */
void send_transaction_controller_b_eth();

/**
 * @brief Next button controller is executed for processing and signing messages
 * of Ethereum.
 * @details This controller is used to process and sign the messages for ETH
 * that is requested from the desktop app.
 *
 */
void sign_message_controller_eth();

/**
 * @brief Back button controller for sign message Ethereum flow.
 * @details This controller is used to handle back button events during
 * processing and signing messages for ETH.
 *
 */
void sign_message_controller_b_eth();

/**
 * @brief Next button controller is executed for processing and signing unsigned
 * transaction.
 */
void send_transaction_controller_solana();

/**
 * @brief Back button controller is executed for handling cancellation of the
 * ongoing process.
 */
void send_transaction_controller_b_solana();

/**
 * @brief Next button controller is executed for generating address using xpub.
 * @details This controller handles is used to generate receiving address for
 * the BTC coins requested by desktop from a list of supported coins.
 *
 * @see receive_transaction_controller_b(), receive_transaction_tasks(),
 * desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller_eth(), receive_transaction_tasks_eth()
 * @since v1.0.0
 */
void receive_transaction_controller_near();

/**
 * @brief Next button controller is executed for generating address using xpub
 * of Ethereum.
 * @details This controller handles is used to generate receiving address for
 * the ETH coins requested by desktop from a list of supported coins.
 *
 * @see receive_transaction_controller_b_eth(), receive_transaction_tasks_eth(),
 * desktop_listener_task(), RECV_TXN_START, receive_transaction_controller(),
 * receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_eth();

/**
 * @brief Back button controller for receive transaction Ethereum flow.
 * @details This controller is used to handle back button events during
 * generating receiving address for ETH.
 *
 * @see receive_transaction_controller_eth(), receive_transaction_tasks_eth(),
 * desktop_listener_task(), RECV_TXN_START, receive_transaction_controller(),
 * receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_b_eth();

/**
 * @brief Back button controller for receive transaction Near flow.
 * @details This controller is used to handle back button events during
 * receiving flow for Near.
 *
 * @see receive_transaction_controller_eth(), receive_transaction_tasks_eth(),
 * desktop_listener_task(), RECV_TXN_START, receive_transaction_controller(),
 * receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_b_near();

/**
 * @brief Next button controller is executed for generating address using xpub
 * of Ethereum.
 * @details This controller handles is used to generate receiving address for
 * the ETH coins requested by desktop from a list of supported coins.
 *
 * @see receive_transaction_controller_b_eth(), receive_transaction_tasks_eth(),
 * desktop_listener_task(), RECV_TXN_START, receive_transaction_controller(),
 * receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_solana();

/**
 * @brief Back button controller for receive transaction Ethereum flow.
 * @details This controller is used to handle back button events during
 * generating receiving address for ETH.
 *
 * @see receive_transaction_controller_eth(), receive_transaction_tasks_eth(),
 * desktop_listener_task(), RECV_TXN_START, receive_transaction_controller(),
 * receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_b_solana();

/**
 * @brief This controller is executed for verifying wallet added or restored on
 * the device and cards.
 * @details This controller is used to verify the wallet added or restored on
 * the device and cards.
 *
 * @see verify_wallet_controller_b(), verify_wallet_tasks(),
 * generate_wallet_controller(),
 * restore_wallet_controller(),
 * @since v1.0.0
 */
void verify_wallet_controller();

/**
 * @brief Back button controller for verify wallet flow.
 * @details This controller is used to handle back button events during wallet
 * verification of newly added wallet.
 *
 * @see verify_wallet_controller(), verify_wallet_tasks(),
 * generate_wallet_controller(),
 * restore_wallet_controller(),
 * @since v1.0.0
 */
void verify_wallet_controller_b();

/**
 * @brief
 * @details
 *
 * @return uint8_t Provision status of the device
 * @retval 0    Not provisioned
 * @retval 1    Semi-provisioned (serial and IO-Protection Key present)
 * @retval 2    Fully provisioned
 * @retval 3    External auth configuration
 */
provision_status_t check_provision_status();

/**
 * @brief   fetch device serial and check if UID in the serial matches MCU UID
 * or not
 * @details
 *
 * @return uint32_t device serial fetch status or failure status
 * @retval 0    fetched successfully
 * @retval 1    device UID doesn't match with serial UID
 */
uint32_t get_device_serial();

/**
 * @brief Request ATECC to generate signature on the hash with private available
 * on SLOT-3
 * @details
 *
 * @param [in] hash     - hash to be signed
 * @param [out] sign    - signature generated
 *
 * @return    ATCA_SUCCESS on success, otherwise an error code.
 *
 * @see atcab_init(), atcab_sign(), ATCAIfaceCfg, cfg_atecc608a_iface
 * @since v1.0.0
 */
uint8_t atecc_nfc_sign_hash(const uint8_t *hash, uint8_t *sign);

/**
 * @brief Request ATECC to perform ECDH operation on pub_key with private key
 * from SLOT-3
 * @details
 *
 * @param [in] pub_key          - public key to be used for ECDH
 * @param [out] shared_secret   - shared secret generated
 *
 * @return    ATCA_SUCCESS on success, otherwise an error code.
 *
 * @see atcab_init(), atcab_ecdh(), atcab_ecdh_ioenc(), ATCAIfaceCfg,
 * cfg_atecc608a_iface
 * @since v1.0.0
 */
uint8_t atecc_nfc_ecdh(const uint8_t *pub_key, uint8_t *shared_secret);

#endif