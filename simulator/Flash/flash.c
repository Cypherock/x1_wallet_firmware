#include "flash.h"

#include <stdint.h>
#include <stdio.h>

#include "flash_if.h"

typedef enum { DATA_FILE, LOG_FILE, PERM_DATA_FILE, FW_DATA_FILE } file_type;

#define DATA_FILE_NAME "sim_data.bin"
#define LOG_FILE_NAME "sim_log.bin"
#define PERM_DATA_FILE_NAME "sim_pdata.bin"
#define FW_DATA_FILE_NAME "sim_fw_data.bin"

#ifndef DEBUGGING_WITHOUT_BOOTLOADER
#define DATA_BASE (FLASH_DATA_ADDRESS)
#define DATA_END (FLASH_DATA_END_ADDRESS)
#define LOG_START (LOG_SECTION_START)
#define LOG_END (LOG_SECTION_START + (LOG_MAX_PAGES * LOG_PAGE_SIZE))
#else
#define DATA_BASE (FLASH_DATA_ADDRESS)
#define DATA_END (FLASH_DATA_END_ADDRESS)
#define LOG_START (LOG_SECTION_START)
#define LOG_END (LOG_SECTION_START + (LOG_MAX_PAGES * LOG_PAGE_SIZE))
#endif

#define PERM_DATA_START (FIREWALL_NVDATA_APP_KEYS_ADDR)
#define PERM_DATA_END (FIREWALL_NVDATA_APP_KEYS_ADDR + LOG_PAGE_SIZE)

#define FW_DATA_START FIREWALL_NVDATA_SEGMENT_ADDR
#define FW_DATA_END                                                            \
  (FIREWALL_NVDATA_SEGMENT_ADDR + FIREWALL_NVDATA_SEGMENT_SIZE)

#define DATA_PG_CNT ((DATA_END - DATA_BASE) / LOG_PAGE_SIZE)
#define LOG_PG_CNT LOG_MAX_PAGES
#define PERM_DATA_PG_CNT ((PERM_DATA_END - PERM_DATA_START) / LOG_PAGE_SIZE)
#define FW_DATA_PG_CNT ((FW_DATA_END - FW_DATA_START) / LOG_PAGE_SIZE)

char *GET_FILE_FROM_ADDRESS(uint32_t addr) {
  if (addr >= LOG_START && addr < LOG_END)
    return LOG_FILE_NAME;
  if (addr >= DATA_BASE && addr < DATA_END)
    return DATA_FILE_NAME;
  if (addr >= PERM_DATA_START && addr < PERM_DATA_END)
    return PERM_DATA_FILE_NAME;
  if (addr >= FW_DATA_START && addr < FW_DATA_END)
    return FW_DATA_FILE_NAME;
  return NULL;
}

uint32_t GET_BASE_ADDRESS(uint32_t addr) {
  if (addr >= LOG_START && addr < LOG_END)
    return LOG_START;
  if (addr >= DATA_BASE && addr < DATA_END)
    return DATA_BASE;
  if (addr >= PERM_DATA_START && addr < PERM_DATA_END)
    return PERM_DATA_START;
  if (addr >= FW_DATA_START && addr < FW_DATA_END)
    return FW_DATA_START;
  return -1;
}

static int init_file(file_type file_tp) {
  FILE *file = NULL;
  uint8_t bytes[] = {0xff, 0xff, 0xff, 0xff};
  uint8_t page_cnt = 0;
  switch (file_tp) {
    case DATA_FILE:
      file = fopen(DATA_FILE_NAME, "r");
      if (file != NULL) {
        fclose(file);
        return STM_SUCCESS;
      }
      file = fopen(DATA_FILE_NAME, "wb");
      page_cnt = DATA_PG_CNT;
      break;

    case LOG_FILE:
      file = fopen(LOG_FILE_NAME, "r");
      if (file != NULL) {
        fclose(file);
        return STM_SUCCESS;
      }
      file = fopen(LOG_FILE_NAME, "wb");
      page_cnt = LOG_PG_CNT;
      break;

    case PERM_DATA_FILE:
      file = fopen(PERM_DATA_FILE_NAME, "r");
      if (file != NULL) {
        fclose(file);
        return STM_SUCCESS;
      }
      file = fopen(PERM_DATA_FILE_NAME, "wb");
      page_cnt = PERM_DATA_PG_CNT;
      break;

    case FW_DATA_FILE:
      file = fopen(FW_DATA_FILE_NAME, "r");
      if (file != NULL) {
        fclose(file);
        return STM_SUCCESS;
      }
      file = fopen(FW_DATA_FILE_NAME, "wb");
      page_cnt = FW_DATA_PG_CNT;
      break;

    default:
      return STM_ERROR_INTERNAL;
  }
  uint32_t count = (FLASH_SIM_PAGE_SIZE * page_cnt) / sizeof(bytes);
  uint32_t i = 0;
  errno = 0;
  for (i = 0; i < count; i++) {
    fwrite(bytes, sizeof(bytes), 1, file);
    if (ferror(file)) {
      fclose(file);
      perror("");
      return STM_ERROR_INTERNAL;
    }
  }
  fclose(file);
  return STM_SUCCESS;
}

static void init() {
  init_file(DATA_FILE);
  init_file(LOG_FILE);
  init_file(PERM_DATA_FILE);
  init_file(FW_DATA_FILE);
}

int read_file(uint32_t addr, uint32_t *dstAddr, uint32_t length) {
  init();
  FILE *file = fopen(GET_FILE_FROM_ADDRESS(addr), "rb");
  if (!file)
    return STM_ERROR_INTERNAL;
  uint32_t offset = addr - GET_BASE_ADDRESS((uint32_t)addr);

  errno = 0;
  fseek(file, offset, SEEK_SET);
  uint32_t r_count = fread(dstAddr, sizeof(uint32_t), length, file);

  fclose(file);
  if (r_count != length) {
    perror("");
    return STM_ERROR_INTERNAL;
  }
  return STM_SUCCESS;
}

int erase_file(uint32_t page_address, uint32_t noOfpages) {
  init();
  FILE *file = fopen(GET_FILE_FROM_ADDRESS(page_address), "rb+");
  uint8_t bytes[] = {0xff, 0xff, 0xff, 0xff};
  uint32_t count = (FLASH_SIM_PAGE_SIZE * noOfpages) / sizeof(bytes);
  if (!file)
    return STM_ERROR_INTERNAL;
  errno = 0;
  uint32_t offset = page_address - GET_BASE_ADDRESS((uint32_t)page_address);
  fseek(file, offset, SEEK_SET);
  for (int i = 0; i < count; i++) {
    fwrite(bytes, sizeof(bytes), 1, file);
    if (ferror(file)) {
      fclose(file);
      perror("");
      return STM_ERROR_INTERNAL;
    }
  }

  fclose(file);
  return 1;
}

int write_file(uint32_t *dstAddr, const uint32_t *srcAddr, uint32_t noOfWords) {
  init();
  FILE *file = fopen(GET_FILE_FROM_ADDRESS((size_t)dstAddr), "rb+");
  if (!file)
    return STM_ERROR_INTERNAL;
  uint32_t offset = ((size_t)dstAddr) - GET_BASE_ADDRESS((size_t)dstAddr);
  errno = 0;

  fseek(file, offset, SEEK_SET);
  fwrite(srcAddr, noOfWords * sizeof(uint32_t), 1, file);
  if (ferror(file)) {
    fclose(file);
    perror("");
    return STM_ERROR_INTERNAL;
  }
  fclose(file);
  return STM_SUCCESS;
}