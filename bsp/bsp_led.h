#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint-gcc.h>

void LED_init();
extern void aRGB_led_show(uint32_t aRGB);

#endif