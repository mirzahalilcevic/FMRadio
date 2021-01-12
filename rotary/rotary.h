#ifndef __ROTARY_H
#define __ROTARY_H

// PB3 -> pinA (DT)
// PB5 -> pinB (CLK)

#include "stm32f4xx.h"
#include <stdint.h>

extern volatile uint8_t button_state;
extern volatile int32_t RE_Count;
extern volatile int32_t previous_RE_Count;

void init_rotary_encoder(void);
void serviceIRQA(void);

#endif /* ifndef __ROTARY_H */
