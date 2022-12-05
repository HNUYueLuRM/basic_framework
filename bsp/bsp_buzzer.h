#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include <stdint-gcc.h>

void BuzzerInit();
extern void BuzzerOn(uint16_t psc, uint16_t pwm, uint8_t level);
extern void BuzzerOff(void);

#endif
