/**
 * @file    sample_test.h
 * @author  Cypherock X1 Team
 * @brief   This header file contains header file template for a unit test module
 * 
 * @details
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */

#ifndef SAMPLE_TEST_H
#define SAMPLE_TEST_H

#include "unity_fixture.h"
#include <stdbool.h>

#if USE_SIMULATOR == 1
/**
 * @brief Sample testcase 1 provides an example of using the testing framework
 * This test case runs on the simulator 
 */
bool sample_simulator(void);
#endif /* USE_SIMULATOR == 1 */

#if USE_SIMULATOR == 0
/**
 * @brief Sample testcase 2 provides an example of using the testing framework,
 * this test case runs on the target
 * 
 */
bool sample_target(void);
#endif /* USE_SIMULATOR == 1 */

#endif /* SAMPLE_TEST_H */
