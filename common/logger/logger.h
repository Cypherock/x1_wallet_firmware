/**
 * @file    logger.h
 * @author  Cypherock X1 Team
 * @brief   Logger public APIs.
 *          All the logger public APIs are defined here.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef _DEVICE_LOGGER_H
#define _DEVICE_LOGGER_H

#include <stdint.h>
#include <stdio.h>

#include "logger_config.h"
#include "mem_config.h"

/// Defines max size of log
#define LOG_MAX_SIZE (128)
/// Size of log sent to desktop
#define LOG_SENT_MAX_SIZE (32)
/// Start address of log
#define LOG_SECTION_START (FLASH_DATA_LOGGER_ADDRESS)
/// Maximum number of log pages allowed
#define LOG_MAX_PAGES (FLASH_DATA_LOGGER_MAX_PAGES)
/// Maximum size of single log page
#define LOG_PAGE_SIZE (FLASH_DATA_LOGGER_PAGE_SIZE)

/**
 * @brief Enum used while reading the log
 *
 */
typedef enum {
  LOG_READ_INIT,
  LOG_READ_ONGOING,
  LOG_READ_END,
  LOG_READ_FINISH,
} log_read_e_t;

/**
 * @brief Records the details about log data.
 *
 */
typedef struct {
  uint16_t log_count;
  uint32_t next_write_loc;
  uint8_t page_index;
  uint8_t initialized;
  uint8_t read_page_index;
  log_read_e_t read_sm_e;
  uint8_t total_page_read;
} logger_data_s_t;

/**
 * @brief Logs the text passed as param with option params for formatting.
 *
 * @param fmt Text to log
 * @param ... Optional arguments for formatting
 */
void logger(char *fmt, ...);

/// Main logger method
#if USE_SIMULATOR == 0

#ifndef RELEASE_BUILD
#define LOG_SWV printf
#define LOG_INFO logger
#else
#define LOG_SWV(...)
#define LOG_INFO(...)
#endif

#define LOG_CRITICAL logger
#define LOG_ERROR logger

#else

#define LOG_SWV printf
#define LOG_INFO printf
#define LOG_ERROR printf
#define LOG_CRITICAL printf

#endif

/// Increments the passed var within the limits of the passed max
#define CYCLIC_INCREMENT(var, max) ((var + 1) % max)

/**
 * @brief Task to read logs from flash to provided buffer in RAM.
 * @details The function internally maintains states to manage transferring logs
 * in chunks of 2kB. The function sends a 'start of log' packet when export
 * starts and 'end of log' packet when all the logs have been sent. The
 * application should handle usb events after each chunk transfer is complete
 * so that next chunk request from host is served by calling this function
 * repeatedly on every usb event.
 *
 * @param data Pointer to a buffer for storing the log data (minimum of 2kB)
 * @param size Reference to size storage variable
 *
 * @see
 * @since v1.0.0
 */
void logger_task(uint8_t *data, size_t *size);

/**
 * @brief Initialises the logger and resets the properties in the global
 * logger_data_s_t instance.
 * @details
 *
 * @param head Start address of logs
 * @param tail End address of logs
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void logger_init();

/**
 * @brief Erases the logs and sets the next location to the start address of
 * logs.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void logger_reset_flash(void);

/**
 * @brief Helper function to log a hex array after converting it to char string.
 * @details
 *
 * @param [in] text     Message to log before hex
 * @param [in] arr      Hex array to log
 * @param [in] length   Length of hex array
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void log_hex_array(const char text[], const uint8_t *arr, uint8_t length);

/**
 * @brief Get the log read status
 * @details
 *
 * @param
 *
 * @return
 * @return log_read_e_t
 * @retval LOG_READ_INIT Read initialized
 * @retval LOG_READ_ONGOING Currently being read
 * @retval LOG_READ_END Reading the log end
 * @retval LOG_READ_FINISH Read complete
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
log_read_e_t get_log_read_status();

void set_start_log_read();

#endif    //_DEVICE_LOGGER_H
