#ifndef _LED_H_
#define _LED_H_

#include "stdint.h"
#include "bsp_pwm.h"

#define LED_MAX_NUM 3

typedef struct
{
    PWMInstance* led_pwm;
    uint8_t led_alpha; // 透明度,通过pwm频率改变
    uint8_t led_brightness; // 亮度,通过电压改变(如果可以,使用dac)
    uint8_t led_color; // rgb value,0-255
    uint8_t led_switch; // 开关,on1 off0
    // void (*action_callback)(void); // led动作回调函数
} LEDInstance;

typedef struct
{
    PWM_Init_Config_s pwm_config;
    uint8_t init_swtich; // 初始化开关
} LED_Init_Config_s;

LEDInstance* LEDRegister(LED_Init_Config_s* led_config);

void LEDSet(LEDInstance *_led,uint8_t alpha,uint8_t color_value,uint8_t brightness);

void LEDSwitch(LEDInstance *_led,uint8_t led_switch);

void LEDShow();

#endif // !_LED_H_
