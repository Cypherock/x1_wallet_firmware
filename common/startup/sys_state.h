/**
 * @file    sys_state.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef SYS_STATE_H
#define SYS_STATE_H

#include "stdint.h"

typedef union {
  struct {
    /*Bit 00*/ uint32_t event_present : 1;
    /*Bit 01*/ uint32_t reset_flow : 1;
    /*Bit 02*/ uint32_t reset_not_allowed : 1;
    /*Bit 03*/ uint32_t external_triggered : 1;
    /*Bit 04*/ uint32_t usb_buffer_free : 1;
    /*Bit 05*/ uint32_t nfc_off : 1;
    /*Bit 06*/ uint32_t app_restricted : 1;
    /*Bit 07*/ uint32_t unused_07 : 1;
    /*Bit 08*/ uint32_t unused_08 : 1;
    /*Bit 09*/ uint32_t unused_09 : 1;
    /*Bit 10*/ uint32_t unused_10 : 1;
    /*Bit 11*/ uint32_t unused_11 : 1;
    /*Bit 12*/ uint32_t unused_12 : 1;
    /*Bit 13*/ uint32_t unused_13 : 1;
    /*Bit 14*/ uint32_t unused_14 : 1;
    /*Bit 15*/ uint32_t unused_15 : 1;
    /*Bit 16*/ uint32_t unused_16 : 1;
    /*Bit 17*/ uint32_t unused_17 : 1;
    /*Bit 18*/ uint32_t unused_18 : 1;
    /*Bit 19*/ uint32_t unused_19 : 1;
    /*Bit 20*/ uint32_t unused_20 : 1;
    /*Bit 21*/ uint32_t unused_21 : 1;
    /*Bit 22*/ uint32_t unused_22 : 1;
    /*Bit 23*/ uint32_t unused_23 : 1;
    /*Bit 24*/ uint32_t unused_24 : 1;
    /*Bit 25*/ uint32_t unused_25 : 1;
    /*Bit 26*/ uint32_t unused_26 : 1;
    /*Bit 27*/ uint32_t unused_27 : 1;
    /*Bit 28*/ uint32_t unused_28 : 1;
    /*Bit 29*/ uint32_t unused_29 : 1;
    /*Bit 30*/ uint32_t unused_30 : 1;
    /*Bit 31*/ uint32_t unused_31 : 1;

  } bits;
  uint32_t byte;
} sys_flow_control_u_t;

void sys_state_init(void);

extern volatile sys_flow_control_u_t sys_flow_cntrl_u;

#define CY_Usb_Buffer_Free() (sys_flow_cntrl_u.bits.usb_buffer_free)
#define CY_Read_Reset_Flow() (sys_flow_cntrl_u.bits.reset_flow)
#define CY_Reset_Flow() (sys_flow_cntrl_u.bits.reset_flow = true)
#define CY_Reset_Not_Allow(a)                                                  \
  (sys_flow_cntrl_u.bits.reset_not_allowed = a > 0 ? true : false)
#define CY_reset_not_allowed() (sys_flow_cntrl_u.bits.reset_not_allowed)
#define CY_External_Triggered() (sys_flow_cntrl_u.bits.external_triggered)
#define CY_Set_External_Triggered(a)                                           \
  (sys_flow_cntrl_u.bits.external_triggered = a)
#define CY_set_app_restricted(a) (sys_flow_cntrl_u.bits.app_restricted = a)
#define CY_is_app_restricted() (sys_flow_cntrl_u.bits.app_restricted)

#endif