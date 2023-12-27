/**
 * @file    flash_api.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef FLASH_API_H
#define FLASH_API_H

#include "flash_if.h"
#include "flash_struct.h"
#include "sec_flash.h"

#define is_device_authenticated() (get_auth_state() == DEVICE_AUTHENTICATED)

#define is_passphrase_enabled() (get_enable_passphrase() == PASSPHRASE_ENABLED)

#define is_passphrase_disabled()                                               \
  (get_enable_passphrase() == PASSPHRASE_DISABLED ||                           \
   get_enable_passphrase() == DEFAULT_VALUE_IN_FLASH)

/**
 * @brief This API checks if a wallet exists at a particular index.
 * Optionally, if a wallet exists in that index, this API can return the status
 * of the wallet.
 *
 * @param index The index required to be checked for wallet existance
 * @param state_output The optional pointer of type wallet_state which will be
 * filled with the wallet_state in case the wallet exists on that index
 * @return true If the wallet exists at the index
 * @return false If the wallet does not exist at the index
 */
bool wallet_is_filled(uint8_t index, wallet_state *state_output);

/**
 * @brief The function checks if a wallet is filled with a share based on its
 * index.
 *
 * @param index The index required to be checked for wallet existance
 *
 * @return a boolean value. It returns true if the wallet at the given index is
 * filled with a share, and false otherwise.
 */
bool wallet_is_filled_with_share(uint8_t index);

/**
 * Update auth state and first_boot_on_update variables in firewall
 */
int set_auth_state(device_auth_state);

/**
 * @brief Adds wallet to flash
 *
 * Make sure this function only gets called if the card tapped was of the family
 * id that is there in the device (or if there are no wallets hence
 * DEFAULT_UINT32_IN_FLASH family id in the device)
 *
 * @param wallet a constant reference to an object of type Flash_Wallet
 * @param index_OUT index of wallet
 * @return SUCCESS,	MEMORY_OVERFLOW, INVALID_ARGUMENT, ALREADY_EXISTS
 * @retval SUCCESS wallet written to flash
 * @retval MEMORY_OVERFLOW no empty slots
 * @retval INVALID_ARGUMENT empty name or name > NAME_SIZE or state is not
 * @retval ALREADY_EXISTS wallet name already exists behavior
 * @retval VALID_WALLET writes a wallet to flash
 */
int add_wallet_to_flash(const Flash_Wallet *wallet, uint32_t *index_OUT);

/**
 * @brief Adds device's share to firewall region in flash
 *
 * @param[in] fwallet a constant reference to an object of type Flash_Wallet
 * @param[out] index_OUT index at which share entry is made
 * @param[in] wallet_share The 5th share of wallet to be written on device
 * @param[in] wallet_nonce Wallet nonce common for all shares
 * @return SUCCESS,	MEMORY_OVERFLOW, INVALID_ARGUMENT, ALREADY_EXISTS
 * @retval SUCCESS Wallet share written to firewall region
 * @retval MEMORY_OVERFLOW in case of no empty slots
 * @retval INVALID_ARGUMENT empty name or name > NAME_SIZE or state is not
 * @retval ALREADY_EXISTS Device's share entry already exists
 */
int add_wallet_share_to_sec_flash(const Flash_Wallet *fwallet,
                                  uint32_t *index_OUT,
                                  const uint8_t *wallet_share,
                                  const uint8_t *wallet_nonce);
/**
 * @brief Deletes a wallet from flash
 *
 * @param wallet_index index of the wallet (as in flash)
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS wallet written to flash
 * @retval INVALID_ARGUMENT: no wallet in given slot/ index greater than or
 * equal to MAX_WALLETS_ALLOWED
 */
int delete_wallet_from_flash(uint8_t wallet_index);

/**
 * @brief Deletes a wallet share from flash
 *
 * @param wallet_index index of the wallet (as in flash)
 * @param wallet_share_index index of the wallet share (as in flash)
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS wallet written to flash
 * @retval INVALID_ARGUMENT: no wallet in given slot/ index greater than or
 * equal to MAX_WALLETS_ALLOWED
 */
int delete_wallet_share_from_sec_flash(uint8_t wallet_index);

/**
 * @brief Sets family_id of the card set right now synced with device.
 * Can be only called if there are no wallets in device.
 *
 * @param family_id An array of bytes family id of size FAMILY_ID_SIZE
 * @return SUCCESS, ILLEGAL
 * @retval SUCCESS family id written
 * @retval ILLEGAL the device already has some wallets
 */
int set_family_id_flash(const uint8_t family_id[FAMILY_ID_SIZE]);

/**
 * @brief Tells if wallet is in partial state
 *
 * @param wallet_index Wallet index
 * @return true, false
 * @retval true Wallet is in partial state
 * @retval false Wallet is not in partial state
 */
bool is_wallet_partial(uint8_t wallet_index);

/**
 * @brief Tells if wallet is in unverified state
 *
 * @param wallet_index Wallet index
 * @return true, false
 * @retval true Wallet is in unverified state
 * @retval false Wallet is not in unverified state
 */
bool is_wallet_unverified(uint8_t wallet_index);

/**
 * @brief Tells if device's share is present in the firewall region
 * @param wallet_index Wallet index
 * @return true, false
 * @retval true Device's share is present in the firewall region
 * @retval false Device's share is not present in the firewall region
 */
bool is_wallet_share_not_present(uint8_t wallet_index);

/**
 * @brief Tells if wallet is in locked state
 *
 * @param wallet_index Wallet index
 * @return true, false
 * @retval true Wallet is in locked state
 * @retval false Wallet is not in locked state
 */
bool is_wallet_locked(uint8_t wallet_index);

/**
 * @brief Replaces(edits) the wallet in flash at given index. like PUT request
 * in REST.
 *
 * Update the contents of new wallet and reflect the wallet addition in
 * wallet_count of flash.
 *
 * @param index an index, a constant reference to an object of type flash_wallet
 * @param wallet Pointer to Flash_Wallet instance.
 * @return SUCCESS, INVALID_ARGUMENT, DOESNT_EXIST
 * @retval SUCCESS wallet written to flash
 * @retval INVALID_ARGUMENT empty name or name > NAME_SIZE
 * @retval ALREADY_EXISTS wallet name already exists
 */
int put_wallet_flash(uint8_t index, const Flash_Wallet *wallet);

/** @brief Save wallet share in flash
 *
 * @param index index of wallet
 * @param wallet_share a constant reference to an array of bytes
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS wallet share written to firewall region
 * @retval INVALID_ARGUMENT non-existent wallet reference or wallet_index >=
 * MAX_WALLETS_ALLOWED
 */
int put_wallet_share_sec_flash(uint8_t index,
                               const uint8_t *wallet_share,
                               const uint8_t *wallet_nonce);

/**
 * @brief Outputs the index of the wallet with given name
 *
 * @param name constant reference to name
 * @param index_OUT wallet index
 * @return SUCCESS, INVALID_ARGUMENT, WALLET_NOT_FOUND
 * @retval SUCCESS wallet with given name found in flash
 * @retval INVALID_ARGUMENT empty name or name > NAME_SIZE
 * @retval WALLET_NOT_FOUND wallet not found
 */
int get_index_by_name(const char *name, uint8_t *index_OUT);

/**
 * @brief Get the first matching wallet index by wallet id
 *
 * @param wallet_id Wallet id byte array
 * @param index_OUT wallet index in flash
 * @return SUCCESS, DOESNT_EXIST
 * @retval SUCCESS found an index for the id
 * @retval DOESNT_EXIST no index found for the id
 *
 */
int get_first_matching_index_by_id(const uint8_t wallet_id[WALLET_ID_SIZE],
                                   uint8_t *index_OUT);

/**
 * @brief Get the ith valid wallet index
 *
 * @param i ith wallet number
 * @param index_OUT wallet index in flash
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS found index for the ith valid wallet
 * @retval INVALID_ARGUMENT i is greater than wallet count
 */
int get_ith_valid_wallet_index(uint8_t i, uint8_t *index_OUT);

/**
 * @brief Get the ith valid wallet to export
 *
 * @param i ith wallet number
 * @param index_OUT wallet index in flash
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS found index for the ith valid wallet
 * @retval INVALID_ARGUMENT i is greater than wallet count
 */
int get_ith_wallet_to_export(uint8_t i, uint8_t *index_OUT);

/**
 * @brief Get the ith valid wallet share index
 * @param i ith wallet number
 * @param index_OUT wallet index in flash
 * @return SUCCESS, INVALID_ARGUMENT
 * @retval SUCCESS found index for the ith valid wallet share
 * @retval INVALID_ARGUMENT i is greater than wallet count
 */
int get_ith_wallet_without_share(uint8_t i, uint8_t *index_OUT);

/**
 * @brief Get the count of valid wallets present on device
 *
 * @return Count of valid wallets
 */
int get_valid_wallet_count();

/**
 * @brief Get the flash wallet by name
 *
 * @param name Name of the wallet
 * @param flash_wallet_OUT Pointer to pointer of Flash_Wallet instance
 * @return SUCCESS, INVALID_ARGUMENT, DOESNT_EXIST
 * @retval SUCCESS Wallet found
 * @retval INVALID_ARGUMENT Passed name invalid
 * @retval DOESNT_EXIST Wallet does not exist
 */
int get_flash_wallet_by_name(const char *name, Flash_Wallet **flash_wallet_OUT);

/**
 * @brief Get the flash wallet by wallet name
 * @param name Name of the wallet
 * @param wallet_share Pointer to wallet share byte array
 * @return SUCCESS, INVALID_ARGUMENT, DOESNT_EXIST
 * @retval SUCCESS Wallet found & wallet share returned
 * @retval INVALID_ARGUMENT Passed name is invalid
 * @retval DOESNT_EXIST Wallet does not exist with given name
 */
int get_flash_wallet_share_by_name(const char *name, uint8_t *wallet_share);

/**
 * Retrieves the wallet nonce associated with a given name from flash memory.
 *
 * @param name A pointer to a character array representing the name of the
 * wallet.
 * @param wallet_nonce A pointer to a uint8_t array where the wallet nonce will
 * be stored.
 *
 * @return SUCCESS, INVALID_ARGUMENT, DOESNT_EXIST
 * @retval SUCCESS Wallet found & wallet share returned
 * @retval INVALID_ARGUMENT Passed name is invalid
 * @retval DOESNT_EXIST Wallet does not exist with given name
 */
int get_flash_wallet_nonce_by_name(const char *name, uint8_t *wallet_nonce);

/**
 * @brief Update the card states(write and attempt states) for the wallet at
 * specified index (on deletion of the wallet from the given card number)
 *
 * @param index Wallet index in flash
 * @param card_number Card number to delete
 * @return SUCCESS
 */
int delete_from_kth_card_flash(uint8_t index, uint8_t card_number);

/**
 * @brief Tells if the wallet at specified index is already deleted from the
 * given card number
 *
 * @details This function checks the write state and attempt state of the wallet
 * to be deleted.
 *
 * @param index Wallet index in flash
 * @param card_number Card number to check
 * @return true, false
 * @retval true Card deleted
 * @retval false Card not deleted
 */
bool card_already_deleted_flash(uint8_t index, uint8_t card_number);

/**
 * @brief Update challenge in flash
 * @note This also resets stored nonce
 * @param name Wallet name
 * @param target Target
 * @param random_number Random number
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index
 * @retval SUCCESS Updated successfully
 */
int update_challenge_flash(const char *name,
                           const uint8_t target[SHA256_SIZE],
                           const uint8_t random_number[POW_RAND_NUMBER_SIZE]);

/**
 * The function sets a wallet as locked and assigns a card number in which the
 * wallet was locked.
 *
 * @param wallet_name A pointer to a string representing the name of the wallet.
 * @param encoded_card_number Encoded locked card number. This card number is
 * represented by the set bit in the lower nibble.
 *
 * @return an integer value. The possible return values are SUCCESS if the
 * wallet locking operation is successful, or an error code if there is an issue
 * with retrieving the flash wallet or saving the flash structure.
 */
int set_wallet_locked(const char *wallet_name, uint8_t encoded_card_number);

/**
 * @brief Add challenge to flash
 * @note This also resets stored nonce
 * @param name Wallet name
 * @param target Target
 * @param random_number Random number
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index
 * @retval SUCCESS Added successfully
 */
int add_challenge_flash(const char *name,
                        const uint8_t target[SHA256_SIZE],
                        const uint8_t random_number[POW_RAND_NUMBER_SIZE]);

/**
 * @brief Update the unlocked status of wallet and reset the nonce.
 *
 * @param name Wallet name in flash
 * @param is_wallet_locked wallet locked status
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index
 * @retval SUCCESS Status updated successfully
 */
int update_wallet_locked_flash(const char *name, bool is_wallet_locked);

/**
 * @brief Update the unlock time of locked wallet
 *
 * @param wallet_index Wallet index in flash
 * @param time_to_unlock_in_secs unlock time in seconds
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index
 * @retval SUCCESS Time updated successfully
 */
int update_time_to_unlock_flash(const char *name,
                                uint32_t time_to_unlock_in_secs);

/**
 * @brief Save the wallet nonce in flash
 *
 * @param wallet_index Wallet index in flash
 * @param nonce nonce byte array
 * @param time_to_unlock_in_secs probabalistic time left to unlock the wallet
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index
 * @retval SUCCESS Nonce saved successfully
 */
int save_nonce_flash(const char *name,
                     const uint8_t nonce[POW_NONCE_SIZE],
                     const uint32_t time_to_unlock_in_secs);

/**
 * @brief Set the wallet state in flash
 *
 * @param wallet_index Wallet index in flash
 * @param new_state Wallet state
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT Invalid index or state
 * @retval SUCCESS State set successfully
 */
int set_wallet_state(uint8_t wallet_index, wallet_state new_state);

/**
 * @brief Set the display rotation
 *
 * @param display_rotation RIGHT_HAND_VIEW, LEFT_HAND_VIEW
 * @param save true/false to save the changes so far to flash
 * @return SUCCESS_ Display rotation set successfully
 */
int set_display_rotation(display_rotation _rotation, flash_save_mode save_mode);

/**
 * @brief Set the enable passphrase field
 *
 * @param enable_passphrase byte value of field to set
 * @param save true/false to save the changes so far to flash
 * @return SUCCESS_ Display rotation set successfully
 */
int set_enable_passphrase(passphrase_config enable_passphrase,
                          flash_save_mode save_mode);

/**
 * @brief Get the io protection key from flash
 *
 * @param key byte array to store the io protection key
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT key is NULL
 * @retval SUCCESS Retrieve successful
 */
int get_io_protection_key(uint8_t *key);

/**
 * @brief Save the io protection key in flash
 *
 * @param key io protection key byte array
 * @return INVALID_ARGUMENT, SUCCESS
 * @retval INVALID_ARGUMENT key is NULL
 * @retval SUCCESS Save successful
 */
int set_io_protection_key(const uint8_t *key);

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
int set_ext_key(const Perm_Ext_Keys_Struct *ext_keys);

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
int get_paired_card_index(const uint8_t *card_key_id);

/**
 * @brief Invalidates the pairing of the card by setting the used flag to 0.
 */
void invalidate_keystore();

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
bool is_logging_enabled();

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
int set_logging_config(log_config state, flash_save_mode save_mode);

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
uint32_t get_first_boot_on_update();

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
device_auth_state get_auth_state();

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
uint32_t get_fwSize();

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
const uint8_t *get_perm_self_key_id();

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
const uint8_t *get_auth_public_key();

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
uint32_t get_boot_count();

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
uint32_t get_fwVer();

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
const uint8_t *get_perm_self_key_path();

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
const uint8_t *get_priv_key();

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
const uint8_t *get_card_root_xpub();

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
const display_rotation get_display_rotation();

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
const passphrase_config get_enable_passphrase();

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
const wallet_state get_wallet_state(uint8_t wallet_index);

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
const uint8_t get_wallet_card_state(uint8_t wallet_index);

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
const uint8_t *get_wallet_name(uint8_t wallet_index);

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
const uint8_t get_wallet_count();

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
const uint8_t get_wallet_info(uint8_t wallet_index);

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
const uint8_t *get_wallet_id(uint8_t wallet_index);

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
const uint8_t get_wallet_locked_status(uint8_t wallet_index);

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
const uint8_t *get_family_id();

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
const uint8_t get_wallet_card_locked(uint8_t wallet_index);

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
const uint32_t get_wallet_time_to_unlock(uint8_t wallet_index);

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
const Flash_Wallet *get_wallet_by_index(uint8_t wallet_index);
/**
 * @brief Returns if the requested keystore entry is used or empty.
 *
 * @return uint8_t  Usage status of the specified keystore entry index.
 * @retval 0        Keystore entry unused
 * @retval 1        Keystore entry used
 * @retval 255      Keystore entry index is invalid
 *
 * @see card_keystore
 * @since v1.0.0
 */
const uint8_t get_keystore_used_status(uint8_t keystore_index);

/**
 * @brief Returns number of used keystore entries.
 *
 * @return uint8_t   Number of used keystore entries
 *
 * @see card_keystore
 * @since v1.0.0
 */
const uint8_t get_keystore_used_count();

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
uint8_t set_keystore_pairing_key(uint8_t keystore_index,
                                 const uint8_t *pairing_key,
                                 uint8_t len,
                                 flash_save_mode save_mode);

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
const uint8_t *get_keystore_pairing_key(uint8_t keystore_index);

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
uint8_t set_keystore_key_id(uint8_t keystore_index,
                            const uint8_t *_key_id,
                            uint8_t len,
                            flash_save_mode save_mode);

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
uint8_t set_keystore_used_status(uint8_t keystore_index,
                                 uint8_t _used,
                                 flash_save_mode save_mode);

/**
 * @brief Delete all wallet data from the flash
 * @details The function deletes all wallet data from the flash and sets the
 * wallet count to 0. The other wallet data (such as family-id, device
 * configuration like display rotation, etc.) is not deleted.
 *
 * @see Flash_Wallet, Flash_Struct
 * @since v1.0.0
 */
void flash_delete_all_wallets();

/**
 * @brief It saves the onboarding step on the flash memory
 *
 * @param onboarding_step The value of step that needs to be stored
 */
void save_onboarding_step(const uint8_t onboarding_step);

/**
 * @brief Get the onboarding step value from the flash
 *
 * @return uint8_t The onboarding step
 */
uint8_t get_onboarding_step(void);

#endif