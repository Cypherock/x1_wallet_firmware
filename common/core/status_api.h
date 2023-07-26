/**
 * @file    status_api.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef STATUS_API_H
#define STATUS_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <common.pb.h>
#include <core.pb.h>
#include <stddef.h>

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
 * @brief This API sets the device_idle_state field of the core_status_t status
 * packet. Important thing to note here is that this API must be called from
 * within the firmware and not from any applet
 *
 * @param idle_state Value of type core_device_idle_state_t to be set
 */
void core_status_set_idle_state(core_device_idle_state_t idle_state);

/**
 * @brief This API sets the device_waiting_on field of the core_status_t status
 * packet.
 *
 * @param waiting_on Value of type core_device_waiting_on_t to be set
 */
void core_status_set_device_waiting_on(core_device_waiting_on_t waiting_on);

/**
 * @brief This API sets the 2nd byte of flow_status field in the
 * core_status_t status packet. This API can be called by the application based
 * on it's requirements.
 *
 * @param status Value of the 2nd byte of flow_status field to be conveyed to
 * the host. First 1 byte will be masked from the 4 byte input.
 */
void set_core_flow_status(uint32_t status);

/**
 * @brief This API sets the lowest (1st) byte of flow_status field in the
 * core_status_t status packet. This API can be called by the application based
 * on it's requirements.
 *
 * @param status Value of the 1st byte of flow_status field to be conveyed to
 * the host. First 1 byte will be masked from the 4 byte input.
 */
void set_app_flow_status(uint32_t status);

/**
 * @brief This API checks if aborts are enabled in the current flow.
 *
 * @return true If aborts are enabled in the current flow
 * @return false If aborts are not enabled
 */
bool core_status_get_abort_disabled(void);

/**
 * @brief This API returns the core status of type core_status_t, which depicts
 * the status of the firmware and the application. This API is designed so that
 * the communication layer can request the core for the current status at any
 * given point in time. An important thing to note here is that fields
 * current_cmd_seq and cmd_state of the struct core_status_t are known only to
 * the communication layer. Therefore, status_api is not responsible for filling
 * those fields. The communication layer will fill these fields by itself.
 *
 * @return core_status_t
 */
core_status_t get_core_status(void);

/**
 * @brief Returns the current active app's applet-id
 *
 * @return uint32_t Applet id that is currently active
 */
uint32_t get_applet_id(void);

#endif /* STATUS_API_H */
