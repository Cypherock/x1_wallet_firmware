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

/**
 * @brief This function executes manual pairing operation between X1 card and X1
 * vault
 *
 */
void pair_x1_cards(void);

/**
 * @brief This function displays the version details of an X1 card
 *
 */
void view_card_version(void);

/**
 * @brief This function implements factory reset flow for the X1 vault. If there
 * are valid wallets present in device flash memory, the user will be prompted
 * to tap two X1 cards and wallet list will be verified before performing the
 * factory reset. ALL data will be cleared.
 *
 */
void factory_reset(void);

/**
 * @brief This function implements card health check for an X1 card. It will
 * show user facing messages if there are any card errors and additionally will
 * show wallet list present in the X1 card to the user.
 *
 */
void card_health_check(void);

/*
 * @brief This function implements flow which allows user to sync wallets
 * between an X1 Vault device and 2 X1 Cards.
 *
 */
void sync_with_cards(void);

/*
 * @brief This function implements clear user data flow for the X1 vault. If
 * there are valid wallets present in device flash memory, the user will be
 * prompted to tap two X1 cards and wallet list will be verified before
 * performing the factory reset. Onboarding data will NOT be cleared.
 */
void clear_device_data(void);

#endif /* SETTINGS_API_H */
