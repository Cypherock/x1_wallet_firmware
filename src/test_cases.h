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
    TEST_RESTORE_SEED = 2
}test_cases_t;

extern uint8_t test_input_data[1000]; 
extern uint16_t test_input_data_len=0;
extern test_cases_t test_case;
extern test_state_t test_state;