#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include "struct_typedef.h"

extern void buzzer_on(uint16_t psc, uint16_t pwm,uint8_t level);
extern void buzzer_off(void);

#endif
