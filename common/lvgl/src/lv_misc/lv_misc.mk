SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_circ.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_area.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_task.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_fs.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_anim.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_mem.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_ll.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_color.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_txt.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_math.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_log.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_gc.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_utils.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_misc/lv_async.c

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_misc
VPATH += :$(OLED_DIR)/lvgl/src/lv_misc

CFLAGS += -I$(OLED_DIR)/lvgl/src/lv_misc
