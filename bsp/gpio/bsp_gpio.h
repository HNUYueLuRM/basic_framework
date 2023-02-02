#include "gpio.h"
#include "stdint.h"

#define GPIO_MX_DEVICE_NUM 10

/**
 * @brief GPIO实例结构体定义
 * 
 */
typedef struct tmpgpio 
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    void* id;
    void (*gpio_model_callback)(struct tmpgpio*); // 随便取个名字当临时声明
} GPIOInstance;

/**
 * @brief GPIO初始化配置结构体定义
 * 
 */
typedef struct
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    void* id;
    void (*gpio_model_callback)(GPIOInstance*);
} GPIO_Init_Config_s;

GPIOInstance* GPIORegister(GPIO_Init_Config_s* GPIO_config);

void GPIOToggel(GPIOInstance* _instance);

void GPIOSet(GPIOInstance* _instance);

void GPIOReset(GPIOInstance* _instance);

GPIO_PinState GPIORead(GPIOInstance* _instance);