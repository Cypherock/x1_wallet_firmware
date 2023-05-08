#ifndef __SIM_USB_HEADER__
#define __SIM_USB_HEADER__

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "communication.h"

/**
 * @brief Enum to be used by application to identify interface from which data
 * was recieved and interface to which, data should be sent.
 */
typedef enum {
  COMM_LIBUSB__UNDEFINED = 0,
  COMM_LIBUSB__CDC,
  COMM_LIBUSB__HID,
  COMM_LIBUSB__WEBUSB,
} comm_libusb__interface_e;

typedef enum {
  USBD_OK = 0U,
  USBD_BUSY,
  USBD_EMEM,
  USBD_FAIL,
} USBSIM_StatusTypeDef;

void SIM_USB_DEVICE_Init();
void SIM_Transmit_FS(uint8_t *data, uint8_t size);
void usbsim_continue_loop();

#endif