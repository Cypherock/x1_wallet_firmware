/**
 * @file    app_registry.c
 * @author  Cypherock X1 Team
 * @brief   App descriptor registry for maintaining apps.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"

#include <stddef.h>
#include <string.h>

#include "assert_conf.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static const cy_app_desc_t *descriptors[REGISTRY_MAX_APPS] = {0};
static uint16_t app_desc_count = 0;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Compares values of two cy_app_desc_t instances.
 *
 * @param a_desc References to first descriptor
 * @param b_desc Reference to second descriptor
 * @return Whether the descriptors match.
 * @retval true If the provided descriptor values match.
 * @retval false If the provided descriptor values do not match.
 */
bool compare_descriptors(const cy_app_desc_t *a_desc,
                         const cy_app_desc_t *b_desc);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

bool compare_descriptors(const cy_app_desc_t *a_desc,
                         const cy_app_desc_t *b_desc) {
  bool status = true;
  status &= (a_desc->id == b_desc->id);
  status &= (a_desc->version == b_desc->version);
  status &= (strncmp(a_desc->name, b_desc->name, REGISTRY_APP_NAME_SIZE) == 0);

  return status;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool registry_add_app(const cy_app_desc_t *app_desc) {
  bool status = false;

  if (NULL == app_desc) {
    return status;
  }

  // ensure app-id collision safety among descriptor list
  if (NULL != registry_get_app_desc(app_desc->id)) {
    return status;
  }

  // ensure registry storage does not overflow
  ASSERT(app_desc_count < REGISTRY_MAX_APPS);

  descriptors[app_desc_count - 1] = app_desc;
  app_desc_count++;
  status = true;
  return status;
}

bool registry_find_app_desc(const cy_app_desc_t *app_desc) {
  bool status = false;
  const cy_app_desc_t *desc = registry_get_app_desc(app_desc->id);

  if (NULL == desc) {
    return status;
  }

  // descriptor with same app-id is found
  status = compare_descriptors(app_desc, desc);

  return status;
}

const cy_app_desc_t *registry_get_app_desc(uint32_t app_id) {
  const cy_app_desc_t *desc = NULL;

  for (int desc_idx = 0; desc_idx < app_desc_count; desc_idx++) {
    if (descriptors[desc_idx]->id == app_id) {
      desc = descriptors[desc_idx];
    }
  }

  return desc;
}

const cy_app_desc_t **registry_get_app_list(uint16_t *app_count) {
  if (NULL != app_count) {
    *app_count = app_desc_count;
  }

  return &descriptors[0];
}