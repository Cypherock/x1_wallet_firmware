#ifndef BUZZER_H
#define BUZZER_H

#include "board.h"
#include "stdint.h"

// Pins for Reader are defined here
#define BUZZER_DURATION 100

void buzzer_init(void);
void buzzer_start(uint32_t);

#endif
