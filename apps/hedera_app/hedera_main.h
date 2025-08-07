/**
 * @file    hedera_main.h
 * @author  Cypherock X1 Team
 * @brief   Header for the main entry point of the Hedera app.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#ifndef HEDERA_MAIN_H
#define HEDERA_MAIN_H

#include "app_registry.h"
#include "events.h"

// Returns the config for the Hedera app descriptor.
const cy_app_desc_t *get_hedera_app_desc();

#endif /* HEDERA_MAIN_H */