/**
 * @file    array_list_type.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef ARRAY_LIST_TYPE_H
#define ARRAY_LIST_TYPE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  void *array; /** <Pointer to start of array buffer */
  size_t
      size_of_element;  /** <Size of one record held by the array list, this
                           field is added to make the implementation generic */
  uint8_t max_capacity; /** <This is an internal field, which stores the count
                           of maximum elements which can be stored in the array
                           buffer */
  uint8_t num_of_elements; /**<This field counts the number of elements
                              currently in the array */
  uint8_t
      current_index; /**<This field stores the current index of the array list*/
} array_list_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* ARRAY_LIST_TYPE_H */
