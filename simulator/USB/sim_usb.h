#ifndef __SIM_USB_HEADER__
#define __SIM_USB_HEADER__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "communication.h"

typedef enum
{
    USBD_OK = 0U,
    USBD_BUSY,
    USBD_EMEM,
    USBD_FAIL,
} USBSIM_StatusTypeDef;

void SIM_USB_DEVICE_Init();
void SIM_Transmit_FS(uint8_t * data, uint8_t size);
void usbsim_continue_loop();

#endif