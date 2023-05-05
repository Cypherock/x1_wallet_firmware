/**
 * @file    array_list.c
 * @author  Cypherock X1 Team
 * @brief
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
#include "array_list.h"

#include <string.h>

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

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function checks if the array_list maintained by the pointer is
 * full or not
 *
 * @param list Pointer to the list currently in use
 * @return true If the array_list is full
 * @return false If the array_list is NOT full
 */
static bool is_full(array_list_t *list);

/**
 * @brief This function checks if the array_list maintained by the pointer is
 * empty or not
 *
 * @param list Pointer to the list currently in use
 * @return true If the array_list is empty
 * @return false If the array_list is NOT empty
 */
static bool is_empty(array_list_t *list);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool is_full(array_list_t *list) {
  if (list->num_of_elements == list->max_capacity) {
    return true;
  }

  return false;
}

static bool is_empty(array_list_t *list) {
  if (0 == list->num_of_elements) {
    return true;
  }

  return false;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool array_list_insert(array_list_t *list, void *const element) {
  if ((NULL == list) || (NULL == element)) {
    return false;
  }

  /* If we have fully filled the capacity of the array buffer, we cannot fill
   * any more datastart */
  if (true == is_full(list)) {
    return false;
  }

  uint8_t *const start_index =
      list->array + (list->size_of_element * list->num_of_elements);

  memcpy(start_index, element, list->size_of_element);

  list->num_of_elements += 1;
  return true;
}

bool array_list_get_element(array_list_t *list, void *const element) {
  if ((NULL == list) || (NULL == element)) {
    return false;
  }

  /* Only return data if the array list is non empty at the moment */
  if (true == is_empty(list)) {
    return false;
  }

  /* Find the starting address of the element: p_start = p_array_start +
   * (size_of_elements * current_index) and copy size_of_element bytes from it
   * to the destination element pointer */
  uint8_t *const start_index =
      list->array + (list->size_of_element * list->current_index);

  memcpy(element, start_index, list->size_of_element);

  return true;
}

bool array_list_iterate_next(array_list_t *list) {
  if ((NULL == list) || (true == is_empty(list))) {
    return false;
  }

  /* Check if we are at the end of the array list already. Keep in mind that the
   * current_index is following 0 indexing */
  if (list->current_index == (list->num_of_elements - 1)) {
    return false;
  }

  list->current_index += 1;
  return true;
}

bool array_list_iterate_back(array_list_t *list) {
  if ((NULL == list) || (true == is_empty(list))) {
    return false;
  }

  /* Check if we are at the start of the array list already */
  if (list->current_index == 0) {
    return false;
  }

  list->current_index -= 1;
  return true;
}

bool array_list_delete_entry(array_list_t *list) {
  if ((NULL == list) || (true == is_empty(list))) {
    return false;
  }

  /* The elements succeeding the current index needs to be shifted to ensure
   * that the data held in the array list is contiguos even after the deletion
   */
  for (uint8_t ele = list->current_index; ele < list->num_of_elements - 1;
       ele++) {
    uint8_t *const src = list->array + (list->size_of_element * (ele + 1));
    uint8_t *const dest = list->array + (list->size_of_element * ele);
    memcpy(dest, src, list->size_of_element);
  }

  /* If the current index is 0, then, we will not be able to decrement the index
   * further */
  if (list->current_index > 0) {
    list->current_index -= 1;
  }

  /* Finally, decrement the count of elements held in the array list */
  list->num_of_elements -= 1;
  return true;
}