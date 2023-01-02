#ifndef __BMI088_H__
#define __BMI088_H__

#include "bsp_spi.h"
#include "controller.h"
#include "bsp_pwm.h"
#include "stdint.h"

typedef enum
{
    BMI088_CALIBRATE_MODE = 0, // 初始化时进行标定
    BMI088_LOAD_PRE_CALI_MODE, // 使用预设标定参数
} BMI088_Work_Mode_e;

/* BMI088实例结构体定义 */
typedef struct
{
    // 传输模式和工作模式控制
    SPIInstance *spi_gyro;
    SPIInstance *spi_acc;
    // 温度控制
    PIDInstance *heat_pid; // 恒温PID
    PWMInstance *heat_pwm; // 加热PWM
    // IMU数据
    float gyro[3];     // 陀螺仪数据,xyz
    float acc[3];      // 加速度计数据,xyz
    float temperature; // 温度
    // 标定数据
    float gyro_offset[3]; // 陀螺仪零偏
    float gNorm;          // 重力加速度模长,从标定获取
    float acc_coef;       // 加速度计原始数据转换系数
    // 用于计算两次采样的时间间隔
    uint32_t bias_dwt_cnt;
} BMI088Instance;

/* BMI088初始化配置 */
typedef struct
{
    SPI_Init_Config_s spi_gyro_config;
    SPI_Init_Config_s spi_acc_config;
    BMI088_Work_Mode_e mode;
    PID_Init_Config_s heat_pid_config;
    PWM_Init_Config_s heat_pwm_config;
} BMI088_Init_Config_s;



#endif // !__BMI088_H__