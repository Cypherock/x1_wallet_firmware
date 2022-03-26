SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_basic.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_rect.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_label.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_line.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_img.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_arc.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_draw_triangle.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_img_decoder.c
SRC_FILES += $(OLED_DIR)/lvgl/src/lv_draw/lv_img_cache.c

DEPPATH += --dep-path $(OLED_DIR)/lvgl/src/lv_draw
VPATH += :$(OLED_DIR)/lvgl/src/lv_draw

CFLAGS += -I$(OLED_DIR)lvgl/src/lv_draw
