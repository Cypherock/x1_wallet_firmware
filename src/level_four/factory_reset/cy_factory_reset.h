/**
 * @file    cy_factory_reset.h
 * @author  Cypherock X1 Team
 * @brief   Header for factory reset task.
 *          This file contains all the functions prototypes for the factory reset task.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef CY_FACTORY_RESET_H
#define CY_FACTORY_RESET_H

#include "controller_main.h"

extern uint8_t factory_reset_mismatch_wallet_index;

void cyc_factory_reset(void);

void cyt_factory_reset(void);

#endif // CY_FACTORY_RESET_H
