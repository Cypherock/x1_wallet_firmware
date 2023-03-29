/**
 * @file    flash_struct_priv.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef FLASH_STRUCT_PRIV_H
#define FLASH_STRUCT_PRIV_H

#include "flash_struct.h"

extern Flash_Struct flash_ram_instance;
extern bool is_flash_ram_instance_loaded;

/**
 * @brief Get the flash ram instance.
 * @details
 * This function returns const because it is designed in such a way
 * that no other module can change values in the ram instance of Flash Struct.
 * Other modules can call this to read values but they will have to call other
 * functions if they need to modify values.
 *
 * @private
 *
 * @return const Flash_Struct*
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
const Flash_Struct *get_flash_ram_instance();

/**
 * @brief Save changes made to Flash_struct instance to flash.
 * @details
 *
 * @private
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
void flash_struct_save();

#endif