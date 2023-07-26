#include "bsp_dwt.h"
#include "ist8310.h"
#include "bsp_log.h"
#include <memory.h>
#include <stdlib.h>

// 一般这个模块只有一个实例,所以直接保存在这里,实际上不保存也可以,application可以自己保存
static IST8310Instance *ist8310_instance = NULL; // 用于存储IST8310实例的指针

#define IST8310_WRITE_REG_NUM 4     // 方便阅读
#define IST8310_DATA_REG 0x03       // ist8310的数据寄存器
#define IST8310_WHO_AM_I 0x00       // ist8310 id 寄存器值
#define IST8310_WHO_AM_I_VALUE 0x10 // 用于检测是否连接成功,读取ist8310的whoami会返回该值

// -------------------初始化写入数组,只使用一次,详见datasheet-------------------------
// the first column:the registers of IST8310. 第一列:IST8310的寄存器
// the second column: the value to be writed to the registers.第二列:需要写入的寄存器值
// the third column: return error value.第三列:返回的错误码
static uint8_t ist8310_write_reg_data_error[IST8310_WRITE_REG_NUM][3] = {
    {0x0B, 0x08, 0x01},  // enalbe interrupt  and low pin polarity.开启中断，并且设置低电平
    {0x41, 0x09, 0x02},  // average 2 times.平均采样两次
    {0x42, 0xC0, 0x03},  // must be 0xC0. 必须是0xC0
    {0x0A, 0x0B, 0x04}}; // 200Hz output rate.200Hz输出频率

/**
 * @brief IST8310解码函数,EXTI中断来临时被调用,将数据放到ist.mag中
 * @note 如果使用IT或DMA方式传输IIC,则传输完成后也会进入此函数
 *
 * @param ist 发生中断的IST8310实例
 */
static void IST8310Decode(IICInstance *iic)
{
    int16_t temp[3];                                     // 用于存储解码后的数据
    IST8310Instance *ist = (IST8310Instance *)(iic->id); // iic的id保存了IST8310实例的指针(父指针)

    memcpy(temp, ist->iic_buffer, 6 * sizeof(uint8_t)); // 不要强制转换,直接cpy
    for (uint8_t i = 0; i < 3; i++)
        ist->mag[i] = (float)temp[i] * MAG_SEN; // 乘以灵敏度转换成uT(微特斯拉)
}

/**
 * @brief EXTI中断回调函数,说明DRDY拉低.主机启动传输并在结束后调用IST8310Decode进行数据解析
 * @note  注意IICAccessMem是阻塞的
 *
 * @param gpio 发生中断的GPIO实例
 */
static void IST8310StartTransfer(GPIOInstance *gpio)
{
    // 先获取IST8310实例的指针(通过gpio实例的父指针id)
    IST8310Instance *ist_for_transfer = (IST8310Instance *)gpio->id;
    // 中断说明ist已经准备好读取数据寄存器;6个字节,读取后会进入IST8310Decode函数
    IICAccessMem(ist_for_transfer->iic, IST8310_DATA_REG, ist_for_transfer->iic_buffer, 6, IIC_READ_MEM, 1);
    // 传输完成后会进入IST8310Decode函数进行数据解析
    IST8310Decode(ist_for_transfer->iic);
}

IST8310Instance *IST8310Init(IST8310_Init_Config_s *config)
{
    static const uint8_t sleepTime = 50; // 50ms,ist8310的复位时间
    uint8_t check_who_i_am = 0;          // 用于检测ist8310是否连接成功
    // 这个变量只会用到一次,出了这个函数就没用了,所以不用分配空间,直接定义在栈上(因为多看一眼就会爆炸)

    // 分配空间,清除flash防止已经填充的垃圾值
    IST8310Instance *ist = (IST8310Instance *)malloc(sizeof(IST8310Instance));
    memset(ist, 0, sizeof(IST8310Instance));

    // c语言赋值从右到左,全部指向同一个地址(这些bspinstnace的父节点都是ist,即ist拥有这些instances)
    config->iic_config.id = config->gpio_conf_exti.id = config->gpio_conf_rst.id = ist;
    // 传入回调函数
    config->iic_config.callback = IST8310Decode;
    config->gpio_conf_exti.gpio_model_callback = IST8310StartTransfer;
    // 分别注册两个GPIO和IIC
    ist->iic = IICRegister(&config->iic_config);
    ist->gpio_exti = GPIORegister(&config->gpio_conf_exti);
    ist->gpio_rst = GPIORegister(&config->gpio_conf_rst);

    // 重置IST8310,需要HAL_Delay()等待传感器完成Reset
    GPIOReset(ist->gpio_rst);
    HAL_Delay(sleepTime);
    GPIOSet(ist->gpio_rst);
    HAL_Delay(sleepTime);

    // 读取IST8310的ID,如果不是0x10(whoami macro的值),则返回错误
    IICAccessMem(ist->iic, IST8310_WHO_AM_I, &check_who_i_am, 1, IIC_READ_MEM, 1);
    if (check_who_i_am != IST8310_WHO_AM_I_VALUE)
        return NULL; // while(1)

    // 进行初始化配置写入并检查是否写入成功,这里用循环把最上面初始化数组的东西都写进去
    for (uint8_t i = 0; i < IST8310_WRITE_REG_NUM; i++)
    { // 写入配置,写一句就读一下看看ist8310是否仍然在线
        IICAccessMem(ist->iic, ist8310_write_reg_data_error[i][0], &ist8310_write_reg_data_error[i][1], 1, IIC_WRITE_MEM, 1);
        IICAccessMem(ist->iic, ist8310_write_reg_data_error[i][0], &check_who_i_am, 1, IIC_READ_MEM, 1); // 读回自身id
        if (check_who_i_am != ist8310_write_reg_data_error[i][1])
            while (1)
                LOGERROR("[ist8310] init error, code %d", ist8310_write_reg_data_error[i][2]); // 掉线/写入失败/未知错误,会返回对应的错误码
    }

    ist8310_instance = ist; // 保存ist8310实例的指针
    return ist;
}
