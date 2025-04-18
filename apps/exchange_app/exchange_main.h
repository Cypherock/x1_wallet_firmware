/**
 * @file    exchange_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various Exchange actions
 supported.
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef EXCHANGE_MAIN_H
#define EXCHANGE_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"
#include "composable_app_queue.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef enum EXCHANGE_FLOW_TAGS {
  EXCHANGE_FLOW_TAG_RECEIVE = 0x1,
  EXCHANGE_FLOW_TAG_FETCH_SIGNATURE,
  EXCHANGE_FLOW_TAG_STORE_SIGNATURE,
  EXCHANGE_FLOW_TAG_SEND,
} exchange_flow_tag_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Returns the config for Exchange chain app descriptors
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_exchange_app_desc();

bool exchange_app_validate_caq(caq_node_data_t data);
#endif /* EXCHANGE_MAIN_H */
