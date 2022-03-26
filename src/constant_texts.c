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
    "Restore From Seed"
};

const char* ui_text_options_advanced_settings[] = {
    "Settings",
    "Restore wallets from CyCards",
    "Rotate Display",
    "Toggle log export",
    "Toggle Passphrase Step",
    "Factory Reset",
    "Device Info",
    "View CyCard Version",
#ifdef DEBUG_BUILD
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
    "You'll need X1Wallet and one CyCard to transact",
    "To backup wallets on new device you need any two CyCards",
    "To transact, export this wallet to the CySync app",
    "Press the center button to continue"
};

const char* ui_text_seed_phrase_will_be_shown_copy_to_verify[] = {
    "Write down the seed phrase on paper to verify",
    "Press the center button to view seed phrase"
};

const char* ui_text_startup_instruction_screen_1[] = {
    "Press the button on device to start"
};

const char* ui_text_startup_instruction_screen_2[] = {
    "This device provides 5 way joystick for screen navigation",
    "Toggle left or right to change options horizontally",
    "Toggle up or down to change options vertically",
    "Use centre click to select highlighted option",
    "If you are ready use centre click to proceed"
};


const char* ui_text_startup_instruction_screen_4[] = {
    "Now visit cypherock.com/gs",
    "And download CySync desktop application to set up the device"

};

const char* ui_text_verification_failed_initiating_delete_wallet_flow[] = {
    "Verification failed",
    "Now we will delete wallet and try again",
    "Press the center button to continue"
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
const char* ui_text_already_have_maxi_wallets = "Your already have\nmaximum number of wallets";
const char* ui_text_aborted = "Aborted from desktop";
const char* ui_text_no_wallet_found = "No wallet found";
const char* ui_text_operation_has_been_cancelled = "Operation has been cancelled!";
const char* ui_text_wallet_name_exists = "Name already exists. Pick a different name.";
const char* ui_text_pin_incorrect_re_enter = "PIN incorrect\nplease re-enter";
const char* ui_text_wallet_with_same_mnemo_exists = "Wallet already present on device";
const char* ui_text_incorrect_mnemonics = "Incorrect mnemonics";
const char* ui_text_incorrect_choice_view_seed_again = "Incorrect choice!\nView the seed phrase again?";
const char* ui_text_xpub_not_found_add_coin_first = "Click OK on CySync to resync coins";
const char* ui_wallet_pin_instruction_1 = "In next step you can setup PIN for %s";
const char* ui_wallet_pin_instruction_2 = "If you forget it, you will lose access to this wallet";
const char* ui_wallet_passphrase_instruction_1 = "In next step you can setup Passphrase for %s";
const char* ui_wallet_passphrase_instruction_2 = "If you forget it, you will lose access to this wallet";
const char* ui_wallet_passphrase_instruction_3 = "Passphrase is recommended only for advanced users";
const char* ui_wallet_passphrase_instruction_4 = "You will be entering it each time you transact with this wallet";
const char* ui_text_invalid_transaction = "Invalid transaction";

// errors cycard flows
const char* ui_text_invalid_card_contact_cypherock = "Authentication failed. Contact Cypherock.";
const char* ui_text_this_card_is_not_valid = "This CyCard is invalid";
const char* ui_text_wrong_remaining_attempts = "Incorrect PIN! Remaining %d attempts";
const char* ui_text_wallet_doesnt_exists_on_this_card = "Wallet does not exist on this CyCard";
const char* ui_text_wallet_verification_failed = "Wallet not created. Proceeding to delete it.";
const char* ui_text_no_response_from_desktop = "No response from desktop! Try again";

const char* ui_text_tap_a_card = "Tap any CyCard";

const char* ui_text_invalid_card_tap_card[] = {
    "Invalid CyCard. Tap the red CyCard.",
    "Invalid CyCard. Tap the blue CyCard.",
    "Invalid CyCard. Tap the green CyCard.",
    "Invalid CyCard. Tap the yellow CyCard."
};

const char* ui_text_place_card_wait_for_three_beep[] = {
    "Place red CyCard on device. Do not lift until you hear 3 beep sound",
    "Place blue CyCard on device. Do not lift until you hear 3 beep sound",
    "Place green CyCard on device. Do not lift until you hear 3 beep sound",
    "Place yellow CyCard on device. Do not lift until you hear 3 beep sound"
};

const char* ui_text_place_card_wait_for_two_beep[] = {
    "Place red CyCard on device. Do not lift until you hear 2 beep sound",
    "Place blue CyCard on device. Do not lift until you hear 2 beep sound",
    "Place green CyCard on device. Do not lift until you hear 2 beep sound",
    "Place yellow CyCard on device. Do not lift until you hear 2 beep sound"
};

const char* ui_text_place_card_wait_for_one_beep[] = {
    "Place red CyCard on device. Do not lift until you hear 1 beep sound",
    "Place blue CyCard on device. Do not lift until you hear 1 beep sound",
    "Place green CyCard on device. Do not lift until you hear 1 beep sound",
    "Place yellow CyCard on device. Do not lift until you hear 1 beep sound"
};

const char* ui_text_device_authenticating[] = {
        "Device authenticating .",
        "Device authenticating ..",
        "Device authenticating ...",
        "Device authenticating ....",
        "Device authenticating ....."
};

const char* ui_text_device_provisioning[] = {
        "Device provisioning .",
        "Device provisioning ..",
        "Device provisioning ...",
        "Device provisioning ....",
        "Device provisioning ....."
};

const char* ui_text_tap_a_card_instruction1 = "This device interacts with CyCards using NFC";
const char* ui_text_tap_a_card_instruction2 = "Place any card above or below the device to test card tapping";
const char* ui_text_tap_1_4_cards = "Please tap the red CyCard";
const char* ui_text_tap_2_4_cards = "Please tap the blue CyCard";
const char* ui_text_tap_3_4_cards = "Please tap the green CyCard";
const char* ui_text_tap_4_4_cards = "Please tap the yellow CyCard";

const char* ui_text_tap_1_2_cards = "Tap any CyCard";
const char* ui_text_tap_2_2_cards = "Now tap any other CyCard";

const char* ui_text_wallet_with_same_name_already_exist_on_this_card = "Wallet name already exists";
const char* ui_text_wallet_id_collision = "Wallet already exists";
const char* ui_text_card_is_full = "Error. CyCard is full.";

const char* ui_text_device_synced_with_card = "Device synced successfully with the CyCard";
const char* ui_text_family_id_retrived = "Family ID retrieved";
const char* ui_text_family_id_mismatch = "Error! Use correct cards of the wallet.";

const char* ui_text_wrong_wallet_is_now_locked = "Wrong! Wallet is now locked.";
const char* ui_text_hash_not_accepted_by_card = "Hash not accepted by CyCard!";

const char* ui_text_is_currently_locked_wait_for = "%s is currently locked. Wait for %d %s";
const char* ui_text_is_currently_locked_wait_for_more_than_a_year = "%s is currently locked. Wait for more than a year.";
const char* ui_text_in_process_to_unlock = "%s is in process to unlock...";
const char* ui_text_multiple_incorrect_attempts_may_block = "Multiple incorrect attempts may block %s";
const char* ui_text_do_not_detach_device = "Keep the device connected throughout the process";

const char* ui_text_tap_card_ap_d = "Tap %s CyCard";
const char* ui_text_correct_pin_is_now_unlocked = "Correct PIN! %s is now unlocked";

const char* ui_text_card_sending_apdu = "Sending APDU\n%d";
const char* ui_text_card_command_send_error = "Error occurred while sending to APDU to CyCard";
const char* ui_text_card_update_done = "CyCard update done!!";
const char* ui_text_card_transaction_exception = "Operation failed on CyCard (Tx Exp)";
const char* ui_text_card_out_of_boundary_exception = "Operation failed on CyCard (OOB)";
const char* ui_text_card_null_pointer_exception = "Error. Invalid memory accessed.";
const char* ui_text_card_crypto_exception = "Operation failed on CyCard (Crypto Exp)";
const char* ui_text_card_invalid_apdu_length = "Operation failed on CyCard (APDU len exp)";
const char* ui_text_card_invalid_tag_in_apdu = "Operation failed on CyCard (Tag exp)";
const char* ui_text_unknown_error_contact_support = "Unknown CyCard error. Contact support.";
const char* ui_text_unauthenticated_device = "Device is unauthenticated";

// headings
const char* ui_text_enter_pin = "Enter PIN";
const char* ui_text_confirm_pin = "Confirm PIN";
const char* ui_text_word_hash = "Word # ";
const char* ui_text_enter_word_hash = "Enter Word # %d";
const char* ui_text_verify_word_hash = "Verify Word # ";
const char* ui_text_is_word = "Is Word #%d\n%s?";
const char* ui_text_enter_wallet_name = "Enter wallet name";
const char* ui_text_number_of_words = "Number of words";
const char* ui_text_add_coins = "Add Coin #";
const char* ui_text_resync_coins = "Resync Coin #";
const char* ui_text_choose_wallet = "Choose Wallet";
const char* ui_text_enter_passphrase = "Enter passphrase";
const char* ui_text_confirm_passphrase = "Confirm passphrase";
const char* ui_text_receive_on = "Receive on";
const char* ui_text_verify_address = "Verify address :";
const char* ui_text_verify_contract = "Verify contract :";
const char* ui_text_unverified_contract = "Warning! Unverified contract";
const char* ui_text_verify_amount_float = "Verify amount : \n %s\n%s?";
const char* ui_text_verify_amount_integer = "Verify amount : \n %d\n%s?";
const char* ui_text_confirm_wallet_name = "Confirm wallet name";
const char* ui_text_enter_data = "Enter data";
const char* ui_text_confirm_data = "Confirm data";
const char* ui_text_verification_cancelled = "Verification cancelled";

// headings cycard flow
const char* ui_text_family_id_hex = "F. Id (Hex)";

// messages
const char* ui_text_use_passphrase_question = "Do you want to use passphrase with this wallet?";
const char* ui_text_wallet_name_size_limit = "Wallet name size cannot exceed 15 letters";
const char* ui_text_check_cysync = "Check the CySync app";
const char* ui_text_tap_cycard_auth_instruction_1 = "Tap a CyCard and do not lift until you hear 2 beep sound";
const char* ui_text_tap_cycard_auth_instruction_2 = "Now tap the same CyCard again";
const char* ui_text_fetching_recipient_amount = "Fetching recipient amount ...";
const char* ui_text_tap_card_to_start = "Tap a CyCard to start";
const char* ui_text_verification_cancelled_delete_wallet = "Verification cancelled, delete this wallet?";
const char* ui_text_wallet_deleted_successfully = "Wallet deleted successfully!";
const char* ui_text_wallet_redirecting_to_new_wallet = "Let's try again";
const char* ui_text_recovery_successfull = "Recovery\nSuccessful!";
const char* ui_text_generating_seed = "Generating seed...";
const char* ui_text_seed_generated_successfully = "Seed generated\nsuccessfully";
const char* ui_text_wallet_synced_with_cycards = "Wallet synced with CyCards!";
const char* ui_text_wallet_created_successfully = "Wallet created successfully!";
const char* ui_text_press_enter_to_start_verification = "Press the center button\nto verify it";
const char* ui_text_now_enter_your_seed_phrase = "Now enter your\nseed phrase";
const char* ui_text_now_enter_your_data = "Now enter your\ndata";
const char* ui_text_please_verify_entered_words = "Please verify the entered words.";
const char* ui_text_generating_xpubs = "Adding coins!";
const char* ui_text_exporting_to_desktop = "Exporting to desktop...";
const char* ui_text_successfully_exported_xpub_to_desktop = "Successfully added coin to CySync app!!";
const char* ui_text_wait_while_balance_fetching = "Please wait while balance is fetched on the CySync app";
const char* ui_text_successfully_exported_wallet_to_desktop = "Successfully added wallet to CySync app!!";
const char* ui_text_deriving_address = "Deriving address";
const char* ui_text_receive_on_address = "%s";
const char* ui_text_sending_confirmation_to_desktop = "Sending confirmation to desktop...";
const char* ui_text_success_received_from_desktop = "Success received from desktop";
const char* ui_text_fetching_unsigned_transaction = "Fetching transaction data ...";
const char* ui_text_unsigned_transaction_received = "Unsigned transaction received";
const char* ui_text_signing_transaction = "Signing transaction \n...";
const char* ui_text_exported_signed_transaction_to_desktop = "Exported signed transaction to desktop!!";
const char* ui_text_start_card_update = "Start CyCard update?\nThis will format the CyCard";
const char* ui_text_start_verification_of_card = "Start verification of CyCard?";
const char* ui_text_wallet_lock_continue_to_unlock = "Wallet is locked. Continue to unlock.";
const char* ui_text_wallet_partial_continue_to_delete = "Wallet in partial state. Continue to delete.";
const char* ui_text_wallet_not_verified_continue_to_verify = "Wallet not verified. Continue to verify.";
const char* ui_text_wallet_out_of_sync_continue_to_sync_with_cycards = "Continue to Sync with CyCards.";
const char* ui_text_wallet_not_verified_prompt = "'%s' is in unverified state";
const char* ui_text_wallet_out_of_sync_prompt = "'%s' is out of Sync with CyCards";
const char* ui_text_wallet_partial_state_prompt = "'%s' is in partial delete state";
const char* ui_text_wallet_locked_prompt = "'%s' is in locked state";
const char* ui_text_wallet_visit_to_verify = "Kindly visit the wallet to fix it";
const char* ui_text_wallet_partial_fix = "Wallets in partial state. Fix before proceeding.";
#ifdef ALLOW_LOG_EXPORT
const char* ui_text_sending_logs = "Sending logs ...";
const char* ui_text_logs_sent = "Logs sent";
#endif
const char* ui_text_creation_failed_delete_wallet = "Wallet creation failed. Proceed to delete.";

const char* ui_text_successfull = "Successful";
const char* ui_text_view_seed_messages[] = {
    "Do not give the seed phrase to anyone",
    "You will lose your coins if the seed phrase gets stolen",
    "Press the center button to continue"
};
const char* ui_text_card_authentication_success = "Card authentication success";
const char* ui_text_card_authentication_failed = "Card authentication failed";
const char* ui_text_enable_passphrase_step = "Do you want to enable passphrase step on wallet creation?";
const char* ui_text_disable_passphrase_step = "Do you want to disable passphrase step on wallet creation?";
const char* ui_text_warning_transaction_fee_too_high = "WARNING\nTransaction fees too high. Continue?";
const char* ui_text_enable_log_export = "Do you want to enable logging?";
const char* ui_text_disable_log_export = "Do you want to disable logging?";

//?
const char* ui_text_do_you_want_import_wallet_to_desktop = "Do you want to import wallet to CySync ?";
const char* ui_text_do_you_want_to_add_coins_to = "Do you want to add coins to %s ?";
const char* ui_text_do_you_want_to_resync_coins_to = "Do you want to resync coins to %s ?";
const char* ui_text_send_transaction_with = "Send %s transaction with %s ?";
const char* ui_text_eth_send_transaction_with = "Send %s transaction with %s on %s?";
const char* ui_text_recv_transaction_with = "Receive %s with %s ?";
const char* ui_text_do_you_want_to_set_pin = "Do you want to set a PIN ?";
const char* ui_text_output_send_value = "Receiver #%d\nSend %f\n%s?";
const char* ui_text_output_send_to_address = "Receiver #%d Address";

const char* ui_text_send_transaction_fee = "Transaction fee\n%f\n%s?";

const char* ui_text_output_send_value_double = "Receiver #%d\nSend %0.8f\n%s?";
const char* ui_text_send_transaction_fee_double = "Transaction fee\n%0.8f\n%s?";

const char* ui_text_start_firmware_update = "Update firmware to version %d.%d.%d ?";

const char* ui_text_firmware_update_process = "Updating device\n...";
const char* ui_text_auth_process = "Waiting for device authentication\n...";

const char* ui_text_sync_cycard_confirm = "Device will be synced with wallets in CyCard. Confirm?";
const char* ui_text_rotate_display_confirm = "Display will rotate 180 degrees. Confirm?";
const char* ui_text_factory_reset_confirm = "This will erase all wallets from device. Confirm?";
const char* ui_text_device_verification_success = "Device successfully authenticated";
const char* ui_text_device_verification_failure = "Device authentication failed";
const char* ui_text_unauthenticate_via_cysync = "Please authenticate the device via CySync";
const char* ui_text_start_device_verification = "Start device authentication?";
const char* ui_text_its_a_while_check_your_cards = "It's been a while. Kindly check if your cards are working properly.";
const char* ui_text_delete_this_wallet = "Delete this wallet?";
const char* ui_text_need_all_cycards_to_delete_wallet_entirely = "You'll need all CyCards\nto delete this wallet entirely. Confirm?";
const char* ui_text_20_spaces = "                    ";
const char* ui_text_wallet_not_found_on_cycard = "No Wallet found on CyCard";
const char* ui_text_sync_wallets_next_steps = "Sync Each wallet in next steps";
const char* ui_text_corrupted_card_contact_support = "Corrupted card. Contact support";
const char* ui_text_incompatible_card_version = "Incompatible card version. Contact support";
const char* ui_text_card_error_contact_support = "Contact support & share the device logs";
const char* ui_text_device_and_card_not_paired = "This device & card are not paired";
const char* ui_text_security_conditions_not_met = "Security conditions not met!";
const char* ui_text_process_reset_due_to_inactivity = "Process was reset due to inactivity";
const char* ui_text_device_already_provisioned = "Device already provisioned";
const char* ui_text_check_cysync_app = "Check the CySync App";
const char* ui_text_provision_success = "Device provision successful";
const char* ui_text_provision_fail = "Device provision unsuccessful";
const char* ui_text_cannot_verify_card_contact_support = "Couldn't verify card. Contact support";
const char* ui_text_wrong_btc_transaction = "Wrong transaction formatting.";
const char* ui_text_btc_change_address_mismatch = "Change address mismatch.";
const char* ui_text_worng_eth_transaction = "Invalid transaction detected";
const char* ui_text_pair_red_card = "Tap Red CyCard to pair";
const char* ui_text_pair_blue_card = "Tap Blue CyCard to pair";
const char* ui_text_pair_green_card = "Tap Green CyCard to pair";
const char* ui_text_pair_yellow_card = "Tap Yellow CyCard to pair";
const char* ui_text_card_pairing_success = "Card pairing success";
const char* ui_text_continue_with_pairing = "Continue with with pairing?";

#ifdef ALLOW_LOG_EXPORT
const char* ui_text_send_logs_prompt = "Send logs to desktop?";
#endif
