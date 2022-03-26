SRC_FILES += $(OLED_DIR)/lvgl/src/lv_hal/lv_hal_disp.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_hal/lv_hal_indev.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_hal/lv_hal_tick.c

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_hal
VPATH += :$(OLED_DIR)/lvgl/src/lv_hal

CFLAGS += -I$(OLED_DIR)/lvgl/src/lv_hal
