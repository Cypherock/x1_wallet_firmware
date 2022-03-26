/**
 * @file    pow.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 *  
 * "Commons Clause" License Condition v1.0
 *  
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *  
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *  
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *  
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */
#include "pow.h"
#include "pow_utilities.h"
#include "board.h"
#include "application_startup.h"

/**
 * @note Every byte array is assumed to be in big endian form
 */

static uint8_t nonce[POW_NONCE_SIZE], hash[SHA256_SIZE];
static bool status;
static SHA256_CTX sha2;
static Flash_Wallet* flash_wallet; // Pointer to wallet which the device is currently trying to unlock
extern Wallet wallet;
//APP_TIMER_DEF(pow_timer_id);

/**
 * @brief Proof of work timer handler
 * @details This is called periodically after POW_TIMER_MS milliseconds.
 * Purpose :
 *
 * 1. Saves the current nonce in flash
 * This ensures that the progress in cracking the challenge is saved in flash.
 * If a user re-plugs the device his progress won't be lossed.
 *
 * 2. Update the approx time left to unlock in flash
 * The approx time to unlock is calculated when the challenge is fetched from card.
 * This timer decreases its value.
 *
 * @param p_context Used by app_timer module
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void pow_timer_handler()
{
    // Defining them static so that memory does not get allocated on stack after every call
    static char new_text[MAX_NUM_OF_CHARS_IN_A_SLIDE];
    static uint32_t new_time_to_unlock_in_secs;

    new_time_to_unlock_in_secs = flash_wallet->challenge.time_to_unlock_in_secs - (uint32_t)(POW_TIMER_MS / 1000);

    if (new_time_to_unlock_in_secs > flash_wallet->challenge.time_to_unlock_in_secs) {
        // This means time has become negative
        new_time_to_unlock_in_secs = 0;
    }

    save_nonce_flash((char *) flash_wallet->wallet_name, nonce);
    update_time_to_unlock_flash((char *) flash_wallet->wallet_name, new_time_to_unlock_in_secs);
    convert_secs_to_time(new_time_to_unlock_in_secs, (char *) wallet.wallet_name, new_text);
    ui_text_slideshow_change_text(new_text, strlen(new_text), 0);
}

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static bool hash_smaller_than_target()
{
    return memcmp(flash_wallet->challenge.target, hash, SHA256_SIZE) > 0;
}

void start_proof_of_work_task(const char* name)
{
    if (get_flash_wallet_by_name(name, &flash_wallet) != SUCCESS_)
        return;

    // Set nonce = nonce in flash
    memcpy(nonce, flash_wallet->challenge.nonce, POW_NONCE_SIZE);
    status = true;

    sha256_Init(&sha2);

    BSP_App_Timer_Create(BSP_POW_TIMER, pow_timer_handler);
   // APP_ERROR_CHECK(err_code);
    BSP_App_Timer_Start(BSP_POW_TIMER, POW_TIMER_MS);
    log_hex_array("hash", hash, sizeof(hash));
    log_hex_array("nonce", nonce, sizeof(nonce));
    log_hex_array("target", flash_wallet->challenge.target, sizeof(flash_wallet->challenge.target));
}

void stop_proof_of_work_task()
{
    status = false;
    log_hex_array("hash", hash, sizeof(hash));
    log_hex_array("nonce", nonce, sizeof(nonce));
    log_hex_array("target", flash_wallet->challenge.target, sizeof(flash_wallet->challenge.target));
    BSP_App_Timer_Stop(BSP_POW_TIMER);
}

void proof_of_work_task()
{
    reset_inactivity_timer();
    if (status) {
        uint16_t limit = SECS_TO_HASHES(1); // The counter will run for x secs if limit = SECS_TO_HASHES(x)
        //        uint32_t ticks = app_timer_cnt_get();

        //        printf("Starting Hash\n");
        for (uint16_t counter = 0; counter < limit; counter++) {
            // TODO : Use hardware accelerated SHA256 here
            sha256_Init(&sha2); // To reset SHA, A better way can be searched to reset
            sha256_Update(&sha2, flash_wallet->challenge.random_number, POW_RAND_NUMBER_SIZE);
            sha256_Update(&sha2, nonce, POW_NONCE_SIZE);
            sha256_Final(&sha2, hash);

            if (hash_smaller_than_target()) {
                stop_proof_of_work_task();
                mark_event_over();
                break;
            }

            increament_byte_array(nonce, POW_NONCE_SIZE);
        }
    }
}

uint8_t* get_proof_of_work_nonce()
{
    return &nonce[0];
}
