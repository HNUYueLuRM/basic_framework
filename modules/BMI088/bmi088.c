#include "bmi088_regNdef.h"
#include "bmi088.h"
#include "user_lib.h"

// ---------------------------以下私有函数,用于读写BMI088寄存器封装,blocking--------------------------------//
/**
 * @brief 读取BMI088寄存器Accel. BMI088要求在不释放CS的情况下连续读取
 *
 * @param bmi088 待读取的BMI088实例
 * @param reg 待读取的寄存器地址
 * @param dataptr 读取到的数据存放的指针
 * @param len 读取长度
 */
static void BMI088AccelRead(BMI088Instance *bmi088, uint8_t reg, uint8_t *dataptr, uint8_t len)
{
    if (len > 6)
        while (1)
            ;
    // 一次读取最多6个字节,加上两个dummy data    第一个字节的第一个位是读写位,1为读,0为写,1-7bit是寄存器地址
    static uint8_t tx[8]; // 读取,第一个字节为0x80|reg ,第二个是dummy data,后面的没用都是dummy write
    static uint8_t rx[8]; // 前两个字节是dummy data,第三个开始是真正的数据
    tx[0] = 0x80 | reg;   // 静态变量每次进来还是上次的值,所以要每次都要给tx[0]赋值0x80
    SPITransRecv(bmi088->spi_acc, rx, tx, len + 2);
    memcpy(dataptr, rx + 2, len); // @todo : memcpy有额外开销,后续可以考虑优化,在SPI中加入接口或模式,使得在一次传输结束后不释放CS,直接接着传输
}

/**
 * @brief 读取BMI088寄存器Gyro, BMI088要求在不释放CS的情况下连续读取
 *
 * @param bmi088 待读取的BMI088实例
 * @param reg  待读取的寄存器地址
 * @param dataptr 读取到的数据存放的指针
 * @param len 读取长度
 */
static void BMI088GyroRead(BMI088Instance *bmi088, uint8_t reg, uint8_t *dataptr, uint8_t len)
{
    if (len > 6)
        while (1)
            ;
    // 一次读取最多6个字节,加上一个dummy data  ,第一个字节的第一个位是读写位,1为读,0为写,1-7bit是寄存器地址
    static uint8_t tx[7] = {0x80}; // 读取,第一个字节为0x80 | reg ,之后是dummy data
    static uint8_t rx[7];          // 第一个是dummy data,第三个开始是真正的数据
    tx[0] = 0x80 | reg;
    SPITransRecv(bmi088->spi_gyro, rx, tx, len + 1);
    memcpy(dataptr, rx + 1, len); // @todo : memcpy有额外开销,后续可以考虑优化,在SPI中加入接口或模式,使得在一次传输结束后不释放CS,直接接着传输
}

/**
 * @brief 写accel寄存器.对spitransmit形式上的封装
 * @attention 只会向目标reg写入一个字节,因为只有1个字节所以直接传值(指针是32位反而浪费)
 *
 * @param bmi088 待写入的BMI088实例
 * @param reg  待写入的寄存器地址
 * @param data 待写入的数据(注意不是指针)
 */
static void BMI088AccelWriteSingleReg(BMI088Instance *bmi088, uint8_t reg, uint8_t data)
{
    uint8_t tx[2] = {reg, data};
    SPITransmit(bmi088->spi_acc, tx, 2);
}

/**
 * @brief 写gyro寄存器.形式上的封装
 * @attention 只会向目标reg写入一个字节,因为只有1个字节所以直接传值(指针是32位反而浪费)
 *
 * @param bmi088 待写入的BMI088实例
 * @param reg  待写入的寄存器地址
 * @param data 待写入的数据(注意不是指针)
 */
static void BMI088GyroWriteSingleReg(BMI088Instance *bmi088, uint8_t reg, uint8_t data)
{
    uint8_t tx[2] = {reg, data};
    SPITransmit(bmi088->spi_gyro, tx, 2);
}
// -------------------------以上为私有函数,封装了BMI088寄存器读写函数,blocking--------------------------------//

// -------------------------以下为私有函数,用于初始化BMI088acc和gyro的硬件和配置--------------------------------//
#define BMI088REG 0
#define BMI088DATA 1
#define BMI088ERROR 2
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

/**
 * @brief 初始化BMI088加速度计,提高可读性分拆功能
 *
 * @param bmi088 待初始化的BMI088实例
 * @return uint8_t BMI088ERROR CODE if any problems here
 */
static uint8_t BMI088AccelInit(BMI088Instance *bmi088)
{
    uint8_t whoami_check = 0;

    // 加速度计以I2C模式启动,需要一次上升沿来切换到SPI模式,因此进行一次fake write
    BMI088AccelRead(bmi088, BMI088_ACC_CHIP_ID, &whoami_check, 1);
    DWT_Delay(0.001);

    BMI088AccelWriteSingleReg(bmi088, BMI088_ACC_SOFTRESET, BMI088_ACC_SOFTRESET_VALUE); // 软复位
    DWT_Delay(BMI088_COM_WAIT_SENSOR_TIME / 1000);

    // 检查ID,如果不是0x1E(bmi088 whoami寄存器值),则返回错误
    BMI088AccelRead(bmi088, BMI088_ACC_CHIP_ID, &whoami_check, 1);
    if (whoami_check != BMI088_ACC_CHIP_ID_VALUE)
        return BMI088_NO_SENSOR;
    DWT_Delay(0.001);
    // 初始化寄存器,提高可读性
    uint8_t reg = 0, data = 0;
    BMI088_ERORR_CODE_e error = 0;
    // 使用sizeof而不是magic number,这样如果修改了数组大小,不用修改这里的代码;或者使用宏定义
    for (uint8_t i = 0; i < sizeof(BMI088_Accel_Init_Table) / sizeof(BMI088_Accel_Init_Table[0]); i++)
    {
        reg = BMI088_Accel_Init_Table[i][BMI088REG];
        data = BMI088_Accel_Init_Table[i][BMI088DATA];
        BMI088AccelWriteSingleReg(bmi088, reg, data); // 写入寄存器
        DWT_Delay(0.01);
        BMI088AccelRead(bmi088, reg, &data, 1); // 写完之后立刻读回检查
        DWT_Delay(0.01);
        if (data != BMI088_Accel_Init_Table[i][BMI088DATA])
            error |= BMI088_Accel_Init_Table[i][BMI088ERROR];
        //{i--;} 可以设置retry次数,如果retry次数用完了,则返回error
    }
    return error;
}

/**
 * @brief 初始化BMI088陀螺仪,提高可读性分拆功能
 *
 * @param bmi088 待初始化的BMI088实例
 * @return uint8_t BMI088ERROR CODE
 */
static uint8_t BMI088GyroInit(BMI088Instance *bmi088)
{
    // 后续添加reset和通信检查?
    // code to go here ...
    BMI088GyroWriteSingleReg(bmi088, BMI088_GYRO_SOFTRESET, BMI088_GYRO_SOFTRESET_VALUE); // 软复位
    DWT_Delay(0.08);

    // 检查ID,如果不是0x0F(bmi088 whoami寄存器值),则返回错误
    uint8_t whoami_check = 0;
    BMI088GyroRead(bmi088, BMI088_GYRO_CHIP_ID, &whoami_check, 1);
    if (whoami_check != BMI088_GYRO_CHIP_ID_VALUE)
        return BMI088_NO_SENSOR;
    DWT_Delay(0.001);

    // 初始化寄存器,提高可读性
    uint8_t reg = 0, data = 0;
    BMI088_ERORR_CODE_e error = 0;
    // 使用sizeof而不是magic number,这样如果修改了数组大小,不用修改这里的代码;或者使用宏定义
    for (uint8_t i = 0; i < sizeof(BMI088_Gyro_Init_Table) / sizeof(BMI088_Gyro_Init_Table[0]); i++)
    {
        reg = BMI088_Gyro_Init_Table[i][BMI088REG];
        data = BMI088_Gyro_Init_Table[i][BMI088DATA];
        BMI088GyroWriteSingleReg(bmi088, reg, data); // 写入寄存器
        DWT_Delay(0.001);
        BMI088GyroRead(bmi088, reg, &data, 1); // 写完之后立刻读回对应寄存器检查是否写入成功
        DWT_Delay(0.001);
        if (data != BMI088_Gyro_Init_Table[i][BMI088DATA])
            error |= BMI088_Gyro_Init_Table[i][BMI088ERROR];
        //{i--;} 可以设置retry次数,尝试重新写入.如果retry次数用完了,则返回error
    }

    return error;
}
// -------------------------以上为私有函数,用于初始化BMI088acc和gyro的硬件和配置--------------------------------//

// -------------------------以下为私有函数,private用于IT模式下的中断处理---------------------------------//

/**
 * @brief 待编写,很快!
 *
 */
static void BMI088AccSPIFinishCallback(SPIInstance *spi)
{
    // static BMI088Instance *bmi088;
    // bmi088 = (BMI088Instance *)(spi->id);
    // 若第一次读取加速度,则在这里启动温度读取
    // 如果使用异步姿态更新,此处唤醒量测更新的任务
}

static void BMI088GyroSPIFinishCallback(SPIInstance *spi)
{
    // static BMI088Instance *bmi088;
    // bmi088 = (BMI088Instance *)(spi->id);
    // 若不是异步,啥也不做;否则启动姿态的预测步(propagation)
}

static void BMI088AccINTCallback(GPIOInstance *gpio)
{
    // static BMI088Instance *bmi088;
    // bmi088 = (BMI088Instance *)(gpio->id);
    // 启动加速度计数据读取(和温度读取,如果有必要),并转换为实际值
    // 读取完毕会调用BMI088AccSPIFinishCallback
}

static void BMI088GyroINTCallback(GPIOInstance *gpio)
{
    // static BMI088Instance *bmi088;
    // bmi088 = (BMI088Instance *)(gpio->id);
    // 启动陀螺仪数据读取,并转换为实际值
    // 读取完毕会调用BMI088GyroSPIFinishCallback
}

// -------------------------以上为私有函数,private用于IT模式下的中断处理---------------------------------//

// -------------------------以下为公有函数,用于注册BMI088,标定和数据读取--------------------------------//

/**
 * @brief
 * @todo 现在要考虑一下数据返回的方式,指针还是结构体?
 *
 * @param bmi088
 * @return BMI088_Data_t
 */
uint8_t BMI088Acquire(BMI088Instance *bmi088, BMI088_Data_t *data_store)
{
    // 如果是blocking模式,则主动触发一次读取并返回数据
    if (bmi088->work_mode == BMI088_BLOCK_PERIODIC_MODE)
    {
        static uint8_t buf[6] = {0}; // 最多读取6个byte(gyro/acc,temp是2)
        // 读取accel的x轴数据首地址,bmi088内部自增读取地址 // 3* sizeof(int16_t)
        BMI088AccelRead(bmi088, BMI088_ACCEL_XOUT_L, buf, 6);
        for (uint8_t i = 0; i < 3; i++)
            data_store->acc[i] = bmi088->acc_coef * (float)(int16_t)(((buf[2 * i + 1]) << 8) | buf[2 * i]);
        BMI088GyroRead(bmi088, BMI088_GYRO_X_L, buf, 6); // 连续读取3个(3*2=6)轴的角速度
        for (uint8_t i = 0; i < 3; i++)
            data_store->gyro[i] = bmi088->BMI088_GYRO_SEN * (float)(int16_t)(((buf[2 * i + 1]) << 8) | buf[2 * i]);
        BMI088AccelRead(bmi088, BMI088_TEMP_M, buf, 2); // 读温度,温度传感器在accel上
        data_store->temperature = (float)(int16_t)(((buf[0] << 3) | (buf[1] >> 5))) * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET;

        return 1;
    }

    // 如果是IT模式,则检查标志位.当传感器数据准备好会触发外部中断,中断服务函数会将标志位置1
    if (bmi088->work_mode == BMI088_BLOCK_TRIGGER_MODE && bmi088->update_flag.imu_ready == 1)
    {
        memcpy(data_store, &bmi088->gyro, sizeof(BMI088_Data_t));
        bmi088->update_flag.imu_ready = 0;
        return 1;
    }

    // 如果数据还没准备好,则返回空数据?或者返回上一次的数据?或者返回错误码? @todo
    if (bmi088->update_flag.imu_ready == 0)
        return 0;
    return 1;
}

/* pre calibrate parameter to go here */
#pragma message "REMEMBER TO SET PRE CALIBRATE PARAMETER IF YOU CHOOSE NOT TO CALIBRATE"
#define BMI088_PRE_CALI_ACC_X_OFFSET 0.0f
#define BMI088_PRE_CALI_ACC_Y_OFFSET 0.0f
#define BMI088_PRE_CALI_ACC_Z_OFFSET 0.0f
#define BMI088_PRE_CALI_G_NORM 9.805f
/**
 * @brief BMI088 acc gyro 标定
 * @note 标定后的数据存储在bmi088->bias和gNorm中,用于后续数据消噪和单位转换归一化
 * @attention 不管工作模式是blocking还是IT,标定时都是blocking模式,所以不用担心中断关闭后无法标定(RobotInit关闭了全局中断)
 * @attention 标定精度和等待时间有关,目前使用线性回归.后续考虑引入非线性回归
 * @todo 将标定次数(等待时间)变为参数供设定
 * @section 整体流程为1.累加加速度数据计算gNrom() 2.累加陀螺仪数据计算零飘
 *          3. 如果标定过程运动幅度过大,重新标定  4.保存标定参数
 *
 * @param _bmi088 待标定的BMI088实例
 */
void BMI088CalibrateIMU(BMI088Instance *_bmi088)
{
    if (_bmi088->cali_mode == BMI088_CALIBRATE_ONLINE_MODE) // 性感bmi088在线标定,耗时6s
    {
        _bmi088->acc_coef = BMI088_ACCEL_6G_SEN;         // 标定完后要乘以9.805/gNorm
        _bmi088->BMI088_GYRO_SEN = BMI088_GYRO_2000_SEN; // 后续改为从initTable中获取
        // 一次性参数用完就丢,不用static
        float startTime;                     // 开始标定时间,用于确定是否超时
        uint16_t CaliTimes = 6000;           // 标定次数(6s)
        float gyroMax[3], gyroMin[3];        // 保存标定过程中读取到的数据最大值判断是否满足标定环境
        float gNormTemp, gNormMax, gNormMin; // 同上,计算矢量范数(模长)
        float gyroDiff[3], gNormDiff;        // 每个轴的最大角速度跨度及其模长

        BMI088_Data_t raw_data;
        startTime = DWT_GetTimeline_s();
        // 循环继续的条件为标定环境不满足
        do // 用do while至少执行一次,省得对上面的参数进行初始化
        {  // 标定超时,直接使用预标定参数(如果有)
            if (DWT_GetTimeline_s() - startTime > 12.01)
            { // 两次都没有成功就切换标定模式,丢给下一个if处理,使用预标定参数
                _bmi088->cali_mode = BMI088_LOAD_PRE_CALI_MODE;
                break;
            }

            DWT_Delay(0.0005);
            _bmi088->gNorm = 0;
            for (uint8_t i = 0; i < 3; i++) // 重置gNorm和零飘
                _bmi088->gyro_offset[i] = 0;

            // @todo : 这里也有获取bmi088数据的操作,后续与BMI088Acquire合并.注意标定时的工作模式是阻塞,且offset和acc_coef要初始化成0和1,标定完成后再设定为标定值
            for (uint16_t i = 0; i < CaliTimes; ++i) // 提前计算,优化
            {
                BMI088Acquire(_bmi088, &raw_data);
                gNormTemp = NormOf3d(raw_data.acc);
                _bmi088->gNorm += gNormTemp; // 计算范数并累加,最后除以calib times获取单次值
                for (uint8_t i = 0; i < 3; i++)
                    _bmi088->gyro_offset[i] += raw_data.gyro[i]; // 因为标定时传感器静止,所以采集到的值就是漂移,累加当前值,最后除以calib times获得零飘

                if (i == 0) // 避免未定义的行为(else中)
                {
                    gNormMax = gNormMin = gNormTemp; // 初始化成当前的重力加速度模长
                    for (uint8_t j = 0; j < 3; ++j)
                    {
                        gyroMax[j] = raw_data.gyro[j];
                        gyroMin[j] = raw_data.gyro[j];
                    }
                }
                else // 更新gNorm的Min Max和gyro的minmax
                {
                    gNormMax = gNormMax > gNormTemp ? gNormMax : gNormTemp;
                    gNormMin = gNormMin < gNormTemp ? gNormMin : gNormTemp;
                    for (uint8_t j = 0; j < 3; ++j)
                    {
                        gyroMax[j] = gyroMax[j] > _bmi088->gyro[j] ? gyroMax[j] : _bmi088->gyro[j];
                        gyroMin[j] = gyroMin[j] < _bmi088->gyro[j] ? gyroMin[j] : _bmi088->gyro[j];
                    }
                }

                gNormDiff = gNormMax - gNormMin; // 最大值和最小值的差
                for (uint8_t j = 0; j < 3; ++j)
                    gyroDiff[j] = gyroMax[j] - gyroMin[j]; // 分别计算三轴
                if (gNormDiff > 0.5f ||
                    gyroDiff[0] > 0.15f ||
                    gyroDiff[1] > 0.15f ||
                    gyroDiff[2] > 0.15f)
                    break;         // 超出范围了,重开! remake到while循环,外面还有一层
                DWT_Delay(0.0005); // 休息一会再开始下一轮数据获取,IMU准备数据需要时间
            }
            _bmi088->gNorm /= (float)CaliTimes; // 加速度范数重力
            for (uint8_t i = 0; i < 3; ++i)
                _bmi088->gyro_offset[i] /= (float)CaliTimes; // 三轴零飘
            // 这里直接存到temperature,可以另外增加BMI088Instance的成员变量TempWhenCalib
            _bmi088->temperature = raw_data.temperature * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET; // 保存标定时的温度,如果已知温度和零飘的关系
            // caliTryOutCount++; 保存已经尝试的标定次数?由你.
        } while (gNormDiff > 0.5f ||
                 fabsf(_bmi088->gNorm - 9.8f) > 0.5f ||
                 gyroDiff[0] > 0.15f ||
                 gyroDiff[1] > 0.15f ||
                 gyroDiff[2] > 0.15f ||
                 fabsf(_bmi088->gyro_offset[0]) > 0.01f ||
                 fabsf(_bmi088->gyro_offset[1]) > 0.01f ||
                 fabsf(_bmi088->gyro_offset[2]) > 0.01f); // 满足条件说明标定环境不好
    }

    // 离线标定
    if (_bmi088->cali_mode == BMI088_LOAD_PRE_CALI_MODE) // 如果标定失败也会进来,直接使用离线数据
    {
        _bmi088->gyro_offset[0] = BMI088_PRE_CALI_ACC_X_OFFSET;
        _bmi088->gyro_offset[1] = BMI088_PRE_CALI_ACC_Y_OFFSET;
        _bmi088->gyro_offset[2] = BMI088_PRE_CALI_ACC_Z_OFFSET;
        _bmi088->gNorm = BMI088_PRE_CALI_G_NORM;
    }
    _bmi088->acc_coef *= 9.805 / _bmi088->gNorm;
}

// 考虑阻塞模式和非阻塞模式的兼容性,通过条件编译(则需要在编译前修改宏定义)或runtime参数判断
// runtime的开销不大(一次性判断),但是需要修改函数原型,增加参数,代码长度增加(但不多)
// runtime如何修改callback?根据参数选择是否给spi传入callback,如果是阻塞模式,则不传入callback,如果是非阻塞模式,则传入callback(bsp会检查是否NULL)
// 条件编译的开销小,但是需要修改宏定义,增加编译时间,同时人力介入
// 根据实际情况选择(说了和没说一样!)

BMI088Instance *BMI088Register(BMI088_Init_Config_s *config)
{
    // 申请内存
    BMI088Instance *bmi088_instance = (BMI088Instance *)zmalloc(sizeof(BMI088Instance));
    // 从右向左赋值,让bsp instance保存指向bmi088_instance的指针(父指针),便于在底层中断中访问bmi088_instance
    config->acc_int_config.id =
        config->gyro_int_config.id =
            config->spi_acc_config.id =
                config->spi_gyro_config.id =
                    config->heat_pwm_config.id = bmi088_instance;
    // @todo:
    // 目前只实现了!!!阻塞读取模式!!!.如果需要使用IT模式,则需要修改这里的代码,为spi和gpio注册callback(默认为NULL)
    // 还需要设置SPI的传输模式为DMA模式或IT模式(默认为blocking)
    // 可以通过conditional compilation或者runtime参数判断
    // code to go here ...

    // INT_ACC EXTI CALLBACK: 检查是否有传输正在进行,如果没有则开启SPI DMA传输,有则置位wait标志位;
    // 第一次是加速度计,第二次是温度.

    // INT_GYRO EXTI CALLBACK: 开启SPI DMA传输,不会出现等待传输的情况
    // SPI_GYRO DMA CALLBACK: 解算陀螺仪数据,
    // SPI_ACC DMA CALLBACK: 解算加速度计数据,清除温度wait标志位并启动温度传输,第二次进入中断时解算温度数据

    // 还有其他方案可用,比如阻塞等待传输完成,但是比较笨.

    // 根据参数选择工作模式
    if (config->work_mode == BMI088_BLOCK_PERIODIC_MODE)
    {
        config->spi_acc_config.spi_work_mode = SPI_BLOCK_MODE;
        config->spi_gyro_config.spi_work_mode = SPI_BLOCK_MODE;
        // callbacks are all NULL
    }
    else if (config->work_mode == BMI088_BLOCK_TRIGGER_MODE)
    {
        config->spi_gyro_config.spi_work_mode = SPI_DMA_MODE; // 如果DMA资源不够,可以用SPI_IT_MODE
        config->spi_gyro_config.spi_work_mode = SPI_DMA_MODE;
        // 设置回调函数
        config->spi_acc_config.callback = BMI088AccSPIFinishCallback;
        config->spi_gyro_config.callback = BMI088GyroSPIFinishCallback;
        config->acc_int_config.gpio_model_callback = BMI088AccINTCallback;
        config->gyro_int_config.gpio_model_callback = BMI088GyroINTCallback;
        bmi088_instance->acc_int = GPIORegister(&config->acc_int_config); // 只有在非阻塞模式下才需要注册中断
        bmi088_instance->gyro_int = GPIORegister(&config->gyro_int_config);
    } // 注册实例
    bmi088_instance->spi_acc = SPIRegister(&config->spi_acc_config);
    bmi088_instance->spi_gyro = SPIRegister(&config->spi_gyro_config);
    bmi088_instance->heat_pwm = PWMRegister(&config->heat_pwm_config);
    PIDInit(&bmi088_instance->heat_pid, &config->heat_pid_config);

    // 初始化acc和gyro
    BMI088_ERORR_CODE_e error = BMI088_NO_ERROR;
    do
    {
        error = BMI088_NO_ERROR;
        error |= BMI088AccelInit(bmi088_instance);
        error |= BMI088GyroInit(bmi088_instance);
        // 可以增加try out times,超出次数则返回错误
    } while (error != 0);

    bmi088_instance->work_mode = BMI088_BLOCK_PERIODIC_MODE; // 临时设置为阻塞模式
    BMI088CalibrateIMU(bmi088_instance);                     // 标定acc和gyro
    bmi088_instance->work_mode = config->work_mode;          // 恢复工作模式

    return bmi088_instance;
}
