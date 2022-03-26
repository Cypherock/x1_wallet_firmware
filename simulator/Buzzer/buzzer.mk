SRC_FILES += $(DRIVER_DIR)/Buzzer/buzzer.c

DEPPATH += --dep-path $(DRIVER_DIR)/Buzzer
VPATH += :$(DRIVER_DIR)/Buzzer

CFLAGS += -I$(DRIVER_DIR)/Buzzer