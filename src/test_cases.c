#include "controller_main.h"
#include "communication.h"
#include "board.h"
#include "logger.h"
#include "sys_state.h"

typedef enum{
    NO_TEST = 0,
    TEST_DATA_READY = 1,
    TEST_INITIATED = 2,
    TEST_END_REACHED = 3,
    TEST_COMPLETED = 4,
    TEST_IDLE = 5
}test_state_t;

typedef enum{
    TEST_GENERATE_SEED = 1,
    TEST_SHARE_CREATE = 2,
    TEST_SHARE_REGENERATE=3
}test_cases_t;

test_cases_t test_case = TEST_GENERATE_SEED;
test_state_t test_state = NO_TEST;

typedef struct{
    uint8_t     level;
    Flow_level  start_flow, 
                end_flow;
}test_block_data_t;

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
            test_data.end_flow.level_three = GENERATE_WALLET_SEED_GENERATED+1;
            LOG_INFO("TEST: generate seed triggered");
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
            break;
        }
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

    switch (test_case)
    {
    case TEST_GENERATE_SEED:{
        for(int i = 0; i < 5; i++)
            log_hex_array("shares: ", wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
        log_hex_array("secret: ", wallet.wallet_share_with_mac_and_nonce, sizeof(wallet.wallet_share_with_mac_and_nonce));
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
        break;
    case TEST_COMPLETED:
        reset_flow_level();
        test_state++;
    default:
        break;
    }

}