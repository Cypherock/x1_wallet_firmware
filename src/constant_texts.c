/**
 * @file    constant_texts.c
 * @author  Cypherock X1 Team
 * @brief   Constant texts.
 *          Stores definitions of all the hardcoded user facing strings/texts.
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
#include "ui_menu.h"
#include "ui_text_slideshow.h"

const char* ui_text_options_main_menu[] = {
    "Main Menu",
    "Create Wallet",
    //"Add arbitrary data",
    "Settings"
};

// level two
const char* ui_text_options_old_wallet[] = {
    "View Seed",
    "Delete Wallet"
};

const char* ui_text_options_new_wallet[] = {
    "Create Wallet",
    "Generate New Wallet",
    "Restore from Seed"
};

const char* ui_text_options_advanced_settings[] = {
    "Settings",
    "Restore wallets from cards",
    "Check Card health",
    "Rotate Display",
    "Toggle log export",
    "Toggle Passphrase Step",
    "Factory Reset",
    "Device Info",
    "View Card Version",
#ifdef DEV_BUILD
    "Buzzer toggle",
#endif
};

const char* ui_text_options_buzzer_adjust[] = {
    "Toggle buzzer",
    "Disable",
    "Enable",
};

const char* u_text_passphrase_options[] = {
    "Disable Passphrase Step",
    "Enable Passphrase Step",
};

const char* ui_text_logging_export_options[] = {
    "Disable logs",
    "Enable logs",
};

// wallet creation
const char* ui_text_verification_is_now_complete_messages[] = {
    "Verification complete",
    "You may now safely wipe out any written seed phrase",
    "You'll need this device and a card to make transactions",
    "To get started export this wallet to the CySync app",
    "Press the center button to return to the main menu",
};

const char* ui_text_seed_phrase_will_be_shown_copy_to_verify[] = {
    "Write down the seed phrase on paper to verify",
    "Press the center button to view seed phrase"
};

const char* ui_text_startup_instruction_screen_1[] = {
    "Press the button on device to start"
};

const char* ui_text_startup_instruction_screen_2[] = {
    "This device provides 5 way joystick for navigation",
    "Toggle left or right to change options horizontally",
    "Toggle up or down to change options vertically",
    "Use centre click to select highlighted option",
    "If you are ready use centre click to proceed"
};


const char* ui_text_startup_instruction_screen_4[] = {
    "Now visit cypherock.com/gs",
    "and download the CySync app to set up the device"

};

const char* ui_text_mnemonics_number_options[] = {
    "12",
    "18",
    "24"
};

const char* ui_text_view_data = "View data";

//product id
const char *product_hash = "0x2ea340d9";

// errors
const char* ui_text_something_went_wrong = "Something went wrong! Retry";
const char* ui_text_something_went_wrong_contact_support_send_logs = "Something went wrong! Contact support";
const char* ui_text_already_have_maxi_wallets = "You already have maximum number of wallets";
const char* ui_text_aborted = "Aborted from\nthe CySync app";
const char* ui_text_no_wallet_found = "No wallet found";
const char* ui_text_operation_has_been_cancelled = "Operation has been cancelled!";
const char* ui_text_wallet_name_exists = "Name already exists Pick a different name";
const char* ui_text_pin_incorrect_re_enter = "Incorrect PIN!\nEnter correct pin";
const char* ui_text_wallet_with_same_mnemo_exists = "Wallet already present on device";
const char* ui_text_incorrect_mnemonics = "Incorrect mnemonics";
const char* ui_text_incorrect_choice_view_seed_again = "Incorrect choice!\nView the seed phrase again?";
const char* ui_text_xpub_not_found_add_coin_first = "Click OK on the CySync app to resync coins";
const char* ui_wallet_pin_instruction_1 = "In next step you can setup PIN for %s";
const char* ui_wallet_pin_instruction_2 = "If you forget it\nyou will lose access\nto this wallet";
const char* ui_wallet_passphrase_instruction_1 = "In next step you can setup Passphrase for %s";
const char* ui_wallet_passphrase_instruction_2 = "If you forget it\nyou will lose access\nto this wallet";
const char* ui_wallet_passphrase_instruction_3 = "Passphrase is recommended only for advanced users";
const char* ui_wallet_passphrase_instruction_4 = "You will be entering it each time you transact with this wallet";
const char* ui_text_invalid_transaction = "Invalid transaction";

// errors X1 Card flows
const char* ui_text_authentication_required = "Device authentication required";
const char* ui_text_start_auth_from_CySync = "Authenticate device from the CySync App";
const char* ui_text_invalid_card_contact_cypherock = "Authentication failed Contact Cypherock";
const char* ui_text_this_card_is_not_valid = "This card is invalid";
const char* ui_text_wrong_remaining_attempts = "Incorrect PIN!\n%d attempt(s) remaining";
const char* ui_text_wallet_doesnt_exists_on_this_card = "Wallet does not exist on this card";
const char* ui_text_wallet_verification_failed = "Wallet not created Proceeding for deletion";
const char* ui_text_no_response_from_desktop = "No response from the CySync app!\nTry again";

const char* ui_text_tap_a_card = "Tap any card";

const char* ui_text_invalid_card_tap_card[] = {
    "Invalid card\nTap Card #1",
    "Invalid card\nTap Card #2",
    "Invalid card\nTap Card #3",
    "Invalid card\nTap Card #4",
};

const char* ui_text_place_card_x_heading = "Place Card #%d";

const char* ui_text_place_card_wait_for_beep = "Do not lift until you hear %d beep sound";

const char* ui_text_device_authenticating[] = {
        "Device authenticating.",
        "Device authenticating..",
        "Device authenticating...",
        "Device authenticating....",
        "Device authenticating....."
};

const char* ui_text_device_provisioning[] = {
        "Device provisioning.",
        "Device provisioning..",
        "Device provisioning...",
        "Device provisioning....",
        "Device provisioning....."
};

const char* ui_text_tap_a_card_instruction1 = "This device interacts with cards using NFC";
const char* ui_text_tap_a_card_instruction2 = "Place and hold any card below the device to test NFC";
const char* ui_text_tap_x_4_cards = "Tap card #%d";

const char* ui_text_tap_1_2_cards = "Tap any card";
const char* ui_text_tap_2_2_cards = "Tap another card";
const char* ui_text_place_card_below = "Place card\nbelow the device";

const char* ui_text_wallet_with_same_name_already_exist_on_this_card = "Wallet name already exists";
const char* ui_text_wallet_id_collision = "Wallet already exists";
const char* ui_text_card_is_full = "Error!\nCard is full";

const char* ui_text_device_synced_with_card = "Device synced successfully with the card";
const char* ui_text_family_id_retrived = "Family ID retrieved";
const char* ui_text_family_id_mismatch = "Error! Use correct cards of the wallet";

const char* ui_text_wrong_wallet_is_now_locked = "Wrong!\nWallet is now locked";
const char* ui_text_hash_not_accepted_by_card = "Hash not accepted by card!";

const char* ui_text_is_currently_locked_wait_for = "%s is currently locked\nWait for %d %s";
const char* ui_text_is_currently_locked_wait_for_more_than_a_year = "%s is currently locked\nWait for more than a year";
const char* ui_text_in_process_to_unlock = "%s is in process to unlock...";
const char* ui_text_multiple_incorrect_attempts_may_block = "Multiple incorrect attempts may block %s";
const char* ui_text_do_not_detach_device = "Keep the device connected throughout the process";

const char* ui_text_tap_card_ap_d = "Tap card #%d";
const char* ui_text_correct_pin_is_now_unlocked = "Correct PIN! %s is now unlocked";

const char* ui_text_card_sending_apdu = "Sending APDU\n%d";
const char* ui_text_card_command_send_error = "Error occurred while sending to APDU to card";
const char* ui_text_card_update_done = "Card update done!!";
const char* ui_text_card_transaction_exception = "Operation failed on card (Tx Exp)";
const char* ui_text_card_out_of_boundary_exception = "Operation failed on card (OOB)";
const char* ui_text_card_null_pointer_exception = "Error! Invalid memory accessed";
const char* ui_text_card_crypto_exception = "Operation failed on card (Crypto Exp)";
const char* ui_text_card_invalid_apdu_length = "Operation failed on card (APDU len exp)";
const char* ui_text_card_invalid_tag_in_apdu = "Operation failed on card (Tag exp)";
const char* ui_text_unknown_error_contact_support = "Unknown card error! Contact support";
const char* ui_text_unauthenticated_device = "Device is unauthenticated";

// headings
const char* ui_text_enter_pin = "Enter PIN";
const char* ui_text_confirm_pin = "Confirm PIN";
const char* ui_text_word_hash = "Word # ";
const char* ui_text_enter_word_hash = "Enter Word # %d";
const char* ui_text_verify_word_hash = "Verify Word # ";
const char* ui_text_is_word = "Is Word #%d\n%s";
const char* ui_text_enter_wallet_name = "Enter wallet name";
const char* ui_text_number_of_words = "Number of words";
const char* ui_text_add_coins = "Add Coin #";
const char* ui_text_resync_coins = "Resync Coin #";
const char* ui_text_choose_wallet = "Choose Wallet";
const char* ui_text_enter_passphrase = "Enter passphrase";
const char* ui_text_confirm_passphrase = "Confirm passphrase";
const char* ui_text_receive_on = "Receive on";
const char* ui_text_verify_address = "Verify address";
const char* ui_text_verify_contract = "Verify contract";
const char* ui_text_unverified_contract = "Warning!\nUnverified contract";
const char* ui_text_verify_amount = "Verify amount\n%s\n%s";
const char* ui_text_confirm_wallet_name = "Confirm wallet name";
const char* ui_text_enter_data = "Enter data";
const char* ui_text_confirm_data = "Confirm data";
const char* ui_text_verification_cancelled = "Verification cancelled";
const char* ui_text_verify_nonce = "Verify nonce";
const char* ui_text_waiting_for_desktop = "Follow the\ninstructions on\nthe CySync app";

// headings near specific
const char* ui_text_verify_create_from = "Verify Create from";
const char* ui_text_verify_new_account_id = "Verify New Account Id";
const char* ui_text_new_account_id= "new_account_id";
const char* ui_text_new_public_key= "new_public_key";
const char* ui_text_replace_account= "Replace Account";
const char* ui_text_confirm_account = "Confirm Account";

// headings X1 Card flow
const char* ui_text_family_id_hex = "F. Id (Hex)";

// messages
const char* ui_text_use_passphrase_question = "Do you want to use passphrase with this wallet?";
const char* ui_text_wallet_name_size_limit = "Wallet name cannot exceed 15 letters";
const char* ui_text_check_cysync = "Check the CySync app";
const char* ui_text_tap_x1card_auth_instruction_1 = "Tap a card and do not lift until you hear 2 beep sound";
const char* ui_text_tap_x1card_auth_instruction_2 = "Now tap the same card again";
const char* ui_text_fetching_recipient_amount = "Fetching recipient amount...";
const char* ui_text_tap_card_to_start = "Tap a card to start";
const char* ui_text_wallet_deleted_successfully = "Wallet deleted successfully!";
const char* ui_text_recovery_successfull = "Recovery Successful!";
const char* ui_text_generating_seed = "Processing...";
const char* ui_text_seed_generated_successfully = "Seed generated successfully";
const char* ui_text_wallet_synced_with_x1cards = "Wallet synced with cards!";
const char* ui_text_wallet_created_successfully = "Wallet created successfully!";
const char* ui_text_press_enter_to_start_verification = "Proceed to verify wallet on cards";
const char* ui_text_now_enter_your_seed_phrase = "Now enter your\nseed phrase";
const char* ui_text_now_enter_your_data = "Now enter your\ndata";
const char* ui_text_verify_entered_words = "Verify the entered words";
const char* ui_text_generating_xpubs = "Adding coins...";
const char* ui_text_exporting_to_desktop = "Exporting to the CySync app...";
const char* ui_text_successfully_exported_xpub_to_desktop = "Coins added successfully to the CySync app!!";
const char* ui_text_wait_while_balance_fetching = "Wait while balance is fetched on the CySync app";
const char* ui_text_deriving_address = "Deriving address...";
const char* ui_text_receive_on_address = "%s";
const char* ui_text_sending_confirmation_to_desktop = "Sending confirmation to the CySync app...";
const char* ui_text_success_received_from_desktop = "Address received successfully from the CySync app";
const char* ui_text_fetching_unsigned_transaction = "Fetching transaction data...";
const char* ui_text_unsigned_transaction_received = "Unsigned transaction received";
const char* ui_text_signing_transaction = "Signing transaction...";
const char* ui_text_exported_signed_transaction_to_desktop = "Exported signed transaction to the CySync app!!";
const char* ui_text_start_card_update = "Start card update?\nThis will format the card";
const char* ui_text_start_verification_of_card = "Start verification of card";
const char* ui_text_wallet_lock_continue_to_unlock = "Wallet is locked Proceed to unlock";
const char* ui_text_wallet_partial_continue_to_delete = "Wallet in partial\nstate, proceed to\ndelete";
const char* ui_text_wallet_not_verified_continue_to_verify = "Wallet not verified Proceed to verify";
const char* ui_text_wallet_out_of_sync_continue_to_sync_with_x1cards = "Proceed to Sync\nwith cards";
const char* ui_text_wallet_not_verified_prompt = "'%s' is in unverified state";
const char* ui_text_wallet_out_of_sync_prompt = "'%s' is out of Sync with cards";
const char* ui_text_wallet_partial_state_prompt = "'%s' is in partial delete state";
const char* ui_text_wallet_locked_prompt = "'%s' is in locked state";
const char* ui_text_wallet_visit_to_verify = "Visit the wallet to fix it";
const char* ui_text_wallet_partial_fix = "Wallets in partial state Fix before proceeding";
#ifdef ALLOW_LOG_EXPORT
const char* ui_text_sending_logs = "Sending logs...";
const char* ui_text_logs_sent = "Logs sent";
#endif
const char* ui_text_creation_failed_delete_wallet = "Wallet creation failed\nProceed to delete";

const char* ui_text_successfull = "Successful";
const char* ui_text_view_seed_messages[] = {
    "Do not give the seed phrase to anyone",
    "You will lose your coins if the seed phrase gets stolen",
    "Press the center button to proceed"
};
const char* ui_text_card_authentication_success = "Card authentication successful";
const char* ui_text_card_authentication_failed = "Card authentication failed";
const char* ui_text_enable_passphrase_step = "Do you want to enable passphrase step on wallet creation?";
const char* ui_text_disable_passphrase_step = "Do you want to disable passphrase step on wallet creation?";
const char* ui_text_warning_transaction_fee_too_high = "WARNING!\nTransaction fees\ntoo high, proceed?";
const char* ui_text_enable_log_export = "Do you want to enable logging?";
const char* ui_text_disable_log_export = "Do you want to disable logging?";

//?
const char* ui_text_do_you_want_import_wallet_to_desktop = "Do you want to import wallet to the CySync app?";
const char* ui_text_do_you_want_to_add_coins_to = "Do you want to add coins to %s?";
const char* ui_text_do_you_want_to_resync_coins_to = "Do you want to resync coins to %s?";
const char* ui_text_send_transaction_with = "Send %s with %s";
const char* ui_text_eth_send_transaction_with = "Send %s with %s on %s";
const char* ui_text_near_add_account_with= "Add %s account with %s";
const char* ui_text_recv_transaction_with = "Receive %s with %s";
const char* ui_text_eth_recv_transaction_with = "Receive %s with %s on %s";
const char* ui_text_do_you_want_to_set_pin = "Do you want to set a PIN?";
const char* ui_text_output_send_value = "Receiver #%d\nSend %f\n%s";
const char* ui_text_output_send_to_address = "Receiver #%d Address";

const char* ui_text_send_transaction_fee = "Transaction fee\n%s\n%s";

const char* ui_text_output_send_value_double = "Receiver #%d\nSend %0.*f\n%s";
const char* ui_text_send_transaction_fee_double = "Transaction fee\n%0.*f\n%s";

const char* ui_text_start_firmware_update = "Update firmware to version %d.%d.%d";

const char* ui_text_firmware_update_process = "Updating device...";
const char* ui_text_auth_process = "Waiting for device authentication...";

const char* ui_text_sync_x1card_confirm = "Device will be synced with wallets in card Confirm?";
const char* ui_text_rotate_display_confirm = "Display will rotate 180 degrees\nConfirm?";
const char* ui_text_factory_reset_confirm[] = {
    "You will need 2 cards to perform factory reset",
    "Make sure at least 2 cards are accessible",
    "Press the center button to proceed",
};

const char* ui_text_card_seems_healthy = "Card seems healthy";
const char* ui_text_click_to_view_wallets = "Click Ok to view wallets";
const char* ui_text_no_wallets_fetched = "No Wallets fetched\nClick Ok to return";
const char* ui_text_no_wallets_present = "No Wallets Present in card";
const char* ui_text_wallets_in_card = "Wallets in Card #%d";
const char* ui_text_card_health_check_start = "Tap a card to check it's health";
const char* ui_text_card_health_check_error[] = {
    "Card error: ",
    "Visit cypherock.com/error-codes for more info"
};
const char* ui_text_confirm_factory_reset = "This will erase all wallets from device Confirm?";
const char* ui_text_device_verification_success = "Device successfully authenticated";
const char* ui_text_device_verification_failure = "Device authentication failed";
const char* ui_text_unauthenticate_via_cysync = "Authenticate the device via the CySync app";
const char* ui_text_start_device_verification = "Start device authentication?";
const char* ui_text_its_a_while_check_your_cards = "It's been a while check if your cards are working properly";
const char* ui_text_delete_this_wallet = "Delete this wallet";
const char* ui_text_need_all_x1cards_to_delete_wallet_entirely = "You'll need all cards to delete this wallet entirely, confirm?";
const char* ui_text_20_spaces = "                    ";
const char* ui_text_wallet_not_found_on_x1card = "No Wallet found on card";
const char* ui_text_sync_wallets_next_steps = "Sync each wallet in next steps";
const char* ui_text_corrupted_card_contact_support = "Corrupted card! Contact support";
const char* ui_text_incompatible_card_version = "Incompatible card version! Contact support";
const char* ui_text_card_error_contact_support = "Contact support & share the device logs";
const char* ui_text_device_and_card_not_paired = "This device & card are not paired";
const char* ui_text_security_conditions_not_met = "Security conditions not met!";
const char* ui_text_process_reset_due_to_inactivity = "Operation aborted due to inactivity";
const char* ui_text_device_already_provisioned = "Device already provisioned";
const char* ui_text_check_cysync_app = "Check the CySync App";
const char* ui_text_provision_success = "Device provision successful";
const char* ui_text_provision_fail = "Device provision unsuccessful";
const char* ui_text_cannot_verify_card_contact_support = "Couldn't verify card! Contact support";
const char* ui_text_wrong_btc_transaction = "Wrong transaction formatting";
const char* ui_text_btc_change_address_mismatch = "Change address mismatch";
const char* ui_text_worng_eth_transaction = "Invalid transaction detected";
const char* ui_text_error_no_card_paired = "No card paired with device";
const char* ui_text_error_pair_all_cards = "Pair all four cards first";
const char* ui_text_error_pair_atleast_2_cards = "Pair at least two cards to restore";
const char* ui_text_pair_card_x = "Tap Card #%d to pair";
const char* ui_text_card_pairing_skipped_count = "%d card pairing skipped, pair all cards for proper use";
const char* ui_text_card_pairing_success = "Card pairing successful";
const char* ui_text_continue_with_pairing = "Proceed with pairing?";
const char* ui_text_device_compromised = "Device is compromised! Contact support";
const char* ui_text_device_compromised_not_provisioned = "Device is not provisioned";
const char* ui_text_device_compromised_partially_provisioned = "Device partially provisioned";
const char* ui_text_device_compromised_v1_config = "Device provisioned with v1 config";
const char* ui_text_pow_challenge_failed = "Wallet unlock failed! Retry";
const char* ui_text_card_removed_fast = "Too fast, tap again and wait for buzzer sound";
const char* ui_text_card_freq_discon_fault = "Frequent disconnections detected!\nContact support";
const char* ui_text_card_align_with_device_screen = "Hold the card and align with the device screen";
const char* ui_text_remove_card_prompt = "Process complete Remove card";
const char* ui_text_card_detected = "Card detected Processing...";
const char* ui_text_nfc_hardware_fault_detected = "NFC antenna fault detected! Contact Support";
const char* ui_text_unreliable_cards = "Warning, your cards are unreliable! Contact support";
const char* ui_critical_card_health_migrate_data = "Card health is critical! Migrate to new set of cards";
const char* ui_text_reset_exit[] = {
    "Wallet '%s' not found on card(s)",
    "Exiting without factory reset",
    "Press the center button to proceed",
};
const char* ui_text_erasing = "Factory reset done Device will restart";
const char* ui_text_performing_checks = "Performing checks...";

#ifdef ALLOW_LOG_EXPORT
const char* ui_text_send_logs_prompt = "Send logs to the CySync app?";
#endif
