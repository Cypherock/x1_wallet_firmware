/**
 * @file    sec_flash_priv.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef SEC_FLASH_PRIV_H
#define SEC_FLASH_PRIV_H

#include "board.h"
#include "flash_config.h"
#include "flash_if.h"
#include "sec_flash.h"
#include "wallet.h"

/**
 * @brief Updates `auth_state` value in firewall boot marker page
 * @details On firmware update Bootloader updates `auth_state` variable to
 * DEVICE_NOT_AUTHENTICATED and `first_boot_on_update` is set. Updating
 * auth_state here also resets the first_boot_on_update
 *
 * @param [in] auth_state updated auth state value
 *
 * @retval SEC_TRUE if auth state updated successfully else SEC_FALSE
 *
 * @since v1.0.0
 */
extern uint32_t FW_update_auth_state(uint32_t auth_state);

#define SEC_TASK_UPDATE_BOOT_STATE                                             \
  0x0062    ///< Firewall task to update Bootloader state
#define SEC_TASK_UPDATE_AUTH_STATE                                             \
  0x0063    ///< Firewall task to update auth state variable
#define SEC_TASK_GET_BOOT_COUNT                                                \
  0x0064    ///< Firewall task to read device boot count

#define SEC_TASK_WRITE_NV_DATA                                                 \
  0x0071    ///< Firewall task to write permanent keys firewall page 1 (FW P1)
#define SEC_TASK_READ_NV_DATA                                                  \
  0x0072    ///< Firewall task to read permanent keys firewall page 1 (FW P1)
#define SEC_TASK_READ_BOOT_FLAGS                                               \
  0x0073    ///< Firewall task to read boot marker data

#define SEC_TASK_WRITE_APPLICATION_DATA                                        \
  0x0091    ///< Firewall task to write on firewall page 2 (FW P2)
#define SEC_TASK_READ_APPLICATION_DATA                                         \
  0x0092    ///< Firewall task to read from firewall page 2 (FW P2)
#define SEC_TASK_DELETE_APPLICATION_DATA                                       \
  0x0093    ///< Firewall task to delete firewall page 2 (FW P2)

#define SEC_TASK_APPLICATION_MASK 0x0100

#define SEC_TASK_CALCULATE_HASH_INIT 0x0101    ///< Firewall task to init SHA2
#define SEC_TASK_CALCULATE_HASH_UPDATE                                         \
  0x0102    ///< Firewall task to update SHA2
#define SEC_TASK_CALCULATE_HASH_FINAL                                          \
  0x0103    ///< Firewall task to get SHA2 digest
#define SEC_TASK_GET_HARDWARE_VERSION                                          \
  0x0105    ///< Firewall task to get hardware version
#define SEC_TASK_GET_BOOTLOADER_VERSION                                        \
  0x0106    ///< Firewall task to get bootloader version

#define FIREWALL_IO_PROTECTION_KEY_ADDR FIREWALL_NVDATA_APP_KEYS_ADDR + 8
#define FIREWALL_EXTERNAL_KEYS_ADDR                                            \
  FIREWALL_IO_PROTECTION_KEY_ADDR + IO_KEY_SIZE
#define FIREWALL_APPLICATION_DATA_START_ADDR FIREWALL_NVDATA_WALLET_DATA_ADDR

extern Sec_Flash_Struct
    sec_flash_instance;    ///< read/write instance for FW P2 data
extern Flash_Perm_Struct
    flash_perm_instance;    ///< instance for FW read-only data

extern bool is_flash_perm_instance_loaded;
extern bool is_sec_flash_ram_instance_loaded;

/**
 * @brief Get the secure flash ram instance.
 *
 * This function returns const because it is designed in such a way
 * that no other module can change values in the ram instance of Flash Struct.
 * Other modules can call this to read values but they will have to call other
 * functions if they need to modify values.
 *
 * @return const Sec_Flash_Struct*
 */
const Sec_Flash_Struct *get_sec_flash_ram_instance();

/**
 * @brief Get the flash perm instance.
 * This function returns const because it is designed in such a way
 * that no other module can change values in the ram instance of Flash Struct.
 * Other modules can call this to read values but they will have to call other
 * functions if they need to modify values.
 *
 * @return const Flash_Perm_Struct*
 */
const Flash_Perm_Struct *get_flash_perm_instance();

/**
 * @brief Save the io_protection_key member of `flash_perm_instance` to FW P1
 * @since v1.0.0
 */
void flash_perm_struct_save_IOProtectKey();

/**
 * @brief Save the ext_keys member of `flash_perm_instance` to FW P1
 *
 * @since v1.0.0
 */
void flash_perm_struct_save_ext_keys();

/**
 * @brief Save changes made to Sec_Flash_struct instance to firewall.
 * @details Used to save sensitive data in firewall flash
 *
 * @since v1.0.0
 */
void sec_flash_struct_save();

#endif