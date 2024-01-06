/**
 * @file    core_error.h
 * @author  Cypherock X1 Team
 * @brief   Core error handling apis
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CORE_ERROR_H
#define CORE_ERROR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>

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
 * @brief Set error message to be displayed to the user.
 *
 * @details The error message is copied to the internal buffer if the buffer was
 * empty. Error is displayed when @ref handle_core_errors function is called.
 *
 * @param error_msg The error description message to be shown to user
 * @param ring_buzzer Boolean to indicate if buzzer sound is required while
 * displaying the message
 */
void mark_core_error_screen(const char *error_msg, bool ring_buzzer);

/**
 * @brief The function checks for a p0 event and displays core errors if it has
 * not occurred.
 *
 * @return returns true p0 event flag is false and vice versa
 */
bool show_errors_if_p0_not_occured();

/**
 * @brief This function clears the message buffer of the core error screen
 *
 * @note It will potentially clear a message set by an operation in the flow.
 */
void clear_core_error_screen(void);
#endif
