/**
 * @file    wallet_selector.h
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef WALLET_SELECTOR_H
#define WALLET_SELECTOR_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <inttypes.h>
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
 * @brief
 *
 * @param index
 * @return true
 * @return false
 */
bool select_wallet_with_index(uint8_t index);

/**
 * @brief
 *
 * @return const char*
 */
const char *selected_wallet_get_name(void);

/**
 * @brief
 *
 * @return const uint8_t*
 */
const uint8_t *selected_wallet_get_wallet_id(void);

/**
 * @brief
 *
 * @return const uint8_t
 */
const uint8_t selected_wallet_get_state(void);

/**
 * @brief
 *
 * @return const uint8_t
 */
const uint8_t selected_wallet_is_locked(void);

/**
 * @brief
 *
 * @return const uint8_t
 */
const uint8_t selected_wallet_is_partial(void);

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool selected_wallet_has_pin(void);

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool selected_wallet_has_passphrase(void);

#endif /* WALLET_SELECTOR_H */
