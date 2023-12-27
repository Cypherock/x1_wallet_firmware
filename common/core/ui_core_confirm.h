/**
 * @file    ui_core_confirm.h
 * @author  Cypherock X1 Team
 * @brief   A ready-made confirmation UI that accepts rejection callback.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef UI_CORE_CONFIRM_H
#define UI_CORE_CONFIRM_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "pb.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef void(ui_core_rejection_cb)(pb_size_t which_error, uint32_t error_code);

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief The function confirms the user intention for a certain action
 * specified in the message
 * @details The function will render a confirmation screen and listen for
 * events. The function will only listen to an UI event and handles UI and P0
 * event. In case of a P0 event, the function will simply return false and do an
 * early exit. In case if the user denied the permission by selecting cancel,
 * the function executes the provided rejection callback if it exists to the
 * host app.
 *
 * @param body Reference to the message to display against user confirmation
 * @param reject_cb Callback to execute if user rejected
 *
 * @return bool Indicating if the user confirmation succeeded.
 * @retval true The user confirmed his/her intention
 * @retval false The user either rejected the prompt or a P0 event occurred
 */
bool core_confirmation(const char *body, ui_core_rejection_cb *reject_cb);

/**
 * @brief The function confirms the user intention for a certain action
 * specified in the message
 * @details The function will render a scroll-page screen and listen for
 * events. The function will only listen to an UI event and handles UI and P0
 * event. In case of a P0 event, the function will simply return false and do an
 * early exit. In case if the user denied the permission by selecting cancel,
 * the function executes the provided rejection callback if it exists to the
 * host app.
 *
 * @param title Reference to the title for the screen
 * @param body Reference to the message to display against user confirmation
 * @param reject_cb Callback to execute if user rejected
 *
 * @return bool Indicating if the user confirmation succeeded.
 * @retval true The user confirmed his/her intention
 * @retval false The user either rejected the prompt or a P0 event occurred
 */
bool core_scroll_page(const char *title,
                      const char *body,
                      ui_core_rejection_cb *reject_cb);

#endif
