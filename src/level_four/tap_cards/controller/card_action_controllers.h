/**
 * @file    card_action_controllers.h
 * @author  Cypherock X1 Team
 * @brief   Header for card action controllers.
 *          This file contains the declaration of the card action controllers.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef CONTROLLER_CARD_ACTIONS
#define CONTROLLER_CARD_ACTIONS
#include <stdint.h>

void readback_share_from_card(uint8_t xcor);

int verify_card_share_data();

#endif