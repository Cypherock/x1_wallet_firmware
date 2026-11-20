/**
 * @file    hedera_main.c
 * @author  Cypherock X1 Team
 * @brief   A common entry point to various Hedera coin actions.
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 */
#include "hedera_main.h"
#include "hedera_api.h"
#include "hedera_priv.h"
#include "status_api.h"

void hedera_main(usb_event_t usb_evt, const void *hedera_app_config);

static const cy_app_desc_t hedera_app_desc = {
    .id = 24, // IMPORTANT: Use an unused app ID here.
    .version = {.major = 1, .minor = 0, .patch = 0},
    .app = hedera_main,
    .app_config = NULL
};

void hedera_main(usb_event_t usb_evt, const void *hedera_app_config) {
    hedera_query_t query = HEDERA_QUERY_INIT_DEFAULT;

    if (!decode_hedera_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
        return;
    }

    core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

    switch ((uint8_t)query.which_request) {
        case HEDERA_QUERY_GET_PUBLIC_KEYS_TAG:
        case HEDERA_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG: {
            hedera_get_pub_keys(&query);
            break;
        }
        case HEDERA_QUERY_SIGN_TXN_TAG: {
            hedera_sign_transaction(&query);
            break;
        }
        default: {
            hedera_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, ERROR_DATA_FLOW_INVALID_QUERY);
            break;
        }
    }
}

const cy_app_desc_t *get_hedera_app_desc() {
    return &hedera_app_desc;
}