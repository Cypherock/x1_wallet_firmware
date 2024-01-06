/**
 * @file    initial_device_provision_contoller.c
 * @author  Cypherock X1 Team
 * @brief   Device provision controller.
 *          Handles post event (only next events) operations for device
 *provision flow initiated by desktop app.
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
#include "bip32.h"
#include "communication.h"
#include "controller_level_four.h"
#include "cryptoauthlib.h"
#include "curves.h"
#include "flash_api.h"
#include "nist256p1.h"
#include "string.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#if USE_SIMULATOR == 0
#include "stm32l4xx_it.h"
#endif

#define ATECC_CFG_88_MASK 0x4C
#define ATECC_CFG_89_MASK 0x01

#define ATECC_CFG_88_MASK_OLD_PROV 44

#if X1WALLET_INITIAL
// slot-5 shouldn't be locked
uint8_t atecc_slot_to_lock[] = {slot_2_auth_key,
                                slot_3_nfc_pair_key,
                                slot_6_io_key,
                                slot_8_serial,
                                slot_0_unused,
                                slot_1_unused,
                                slot_4_unused,
                                slot_7_unused,
                                slot_9_unused,
                                slot_10_unused,
                                slot_11_unused,
                                slot_12_unused,
                                slot_13_unused,
                                slot_14_unused,
                                slot_15_unused};

uint8_t provision_date[4];
Provision_Data_struct provision_keys_data;

extern lv_task_t *listener_task;
static lv_task_t *timeout_task;

static void __timeout_listener();
static void lock_all_slots();
#endif

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

void device_provision_controller() {
#if X1WALLET_INITIAL
  switch (flow_level.level_three) {
    case GENERATE_PROVSION_DATA: {
      uint8_t io_protection_key[IO_KEY_SIZE] = {0};
      uint8_t serial_no[38] = {0};
      uint8_t test_ecc608_configdata[128] = {
          // changes : i2c address = 0x30, secure boot disabled,
          0x01, 0x23, 0x00, 0x00,          // serial no part1
          0x00, 0x00, 0x00, 0x00,          // rev no
          0x00, 0x00, 0x00, 0x00, 0x01,    // serial no part2
          0x01,                            // AES enabled
          0x01,                            // I2C enabled
          0x00,                            // reserved
          0xC0,    // 0x60, //I2C address : coverts to 0x30
          0x00,    // reserved
          0x00,    // counter match disabled
          0x00,    // 0x01, //chipmode
          0x85, 0x00, 0x82, 0x00, 0x86, 0x46, 0x87, 0x46, 0x87, 0x46, 0x8F,
          0x46, 0x8F, 0x0F, 0x9F, 0x8F, 0x0F, 0x0F, 0x8F, 0x0F, 0x0F, 0x8F,
          0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x0F, 0x0D, 0x1F, 0x0F, 0x0F, 0xFF,
          0xFF, 0xFF,
          0xFF,    // monotonic counter 0 not attached to any key
          0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
          0xFF,    // monotonic counter 1 not attached to any key
          0x00, 0x00, 0x00, 0x00,
          0x00,          // use lock key disabled
          0x00,          // volatile key permission disabled
          0x00, 0x00,    // secure boot not used
          0x00,          // KdflvLoc not used
          0x69, 0x76,    // KdflvStr not used
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // reserved
          0x00,                                                    // use later
          0x00,                      // not used I2C address
          0x55,                      // data zone unlocked
          0x55,                      // config zone unlocked
          0xFF, 0xFF,                // slot unlocked
          0x0E, 0x61,                // ChipOptions
          0x00, 0x00, 0x00, 0x00,    // certificate formatting disabled
          0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x18,
          0x00, 0x7C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3A, 0x00, 0x3C, 0x00,
          0x30, 0x00, 0x3C, 0x00, 0x30, 0x00, 0x32, 0x00, 0x30, 0x00};

      atecc_data.retries = DEFAULT_ATECC_RETRIES;

      bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
      NVIC_DisableIRQ(OTG_FS_IRQn);
      do {
        OTG_FS_IRQHandler();
        if (atecc_data.status != ATCA_SUCCESS) {
          LOG_ERROR("PERR0-0x%02x", atecc_data.status);
        }

        // atecc initialize
        atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
        if (atecc_data.status != ATCA_SUCCESS) {
          continue;
        }

        // check atecc config and data zone lock atecc_data.status
        bool is_locked = false;
        atecc_data.status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_locked);

        if (!is_locked || atecc_data.status != ATCA_SUCCESS) {
          atecc_data.status = atcab_write_config_zone(test_ecc608_configdata);
          if (atecc_data.status != ATCA_SUCCESS) {
            continue;
          }

          atecc_data.status = atcab_lock_config_zone();
        }

        is_locked = false;
        atecc_data.status = atcab_is_locked(LOCK_ZONE_DATA, &is_locked);

        if (!is_locked || atecc_data.status != ATCA_SUCCESS) {
          atecc_data.status = atcab_lock_data_zone();
        }

        memset(atecc_data.device_serial, 0, DEVICE_SERIAL_SIZE);

        provision_status_t provision_status = check_provision_status();
        // check if device already provisioned
        if (provision_status == provision_empty) {
          // called again if not locked in previous configuration
          is_locked = false;
          atecc_data.status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_locked);

          if (!is_locked || atecc_data.status != ATCA_SUCCESS) {
            atecc_data.status = atcab_lock_config_zone();
          }

          is_locked = false;
          atecc_data.status = atcab_is_locked(LOCK_ZONE_DATA, &is_locked);

          if (!is_locked || atecc_data.status != ATCA_SUCCESS) {
            atecc_data.status = atcab_lock_data_zone();
          }

          // fill whole serial_no with random
          atecc_data.status = atcab_random(serial_no);

          // overwrite date 4bytes
          memcpy(serial_no, provision_date, 4);

          // overwrite hw no 4bytes
          uint32_t u32Temp = FW_get_hardware_version();
          memcpy(serial_no + 4, &u32Temp, 4);

#if USE_SIMULATOR == 0
          // overwrite MCU UID 12 bytes
          u32Temp = HAL_GetUIDw0();
          memcpy(serial_no + 8, &u32Temp, 4);
          u32Temp = HAL_GetUIDw1();
          memcpy(serial_no + 12, &u32Temp, 4);
          u32Temp = HAL_GetUIDw2();
          memcpy(serial_no + 16, &u32Temp, 4);
#endif

          atecc_data.status = atcab_write_zone(
              ATCA_ZONE_DATA, slot_8_serial, 0, 0, serial_no, 32);
          if (atecc_data.status != ATCA_SUCCESS) {
            continue;
          }

          // generate and write IO key
          random_generate(io_protection_key, IO_KEY_SIZE);
          if (atecc_data.status != ATCA_SUCCESS) {
            continue;
          }

          atecc_data.status = atcab_write_zone(ATCA_ZONE_DATA,
                                               slot_6_io_key,
                                               0,
                                               0,
                                               io_protection_key,
                                               IO_KEY_SIZE);
          if (atecc_data.status != ATCA_SUCCESS) {
            continue;
          }

          if (atecc_data.status == ATCA_SUCCESS) {
            if (set_io_protection_key(io_protection_key) == SUCCESS_) {
              // locking IO_KEY and serial number slots
              atecc_data.status = atcab_lock_data_slot(slot_6_io_key);
              atecc_data.status = atcab_lock_data_slot(slot_8_serial);
              get_device_serial();
            } else {
              memset(serial_no, 0, 32);
              LOG_ERROR("PERR1-IO");
              continue;
            }
          } else {
            memset(serial_no, 0, 32);
            LOG_ERROR("PERR1-0x%02x", atecc_data.status);
            continue;
          }
        } else if (provision_status == provision_incomplete) {
          get_device_serial();
        } else {
          lv_obj_clean(lv_scr_act());
          mark_error_screen(ui_text_device_already_provisioned);
          reset_flow_level();
          flow_level.level_one = 6;
          flow_level.show_error_screen = true;
          return;
        }
      } while ((atecc_data.status != ATCA_SUCCESS) && (--atecc_data.retries));
      if (usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

      transmit_data_to_app(ADD_DEVICE_PROVISION, atecc_data.device_serial, 32);

      flow_level.level_three = PROVISION_STATUS_WAIT;
      lv_task_set_prio(listener_task,
                       LV_TASK_PRIO_MID);    // explicitly enable task listener

      timeout_task =
          lv_task_create(__timeout_listener, 10000, LV_TASK_PRIO_HIGH, NULL);
      lv_task_once(timeout_task);
    } break;

    case PROVISION_STATUS_WAIT: {
      // do nothing, just wait
    } break;

    case PROVISION_SAVE_EXT_KEYS: {
      uint8_t private_write_key[36] = {0};
      Perm_Key_Data_Struct perm_key_data;
      uint8_t digest[SHA256_DIGEST_LENGTH];
      lv_task_del(timeout_task);

      get_io_protection_key(perm_key_data.io_protection_key);

      ecdsa_get_public_key33(&nist256p1,
                             provision_keys_data.device_private_key,
                             perm_key_data.ext_keys.device_auth_public_key);
      if (0 != memcmp(provision_keys_data.device_public_key,
                      perm_key_data.ext_keys.device_auth_public_key,
                      33)) {
        comm_reject_request(CONFIRM_PROVISION, 0);
        flow_level.level_three = PROVISION_UNSUCCESSFUL;
        break;
      }

      atecc_data.retries = DEFAULT_ATECC_RETRIES;
      bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
      NVIC_DisableIRQ(OTG_FS_IRQn);
      do {
        OTG_FS_IRQHandler();
        atecc_data.status = atcab_init(atecc_data.cfg_atecc608a_iface);
        if (atecc_data.status != ATCA_SUCCESS) {
          continue;
        }

        memset(private_write_key, 0, sizeof(private_write_key));
        memcpy(
            &private_write_key[4], provision_keys_data.device_private_key, 32);
        atecc_data.status = atcab_priv_write(slot_2_auth_key,
                                             private_write_key,
                                             slot_6_io_key,
                                             perm_key_data.io_protection_key);
        if (atecc_data.status != ATCA_SUCCESS) {
          LOG_ERROR("PERR2-0x%02x", atecc_data.status);
          continue;
        }

        memset(private_write_key, 0, sizeof(private_write_key));
        memcpy(&private_write_key[4], provision_keys_data.priv_key, 32);
        atecc_data.status = atcab_priv_write(slot_3_nfc_pair_key,
                                             private_write_key,
                                             slot_6_io_key,
                                             perm_key_data.io_protection_key);
        if (atecc_data.status != ATCA_SUCCESS) {
          LOG_ERROR("PERR3-0x%02x", atecc_data.status);
          continue;
        }
      } while (atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
      if (usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

      if (atecc_data.status != ATCA_SUCCESS) {
        comm_reject_request(CONFIRM_PROVISION, 0);
        flow_level.level_three = PROVISION_UNSUCCESSFUL;
        break;
      }

      sha256_Raw(provision_keys_data.priv_key,
                 sizeof(provision_keys_data.priv_key),
                 digest);
      memcpy(perm_key_data.ext_keys.self_key_id, digest, FS_KEYSTORE_KEYID_LEN);
      memcpy(perm_key_data.ext_keys.self_key_path,
             provision_keys_data.self_key_path,
             FS_KEYSTORE_KEYPATH_LEN);
      memcpy(perm_key_data.ext_keys.card_root_xpub,
             provision_keys_data.card_root_xpub,
             FS_KEYSTORE_XPUB_LEN);

      if (set_ext_key(&perm_key_data.ext_keys) == SUCCESS_) {
        lock_all_slots();

        if (check_provision_status() == provision_complete)
          transmit_one_byte_confirm(CONFIRM_PROVISION);
        else {
          comm_reject_request(CONFIRM_PROVISION, 0);
          flow_level.level_three = PROVISION_UNSUCCESSFUL;
          LOG_ERROR("PERR5-LOCK");
          break;
        }
      } else {
        comm_reject_request(CONFIRM_PROVISION, 0);
        flow_level.level_three = PROVISION_UNSUCCESSFUL;
        LOG_ERROR("PERR2-KEY");
        break;
      }
      reset_flow_level();
      flow_level.level_one = 10;
      lv_obj_clean(lv_scr_act());

    } break;

    case PROVISION_UNSUCCESSFUL: {
      lv_obj_clean(lv_scr_act());
      mark_error_screen(ui_text_provision_fail);
      reset_flow_level();
      flow_level.level_one = 6;
      flow_level.show_error_screen = true;
    } break;

    default:
      break;
  }
}

void lock_all_slots() {
  uint8_t usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);

  atecc_data.retries = DEFAULT_ATECC_RETRIES;
  bool lock = false;
  uint32_t err_count = 0;
  atecc_data.status = ATCA_FUNC_FAIL;
  do {
    err_count = 0;
    for (uint32_t i = 0; i < sizeof(atecc_slot_to_lock); i++) {
      lock = false;
      atecc_data.status = atcab_is_slot_locked(atecc_slot_to_lock[i], &lock);
      if (atecc_data.status != ATCA_SUCCESS) {
        LOG_CRITICAL("PERR4=0x%02x, retry-%d, slot=%d",
                     atecc_data.retries,
                     atecc_data.status,
                     atecc_slot_to_lock[i]);
        err_count++;
      } else if (lock == true) {
        continue;
      }
      atecc_data.status = atcab_lock_data_slot(atecc_slot_to_lock[i]);
      if (atecc_data.status != ATCA_SUCCESS) {
        LOG_CRITICAL("PERR5=0x%02x, retry-%d, slot=%d",
                     atecc_data.retries,
                     atecc_data.status,
                     atecc_slot_to_lock[i]);
        err_count++;
      }
    }
  } while (err_count != 0 && --atecc_data.retries);

  if (usb_irq_enable_on_entry == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

static void __timeout_listener() {
  mark_error_screen(ui_text_provision_fail);
  instruction_scr_destructor();
  reset_flow_level();
  flow_level.level_one =
      6;    // on command not received take to get-started screen
  lv_task_del(timeout_task);
#endif
}