/**
 * @file    application_startup.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
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
/**
 * @file application_startup.c
 * @author Shreyash H. Turkar
 * @brief This file contains definitions of functions which handle application
 * level initialisation of system.
 * @version 0.1
 * @date 2020-05-23
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "application_startup.h"

#include "core_error.h"
#include "core_flow_init.h"
#include "cryptoauthlib.h"
#include "device_authentication_api.h"
#include "flash_api.h"
#include "flash_if.h"
#include "logger.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "nfc.h"
#include "pow.h"
#include "sec_flash.h"
#include "sys_state.h"
#include "systick_timer.h"
#include "ui_screens.h"

#ifdef DEV_BUILD
#include "dev_utils.h"
#endif

#if USE_SIMULATOR == 0
#include "libusb/libusb.h"

#else
#include <SDL.h>

#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drv_conf.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "sim_usb.h"
#include "time.h"

static void sim_hal_init(void);
static int tick_thread(void *data);
static void memory_monitor(lv_task_t *param);
#endif

extern lv_task_t *listener_task;
extern lv_task_t *timeout_task;
extern lv_indev_t *indev_keypad;

extern const char *GIT_REV;
extern const char *GIT_TAG;
extern const char *GIT_BRANCH;

/**
 * @brief
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
static void clock_init(void) {
#if USE_SIMULATOR == 0
  HAL_Init();
#endif
  SystemClock_Config();
}

void reset_inactivity_timer() {
  inactivity_counter = 0;
}

/**
 * @brief Create a software timer
 *
 */
/**
 * @brief
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
static void create_timers() {
  BSP_App_Timer_Create(BSP_APPLICATION_TIMER, systick_interrupt_cb);
}

/**
 * @brief Initializes the display, lvgl library and lvgl port
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
static void display_init() {
  ui_set_event_over_cb(&mark_event_over);
  ui_set_event_cancel_cb(&mark_event_cancel);
  ui_set_list_choice_cb(&mark_list_choice);
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  ui_init(indev_keypad);
#ifdef DEV_BUILD
  ekp_register_process_func(ekp_process_queue);
#endif
}

#if X1WALLET_MAIN
device_auth_state device_auth_check() {
  if ((is_device_authenticated() == true) &&
      (get_first_boot_on_update() == false)) {
    return DEVICE_AUTHENTICATED;
  }
  return DEVICE_NOT_AUTHENTICATED;
}

void restrict_app() {
  reset_flow_level();
  const char *pptr[2] = {ui_text_authentication_required,
                         ui_text_start_auth_from_CySync};
  multi_instruction_init(pptr, 2, DELAY_TIME, false);
  lv_task_set_prio(listener_task, LV_TASK_PRIO_OFF);
  counter.next_event_flag = false;
  CY_Reset_Not_Allow(true);
  mark_device_state(CY_APP_IDLE_TASK | CY_APP_IDLE, 0);
  CY_set_app_restricted(true);
}
#endif

/**
 * @brief
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
static void atecc_mode_detect() {
#if USE_SIMULATOR == 0
  do {
    switch (++atecc_mode) {
      case ATECC_MODE_I2C2:
        BSP_I2C2_Init(BSP_ATECC_I2C_MODE_STANDARD);
        atecc_data.cfg_atecc608a_iface = &cfg_ateccx08a_i2c_def;
        break;
      case ATECC_MODE_I2C2_ALT:
        BSP_I2C2_Init(BSP_ATECC_I2C_MODE_STANDARD);
        atecc_data.cfg_atecc608a_iface = &cfg_ateccx08a_i2c_def;
        break;
      case ATECC_MODE_SWI:
        BSP_I2C2_DeInit();
        atecc_data.cfg_atecc608a_iface = &cfg_ateccx08a_swi_default;
        break;
      default:
        atecc_mode = 0;
        return;
    }
    atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
  } while (atecc_data.status != ATCA_SUCCESS);
#endif
}

void application_init() {
  sys_flow_cntrl_u.bits.usb_buffer_free = true;
  sys_flow_cntrl_u.bits.nfc_off = true;
  CY_Reset_Not_Allow(false);
  mark_device_state(CY_APP_DEVICE_TASK | CY_APP_BUSY, 0xFF);
#if USE_SIMULATOR == 0
  uint32_t ret;
  clock_init();

  // Peripherals initialize
  comm_init();
  BSP_USB_Clock_Init();
  BSP_GPIO_Init(FW_get_hardware_version());
  BSP_TIM2_Init();
  BSP_TIM3_Init();
  BSP_TIM6_Init();
  BSP_I2C1_Init();
  BSP_RNG_Init();
  atecc_mode_detect();
#if X1WALLET_MAIN
  libusb_init();
#endif
  // Timer3 interrupt
  BSP_TIM3_Base_Start_IT();
  BSP_App_Timer_Init();

  create_timers();
  BSP_App_Timer_Start(BSP_APPLICATION_TIMER, POLLING_TIME);

  ret = adafruit_pn532_init(false);
  uint32_t response;
  ret = adafruit_pn532_firmware_version_get(&response);

  display_init();
  if (get_display_rotation() == LEFT_HAND_VIEW) {
    ui_rotate();
  }
  logger_init();
#else
  srand(time(0));
  /*Initialize LittlevGL*/
  lv_init();
  sim_hal_init();
  lv_port_disp_init();
  ui_init(indev_keypad);

  ui_set_event_over_cb(&mark_event_over);
  ui_set_event_cancel_cb(&mark_event_cancel);
  ui_set_list_choice_cb(&mark_list_choice);

  SIM_USB_DEVICE_Init();
#endif
  set_wallet_init();
  reset_flow_level();
#if X1WALLET_MAIN
  CY_Reset_Not_Allow(true);

#endif
  nfc_set_device_key_id(get_perm_self_key_id());
  pow_init_hash_rate();
  if (get_first_boot_on_update() == true) {
    logger("%X-%s", get_fwVer(), GIT_REV);
    set_auth_state(get_auth_state());
  }
#ifdef DEV_BUILD
#if USE_SIMULATOR == 0
  buzzer_disabled = true;
#endif
#endif
  core_init_app_registry();
}

void check_invalid_wallets() {
  bool fix = false;
  char display[64];
  uint8_t paired_card_count = get_keystore_used_count();

  if (paired_card_count < MAX_KEYSTORE_ENTRY) {
    char msg[64] = {0};
    snprintf(msg,
             sizeof(msg),
             "%u card(s) not paired with device",
             (MAX_KEYSTORE_ENTRY - paired_card_count));
    delay_scr_init(paired_card_count == 0 ? ui_text_error_no_card_paired : msg,
                   DELAY_TIME);
    mark_core_error_screen(ui_text_card_pairing_warning, false);
    return;
  }

  for (uint8_t i = 0; i < MAX_WALLETS_ALLOWED; i++) {
    if (get_wallet_state(i) != VALID_WALLET &&
        get_wallet_state(i) != INVALID_WALLET &&
        get_wallet_state(i) != UNVERIFIED_VALID_WALLET &&
        get_wallet_state(i) != VALID_WALLET_WITHOUT_DEVICE_SHARE)
      continue;

    if (get_wallet_card_state(i) != 0xff && is_wallet_partial(i)) {
      snprintf(display,
               sizeof(display),
               "'%s' is in partial delete state",
               get_wallet_name(i));
      delay_scr_init(display, DELAY_TIME);
      fix = true;
    }
    if (is_wallet_unverified(i)) {
      snprintf(display,
               sizeof(display),
               "'%s' is in unverified state",
               get_wallet_name(i));
      delay_scr_init(display, DELAY_TIME);
      fix = true;
    }
    if (is_wallet_share_not_present(i)) {
      snprintf(display,
               sizeof(display),
               "'%s' is out of Sync with cards",
               get_wallet_name(i));
      delay_scr_init(display, DELAY_TIME);
      fix = true;
    }
    if (is_wallet_locked(i)) {
      snprintf(display,
               sizeof(display),
               "'%s' is in locked state",
               get_wallet_name(i));
      delay_scr_init(display, DELAY_TIME);
      fix = true;
    }
    if (get_wallet_state(i) == INVALID_WALLET) {
      snprintf(display, sizeof(display), "'%s' is invalid", get_wallet_name(i));
      delay_scr_init(display, DELAY_TIME);
      fix = true;
    }
  }
  if (fix)
    mark_core_error_screen(ui_text_wallet_visit_to_verify, false);
}

void check_boot_count() {
  if (get_boot_count() % 20 == 0) {
    delay_scr_init(ui_text_its_a_while_check_your_cards, DELAY_TIME);
  }
}

void log_error_handler_faults() {
#if USE_SIMULATOR == 0
  if (RTC->BKP1R > 1) {
    LOG_CRITICAL(
        "Error Handler Log "
        "\nErr:%d\npc:%08x\nlr:%08x\nr0:%ld\nr1:%ld\nr2:%ld\nr3:%ld\nr12:%"
        "08x\nxpsr:%08x",
        RTC->BKP1R,
        RTC->BKP2R,
        RTC->BKP3R,
        RTC->BKP4R,
        RTC->BKP5R,
        RTC->BKP6R,
        RTC->BKP7R,
        RTC->BKP8R,
        RTC->BKP9R);
  }
  WRITE_REG(RTC->BKP1R, 0x00);
  WRITE_REG(RTC->BKP2R, 0x00);
  WRITE_REG(RTC->BKP3R, 0x00);
  WRITE_REG(RTC->BKP4R, 0x00);
  WRITE_REG(RTC->BKP5R, 0x00);
  WRITE_REG(RTC->BKP6R, 0x00);
  WRITE_REG(RTC->BKP7R, 0x00);
  WRITE_REG(RTC->BKP8R, 0x00);
  WRITE_REG(RTC->BKP9R, 0x00);
#endif    // USE_SIMULATOR
}

bool fault_in_prev_boot() {
#if USE_SIMULATOR == 0
  if (READ_REG(RTC->BKP1R) == 0) {
    return false;
  } else {
    return true;
  }
#else
  return false;
#endif
}

void handle_fault_in_prev_boot() {
#if USE_SIMULATOR == 0
  log_error_handler_faults();
  WRITE_REG(RTC->BKP1R, 0x00);
  delay_scr_init(ui_text_something_went_wrong_contact_support_send_logs,
                 DELAY_TIME);
#endif
}

void device_hardware_check() {
#if USE_SIMULATOR == 0
  ui_set_event_over_cb(NULL);
  if (nfc_diagnose_antenna_hw() & (1 << NFC_ANTENNA_STATUS_BIT))
    delay_scr_init(ui_text_nfc_hardware_fault_detected, DELAY_LONG_STRING);
  ui_set_event_over_cb(&mark_event_over);
#endif
}

uint32_t get_device_serial() {
  atecc_data.retries = DEFAULT_ATECC_RETRIES;
  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);

  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
    atecc_data.status = atcab_read_zone(ATCA_ZONE_DATA,
                                        slot_8_serial,
                                        0,
                                        0,
                                        atecc_data.device_serial,
                                        DEVICE_SERIAL_SIZE);
  } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  if (atecc_data.status == ATCA_SUCCESS) {
    if (0 != memcmp(atecc_data.device_serial + 8, (void *)UID_BASE, 12)) {
      return 1;
    } else {
      return SUCCESS;
    }
  }
  return atecc_data.status;
}

provision_status_t check_provision_status() {
  uint8_t cfg[128];
  memset(cfg, 0, 128);
  atecc_data.retries = DEFAULT_ATECC_RETRIES;

  bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  do {
    atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
    atecc_data.status = atcab_read_config_zone(cfg);
  } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);

  if (atecc_data.status != ATCA_SUCCESS) {
    LOG_CRITICAL("xxx30: %d", atecc_data.status);
    return -1;
  }

  if (cfg[86] == 0x00 &&
      cfg[87] == 0x00) {    // config zone and data zones are locked

    if (cfg[88] == 0xBF &&
        cfg[89] ==
            0xFE) {    // device serial and IO key are programmed and locked
      return provision_incomplete;

    } else if ((cfg[88] & ATECC_CFG_88_MASK) == 0x00 &&
               (cfg[89] & ATECC_CFG_89_MASK) ==
                   0x00) {    // private key slots are locked
      return provision_complete;
    } else if ((cfg[88] & ATECC_CFG_88_MASK_OLD_PROV) == 0x00 &&
               (cfg[89] & ATECC_CFG_89_MASK) ==
                   0x00) {    // NFC private key slot not locked
      return provision_v1_complete;
    } else {
      return provision_empty;
    }
  } else {
    return provision_empty;
  }
}

void device_provision_check() {
#if USE_SIMULATOR == 0
  const char *msg = NULL;

  switch (check_provision_status()) {
    default:
    case provision_empty:
      msg = ui_text_device_compromised_not_provisioned;
      break;
    case provision_incomplete:
      msg = ui_text_device_compromised_partially_provisioned;
      break;
    case provision_complete: {
      if (get_device_serial() != SUCCESS) {
        msg = ui_text_device_compromised;
        break;
      }
      return;
    }
    case provision_v1_complete:
      msg = ui_text_device_compromised_v1_config;
      break;
  }

#if NDEBUG
  msg = ui_text_device_compromised;
#endif
  delay_scr_init(msg, DELAY_TIME);
#endif
}

#if USE_SIMULATOR == 1

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics
 * library
 */
static void sim_hal_init(void) {
  /* Use the 'monitor' driver which creates window on PC's monitor to simulate a
   * display*/
  monitor_init();

  /*Create a display buffer*/
  static lv_disp_buf_t disp_buf1;
  static lv_color_t buf1_1[480 * 10];
  lv_disp_buf_init(&disp_buf1, buf1_1, NULL, 480 * 10);

  /*Create a display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.buffer = &disp_buf1;
  disp_drv.flush_cb = monitor_flush; /*Used when `LV_VDB_SIZE != 0` in lv_conf.h
                                        (buffered drawing)*/
  lv_disp_drv_register(&disp_drv);

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
  mouse_init();
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv); /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb =
      mouse_read; /*This function will be called periodically (by the library)
                     to get the mouse position and state*/
  lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);

  /*Set a cursor for the mouse*/
  LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t *cursor_obj =
      lv_img_create(lv_disp_get_scr_act(NULL),
                    NULL); /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
  lv_indev_set_cursor(mouse_indev,
                      cursor_obj); /*Connect the image  object to the driver*/

  // Keyboard
  keyboard_init();

  lv_indev_drv_t kb_drv;
  lv_indev_drv_init(&kb_drv);
  kb_drv.type = LV_INDEV_TYPE_KEYPAD;
  kb_drv.read_cb = keyboard_read;
  indev_keypad = lv_indev_drv_register(&kb_drv);

  /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
   * how much time were elapsed Create an SDL thread to do this*/
  SDL_CreateThread(tick_thread, "tick", NULL);

  /* Optional:
   * Create a memory monitor task which prints the memory usage in
   * periodically.*/
  lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);
}

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
  (void)data;

  while (1) {
    SDL_Delay(5);   /*Sleep for 5 millisecond*/
    lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
  }

  return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_task_t *param) {
  (void)param; /*Unused*/

  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n",
         (int)mon.total_size - mon.free_size,
         mon.used_pct,
         mon.frag_pct,
         (int)mon.free_biggest_size);
}

#endif
