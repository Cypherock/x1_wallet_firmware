/**
 * @file    reconstruct_wallet_flow.h
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

#include "wallet.h"
#include "wallet_list.h"

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
 * reconstruction. The function informs the host in case of any early exit or
 * card abort errors
 *
 * @param wallet_id The wallet_id of the wallet which needs to be reconstructed
 * @param seed_out Pointer to buffer where the seed will be copied after
 * regeneration
 * @param reject_cb Callback to execute if there is any rejection during PIN or
 * passphrase input occurs, or a card abort error occurs
 * @return true If the reconstruction flow completed successfully and buffer
 * pointed by seed_out is filled with the seed
 * @return false If the reconstruction flow could not be completed
 */
bool reconstruct_seed(const uint8_t *wallet_id,
                      uint8_t *seed_out,
                      rejection_cb *reject_cb);

/**
 * This function takes a wallet ID as input and a 2D array buffer to populate
 * mnemonics to. It reconstructs a secret using for the associated wallet, and
 * returns the number of mnemonics in the wallet.
 *
 * @param wallet_id A pointer to an array of uint8_t representing the wallet ID.
 * @param mnemonic_list A 2-dimensional buffer to store mnemonics to.
 *
 * @return number of nmemonics for the wallet for success, else 0
 */
uint8_t reconstruct_mnemonics(
    const uint8_t *wallet_id,
    char mnemonic_list[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH]);
#endif /* RECONSTRUCT_SEED_FLOW_H */
