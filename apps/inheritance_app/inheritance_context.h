/**
 * @file    inheritance_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the INHERITANCE app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INHERITANCE_CONTEXT_H
#define INHERITANCE_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define INHERITANCE_MESSAGES_MAX_COUNT 5
#define INHERITANCE_PACKET_MAX_SIZE 6000
#define INHERITANCE_PIN_TAG 0x50

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum inheritance_message_type {
  INHERITANCE_DEFAULT_MESSAGE = 0x00,
  INHERITANCE_ONLY_SHOW_ON_DEVICE = 0x50,
} inheritance_message_type_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* INHERITANCE_CONTEXT_H */
