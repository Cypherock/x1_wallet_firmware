/**
 * @file    constant_texts.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef ASSERT_CONF_H
#define ASSERT_CONF_H

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "logger.h"

#if USE_SIMULATOR == 0

// TODO: get return address same as PC
#define GET_LR() __builtin_return_address(0)
// This is ARM and GCC specific syntax
#define GET_PC(_a) __asm volatile("mov %0, pc" : "=r"(_a))

#define MY_ASSERT_RECORD()                                                     \
  do {                                                                         \
    void *pc;                                                                  \
    GET_PC(pc);                                                                \
    const void *lr = GET_LR();                                                 \
    assert_handler((uint32_t)pc, (uint32_t)lr);                                \
  } while (0)

#define ASSERT(exp)                                                            \
  do {                                                                         \
    if ((exp) == false) {                                                      \
      MY_ASSERT_RECORD();                                                      \
    }                                                                          \
  } while (0)

/**
 * @brief   Logs PC and LR register for debugging of assert failure
 * @details    To debug the line number and file of the project we can run gdb
 * server with the elf file Open cmd and start gdb with executable name
 *          >> gdb Cypherock.elf
 *          (gdb) info line *<pc or lr value>
 *
 *          running this command returns file name, function and line number for
 * that address
 *
 * @param   pc :program counter, tells the next instruction address
 *          lr :link register, store the return address
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void assert_handler(uint32_t pc, uint32_t lr);

#else

#define ASSERT(expr)                                                           \
  ((expr) ? (void)0U                                                           \
          : assert_handler(                                                    \
                (uint8_t *)__FILE__, (uint8_t *)__FUNCTION__, __LINE__))

void assert_handler(uint8_t *file, uint8_t *function, uint32_t line);

#endif

#endif    // end ASSERT_CONF_H