#ifndef __BMI088_H__ // 防止重复包含
#define __BMI088_H__

#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "controller.h"
#include "bsp_pwm.h"
#include "stdint.h"

#define BMI088_PRE_CALI_ACC_X_OFFSET 0.0f
#define BMI088_PRE_CALI_ACC_Y_OFFSET 0.0f
// macro to go here... 预设标定参数

// bmi088工作模式枚举
typedef enum
{
    BMI088_BLOCK_PERIODIC_MODE = 0, // 阻塞模式,周期性读取
    BMI088_BLOCK_TRIGGER_MODE,      // 阻塞模式,触发读取(中断)
} BMI088_Work_Mode_e;

// bmi088标定方式枚举,若使用预设标定参数,注意修改预设参数
typedef enum
{
    BMI088_CALIBRATE_MODE = 0, // 初始化时进行标定
    BMI088_LOAD_PRE_CALI_MODE, // 使用预设标定参数,
} BMI088_Calibrate_Mode_e;

/* BMI088实例结构体定义 */
typedef struct
{
    // 传输模式和工作模式控制
    BMI088_Work_Mode_e work_mode;
    BMI088_Calibrate_Mode_e cali_mode;
    // SPI接口
    SPIInstance *spi_gyro; // 注意,SPIInstnace内部也有一个GPIOInstance,用于控制片选CS
    SPIInstance *spi_acc;  // 注意,SPIInstnace内部也有一个GPIOInstance,用于控制片选CS
    // EXTI GPIO,如果BMI088工作在中断模式,则需要配置中断引脚(有数据产生时触发解算)
    GPIOInstance *gyro_int;
    GPIOInstance *acc_int;
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
    // 传感器灵敏度,用于计算实际值(regNdef.h中定义)
    float BMI088_ACCELL_SEN;
    float BMI088_GYRO_SEN;
    // 用于计算两次采样的时间间隔
    uint32_t bias_dwt_cnt;
} BMI088Instance;

/* BMI088初始化配置 */
typedef struct
{
    SPI_Init_Config_s spi_gyro_config;
    SPI_Init_Config_s spi_acc_config;
    GPIO_Init_Config_s gyro_int_config;
    GPIO_Init_Config_s acc_int_config;
    BMI088_Work_Mode_e work_mode;
    BMI088_Calibrate_Mode_e cali_mode;
    PID_Init_Config_s heat_pid_config;
    PWM_Init_Config_s heat_pwm_config;
} BMI088_Init_Config_s;


/**
 * @brief 初始化BMI088,返回BMI088实例指针
 * @note  一般一个开发板只有一个BMI088,所以这里就叫BMI088Init而不是Register
 *
 * @param config bmi088初始化配置
 * @return BMI088Instance* 实例指针
 */
BMI088Instance *BMI088Register(BMI088_Init_Config_s *config);

#endif // !__BMI088_H__