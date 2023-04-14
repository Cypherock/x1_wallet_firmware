#include "board.h"
#include "buzzer.h"
#include "events.h"
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

  nfc_enable_card_detect_event();
  instruction_scr_init("Place a card", "NFC EVENT TEST");
  lv_task_handler();

  get_events(evt_config, &evt_status);

  if (evt_status.nfc_event.event_occured) {
    instruction_scr_change_text("Card Detected", true);
    buzzer_start(500);
  } else {
    instruction_scr_change_text("Something bad happened", true);
  }

  BSP_DelayMs(5000);
  nfc_disable_card_detect_event();
  instruction_scr_destructor();
  lv_task_handler();
}
#endif