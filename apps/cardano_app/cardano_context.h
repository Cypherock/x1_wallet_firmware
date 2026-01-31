/**
 * @file    cardano_context.h
 * @author  Cypherock X1 Team
 * @brief   Common header defs for cardano app
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARDANO_CONTEXT_H
#define CARDANO_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define CARDANO_NAME "Cardano"
#define CARDANO_LUNIT "Ada"

#define CARDANO_IMPLICIT_ACCOUNT_DEPTH 5

#define CARDANO_PURPOSE_INDEX (0x80000000 + 1852)    // 1852'
#define CARDANO_COIN_INDEX (0x80000000 + 1815)       // 1815'
#define CARDANO_ACCOUNT_INDEX (0x80000000 + 0)       // 0'
#define CARDANO_CHANGE_INDEX_PAYMENT 0               // 0
#define CARDANO_CHANGE_INDEX_STAKE 2                 // 2

/*
 * These macros define the position of specific index in the derivation path
 * root/purpose'/coin_type'/account'/change/index
 *        ^ 0        ^ 1      ^ 2      ^ 3   ^ 4
 */
#define CARDANO_PURPOSE_INDEX_POS 0
#define CARDANO_COIN_INDEX_POS 1
#define CARDANO_ACCOUNT_INDEX_POS 2
#define CARDANO_CHANGE_INDEX_POS 3
#define CARDANO_WALLET_INDEX_POS 4

#define CARDANO_STAKE_ADDR_LENGTH 60       // 59 should work
#define CARDANO_PAYMENT_ADDR_LENGTH 105    // 103 should work

#define CARDANO_PUBLIC_KEY_SIZE 32
#define CARDANO_HASHED_TXN_SIZE 32

#define CARDANO_STAKE_ADDR_DRV_CONST 0xe0
#define CARDANO_NETWORK_ID_TESTNET 0x02
#define CARDANO_NETWORK_ID_MAINNET 0x01
#define CARDANO_NETWORK_ID CARDANO_NETWORK_ID_MAINNET

#define STAKE_BECH32_PREFIX "stake"
#define PAYMENT_BECH32_PREFIX "addr"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif    // CARDANO_CONTEXT_H
