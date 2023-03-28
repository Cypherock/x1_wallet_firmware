/**
 * @file    flash_if.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assert_conf.h"
#include "board.h"

/// Default 8-bit value on persistent memory
#define DEFAULT_VALUE_IN_FLASH 0xff

/// Default 32-bit value on persistent memory
#define DEFAULT_UINT32_IN_FLASH 0xffffffff

/**
 * @brief Erase specified pages on the persistent memory region
 * @details
 *
 * @param [in] addr         Start address of the first page to erase
 * @param [in] pages_cnt    Number of pages to erase
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void erase_cmd(uint32_t addr, uint32_t pages_cnt);

/**
 * @brief Write specified pages on the persistent memory region
 * @details
 *
 * @param [in] addr     Start address of the first page to start writing
 * @param [in] data     Data that is to be written
 * @param [in] len      Length of data to write
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void write_cmd(uint32_t addr, const uint32_t *data, uint32_t len);

/**
 * @brief Read specified pages on the persistent memory region
 * @details
 *
 * @param addr Start address of the first page to start reading
 * @param data Data that is to be read
 * @param len Length of data to read
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void read_cmd(uint32_t addr, uint32_t *source_addr, uint32_t len);

#endif /* __FLASH_IF_H */
