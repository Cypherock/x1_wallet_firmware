#include "board.h"
#include "buzzer.h"
#include "events.h"
#include "nfc.h"
#include "nfc_events.h"
#include "ui_instruction.h"

#ifdef NFC_EVENT_CARD_DETECT_MANUAL_TEST

void test_nfc_operation_with_os_getter() {
  evt_config_t evt_config = {.evt_selection.bits.nfc_events = 1,
                             .evt_selection.bits.ui_events = 0,
                             .evt_selection.bits.usb_events = 0,
                             .abort_disabled = true,
                             .timeout = 500000};
  evt_status_t evt_status = {0};

  instruction_scr_init("Place a card", "NFC EVENT TEST");
  lv_task_handler();

  nfc_en_select_card_task();

  get_events(evt_config, &evt_status);

  if (evt_status.nfc_event.event_occured) {
    instruction_scr_change_text("Card Detected", true);
    buzzer_start(500);
  } else {
    instruction_scr_change_text("Something bad happened", true);
  }

  nfc_select_card();
  instruction_scr_change_text("Remove card", true);

  uint32_t err = nfc_en_wait_for_card_removal_task();

  if (err != 0) {
    LOG_CRITICAL("xxx39 %08x", err);
    instruction_scr_change_text("Something bad happened 2", true);
  } else {
    get_events(evt_config, &evt_status);

    if (evt_status.nfc_event.event_occured) {
      instruction_scr_change_text("Card Removed", true);
      buzzer_start(500);
    } else {
      instruction_scr_change_text("Something bad happened 3", true);
    }
  }

  BSP_DelayMs(5000);
  instruction_scr_destructor();
  lv_task_handler();
}
#endif