/**
 * @file    mem_config.h
 * @author  Cypherock X1 Team
 * @brief   Flash Memory map.
 *          All address for section defined in memory map are defined in this
 * file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef __MEM_CONFIG_H
#define __MEM_CONFIG_H

/**  ***Firewall defines*** */
#define FIREWALL_CODE_SEGMENT_ADDR                                             \
  0x08009004                                 /// CODE segment addr: 0x08009004
#define FIREWALL_CODE_SEGMENT_SIZE 0xE000    /// CODE segment size: 0x0E000
#define FIREWALL_NVDATA_SEGMENT_ADDR                                           \
  0x08017000    /// NVData segment addr: 0x08017000
#define FIREWALL_NVDATA_SEGMENT_SIZE 0x2000    /// NVData segment size: 0x2000

/** ***Firewall NVDATA segment usage*** */
#define FIREWALL_BOOT_FLAGS_SECTOR_ADDR                                        \
  0x08017000    /// BOOT marker page addr: 0x08017000
#define FIREWALL_BACKUP_PAGE_ADDR                                              \
  0x08017800    /// BOOT marker backup addr: 0x08017800
#define FIREWALL_NVDATA_APP_KEYS_ADDR                                          \
  0x08018000    /// Firewall perm keys Page 1(FW P1) addr: 0x08018000
#define FIREWALL_FIXED_NOERASE_START_ADDR FIREWALL_NVDATA_APP_KEYS_ADDR
#define FIREWALL_FIXED_NOERASE_END_ADDR                                        \
  FIREWALL_NVDATA_APP_KEYS_ADDR + FLASH_PAGE_SIZE - 1
#define FIREWALL_NVDATA_WALLET_DATA_ADDR                                       \
  0x08018800    /// Firewall User data Page 2(FW P2) addr: 0x08018800
#define BOOT_FLAGS_SECTOR_ADDR_BASE FIREWALL_BOOT_FLAGS_SECTOR_ADDR
#define BOOT_FLAGS_SECTOR_ADDR ((__IO uint64_t *)BOOT_FLAGS_SECTOR_ADDR_BASE)

/** ***Application Flash Usage*** */
#define FLASH_DATA_ADDRESS (0x08019000)        /// 0x08019000
#define FLASH_DATA_END_ADDRESS (0x0801CFFF)    /// 0x0801cfff
#define FLASH_DATA_SIZE_LIMIT (FLASH_DATA_END_ADDRESS - FLASH_DATA_ADDRESS)
#define FLASH_DATA_LOGGER_ADDRESS (0x0801D000)    /// Logger Address 0x0801D000
#define FLASH_DATA_LOGGER_MAX_PAGES 12
#define FLASH_DATA_LOGGER_PAGE_SIZE 0x800    /// Logger page size - 0x800

/** ***Firmware Start Address*** */
#define APPLICATION_ADDRESS_BASE                                               \
  (0x08023000)    /// Firmware start address: 0x08022000
#define APPLICATION_ADDRESS ((__IO uint64_t *)APPLICATION_ADDRESS_BASE)
#define MAX_APPLICATION_FILE_SIZE                                              \
  ((0x08100000) - (APPLICATION_ADDRESS_BASE))    /// Max storage which can be
                                                 /// allocated for Firmware

#define FIRMWARE_START_ADDRESS                                                 \
  APPLICATION_ADDRESS_BASE    ///< firmware boot address

#define OTP_HARDWARE_VERSION_ADDRESS 0x1FFF7000
#define OTP_BOOTLOADER_VERSION_ADDRESS 0x1FFF7004

#if (DEV_BUILD == 1)
#define MAX_PUBLIC_KEYS 8
#else
#define MAX_PUBLIC_KEYS 4
#endif

#endif