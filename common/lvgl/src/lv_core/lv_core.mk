SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_group.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_indev.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_disp.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_obj.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_refr.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_core/lv_style.c

SRC_FILES += \
$(OLED_DIR)/lvgl/porting/lv_port_disp.c \
$(OLED_DIR)/lvgl/porting/lv_port_indev.c \

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_core
VPATH += :$(OLED_DIR)/lvgl/src/lv_core

CFLAGS += -I$(OLED_DIR)/lvgl/src/lv_core
