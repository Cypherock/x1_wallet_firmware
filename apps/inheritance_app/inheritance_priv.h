/**
 * @file    inheritance_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for inheritance app internal operations
 *          This file is defined to separate INHERITANCE's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef INHERITANCE_PRIV_H
#define INHERITANCE_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <inheritance/core.pb.h>
#include <stdint.h>

#include "inheritance_context.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
} inheritance_encryption_context_t;

/**
 * @brief Handler for inheritance messages encrytion flow.
 * @details The expected request type is INHERITANCE_QUERY_ENCRYPT_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for encrytion of inheritance messages.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void inheritance_encrypt_data(inheritance_query_t *query);
#endif /* INHERITANCE_PRIV_H */
