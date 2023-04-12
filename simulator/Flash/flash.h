#ifndef _FLASH_SIM_
#define _FLASH_SIM_

#include <errno.h>
#include <string.h>

#include "board.h"
#include "logger.h"

#define FLASH_SIM_PAGE_SIZE LOG_PAGE_SIZE
#define TRANSLATE_ADDR(addr, base) (addr - base)

int read_file(uint32_t addr, uint32_t *dstAddr, uint32_t length);
int erase_file(uint32_t page_address, uint32_t noOfpages);
int write_file(uint32_t *dstAddr, const uint32_t *srcAddr, uint32_t noOfWords);

#endif