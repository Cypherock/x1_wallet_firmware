/**
 * @file    app_registry.h
 * @author  Cypherock X1 Team
 * @brief   App descriptor registry APIs for application to work with.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define REGISTRY_MAX_APPS 15
#define REGISTRY_APP_NAME_SIZE 20

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct cy_app_desc {
  const uint32_t id;
  const uint32_t version;

  const char name[REGISTRY_APP_NAME_SIZE];
} cy_app_desc_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Adds the app with provided information into the registry.
 * @details The function will ensure that the list is collision free. The
 * registry considers cy_app_desc_t.id as the unique identifier for every app
 * from the rest of the apps. The app descriptors can be registered during the
 * OS startup.
 *
 * @param app_desc Reference to cy_app_desc_t instance with app's description
 * @return Status of whether the app was added to the registry or not.
 * @retval true The provided app descriptor was successfully added to the list.
 * @retval false The provided app descriptor was not added to the registry. This
 * could be due to collision in the app-id which means another app has been
 * already defined with the same id. The failure could also arise due to invalid
 * reference to the descriptor.
 */
bool registry_add_app(const cy_app_desc_t *app_desc);

/**
 * @brief Looks for the specified app descriptor in the registry list.
 * @details The function first tries to match the app-id, then goes on to do
 * a compare of the other descriptor members.
 * The function only considers the app-id as a mandatory check irrespective of
 * whether strict check is passes or not, in other words the function searches
 * by app-id only.
 * The function performs a linear search in the list of app descriptors.
 *
 * @param app_desc Reference to cy_app_desc_t instance with app's description
 *
 * @return Status of the search operation.
 * @retval true If the app description match was found.
 * @retval false If the app descriptor match was not found or the provided
 * descriptor reference is invalid.
 */
bool registry_find_app_desc(const cy_app_desc_t *app_desc);

/**
 * @brief Finds and returns the descriptor of an app with provided app id.
 *
 * @param app_id The expected app-id to look for in the registry.
 * @return A const reference to the matched app descriptor.
 * @retval NULL If the provided app-id match is not found in the registry.
 * @retval `const cy_app_desc_t *` a valid reference to app_desc_t.
 */
const cy_app_desc_t *registry_get_app_desc(uint32_t app_id);

/**
 * @brief Provides the full list of registered app descriptors.
 *
 * @param app_count Modifiable reference to the count of entries in the list
 * reference.
 *
 * @return A const reference to the list of app descriptor references.
 */
const cy_app_desc_t **registry_get_app_list(uint16_t *app_count);

#endif
