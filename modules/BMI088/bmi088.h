#ifndef __BMI088_H__ // 防止重复包含
#define __BMI088_H__

#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "controller.h"
#include "bsp_pwm.h"
#include "stdint.h"

// bmi088工作模式枚举
typedef enum
{
    BMI088_BLOCK_PERIODIC_MODE = 0, // 阻塞模式,周期性读取
    BMI088_BLOCK_TRIGGER_MODE,      // 阻塞模式,触发读取(中断)
} BMI088_Work_Mode_e;

// bmi088标定方式枚举,若使用预设标定参数,注意修改预设参数
typedef enum
{
    BMI088_CALIBRATE_ONLINE_MODE = 0, // 初始化时进行标定
    BMI088_LOAD_PRE_CALI_MODE,        // 使用预设标定参数,
} BMI088_Calibrate_Mode_e;

#pragma pack(1) // 1字节对齐
/* BMI088数据*/
typedef struct
{
    float gyro[3];     // 陀螺仪数据,xyz
    float acc[3];      // 加速度计数据,xyz
    float temperature; // 温度

    // float timestamp; // 时间戳,单位为ms,用于计算两次采样的时间间隔,同时给视觉提供timeline
    // uint32_t count;  // 第count次采样,用于对齐时间戳
} BMI088_Data_t;
#pragma pack() // 恢复默认对齐,需要传输的结构体务必开启1字节对齐

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
    PIDInstance heat_pid; // 恒温PID
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
    float BMI088_ACCEL_SEN;
    float BMI088_GYRO_SEN;
    // 用于计算两次采样的时间间隔
    uint32_t bias_dwt_cnt;
    // 数据更新标志位
    struct // 位域,节省空间提高可读性
    {
        uint8_t gyro : 1; // 1:有新数据,0:无新数据
        uint8_t acc : 1;
        uint8_t temp : 1;
        uint8_t gyro_overrun : 1; // 1:数据溢出,0:无溢出
        uint8_t acc_overrun : 1;
        uint8_t temp_overrun : 1;
        uint8_t imu_ready : 1; // 1:IMU数据准备好,0:IMU数据未准备好(gyro+acc)
        // 后续可添加其他标志位,不够用可以扩充16or32,太多可以删
    } update_flag;
} BMI088Instance;

/* BMI088初始化配置 */
typedef struct
{
    BMI088_Work_Mode_e work_mode;
    BMI088_Calibrate_Mode_e cali_mode;
    SPI_Init_Config_s spi_gyro_config;
    SPI_Init_Config_s spi_acc_config;
    GPIO_Init_Config_s gyro_int_config;
    GPIO_Init_Config_s acc_int_config;
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

/**
 * @brief 读取BMI088数据
 * @param bmi088 BMI088实例指针
 * @return BMI088_Data_t 读取到的数据
 */
uint8_t BMI088Acquire(BMI088Instance *bmi088,BMI088_Data_t* data_store);

/**
 * @brief 标定传感器.BMI088在初始化的时候会调用此函数. 提供接口方便标定离线数据
 * @attention @todo 注意,当操作系统开始运行后,此函数会和INS_Task冲突.目前不允许在运行时调用此函数,后续加入标志位判断以提供运行时重新的标定功能
 *
 * @param _bmi088 待标定的实例
 */
void BMI088CalibrateIMU(BMI088Instance *_bmi088);

#endif // !__BMI088_H__