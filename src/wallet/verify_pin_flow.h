/**
 * @author  Cypherock X1 Team
 * @brief   Header file for the pin verification flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef VERIFY_PIN_FLOW_H
#define VERIFY_PIN_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "ui_input_text.h"
#include "wallet.h"
#include "wallet_list.h"

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
 * @brief This API executes the wallet pin verification flow on the device
 * and returns the pin.
 * @details This function takes user inputs based wallet configuration
 * corresponding to wallet with wallet_id, verifies the pin against any 1 X1
 * card, and returns it. The function informs the host in case of any early exit
 * or card abort errors
 *
 * @param wallet_id The wallet_id of the wallet which needs to be verify the pin
 * @param pin_out Pointer to buffer where the pin will be copied after
 * verification
 * @param reject_cb Callback to execute if there is any rejection during PIN
 * input occurs, or a card abort error occurs
 * @return true If the verification flow completed successfully and buffer
 * pointed by pin_out is filled with the seed
 * @return false If the verification flow could not be completed
 */
bool verify_pin(const uint8_t *wallet_id,
                uint8_t pin_out[MAX_PIN_SIZE],
                rejection_cb *reject_cb);

#endif /* VERIFY_PIN_FLOW_H */
