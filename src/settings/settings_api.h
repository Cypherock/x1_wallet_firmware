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

/**
 * @brief This function configures the X1 vault to enable/disable exporting of
 * log data to the host
 *
 */
void toggle_log_export(void);

/**
 * @brief This function configures the X1 vault to enable/disable usage of
 * passphrase on the X1 vault
 *
 */
void toggle_passphrase(void);

/**
 * @brief This function configures the X1 vault to switch between left and right
 * handed view
 *
 */
void rotate_display(void);

/**
 * @brief This function displays regulatory information of the X1 vault.
 *
 */
void view_device_regulatory_information(void);
#endif /* SETTINGS_API_H */
