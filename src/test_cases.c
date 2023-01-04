#if X1WALLET_TEST_SUITE==1

#include "controller_main.h"
#include "communication.h"
#include "board.h"
#include "logger.h"
#include "sys_state.h"
#include "test_cases.h"

typedef struct{
    uint8_t     level;
    Flow_level  start_flow, 
                end_flow;
}test_block_data_t;

test_cases_t test_case = 0xFF;
test_state_t test_state = 0xff;
uint8_t test_input_data[1000] = {0}, test_output_data[1000] = {0};
uint16_t test_data_len=0;
test_block_data_t test_data = {0};
extern lv_task_t* listener_task;

void set_level_one_variables(){
    flow_level.level_one = test_data.start_flow.level_one;
    lv_task_set_prio(listener_task, LV_TASK_PRIO_OFF); // Task will now not run
    mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
    clear_list_choice();
}

void set_level_two_variables(){
    flow_level.level_two = test_data.start_flow.level_two;
}

void set_level_three_variables(){
    flow_level.level_three = test_data.start_flow.level_three;
}

void set_test_case(){
    if(test_state != NO_TEST)
        return;

    switch(test_case){
        case TEST_GENERATE_SEED:
            test_data.level = LEVEL_THREE;
            test_data.start_flow.level_one = LEVEL_TWO_NEW_WALLET;
            test_data.start_flow.level_two = LEVEL_THREE_GENERATE_WALLET;
            test_data.start_flow.level_three = GENERATE_WALLET_PROCESSING;

            test_data.end_flow.level_one = LEVEL_TWO_NEW_WALLET;
            test_data.end_flow.level_two = LEVEL_THREE_GENERATE_WALLET;
            test_data.end_flow.level_three = GENERATE_WALLET_SEED_GENERATED;
            LOG_INFO("TEST: generate seed triggered");
            test_state = TEST_DATA_READY;
        break;
        case TEST_RESTORE_SEED:
            test_data.level = LEVEL_THREE;
            test_data.start_flow.level_one = LEVEL_TWO_NEW_WALLET;
            test_data.start_flow.level_two = LEVEL_THREE_RESTORE_WALLET;
            test_data.start_flow.level_three = RESTORE_WALLET_CREATING_WAIT_SCREEN;

            test_data.end_flow.level_one = LEVEL_TWO_NEW_WALLET;
            test_data.end_flow.level_two = LEVEL_THREE_RESTORE_WALLET;
            test_data.end_flow.level_three = RESTORE_WALLET_CREATE+1;
            LOG_INFO("TEST: restore seed triggered");
            test_state = TEST_DATA_READY;
        break;
        case TEST_VERIFY_SHARES:
            test_data.level = LEVEL_THREE;
            test_data.start_flow.level_one = LEVEL_TWO_OLD_WALLET;
            test_data.start_flow.level_two = LEVEL_THREE_VERIFY_WALLET;
            test_data.start_flow.level_three = VERIFY_WALLET_SHOW_MNEMONICS;

            test_data.end_flow.level_one = LEVEL_TWO_OLD_WALLET;
            test_data.end_flow.level_two = LEVEL_THREE_VERIFY_WALLET;
            test_data.end_flow.level_three = VERIFY_WALLET_SHOW_MNEMONICS;
            LOG_INFO("TEST: restore seed triggered");
            test_state = TEST_DATA_READY;
        break;
        default:
        break;
    }
}

void jump_to_test(){
    if(test_state != TEST_DATA_READY)
        return;

    if(counter.level != LEVEL_ONE){
        LOG_INFO("TEST: Device Busy");
        return;
    }

    set_level_one_variables();
    if(test_data.level > LEVEL_ONE){
        set_level_two_variables();
        increase_level_counter();
    }
    if(test_data.level > LEVEL_TWO){
        set_level_three_variables();
        increase_level_counter();
    }
    counter.next_event_flag = true;
    LOG_INFO("TEST: initiating");

    switch (test_case)
    {
        case TEST_GENERATE_SEED:{
            Flash_Wallet wallet_for_flash;
            WALLET_UNSET_PIN(wallet_for_flash.wallet_info);
            WALLET_UNSET_PIN(wallet.wallet_info);
            memcpy(wallet.wallet_share_with_mac_and_nonce, test_input_data, 32);
        }break;
        case TEST_RESTORE_SEED:{
            Flash_Wallet wallet_for_flash;
            WALLET_UNSET_PIN(wallet_for_flash.wallet_info);
            WALLET_UNSET_PIN(wallet.wallet_info);
            wallet.number_of_mnemonics = 24;
            __single_to_multi_line((const char*)test_input_data, strlen(test_input_data), wallet_credential_data.mnemonics);            
            LOG_INFO("TEST: Input mnemonics");
            for(int i = 0; i < 24; i++)
                LOG_INFO("MNEMO WORD %d: %s", i+1, wallet_credential_data.mnemonics[i]);
        }break;
        case TEST_VERIFY_SHARES:{
            WALLET_UNSET_PIN(wallet.wallet_info);
            WALLET_UNSET_PASSPHRASE(wallet.wallet_info);
            for(int i = 0; i < 5; i++){
                memcpy(wallet_shamir_data.mnemonic_shares[i], test_input_data + (i * 32), 32);
                wallet_shamir_data.share_x_coords[i] = i+1;
            }
        }break;
        default:
            break;
    }
    lv_obj_clean(lv_scr_act());
    test_state = TEST_INITIATED;
}

void detect_end(){
    if(test_state != TEST_INITIATED)
        return;

    switch(test_data.level){
        case LEVEL_TWO:
            if(flow_level.level_two == test_data.end_flow.level_two){
                test_state = TEST_END_REACHED;
            }
            break;
        case LEVEL_THREE:
            if(flow_level.level_three == test_data.end_flow.level_three){
                test_state = TEST_END_REACHED;
            }
            break;
        case LEVEL_FOUR:
            if(flow_level.level_four == test_data.end_flow.level_four){
                test_state = TEST_END_REACHED;
            }
            break;
        default:
            break;
    }

    if(test_state == TEST_END_REACHED){
        LOG_INFO("TEST: stopped");
        counter.next_event_flag = false;
    }
}

void log_test_result(){
    if(test_state != TEST_END_REACHED)
        return;

    test_data_len=0;

    switch (test_case)
    {
    case TEST_GENERATE_SEED:{
        LOG_INFO("TEST: Generated seed from random secret");
        for(int i = 0; i < 24; i++)
            LOG_INFO("MNEMO WORD %d: %s", i+1, wallet_credential_data.mnemonics[i]);
        LOG_INFO("TEST: Generated shares");
        for(int i = 0; i < 5; i++){
            log_hex_array("shares: ", wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
            memcpy(test_output_data+test_data_len, wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
            test_data_len+=32;
        }
        LOG_INFO("TEST: Log done.");
        __multi_to_single_line(wallet_credential_data.mnemonics, 24, (char*)(test_output_data+test_data_len));
        for(int i=0; i<24; i++){
            test_data_len+=strlen(wallet_credential_data.mnemonics[i])+1;
        }
        transmit_data_to_app(DEVICE_SHAMIR_GENERATE_TEST, test_output_data, test_data_len);
    }break;
    case TEST_RESTORE_SEED:{
        LOG_INFO("TEST: Generated shares");
        for(int i = 0; i < 5; i++){
            log_hex_array("shares: ", wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
            memcpy(test_output_data+test_data_len, wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
            test_data_len+=32;
        }
        LOG_INFO("TEST: Log done.");
        transmit_data_to_app(DEVICE_SHAMIR_RESTORE_SEED, test_output_data, test_data_len);
    }break;
    case TEST_VERIFY_SHARES:{
        LOG_INFO("TEST: VERIFY SHARES 5C2");
        for(int i = 0; i < 24; i++)
            LOG_INFO("MNEMO WORD %d: %s", i+1, wallet_credential_data.mnemonics[i]);
        LOG_INFO("TEST: Log done.");
        __multi_to_single_line(wallet_credential_data.mnemonics, 24, (char*)(test_output_data+test_data_len));
        for(int i=0; i<24; i++){
            test_data_len+=strlen(wallet_credential_data.mnemonics[i])+1;
        }
        transmit_data_to_app(DEVICE_SHAMIR_VERIFY_SHARES, test_output_data, test_data_len);
    }break;
    default:
        break;
    }
    test_state = TEST_COMPLETED;
}

void repeated_test_task(){
    switch (test_state)
    {
    case NO_TEST:
        set_test_case();
        break;
    case TEST_DATA_READY:
        jump_to_test();
        break;
    case TEST_INITIATED:
        detect_end();
        break;
    case TEST_END_REACHED:
        log_test_result();
        // break;
    case TEST_COMPLETED:
        reset_flow_level();
        test_state++;
        break;
    case TEST_IDLE:
        memzero(test_input_data, sizeof(test_input_data));
        memzero(test_output_data, sizeof(test_output_data));
        memzero(&test_data, sizeof(test_data));
        test_data_len=0;
        break;
    default:
        test_case = TEST_IDLE;
        break;
    }

}

#endif