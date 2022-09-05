#include "cy_factory_reset.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_multi_instruction.h"
#include "ui_confirmation.h"

void cyt_factory_reset() {
    switch (flow_level.level_three) {
        case FACTORY_RESET_INFO:
            multi_instruction_init(ui_text_factory_reset_confirm, 3, DELAY_LONG_STRING, true);
            break;

        case FACTORY_RESET_CONFIRM:
            confirm_scr_init(ui_text_confirm_factory_reset);
            break;

        case FACTORY_RESET_TAP_CARD1:
            instruction_scr_init(ui_text_tap_1_2_cards);
            instruction_scr_change_text(ui_text_tap_1_2_cards, true);
            mark_event_over();
            break;

        case FACTORY_RESET_TAP_CARD2:
            instruction_scr_init(ui_text_tap_2_2_cards);
            instruction_scr_change_text(ui_text_tap_2_2_cards, true);
            mark_event_over();
            break;

        case FACTORY_RESET_CHECK:
            instruction_scr_init(ui_text_performing_checks);
            mark_event_over();
            break;

        case FACTORY_RESET_ERASING:
            instruction_scr_destructor();
            delay_scr_init(ui_text_erasing, DELAY_TIME);
            break;

        case FACTORY_RESET_CANCEL: {
            char msg[64] = "\0";
            const char *msg_list[3];
            instruction_scr_destructor();
            msg_list[0] = msg;
            msg_list[1] = ui_text_reset_exit[1];
            msg_list[2] = ui_text_reset_exit[2];
            snprintf(msg, sizeof(msg), ui_text_reset_exit[0], get_wallet_name(factory_reset_mismatch_wallet_index));
            multi_instruction_init(msg_list, 3, DELAY_LONG_STRING, true);
        } break;

        default:
            mark_event_over();
    }
}