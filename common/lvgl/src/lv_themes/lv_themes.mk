SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_alien.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_default.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_night.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_templ.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_zen.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_material.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_nemo.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_themes/lv_theme_mono.c

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_themes
VPATH += :$(OLED_DIR)/lvgl/src/lv_themes

CFLAGS += -I$(OLED_DIR)/lvgl/src/lv_themes
