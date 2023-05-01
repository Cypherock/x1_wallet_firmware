/**
 * @file    array_list.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
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
/**
 * @brief Initialize the array list and it's fields based on input parameters.
 *
 * @param list Pointer to the array list instance
 * @param array_buffer Pointer to the static buffer which needs to be maintained
 * by the array list
 * @param size_of_element Size of each element in bytes which will be stored in
 * the array. This field can NOT be 0 bytes.
 * @param capacity Capacity in bytes of the array buffer, which will be used to
 * calculate the maximum number of elements which can be stored
 * @return true If the initialization was successful
 * @return false If the initialization in not successful, in case of bad
 * arguments.
 */
bool array_list_initialize(array_list_t *const list,
                           void *array_buffer,
                           size_t size_of_element,
                           size_t capacity);

/**
 * @brief This API inserts element of size size_of_element pointed by the
 * element pointer into the static buffer maintained by the corresponding array
 * list. Data of size size_of_element is copied to the buffer.
 *
 * @param list Pointer to the array list instance
 * @param element Pointer to the element which needs to be stored in the buffer
 * @return true If the insertion was done successfully
 * @return false If the insertion failed, it could be due to NULL arguments, or
 * the static buffer is full
 */
bool array_list_insert(array_list_t *list, void *const element);

/**
 * @brief This API gets the data of size size_of_elements into the buffer
 * pointed by element pointer at the current index of the array.
 *
 * @param list Pointer to the array list instance
 * @param element Pointer to buffer which will be filled by this API
 * @return true If the element was returned in the pointer
 * @return false If the element was not returned, it could be due to NULL
 * pointer, or the list is empty.
 */
bool array_list_get_element(array_list_t *list, void *const element);

/**
 * @brief This API increments the current_index of the list array.
 *
 * @param list Pointer to the array list instance
 * @return true If the increment of index occurred successfully
 * @return false If the increment could not happen, it could be due to NULL
 * pointer or the index is already at the end
 */
bool array_list_iterate_next(array_list_t *list);

/**
 * @brief This API decrements the current_index of the list array.
 *
 * @param list Pointer to the array list instance
 * @return true If the decrement of index occurred successfully
 * @return false If the increment could not happen, it could be due to NULL
 * pointer or the index is already at the start
 */
bool array_list_iterate_back(array_list_t *list);

/**
 * @brief This API deletes the element entry at the current index.
 *
 * @param list Pointer to the array list instance
 * @return true If the delete operation completed successfully
 * @return false If the delete operation did not complete, it could be due to
 * NULL pointer or the list is already empty
 */
bool array_list_delete_entry(array_list_t *list);

#endif /* ARRAY_LIST_H */
