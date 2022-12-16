/**
 * @file    controller_level_four.h
 * @author  Cypherock X1 Team
 * @brief   Header for level four controllers.
 *          Houses the declarations of controllers for level one tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */

#ifndef CONTROLLER_LEVEL_FOUR_H
#define CONTROLLER_LEVEL_FOUR_H

#pragma once

#include "btc.h"
#include "controller_main.h"
#include "cryptoauthlib.h"
#include "eth.h"
#include "flash_config.h"
#include "near.h"
#include "solana.h"
#include "tasks.h"
#include "atecc_utils.h"

#define DEFAULT_ATECC_RETRIES 5

#define MAXIMUM_COIN_SUPPORTED (14 + 5)  // 5 for segwit support

extern atecc_data_t atecc_data;

typedef enum {
  provision_empty       = 0,
  provision_incomplete  = 1,
  provision_complete    = 2,
  provision_v1_complete = 3
} provision_status_t;

extern uint8_t provision_date[4];
extern uint8_t auth_card_number;
extern near_unsigned_txn near_utxn;
extern solana_unsigned_txn solana_unsigned_txn_ptr;

/**
 * @brief Stores the device specific information during provisioning process.
 * @details The device specific information is stored temporarily in the RAM during the 2nd phase of device provisioning.
 * These are received from the in-house provisioning tool before a device is ready for use.
 *
 * @see Perm_Ext_Keys_Struct, Perm_Key_Data_Struct, Flash_Perm_Struct, device_provision_controller(),
 * FIREWALL_NVDATA_APP_KEYS_ADDR, START_DEVICE_PROVISION, desktop_listener_task()
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
 * @brief Stores the chosen wallet index for the export wallet process.
 * @details The chosen wallet index is stored temporarily in the RAM during the export wallet process.
 *
 * @see export_wallet_controller(), export_wallet_task(), desktop_listener_task(), START_EXPORT_WALLET, Cmd_Export_Wallet_t
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Export_Wallet_Data {
  uint8_t chosen_wallet_index;
} Export_Wallet_Data;
#pragma pack(pop)

/**
 * @brief Stores the chosen wallet's public information for the export wallet process.
 * @details
 *
 * @see export_wallet_controller(), export_wallet_task(), desktop_listener_task(), START_EXPORT_WALLET, Export_Wallet_Data
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Cmd_Export_Wallet_t {
  uint8_t wallet_name[NAME_SIZE];
  uint8_t wallet_info;
  uint8_t wallet_id[WALLET_ID_SIZE];
} Cmd_Export_Wallet_t;
#pragma pack(pop)

/**
 * @brief Stores the generated xpubs list for the add coin process.
 * @details The generated xpubs list is stored temporarily in the RAM during the add coin process before it is sent
 * to the desktop app.
 *
 * @see MAXIMUM_COIN_SUPPORTED, Add_Coin_Data, add_coin_controller(), add_coin_task(), desktop_listener_task(), ADD_COIN_START
 * @since v1.0.0
 *
 * @note The number of entries in the list should not be greater than the maximum number of coins supported (ref MAXIMUM_COIN_SUPPORTED).
 */
#pragma pack(push, 1)
typedef struct Cmd_Add_Coin_t {
  uint8_t xpub[XPUB_SIZE];
} Cmd_Add_Coin_t;
#pragma pack(pop)

/**
 * @brief send transaction data
 * @details The transaction data is stored temporarily in the RAM during the intermediate phases of send transaction process.
 *
 * @see send_transaction_controller(), send_transaction_task(), desktop_listener_task(), SEND_TXN_START
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Send_Transaction_Data {
  uint8_t transaction_confirmation_list_index;
  unsigned_txn unsigned_transaction;
  txn_metadata transaction_metadata;
  signed_txn signed_transaction;

} Send_Transaction_Data;
#pragma pack(pop)

extern Coin_Specific_Data_Struct coin_specific_data;
extern uint8_t *eth_unsigned_txn_byte_array;
extern Send_Transaction_Data var_send_transaction_data;
extern Send_Transaction_Cmd send_transaction_cmd;

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
 * @details This controller is used to provision the device using the desktop app.
 *
 * @see task_device_authentication(), desktop_listener_task(), START_DEVICE_PROVISION
 * @since v1.0.0
 */
void device_provision_controller();

/**
 * @brief This controller is executed for verifying card in main application.
 * @details This controller is used to verify the card with the servers using the desktop app.
 *
 * @see task_card_verification(), desktop_listener_task(), START_CARD_AUTH
 * @since v1.0.0
 */
void verify_card_controller();

/**
 * @brief This controller is executed for verifying card in initial application.
 * @details This controller is used to verify the card using the desktop app in initial application.
 *
 * @see initial_verify_card_task(), desktop_listener_task(), START_CARD_AUTH
 * @since v1.0.0
 */
void initial_verify_card_controller();

/**
 * @brief This controller is executed when the name of a wallet is clicked and it is locked.
 * @details This controller is used to handle next event for unlocking the wallet using the proof of work mechanism
 * where the X1Wallet does the proof of work and sends the result to the card.
 *
 * @see wallet_locked_task(), wallet_locked_controller_b()
 * @since v1.0.0
 */
void wallet_locked_controller();

/**
 * @brief Back button controller for wallet locked flow
 * @details This controller is used to handle back button event for unlocking the wallet using the proof of work mechanism.
 *
 * @see wallet_locked_controller(), wallet_locked_task()
 * @since v1.0.0
 */
void wallet_locked_controller_b();

/**
 * @brief This controller is executed for exporting wallet on device
 * @details This controller is used to export the wallet on device using the desktop app.
 *
 * @see export_wallet_task(), desktop_listener_task(), START_EXPORT_WALLET
 * @since v1.0.0
 */
void export_wallet_controller();

/**
 * @brief Back button controller for exporting wallet flow.
 * @details This controller is used to handle back button events during exporting the wallet to the desktop app.
 *
 * @see export_wallet_controller(), export_wallet_task()
 * @since v1.0.0
 */
void export_wallet_controller_b();
/**
 * @brief This controller is executed for adding xpub to the device.
 * @details This controller is used to add xpub to the desktop app for the coins requested by desktop from a list of
 * supported coins.
 *
 * @see add_coin_tasks(), desktop_listener_task(), ADD_COIN_START
 * @since v1.0.0
 */
void add_coin_controller();

/**
 * @brief Back button controller for add coin flow.
 * @details This controller is used to handle back button events during adding xpub to the device.
 *
 * @see add_coin_controller(), add_coin_tasks(), ADD_COIN_START
 * @since v1.0.0
 */
void add_coin_controller_b();

/**
 * @brief Next button controller is executed for processing and signing unsigned transaction.
 * @details This controller is used to process and sign the unsigned transaction for BTC that is requested from the desktop app.
 *
 * @see send_transaction_controller_b(), send_transaction_tasks(), send_transaction_controller_eth(), send_transaction_tasks_eth()
 * @since v1.0.0
 */
void send_transaction_controller();

/**
 * @brief Back button controller for send transaction flow.
 * @details This controller is used to handle back button events during processing and signing unsigned transaction for BTC.
 *
 * @see send_transaction_controller(), send_transaction_tasks(), send_transaction_controller_eth(), send_transaction_tasks_eth()
 * @since v1.0.0
 */
void send_transaction_controller_b();

/**
 * @brief Next button controller is executed for processing and signing unsigned
 * transaction.
 */
void send_transaction_controller_near();

/**
 * @brief Back button controller is executed for handling cancellation of the
 * ongoing process.
 */
void send_transaction_controller_near_b();

/**
 * @brief Next button controller is executed for processing and signing unsigned transaction of Ethereum.
 * @details This controller is used to process and sign the unsigned transaction for ETH that is requested from the desktop app.
 *
 * @see send_transaction_controller_b_eth(), send_transaction_tasks_eth(), send_transaction_controller(), send_transaction_tasks(),
 * send_transaction_controller_b()
 * @since v1.0.0
 */
void send_transaction_controller_eth();

/**
 * @brief Back button controller for send transaction Ethereum flow.
 * @details This controller is used to handle back button events during processing and signing unsigned transaction for ETH.
 *
 * @see send_transaction_controller_eth(), send_transaction_tasks_eth(), send_transaction_controller(), send_transaction_tasks()
 * @since v1.0.0
 */
void send_transaction_controller_b_eth();

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
 * @details This controller handles is used to generate receiving address for the BTC coins requested by desktop from a
 * list of supported coins.
 *
 * @see receive_transaction_controller_b(), receive_transaction_tasks(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller_eth(), receive_transaction_tasks_eth()
 * @since v1.0.0
 */
void receive_transaction_controller();

void swap_transaction_controller();

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
 * @brief Back button controller for receive transaction flow.
 * @details This controller is used to handle back button events during generating receiving address for BTC.
 *
 * @see receive_transaction_controller(), receive_transaction_tasks(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller_eth(), receive_transaction_tasks_eth()
 * @since v1.0.0
 */
void receive_transaction_controller_b();

void swap_transaction_controller_b();
/**
 * @brief Next button controller is executed for generating address using xpub of Ethereum.
 * @details This controller handles is used to generate receiving address for the ETH coins requested by desktop from a
 * list of supported coins.
 *
 * @see receive_transaction_controller_b_eth(), receive_transaction_tasks_eth(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller(), receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_eth();

/**
 * @brief Back button controller for receive transaction Ethereum flow.
 * @details This controller is used to handle back button events during generating receiving address for ETH.
 *
 * @see receive_transaction_controller_eth(), receive_transaction_tasks_eth(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller(), receive_transaction_tasks()
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
 * @brief Next button controller is executed for generating address using xpub of Ethereum.
 * @details This controller handles is used to generate receiving address for the ETH coins requested by desktop from a
 * list of supported coins.
 *
 * @see receive_transaction_controller_b_eth(), receive_transaction_tasks_eth(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller(), receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_solana();

/**
 * @brief Back button controller for receive transaction Ethereum flow.
 * @details This controller is used to handle back button events during generating receiving address for ETH.
 *
 * @see receive_transaction_controller_eth(), receive_transaction_tasks_eth(), desktop_listener_task(), RECV_TXN_START,
 * receive_transaction_controller(), receive_transaction_tasks()
 * @since v1.0.0
 */
void receive_transaction_controller_b_solana();

/**
 * @brief This controller is executed for verifying wallet added or restored on the device and cards.
 * @details This controller is used to verify the wallet added or restored on the device and cards.
 *
 * @see verify_wallet_controller_b(), verify_wallet_tasks(), generate_wallet_controller(), tasks_add_new_wallet(),
 * restore_wallet_controller(), tasks_restore_wallet()
 * @since v1.0.0
 */
void verify_wallet_controller();

/**
 * @brief Back button controller for verify wallet flow.
 * @details This controller is used to handle back button events during wallet verification of newly added wallet.
 *
 * @see verify_wallet_controller(), verify_wallet_tasks(), generate_wallet_controller(), tasks_add_new_wallet(),
 * restore_wallet_controller(), tasks_restore_wallet()
 * @since v1.0.0
 */
void verify_wallet_controller_b();

/**
 * @brief Sync all the available wallets on the cards with the device.
 * @details This controller is used to sync all the available wallets on the cards with the device. The sync is done
 * one wallet at a time.
 *
 * @see sync_wallet_controller_b(), sync_cards_task()
 * @since v1.0.0
 */
void sync_cards_controller();

/**
 * @brief Back button controller for sync wallet flow.
 * @details This controller is used to handle back button events during sync wallet flow.
 *
 * @see sync_cards_controller(), sync_cards_task()\
 * @since v1.0.0
 */
void sync_cards_controller_b();

/**
 * @brief The function handles post-processing successful execution of card pairing on the card.
 * @details The function verifies the signature of the card's response received. Upon successful verification, the
 * shared secret is generated and stored in the device for further use in secure communication with the card.
 *
 * @param [in] card_number       - card number currently being processed
 * @param [in] session_nonce     - session nonce of the device
 * @param [in] card_pairing_data - card pairing data received from the card
 */
void handle_pair_card_success(uint8_t card_number, uint8_t *session_nonce, uint8_t *card_pairing_data);

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
 * @brief   fetch device serial and check if UID in the serial matches MCU UID or not
 * @details
 *
 * @return uint32_t device serial fetch status or failure status
 * @retval 0    fetched successfully
 * @retval 1    device UID doesn't match with serial UID
 */
uint32_t get_device_serial();

/**
 * @brief Request ATECC to generate signature on the hash with private available on SLOT-3
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
 * @brief Request ATECC to perform ECDH operation on pub_key with private key from SLOT-3
 * @details
 *
 * @param [in] pub_key          - public key to be used for ECDH
 * @param [out] shared_secret   - shared secret generated
 *
 * @return    ATCA_SUCCESS on success, otherwise an error code.
 *
 * @see atcab_init(), atcab_ecdh(), atcab_ecdh_ioenc(), ATCAIfaceCfg, cfg_atecc608a_iface
 * @since v1.0.0
 */
uint8_t atecc_nfc_ecdh(const uint8_t *pub_key, uint8_t *shared_secret);

/**
 * @brief Updates the state variables to jump to the swap txn flow
 */
void jump_to_swap();

#endif