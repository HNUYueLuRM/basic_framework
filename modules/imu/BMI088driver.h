/**
 ******************************************************************************
 * @file    BMI088driver.h
 * @author
 * @version V1.1.2
 * @version V1.2.0
 * @date    2022/3/8
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef BMI088DRIVER_H
#define BMI088DRIVER_H

#include "stdint.h"
#include "main.h"

#define BMI088_TEMP_FACTOR 0.125f
#define BMI088_TEMP_OFFSET 23.0f

#define BMI088_WRITE_ACCEL_REG_NUM 6
#define BMI088_WRITE_GYRO_REG_NUM 6

#define BMI088_GYRO_DATA_READY_BIT 0
#define BMI088_ACCEL_DATA_READY_BIT 1
#define BMI088_ACCEL_TEMP_DATA_READY_BIT 2

#define BMI088_LONG_DELAY_TIME 80
#define BMI088_COM_WAIT_SENSOR_TIME 150

#define BMI088_ACCEL_IIC_ADDRESSE (0x18 << 1)
#define BMI088_GYRO_IIC_ADDRESSE (0x68 << 1)

#define BMI088_ACCEL_3G_SEN 0.0008974358974f
#define BMI088_ACCEL_6G_SEN 0.00179443359375f
#define BMI088_ACCEL_12G_SEN 0.0035888671875f
#define BMI088_ACCEL_24G_SEN 0.007177734375f

#define BMI088_GYRO_2000_SEN 0.00106526443603169529841533860381f
#define BMI088_GYRO_1000_SEN 0.00053263221801584764920766930190693f
#define BMI088_GYRO_500_SEN 0.00026631610900792382460383465095346f
#define BMI088_GYRO_250_SEN 0.00013315805450396191230191732547673f
#define BMI088_GYRO_125_SEN 0.000066579027251980956150958662738366f

// 需手动修改
#if INFANTRY_ID == 0
#define GxOFFSET 0.00247530174f
#define GyOFFSET 0.000393082853f
#define GzOFFSET 0.000393082853f
#define gNORM 9.69293118f
#elif INFANTRY_ID == 1
#define GxOFFSET 0.0007222f
#define GyOFFSET -0.001786f
#define GzOFFSET 0.0004346f
#define gNORM 9.876785f
#elif INFANTRY_ID == 2
#define GxOFFSET 0.0007222f
#define GyOFFSET -0.001786f
#define GzOFFSET 0.0004346f
#define gNORM 9.876785f
#elif INFANTRY_ID == 3
#define GxOFFSET 0.00270364084f
#define GyOFFSET -0.000532632112f
#define GzOFFSET 0.00478090625f
#define gNORM 9.73574924f
#elif INFANTRY_ID == 4
#define GxOFFSET 0.0007222f
#define GyOFFSET -0.001786f
#define GzOFFSET 0.0004346f
#define gNORM 9.876785f
#endif

typedef struct
{
    float Accel[3];

    float Gyro[3];

    float TempWhenCali;
    float Temperature;

    float AccelScale;
    float GyroOffset[3];

    float gNorm;
} IMU_Data_t;

enum
{
    BMI088_NO_ERROR = 0x00,
    BMI088_ACC_PWR_CTRL_ERROR = 0x01,
    BMI088_ACC_PWR_CONF_ERROR = 0x02,
    BMI088_ACC_CONF_ERROR = 0x03,
    BMI088_ACC_SELF_TEST_ERROR = 0x04,
    BMI088_ACC_RANGE_ERROR = 0x05,
    BMI088_INT1_IO_CTRL_ERROR = 0x06,
    BMI088_INT_MAP_DATA_ERROR = 0x07,
    BMI088_GYRO_RANGE_ERROR = 0x08,
    BMI088_GYRO_BANDWIDTH_ERROR = 0x09,
    BMI088_GYRO_LPM1_ERROR = 0x0A,
    BMI088_GYRO_CTRL_ERROR = 0x0B,
    BMI088_GYRO_INT3_INT4_IO_CONF_ERROR = 0x0C,
    BMI088_GYRO_INT3_INT4_IO_MAP_ERROR = 0x0D,

    BMI088_SELF_TEST_ACCEL_ERROR = 0x80,
    BMI088_SELF_TEST_GYRO_ERROR = 0x40,
    BMI088_NO_SENSOR = 0xFF,
};

void BMI088_Init(SPI_HandleTypeDef *bmi088_SPI, uint8_t calibrate);
extern uint8_t BMI088_init(SPI_HandleTypeDef *bmi088_SPI, uint8_t calibrate);
extern uint8_t bmi088_accel_init(void);
extern uint8_t bmi088_gyro_init(void);

extern IMU_Data_t BMI088;

extern void BMI088_Read(IMU_Data_t *bmi088);

#endif
