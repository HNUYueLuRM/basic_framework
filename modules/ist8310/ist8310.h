#pragma once

#include "bsp_iic.h"
#include "bsp_gpio.h"
#include "stdint.h"

// 传感器灵敏度系数
#define MAG_SEN 0.3f // raw int16 data change to uT unit. 原始整型数据变成 单位ut

// IST8310 ret ERROR CODE
#define IST8310_NO_ERROR 0x00    // seldom used. 一般不会用到
#define IST8310_NO_SENSOR 0x40   // seldom used. 一般不会用到
#define IST8310_IIC_ADDRESS 0x0E // the I2C slave address of IST8310

/**
 * @brief IST8310 实例定义
 *
 */
typedef struct tempist8310
{
    IICInstance *iic;             // iic实例
    GPIOInstance* gpio_rst;       // gpio实例,用于复位
    GPIOInstance* gpio_exti;      // gpio实例,用于获取MAG_DRDY引脚状态,判断数据是否准备好(EXTI外部中断)
    uint8_t iic_buffer[8];        // iic接收缓冲区
    float mag[3];                 // 三轴磁力计数据,[x,y,z]

    void (*ist_module_callback)(IICInstance *); // 模块回调函数
    // 后续有需要再添加
    // ...
} IST8310Instance;

/**
 * @brief IST8310 初始化配置结构体
 *
 */
typedef struct
{
    IIC_Init_Config_s iic_config; // iic初始化配置
    GPIO_Init_Config_s gpio_conf_rst;  // gpio初始化配置,用于复位
    GPIO_Init_Config_s gpio_conf_exti; // gpio初始化配置,用于获取MAG_DRDY引脚状态,判断数据是否准备好(EXTI外部中断)
} IST8310_Init_Config_s;

/**
 * @brief IST8310 初始化.
 * @note  一条i2c总线只能挂载一个IST8310
 *
 */
IST8310Instance *IST8310Init(IST8310_Init_Config_s *config);

void rest(int aa);

int nit(int bb);
