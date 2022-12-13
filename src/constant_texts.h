/**
 * @file    constant_texts.h
 * @author  Cypherock X1 Team
 * @brief   Header for constant texts.
 *          Stores declaration of all the hardcoded user facing strings/texts.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef CONSTANT_TEXTS_H
#define CONSTANT_TEXTS_H

#include "ui_menu.h"
#include "ui_text_slideshow.h"

//product hash
extern const char *product_hash ;

// level one
#define NUMBER_OF_OPTIONS_MAIN_MENU 2
extern const char* ui_text_options_main_menu[];

// level two
#define NUMBER_OF_OPTIONS_OLD_WALLET 2
extern const char* ui_text_options_old_wallet[];

#define NUMBER_OF_OPTIONS_NEW_WALLET 2
extern const char* ui_text_options_new_wallet[];

#ifdef DEV_BUILD
#define NUMBER_OF_OPTIONS_ADVANCED_OPTIONS 10
#else
#define NUMBER_OF_OPTIONS_ADVANCED_OPTIONS 9
#endif
extern const char* ui_text_options_advanced_settings[];
extern const char* ui_text_options_buzzer_adjust[];
extern const char* ui_text_logging_export_options[];
extern const char* u_text_passphrase_options[];

extern const char* ui_text_verification_is_now_complete_messages[];

extern const char* ui_text_seed_phrase_will_be_shown_copy_to_verify[];

extern const char* ui_text_startup_instruction_screen_1[];

extern const char* ui_text_startup_instruction_screen_2[];

extern const char* ui_text_startup_instruction_screen_3[];

extern const char* ui_text_startup_instruction_screen_4[];

extern const char* ui_text_mnemonics_number_options[];

extern const char* ui_text_view_data;

// errors
extern const char* ui_text_something_went_wrong;
extern const char* ui_text_something_went_wrong_contact_support_send_logs;
extern const char* ui_text_already_have_maxi_wallets;
extern const char* ui_text_aborted;
extern const char* ui_text_no_wallet_found;
extern const char* ui_text_operation_has_been_cancelled;
extern const char* ui_text_wallet_name_exists;
extern const char* ui_text_pin_incorrect_re_enter;
extern const char* ui_text_wallet_with_same_mnemo_exists;
extern const char* ui_text_incorrect_mnemonics;
extern const char* ui_text_incorrect_choice_view_seed_again;
extern const char* ui_text_xpub_not_found_add_coin_first;
extern const char* ui_wallet_pin_instruction_1;
extern const char* ui_wallet_pin_instruction_2;
extern const char* ui_wallet_passphrase_instruction_1;
extern const char* ui_wallet_passphrase_instruction_2;
extern const char* ui_wallet_passphrase_instruction_3;
extern const char* ui_wallet_passphrase_instruction_4;
extern const char* ui_text_no_response_from_desktop;
extern const char* ui_text_invalid_transaction;

// errors card flows
extern const char* ui_text_invalid_card_contact_cypherock;
extern const char*ui_text_wrong_card_sequence;
extern const char* ui_text_wallet_doesnt_exists_on_this_card;
extern const char* ui_text_wrong_wallet_is_now_locked;
extern const char* ui_text_wallet_verification_failed;

extern const char* ui_text_invalid_card_tap_card[];
extern const char* ui_text_place_card_x_heading;
extern const char* ui_text_place_card_wait_for_beep;
extern const char* ui_text_device_authenticating[];
extern const char* ui_text_device_provisioning[];

extern const char* ui_text_tap_a_card_instruction1;
extern const char* ui_text_tap_a_card_instruction2;
extern const char* ui_text_tap_a_card;
extern const char* ui_text_tap_x_4_cards;

extern const char* ui_text_tap_1_2_cards;
extern const char* ui_text_tap_2_2_cards;
extern const char* ui_text_place_card_below;

extern const char* ui_text_wallet_with_same_name_already_exist_on_this_card;
extern const char* ui_text_card_is_full;

extern const char* ui_text_device_synced_with_card;
extern const char* ui_text_family_id_retrived;
extern const char* ui_text_family_id_mismatch;

extern const char* ui_text_hash_not_accepted_by_card;
extern const char* ui_text_wrong_remaining_attempts;

extern const char* ui_text_is_currently_locked_wait_for;
extern const char* ui_text_is_currently_locked_wait_for_more_than_a_year;
extern const char* ui_text_in_process_to_unlock;
extern const char* ui_text_multiple_incorrect_attempts_may_block;
extern const char* ui_text_do_not_detach_device;
extern const char* ui_text_tap_card_ap_d;
extern const char* ui_text_correct_pin_is_now_unlocked;

extern const char* ui_text_card_sending_apdu;
extern const char* ui_text_card_command_send_error;
extern const char* ui_text_card_update_done;
extern const char* ui_text_card_transaction_exception;
extern const char* ui_text_card_out_of_boundary_exception;
extern const char* ui_text_card_null_pointer_exception;
extern const char* ui_text_card_crypto_exception;
extern const char* ui_text_card_invalid_apdu_length;
extern const char* ui_text_card_invalid_tag_in_apdu;
extern const char* ui_text_unknown_error_contact_support;
extern const char* ui_text_unauthenticated_device;

// headings
extern const char* ui_text_enter_pin;
extern const char* ui_text_confirm_pin;
extern const char* ui_text_word_hash;
extern const char* ui_text_enter_word_hash;
extern const char* ui_text_verify_word_hash;
extern const char* ui_text_is_word;
extern const char* ui_text_enter_wallet_name;
extern const char* ui_text_number_of_words;
extern const char* ui_text_add_coins;
extern const char* ui_text_resync_coins;
extern const char* ui_text_choose_wallet;
extern const char* ui_text_enter_passphrase;
extern const char* ui_text_confirm_passphrase;
extern const char* ui_text_receive_on;
extern const char* ui_text_verify_address;
extern const char* ui_text_verify_contract;
extern const char* ui_text_unverified_contract;
extern const char* ui_text_verify_amount;
extern const char* ui_text_confirm_wallet_name;
extern const char* ui_text_enter_data;
extern const char* ui_text_confirm_data;
extern const char* ui_text_verification_cancelled;
extern const char* ui_text_verify_nonce;
extern const char* ui_text_waiting_for_desktop;

// headings near specific
extern const char* ui_text_verify_create_from;
extern const char* ui_text_verify_new_account_id;
extern const char* ui_text_new_account_id;
extern const char* ui_text_new_public_key;
extern const char* ui_text_replace_account;
extern const char* ui_text_confirm_account;

// headings card flow
extern const char* ui_text_family_id_hex;

// messages
extern const char* ui_text_use_passphrase_question;
extern const char* ui_text_wallet_name_size_limit;
extern const char* ui_text_check_cysync;
extern const char* ui_text_tap_x1card_auth_instruction_1;
extern const char* ui_text_tap_x1card_auth_instruction_2;
extern const char* ui_text_tap_card_to_start;
extern const char* ui_text_wallet_deleted_successfully;
extern const char* ui_text_recovery_successfull;
extern const char* ui_text_processing;
extern const char* ui_text_seed_generated_successfully;
extern const char* ui_text_wallet_synced_with_x1cards;
extern const char* ui_text_wallet_created_successfully;
extern const char* ui_text_press_enter_to_start_verification;
extern const char* ui_text_now_enter_your_seed_phrase;
extern const char* ui_text_now_enter_your_data;
extern const char*ui_text_verify_entered_words;
extern const char* ui_text_generating_xpubs;
extern const char* ui_text_add_account_with_wallet;
extern const char* ui_text_exporting_to_desktop;
extern const char* ui_text_receive_on_address;
extern const char* ui_text_exported_signed_transaction_to_desktop;
extern const char* ui_text_start_card_update;
extern const char* ui_text_start_verification_of_card;
extern const char* ui_text_wallet_lock_continue_to_unlock;
extern const char* ui_text_wallet_partial_continue_to_delete;
extern const char* ui_text_wallet_not_verified_continue_to_verify;
extern const char* ui_text_wallet_out_of_sync_continue_to_sync_with_x1cards;
extern const char* ui_text_wallet_not_verified_prompt;
extern const char* ui_text_wallet_out_of_sync_prompt;
extern const char* ui_text_wallet_partial_state_prompt;
extern const char* ui_text_wallet_locked_prompt;
extern const char* ui_text_wallet_visit_to_verify;
extern const char* ui_text_wallet_partial_fix;
#ifdef ALLOW_LOG_EXPORT
extern const char* ui_text_sending_logs;
extern const char* ui_text_logs_sent;
#endif
extern const char* ui_text_creation_failed_delete_wallet;

extern const char* ui_text_successfull;
extern const char* ui_text_view_seed_messages[];
extern const char* ui_text_card_authentication_success;
extern const char* ui_text_card_authentication_failed;
extern const char* ui_text_authentication_required;
extern const char* ui_text_start_auth_from_CySync;
extern const char* ui_text_enable_passphrase_step;
extern const char* ui_text_disable_passphrase_step;
extern const char* ui_text_warning_transaction_fee_too_high;
extern const char* ui_text_enable_log_export;
extern const char* ui_text_disable_log_export;

//?
extern const char* ui_text_do_you_want_import_wallet_to_desktop;
extern const char* ui_text_do_you_want_to_add_coins_to;
extern const char* ui_text_do_you_want_to_resync_coins_to;
extern const char* ui_text_send_transaction_with;
extern const char* ui_text_eth_send_transaction_with;
extern const char* ui_text_recv_transaction_with;
extern const char* ui_text_eth_recv_transaction_with;
extern const char* ui_text_do_you_want_to_set_pin;
extern const char* ui_text_output_send_value;
extern const char* ui_text_output_send_to_address;
extern const char* ui_text_send_transaction_fee;
extern const char* ui_text_start_firmware_update;
extern const char* ui_text_send_transaction_fee_double;
extern const char* ui_text_output_send_value_double;

extern const char* ui_text_firmware_update_process;
extern const char* ui_text_auth_process;

extern const char* ui_text_sync_x1card_confirm;
extern const char* ui_text_rotate_display_confirm;

extern const char* ui_text_factory_reset_confirm[];
extern const char* ui_text_confirm_factory_reset;
extern const char* ui_text_no_wallets_present;
extern const char* ui_text_wallets_in_card;
extern const char* ui_text_card_seems_healthy;
extern const char* ui_text_click_to_view_wallets;
extern const char* ui_text_no_wallets_fetched;
extern const char* ui_text_card_health_check_start;
extern const char* ui_text_card_health_check_error[];
extern const char* ui_text_device_verification_success;
extern const char* ui_text_device_verification_failure;
extern const char* ui_text_unauthenticate_via_cysync;
extern const char* ui_text_start_device_verification;
extern const char* ui_text_its_a_while_check_your_cards;
extern const char* ui_text_delete_this_wallet;
extern const char* ui_text_need_all_x1cards_to_delete_wallet_entirely;
extern const char* ui_text_20_spaces;
extern const char* ui_text_wallet_not_found_on_x1card;
extern const char* ui_text_sync_wallets_next_steps;
extern const char* ui_text_corrupted_card_contact_support;
extern const char* ui_text_incompatible_card_version;
extern const char* ui_text_card_error_contact_support;
extern const char* ui_text_device_and_card_not_paired;
extern const char* ui_text_security_conditions_not_met;
extern const char* ui_text_process_reset_due_to_inactivity;
extern const char* ui_text_device_already_provisioned;
extern const char* ui_text_check_cysync_app;
extern const char* ui_text_provision_success;
extern const char* ui_text_provision_fail;
extern const char* ui_text_cannot_verify_card_contact_support;
extern const char* ui_text_wrong_btc_transaction;
extern const char* ui_text_btc_change_address_mismatch;
extern const char* ui_text_worng_eth_transaction;
extern const char* ui_text_error_no_card_paired;
extern const char* ui_text_error_pair_all_cards;
extern const char* ui_text_error_pair_atleast_2_cards;
extern const char* ui_text_pair_card_x;
extern const char* ui_text_card_pairing_skipped_count;
extern const char* ui_text_card_pairing_success;
extern const char* ui_text_continue_with_pairing;
extern const char* ui_text_device_compromised;
extern const char* ui_text_device_compromised_not_provisioned;
extern const char* ui_text_device_compromised_partially_provisioned;
extern const char* ui_text_device_compromised_v1_config;
extern const char* ui_text_pow_challenge_failed;
extern const char* ui_text_card_removed_fast;
extern const char* ui_text_card_freq_discon_fault;
extern const char* ui_text_card_align_with_device_screen;
extern const char* ui_text_syncing_complete;
extern const char* ui_text_remove_card_prompt;
extern const char* ui_text_card_detected;
extern const char* ui_text_nfc_hardware_fault_detected;
extern const char* ui_text_unreliable_cards;
extern const char* ui_critical_card_health_migrate_data;
extern const char* ui_text_reset_exit[];
extern const char* ui_text_erasing;
extern const char* ui_text_performing_checks;

#ifdef ALLOW_LOG_EXPORT
extern const char* ui_text_send_logs_prompt;
#endif
#endif //CONSTANT_TEXTS_H
