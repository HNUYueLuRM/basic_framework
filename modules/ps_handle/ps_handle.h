#ifndef PS_HANDLE_H
#define PS_HANDLE_H

#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "bsp_dwt.h"

#define PS2_CS_GPIOx GPIOB
#define PS2_CS_Pin GPIO_PIN_12

#define PS2_CLK_GPIOx GPIOB
#define PS2_CLK_Pin GPIO_PIN_13

#define PS2_DO_GPIOx GPIOB
#define PS2_DO_Pin GPIO_PIN_15

#define PS2_DI_GPIOx GPIOB
#define PS2_DI_Pin GPIO_PIN_14

typedef struct
{
    uint8_t A_D;                                       //模拟(红灯)为1 数字(无灯)为0
    int8_t Rocker_RX, Rocker_RY, Rocker_LX, Rocker_LY; //摇杆值(模拟状态为实际值0-0xFF)(数字态为等效的值0,0x80,0xFF)
    //按键值0为未触发,1为触发态
    uint8_t Key_L1, Key_L2, Key_R1, Key_R2;                //后侧大按键
    uint8_t Key_L_Right, Key_L_Left, Key_L_Up, Key_L_Down; //左侧按键
    uint8_t Key_R_Right, Key_R_Left, Key_R_Up, Key_R_Down; //右侧按键
    uint8_t Key_Select;                                    //选择键
    uint8_t Key_Start;                                     //开始键
    uint8_t Key_Rocker_Left, Key_Rocker_Right;             //摇杆按键

} PS2_Instance;





#endif // !PS_HANDLE_H#define PS_HANDLE_H
