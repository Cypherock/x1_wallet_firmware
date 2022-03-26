#ifndef _SIM_APPLET_

#include "board.h"
#include "apdu.h"
#include "adafruit_pn532.h"

ret_code_t applet_read(uint8_t* buffer, uint8_t size);
ret_code_t applet_write(uint8_t* buffer, uint8_t size);

#endif