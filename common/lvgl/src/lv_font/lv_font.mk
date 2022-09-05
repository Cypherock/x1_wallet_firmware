SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_fmt_txt.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_roboto_12.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_roboto_16.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_roboto_22.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_roboto_28.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/lv_font_unscii_8.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/my_logo.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_font/nfc_logo.c

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_font
VPATH += :$(OLED_DIR)/lvgl/src/lv_font

CFLAGS += -I$(OLED_DIR)/lvgl/src/lv_font
