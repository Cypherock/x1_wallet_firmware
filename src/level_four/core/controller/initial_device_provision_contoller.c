/**
 * @file    initial_device_provision_contoller.c
 * @author  Cypherock X1 Team
 * @brief   Device provision controller.
 *          Handles post event (only next events) operations for device provision flow initiated by desktop app.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include "bip39_english.h"
#include "btc.h"
#include "communication.h"
#include "controller_level_four.h"
#include "curves.h"
#include "nist256p1.h"
#include "ui_delay.h"
#include "atca_host.h"
#include "atca_cfgs.h"
#include "atca_basic.h"
#include "string.h"
#include "ui_instruction.h"
#include "flash_api.h"

#define ATECC_CFG_88_MASK   0x4C
#define ATECC_CFG_89_MASK   0x01

#define ATECC_CFG_88_MASK_OLD_PROV  44

#if X1WALLET_INITIAL
uint8_t provision_date[4];
Provision_Data_struct provision_keys_data;

extern lv_task_t* listener_task;
static lv_task_t *timeout_task;

static void __timeout_listener();
#endif

provision_status_t check_provision_status(){
    uint8_t cfg[128];
    memset(cfg, 0, 128);
    atcab_read_config_zone(cfg);
    if(cfg[86]==0x00 || cfg[87]==0x00){ //config zone and data zones are locked

        if(cfg[88]==0xBF && cfg[89]==0xFE ){    //device serial and IO key are programmed and locked
            return provision_incomplete;
        }
        else if((cfg[88] & ATECC_CFG_88_MASK)==0x00 && (cfg[89] & ATECC_CFG_89_MASK) ==0x00){   //private key slots are locked
            return provision_complete;
        }
        else if((cfg[88] & ATECC_CFG_88_MASK_OLD_PROV)==0x00 && (cfg[89] & ATECC_CFG_89_MASK) ==0x00 ){     //NFC private key slot not locked
            return provision_v1_complete;
        }
        else{
            return provision_empty;
        }
    }
    else {
        return provision_empty;
    }
}

void device_provision_controller(){
#if X1WALLET_INITIAL
    switch (flow_level.level_three) {
        
        case GENERATE_PROVSION_DATA: {
            uint8_t io_protection_key[IO_KEY_SIZE] = {0};
            uint8_t serial_no[38] = {0};
            uint8_t test_ecc608_configdata[128] = {
            //changes : i2c address = 0x30, secure boot disabled,
            0x01, 0x23, 0x00, 0x00, //serial no part1
            0x00, 0x00, 0x00, 0x00, //rev no
            0x00, 0x00, 0x00, 0x00, 0x01, //serial no part2
            0x01, //AES enabled
            0x01, //I2C enabled
            0x00, //reserved
            0xC0,//0x60, //I2C address : coverts to 0x30
            0x00, //reserved
            0x00, //counter match disabled
            0x00,//0x01, //chipmode
            0x85, 0x00, 0x82, 0x00, 0x86, 0x46, 0x87, 0x46, 0x87, 0x46, 0x8F, 0x46, 0x8F, 0x0F, 0x9F, 0x8F, 0x0F, 0x0F, 0x8F, 0x0F, 0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x0F, 0x0D, 0x1F, 0x0F, 0x0F,
            0xFF, 0xFF, 0xFF, 0xFF, //monotonic counter 0 not attached to any key
            0x00, 0x00, 0x00, 0x00, 
            0xFF, 0xFF, 0xFF, 0xFF, //monotonic counter 1 not attached to any key
            0x00, 0x00, 0x00, 0x00, 
            0x00, //use lock key disabled
            0x00, //volatile key permission disabled
            0x00, 0x00, //secure boot not used
            0x00, //KdflvLoc not used
            0x69, 0x76, //KdflvStr not used
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //reserved
            0x00, //use later
            0x00, //not used I2C address
            0x55, //data zone unlocked
            0x55, //config zone unlocked 
            0xFF, 0xFF, //slot unlocked
            0x0E, 0x61, //ChipOptions
            0x00, 0x00, 0x00, 0x00, //certificate formatting disabled
            0x53, 0x00, 0x53, 0x00, 0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x38, 0x00, 0x7C, 0x00, 0x1C, 0x00, 0x3C, 0x00, 0x1A, 0x00, 0x1C, 0x00, 0x10, 0x00, 0x1C, 0x00, 0x30, 0x00, 0x12, 0x00, 0x30, 0x00
            };
            
            //atecc initialize
            ATCA_STATUS fault_status = ATCA_SUCCESS;
            ATCA_STATUS status = atcab_init(cfg_atecc608a_iface);
            if(status!=ATCA_SUCCESS){
            	fault_status = status;
            }

            //check atecc config and data zone lock status
            bool is_locked = false;
            status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_locked);

            if(!is_locked  || status != ATCA_SUCCESS){
				status = atcab_write_config_zone(test_ecc608_configdata);
				if(status!=ATCA_SUCCESS){
	            	fault_status = status;
				}

				status = atcab_lock_config_zone();
            }

            is_locked = false;
            status = atcab_is_locked(LOCK_ZONE_DATA, &is_locked);

            if(!is_locked  || status != ATCA_SUCCESS){
				status = atcab_lock_data_zone();
            }

            provision_status_t provision_status = check_provision_status();
            //check if device already provisioned
            if(provision_status == provision_empty){
                //called again if not locked in previous configuration
            	is_locked = false;
				status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_locked);

				if(!is_locked  || status != ATCA_SUCCESS){
					status = atcab_lock_config_zone();
				}

	            is_locked = false;
	            status = atcab_is_locked(LOCK_ZONE_DATA, &is_locked);

	            if(!is_locked  || status != ATCA_SUCCESS){
					status = atcab_lock_data_zone();
	            }

                //fill whole serial_no with random
                status = atcab_random(serial_no);

                //overwrite date 4bytes
                memcpy(serial_no, provision_date, 4);

                //overwrite hw no 4bytes
                uint32_t u32Temp = (uint32_t)DEVICE_HARDWARE_VERSION;
                memcpy(serial_no+4, &u32Temp, 4);

#if USE_SIMULATOR == 0
                //overwrite MCU UID 12 bytes
                u32Temp = HAL_GetUIDw0();
                memcpy(serial_no+8, &u32Temp, 4);
                u32Temp = HAL_GetUIDw1();
                memcpy(serial_no+12, &u32Temp, 4);
                u32Temp = HAL_GetUIDw2();
                memcpy(serial_no+16, &u32Temp, 4);
#endif

                status = atcab_write_zone(ATCA_ZONE_DATA, slot_8_serial, 0, 0, serial_no, 32);
                if(status != ATCA_SUCCESS){
                	fault_status = status;
                }

                //generate and write IO key
                random_generate(io_protection_key, IO_KEY_SIZE);
                if(status != ATCA_SUCCESS){
                	fault_status = status;
                }

                status = atcab_write_zone(ATCA_ZONE_DATA, slot_6_io_key, 0, 0, io_protection_key, IO_KEY_SIZE);
                if(status != ATCA_SUCCESS){
                	fault_status = status;
                }

                if(fault_status == ATCA_SUCCESS){
                    if(set_io_protection_key(io_protection_key) == SUCCESS_){
                        //locking IO_KEY and serial number slots
                        status = atcab_lock_data_slot(slot_6_io_key);
                        status = atcab_lock_data_slot(slot_8_serial);
                    }
                    else{
                        memset(serial_no, 0, 32);
                        LOG_ERROR("PERR1-IO");

                    }
                }
                else{
                	memset(serial_no, 0, 32);
                    LOG_ERROR("PERR1-0x%02x", fault_status);
                }

            }
            else if(provision_status == provision_incomplete){
                status = atcab_read_zone(ATCA_ZONE_DATA, slot_8_serial, 0, 0, serial_no, 32);
            }
            else{
                 lv_obj_clean(lv_scr_act());
                 mark_error_screen(ui_text_device_already_provisioned);
                 reset_flow_level();
                 flow_level.level_one = 6;
                 flow_level.show_error_screen = true;
                 break;
            }

            transmit_data_to_app(ADD_DEVICE_PROVISION, serial_no, 32);

            flow_level.level_three = PROVISION_STATUS_WAIT;
            lv_task_set_prio(listener_task, LV_TASK_PRIO_MID);  // explicitly enable task listener

            timeout_task = lv_task_create(__timeout_listener, 10000, LV_TASK_PRIO_HIGH, NULL);
            lv_task_once(timeout_task);
        }break;

        case PROVISION_STATUS_WAIT: {
            // do nothing, just wait
        } break;

        case PROVISION_SAVE_EXT_KEYS: {
            ATCA_STATUS status;
            uint8_t private_write_key[36] = {0};
            Perm_Key_Data_Struct perm_key_data;
            uint8_t digest[SHA256_DIGEST_LENGTH];
        	lv_task_del(timeout_task);

            get_io_protection_key(perm_key_data.io_protection_key);

            ecdsa_get_public_key33(&nist256p1, provision_keys_data.device_private_key, perm_key_data.ext_keys.device_auth_public_key);
            if(0 != memcmp(provision_keys_data.device_public_key, perm_key_data.ext_keys.device_auth_public_key, 33)){
                transmit_one_byte_reject(CONFIRM_PROVISION);
                flow_level.level_three = PROVISION_UNSUCCESSFUL;
                break;
            }

            memcpy(&private_write_key[4], provision_keys_data.device_private_key, 32);

            BSP_DelayMs(500);
            status = atcab_priv_write(slot_2_auth_key, private_write_key, slot_6_io_key, perm_key_data.io_protection_key);
            if(status!=ATCA_SUCCESS){
                transmit_one_byte_reject(CONFIRM_PROVISION);
                flow_level.level_three = PROVISION_UNSUCCESSFUL;
                LOG_ERROR("PERR2-0x%02x", status);
                break;
            }


            memset(private_write_key, 0, sizeof(private_write_key));
            memcpy(&private_write_key[4], provision_keys_data.priv_key, 32);

            BSP_DelayMs(500);
            status = atcab_priv_write(slot_3_nfc_pair_key, private_write_key, slot_6_io_key, perm_key_data.io_protection_key);
            if(status!=ATCA_SUCCESS){
                transmit_one_byte_reject(CONFIRM_PROVISION);
                flow_level.level_three = PROVISION_UNSUCCESSFUL;
                LOG_ERROR("PERR2-0x%02x", status);
                break;
            }

            sha256_Raw(provision_keys_data.priv_key, sizeof(provision_keys_data.priv_key), digest);
            memcpy(perm_key_data.ext_keys.self_key_id, digest, FS_KEYSTORE_KEYID_LEN);
            memcpy(perm_key_data.ext_keys.self_key_path, provision_keys_data.self_key_path, FS_KEYSTORE_KEYPATH_LEN);
            memcpy(perm_key_data.ext_keys.card_root_xpub, provision_keys_data.card_root_xpub, FS_KEYSTORE_XPUB_LEN);

            if(set_ext_key(&perm_key_data.ext_keys) == SUCCESS_){
                atcab_lock_data_slot(slot_2_auth_key);
                atcab_lock_data_slot(slot_3_nfc_pair_key);
                atcab_lock_data_slot(slot_0_unused);
                atcab_lock_data_slot(slot_1_unused);
                atcab_lock_data_slot(slot_4_unused);
                atcab_lock_data_slot(slot_7_unused);
                atcab_lock_data_slot(slot_9_unused);
                atcab_lock_data_slot(slot_10_unused);
                atcab_lock_data_slot(slot_11_unused);
                atcab_lock_data_slot(slot_12_unused);
                atcab_lock_data_slot(slot_13_unused);
                atcab_lock_data_slot(slot_14_unused);
                atcab_lock_data_slot(slot_15_unused);
                transmit_one_byte_confirm(CONFIRM_PROVISION);
            }
            else{
                transmit_one_byte_reject(CONFIRM_PROVISION);
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

static void __timeout_listener() {
    mark_error_screen(ui_text_provision_fail);
    instruction_scr_destructor();
    reset_flow_level();
    flow_level.level_one = 6;   // on command not received take to get-started screen
    lv_task_del(timeout_task);
#endif
}

uint8_t atecc_nfc_sign_hash(const uint8_t *hash, uint8_t *sign){
    ATCAIfaceCfg *atca_cfg;
        atca_cfg = cfg_atecc608a_iface;

    ATCA_STATUS status = atcab_init(atca_cfg);

    if(status == ATCA_SUCCESS)
        status = atcab_sign(slot_3_nfc_pair_key, hash, sign);

    return status;
}

uint8_t atecc_nfc_ecdh(const uint8_t *pub_key, uint8_t *shared_secret){
    ATCAIfaceCfg *atca_cfg;
    atca_cfg = cfg_atecc608a_iface;
    uint8_t io_key[IO_KEY_SIZE];

    ATCA_STATUS status = atcab_init(atca_cfg);

    if(get_io_protection_key(io_key) == SUCCESS_)

    if(status == ATCA_SUCCESS)
        status = atcab_ecdh_ioenc(slot_3_nfc_pair_key, pub_key, shared_secret, io_key);
    return status;
}