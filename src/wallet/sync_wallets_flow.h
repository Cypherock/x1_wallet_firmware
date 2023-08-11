/**
 * @file    sync_wallets_flow.h
 * @author  Cypherock X1 Team
 * @brief
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SYNC_WALLETS_FLOW
#define SYNC_WALLETS_FLOW

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "wallet.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum {
  SYNC_PIN_INPUT,
  SYNC_TAP_CARD_FLOW,
  SYNC_RECONSTRUCT_SEED,
  SYNC_COMPLETED,
  SYNC_COMPLETED_WITH_ERRORS,
  SYNC_TIMED_OUT,
  SYNC_EARLY_EXIT,
  SYNC_EXIT,
} sync_state_e;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This flow syncs the wallet in X1 Vault with those present in X1 Cards.
 * @details The flow takes PIN input based on wallet configuration and executes
 * card flow to fetch atleast threshold shares. If shares are received
 * successfully, the device share is generated and the wallet is marked valid.
 *
 * @param wallet_id Pointer to buffer containing the wallet ID of the wallet
 * that needs to be synced on the X1 Vault
 * @return sync_state_e Final state of the flow
 * It could be of of the following
 * SYNC_COMPLETED: If the wallet was synced successfully
 * SYNC_COMPLETED_WITH_ERRORS: If the wallet could not be synced due to error in
 * card flow. It could be either CARD_OPERATION_ABORT or
 * CARD_OPERATION_LOCKED_WALLET
 * SYNC_TIMED_OUT: If the flow timed out due to user inactivity
 * SYNC_EARLY_EXIT: If the user tries to exit the flow (by pressing back button
 * on PIN input)
 */
sync_state_e sync_wallets_flow(const uint8_t *wallet_id);

#endif /* SYNC_WALLETS_FLOW */
