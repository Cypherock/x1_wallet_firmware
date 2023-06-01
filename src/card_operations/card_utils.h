/**
 * @file    ${file_name}
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARD_UTILS_H
#define CARD_UTILS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "stdbool.h"

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
NFC_connection_data get_default_nfc_data(uint8_t *family_id,
                                         uint8_t acceptable_cards);

card_error_type_e wait_for_user_confirm(const char *error_message);

void get_card_serial(NFC_connection_data *nfc_data, uint8_t *serial);

card_error_type_e wait_for_card_removal(void);
#endif
