/**
 * @file    settings_api.h
 * @author  Cypherock X1 Team
 * @brief   Header file which exports functionalities to the settings menu
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SETTINGS_API_H
#define SETTINGS_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This function displays the currently installed firmware version and
 * the bootloader version of the X1 vault device
 *
 */
void view_firmware_version(void);

#endif /* SETTINGS_API_H */
