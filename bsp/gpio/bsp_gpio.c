#include "bsp_gpio.h"
#include "memory.h"
#include "stdlib.h"

static uint8_t idx;
static GPIOInstance* gpio_instance[GPIO_MX_DEVICE_NUM] = {NULL};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    
}


GPIOInstance *GPIORegister(GPIO_Init_Config_s *GPIO_config)
{
    GPIOInstance *ins=(GPIOInstance*)malloc(sizeof(GPIOInstance));
    memset(ins,0,sizeof(GPIOInstance));

    ins->GPIOx=GPIO_config->GPIOx;
    ins->GPIO_Pin=GPIO_config->GPIO_Pin;
    ins->id=GPIO_config->id;
    ins->gpio_model_callback=GPIO_config->gpio_model_callback;

    gpio_instance[idx++]=ins;
    return ins;
}

// ----------------- GPIO API -----------------
// 都是对HAL的形式上的封装,后续考虑增加GPIO state变量,可以直接读取state

void GPIOToggel(GPIOInstance *_instance)
{
    HAL_GPIO_TogglePin(_instance->GPIOx,_instance->GPIO_Pin);
}

void GPIOSet(GPIOInstance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx,_instance->GPIO_Pin,GPIO_PIN_SET);
}

void GPIOReset(GPIOInstance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx,_instance->GPIO_Pin,GPIO_PIN_RESET);
}

GPIO_PinState GPIORead(GPIOInstance *_instance)
{
    return HAL_GPIO_ReadPin(_instance->GPIOx,_instance->GPIO_Pin);
}
