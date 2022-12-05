#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint-gcc.h>

void LEDInit();
extern void FlowRGBShow(uint32_t aRGB);

#endif