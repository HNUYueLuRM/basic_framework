#include "bmi088_regNdef.h"
#include "bmi088.h"
#include "stdlib.h"
#include "memory.h"

#define REG 0
#define DATA 1
#define ERROR 2
// BMI088初始化配置数组for accel,第一列为reg地址,第二列为写入的配置值,第三列为错误码(如果出错)
static uint8_t BMI088_Accel_Init_Table[BMI088_WRITE_ACCEL_REG_NUM][3] =
    {
        {BMI088_ACC_PWR_CTRL, BMI088_ACC_ENABLE_ACC_ON, BMI088_ACC_PWR_CTRL_ERROR},
        {BMI088_ACC_PWR_CONF, BMI088_ACC_PWR_ACTIVE_MODE, BMI088_ACC_PWR_CONF_ERROR},
        {BMI088_ACC_CONF, BMI088_ACC_NORMAL | BMI088_ACC_800_HZ | BMI088_ACC_CONF_MUST_Set, BMI088_ACC_CONF_ERROR},
        {BMI088_ACC_RANGE, BMI088_ACC_RANGE_6G, BMI088_ACC_RANGE_ERROR},
        {BMI088_INT1_IO_CTRL, BMI088_ACC_INT1_IO_ENABLE | BMI088_ACC_INT1_GPIO_PP | BMI088_ACC_INT1_GPIO_LOW, BMI088_INT1_IO_CTRL_ERROR},
        {BMI088_INT_MAP_DATA, BMI088_ACC_INT1_DRDY_INTERRUPT, BMI088_INT_MAP_DATA_ERROR}};
// BMI088初始化配置数组for gyro,第一列为reg地址,第二列为写入的配置值,第三列为错误码(如果出错)
static uint8_t BMI088_Gyro_Init_Table[BMI088_WRITE_GYRO_REG_NUM][3] =
    {
        {BMI088_GYRO_RANGE, BMI088_GYRO_2000, BMI088_GYRO_RANGE_ERROR},
        {BMI088_GYRO_BANDWIDTH, BMI088_GYRO_2000_230_HZ | BMI088_GYRO_BANDWIDTH_MUST_Set, BMI088_GYRO_BANDWIDTH_ERROR},
        {BMI088_GYRO_LPM1, BMI088_GYRO_NORMAL_MODE, BMI088_GYRO_LPM1_ERROR},
        {BMI088_GYRO_CTRL, BMI088_DRDY_ON, BMI088_GYRO_CTRL_ERROR},
        {BMI088_GYRO_INT3_INT4_IO_CONF, BMI088_GYRO_INT3_GPIO_PP | BMI088_GYRO_INT3_GPIO_LOW, BMI088_GYRO_INT3_INT4_IO_CONF_ERROR},
        {BMI088_GYRO_INT3_INT4_IO_MAP, BMI088_GYRO_DRDY_IO_INT3, BMI088_GYRO_INT3_INT4_IO_MAP_ERROR}};
// @attention : 以上两个数组配合各自的初始化函数使用. 若要修改请参照BMI088 datasheet

// ---------------------------以下私有函数,用于读写BMI088寄存器--------------------------------//
/**
 * @brief 读取BMI088寄存器Accel
 *
 * @param bmi088 待读取的BMI088实例
 * @param reg 待读取的寄存器地址
 * @param dataptr 读取到的数据存放的指针
 * @param len 读取长度
 */
static void BMI088AccelRead(BMI088Instance *bmi088, uint8_t reg, uint8_t *dataptr, uint8_t len)
{
    static uint8_t tx[8] = {0x80, 0}; // 读取,第一个字节为0x80 | reg,第二个是dummy data
    tx[0] |= reg;
    SPITransRecv(bmi088->spi_acc, dataptr, tx, 2); // 第一个先发送reg地址,第二个发送dummy data
    SPIRecv(bmi088->spi_acc, dataptr, len);         // 第三个开始发送数据,别担心,会覆盖掉前面的数据(2个字节)
}

/**
 * @brief 读取BMI088寄存器Gyro
 *
 * @param bmi088 待读取的BMI088实例
 * @param reg  待读取的寄存器地址
 * @param dataptr 读取到的数据存放的指针
 * @param len 读取长度
 */
static void BMI088GyroRead(BMI088Instance *bmi088, uint8_t reg, uint8_t *dataptr, uint8_t len)
{
    static uint8_t tx = 0x80; // 读取,第一个字节为0x80 | reg
    tx |= reg;
    SPITransRecv(bmi088->spi_gyro, dataptr, &tx, 1); // 发送reg地址
    SPIRecv(bmi088->spi_gyro, dataptr, len);         // 别担心,会覆盖掉前面的数据(1个字节)
}

/**
 * @brief 写accel寄存器.对spitransmit形式上的封装
 * @attention 只会向目标reg写入一个字节,因为只有1个字节所以直接传值(指针是32位反而浪费)
 *
 * @param bmi088 待写入的BMI088实例
 * @param reg  待写入的寄存器地址
 * @param data 待写入的数据(注意不是指针)
 */
static void BMI088AccelWrite(BMI088Instance *bmi088, uint8_t reg, uint8_t data)
{
    SPITransmit(bmi088->spi_acc, &data, 1);
}

/**
 * @brief 写gyro寄存器.形式上的封装
 * @attention 只会向目标reg写入一个字节,因为只有1个字节所以直接传值(指针是32位反而浪费)
 *
 * @param bmi088 待写入的BMI088实例
 * @param reg  待写入的寄存器地址
 * @param data 待写入的数据(注意不是指针)
 */
static void BMI088GyroWrite(BMI088Instance *bmi088, uint8_t reg, uint8_t data)
{
    SPITransmit(bmi088->spi_gyro, &data, 1);
}

// -------------------------以上为私有函数,用于读写BMI088寄存器--------------------------------//

// -------------------------以下为私有函数,用于初始化BMI088acc和gyro--------------------------------//

/**
 * @brief 初始化BMI088加速度计,提高可读性分拆功能
 *
 * @param bmi088 待初始化的BMI088实例
 * @return uint8_t ERROR CODE if any problems here
 */
static uint8_t BMI088AccelInit(BMI088Instance *bmi088)
{
    // 后续添加reset和通信检查
    // code to go here ...

    // 检查ID,如果不是0x1E(bmi088 whoami寄存器值),则返回错误
    uint8_t whoami_check = 0;
    BMI088AccelRead(bmi088, BMI088_ACC_CHIP_ID, &whoami_check, 1);
    if (whoami_check != BMI088_ACC_CHIP_ID_VALUE)
        return BMI088_NO_SENSOR;

    // 初始化寄存器,提高可读性
    uint8_t reg = 0;
    uint8_t data = 1;
    uint8_t error = 2;
    // 使用sizeof而不是magic number,这样如果修改了数组大小,不用修改这里的代码;或者使用宏定义
    for (uint8_t i = 0; i < sizeof(BMI088_Accel_Init_Table) / sizeof(BMI088_Accel_Init_Table[0]); i++)
    {
        reg = BMI088_Accel_Init_Table[i][REG];
        data = BMI088_Accel_Init_Table[i][DATA];
        BMI088AccelWrite(bmi088, reg, data);    // 写入寄存器
        BMI088AccelRead(bmi088, reg, &data, 1); // 写完之后立刻读回检查
        if (data != BMI088_Accel_Init_Table[i][DATA])
            error |= BMI088_Accel_Init_Table[i][ERROR];
        //{i--;} 可以设置retry次数,如果retry次数用完了,则返回error
    }
    return error;
}

/**
 * @brief 初始化BMI088陀螺仪,提高可读性分拆功能
 *
 * @param bmi088 待初始化的BMI088实例
 * @return uint8_t ERROR CODE
 */
static uint8_t BMI088GyroInit(BMI088Instance *bmi088)
{
    // 后续添加reset和通信检查
    // code to go here ...

    // 检查ID,如果不是0x0F(bmi088 whoami寄存器值),则返回错误
    uint8_t whoami_check = 0;
    BMI088GyroRead(bmi088, BMI088_GYRO_CHIP_ID, &whoami_check, 1);
    if (whoami_check != BMI088_GYRO_CHIP_ID_VALUE)
        return BMI088_NO_SENSOR;

    // 初始化寄存器,提高可读性
    uint8_t reg = 0;
    uint8_t data = 1;
    uint8_t error = 2;
    // 使用sizeof而不是magic number,这样如果修改了数组大小,不用修改这里的代码;或者使用宏定义
    for (uint8_t i = 0; i < sizeof(BMI088_Gyro_Init_Table) / sizeof(BMI088_Gyro_Init_Table[0]); i++)
    {
        reg = BMI088_Gyro_Init_Table[i][REG];
        data = BMI088_Gyro_Init_Table[i][DATA];
        BMI088GyroWrite(bmi088, reg, data);    // 写入寄存器
        BMI088GyroRead(bmi088, reg, &data, 1); // 写完之后立刻读回检查
        if (data != BMI088_Gyro_Init_Table[i][DATA])
            error |= BMI088_Gyro_Init_Table[i][ERROR];
        //{i--;} 可以设置retry次数,如果retry次数用完了,则返回error
    }
    return error;
}
// -------------------------以上为私有函数,用于初始化BMI088acc和gyro--------------------------------//

// 考虑阻塞模式和非阻塞模式的兼容性,通过条件编译(则需要在编译前修改宏定义)或runtime参数判断
// runtime的开销不大(一次性判断),但是需要修改函数原型,增加参数,代码长度增加(但不多)
// runtime如何修改callback?根据参数选择是否给spi传入callback,如果是阻塞模式,则不传入callback,如果是非阻塞模式,则传入callback(bsp会检查是否NULL)
// 条件编译的开销小,但是需要修改宏定义,增加编译时间,同时人力介入
// 根据实际情况选择(说了和没说一样!)

BMI088Instance *BMI088Register(BMI088_Init_Config_s *config)
{
    
}
