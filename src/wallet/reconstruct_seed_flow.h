/**
 * @file    reconstruct_seed_flow.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the reconstruct wallet seed flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef RECONSTRUCT_SEED_FLOW_H
#define RECONSTRUCT_SEED_FLOW_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

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
 * @brief This API executes the wallet seed reconstruction flow on the device
 * and returns the seed.
 * @details This function takes user inputs based wallet configuration
 * corresponding to wallet with wallet_id, reads the wallet shares from the X1
 * vault flash and any 1 X1 card, and reconstructs each seed using Shamir
 * reconstruction.
 *
 * @param wallet_id The wallet_id of the wallet which needs to be reconstructed
 * @param seed_out Pointer to buffer where the seed will be copied after
 * regeneration
 * @return true If the reconstruction flow completed successfully and buffer
 * pointed by seed_out is filled with the seed
 * @return false If the reconstruction flow could not be completed
 */
bool reconstruct_seed_flow(const uint8_t *wallet_id, uint8_t *seed_out);

#endif /* RECONSTRUCT_SEED_FLOW_H */
