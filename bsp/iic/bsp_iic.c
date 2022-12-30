#include "bsp_iic.h"
#include "memory.h"
#include "stdlib.h"

static uint8_t idx = 0; // 配合中断以及初始化
static IICInstance *iic_instance[IIC_DEVICE_CNT] = {NULL};

/**
 * @brief 接收完成回调函数
 *
 * @param hi2c handle
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // 如果是当前i2c硬件发出的complete,且dev_address和之前发起接收的地址相同,同时回到函数不为空, 则调用回调函数
    for (uint8_t i = 0; i < idx; i++)
    {
        if (iic_instance[i]->handle == hi2c && hi2c->Devaddress == iic_instance[i]->dev_address)
        {
            if (iic_instance[i]->callback != NULL)
                iic_instance[i]->callback(iic_instance[i]);
            return;
        }
    }
}

/**
 * @brief 仅做形式上的封装,仍然使用HAL_I2C_MasterRxCpltCallback
 *
 * @param hi2c handle
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_MasterRxCpltCallback(hi2c);
}

IICInstance *IICRegister(IIC_Init_Config_s *conf)
{
    // 申请到的空间未必是0, 所以需要手动初始化
    iic_instance[idx] = (IICInstance *)malloc(sizeof(IICInstance));
    memset(iic_instance[idx], 0, sizeof(IICInstance));

    iic_instance[idx]->dev_address = conf->dev_address;
    iic_instance[idx]->callback = conf->callback;
    iic_instance[idx]->work_mode = conf->work_mode;
    iic_instance[idx]->handle = conf->handle;
    iic_instance[idx]->id = conf->id;

    return iic_instance[idx++];
}

void IICSetMode(IICInstance *iic, IIC_Work_Mode_e mode)
{ // HAL自带重入保护,不需要手动终止或等待传输完成
    if (iic->work_mode != mode) // 如果不同才需要修改
    {
        iic->work_mode = mode;
    }
}

void IICTransmit(IICInstance *iic, uint8_t *data, uint16_t size, IIC_Seq_Mode_e seq_mode)
{
    if (seq_mode != IIC_RELEASE && seq_mode != IIC_HOLD_ON)
        while (1)
            ; // 未知传输模式, 程序停止

    switch (iic->work_mode)
    {
    case IIC_BLOCK_MODE:
        if (seq_mode != IIC_RELEASE)
            while (1)
                ; // 阻塞模式下不支持HOLD ON模式!!!
        HAL_I2C_Master_Transmit(iic->handle, iic->dev_address, data, size, 100);
        break;
    case IIC_IT_MODE:
        if (seq_mode == IIC_RELEASE)
            HAL_I2C_Master_Seq_Transmit_IT(iic->handle, iic->dev_address, data, size, I2C_OTHER_AND_LAST_FRAME);
        else if (seq_mode == IIC_HOLD_ON)
            HAL_I2C_Master_Seq_Transmit_IT(iic->handle, iic->dev_address, data, size, I2C_OTHER_FRAME);
        break;
    case IIC_DMA_MODE:
        if (seq_mode == IIC_RELEASE)
            HAL_I2C_Master_Seq_Transmit_DMA(iic->handle, iic->dev_address, data, size, I2C_OTHER_AND_LAST_FRAME);
        else if (seq_mode == IIC_HOLD_ON)
            HAL_I2C_Master_Seq_Transmit_DMA(iic->handle, iic->dev_address, data, size, I2C_OTHER_FRAME);
        break;
    default:
        while (1)
            ; // 未知传输模式, 程序停止
        break;
    }
}

void IICReceive(IICInstance *iic, uint8_t *data, uint16_t size, IIC_Seq_Mode_e seq_mode)
{
    if (seq_mode != IIC_RELEASE && seq_mode != IIC_HOLD_ON)
        while (1)
            ; // 未知传输模式, 程序停止

    // 初始化接收缓冲区地址以及接受长度, 用于中断回调函数
    iic->rx_buffer = data;
    iic->rx_len = size;

    switch (iic->work_mode)
    {
    case IIC_BLOCK_MODE:
        if (seq_mode != IIC_RELEASE)
            while (1)
                ; // 阻塞模式下不支持HOLD ON模式!!!
        HAL_I2C_Master_Receive(iic->handle, iic->dev_address, data, size, 100);
        break;
    case IIC_IT_MODE:
        if (seq_mode == IIC_RELEASE)
            HAL_I2C_Master_Seq_Receive_IT(iic->handle, iic->dev_address, data, size, I2C_OTHER_AND_LAST_FRAME);
        else if (seq_mode == IIC_HOLD_ON)
            HAL_I2C_Master_Seq_Receive_IT(iic->handle, iic->dev_address, data, size, I2C_OTHER_FRAME);
        break;
    case IIC_DMA_MODE:
        if (seq_mode == IIC_RELEASE)
            HAL_I2C_Master_Seq_Receive_DMA(iic->handle, iic->dev_address, data, size, I2C_OTHER_AND_LAST_FRAME);
        else if (seq_mode == IIC_HOLD_ON)
            HAL_I2C_Master_Seq_Receive_DMA(iic->handle, iic->dev_address, data, size, I2C_OTHER_FRAME);
        break;
    default:
        while (1)
            ; // 未知传输模式, 程序停止
        break;
    }
}

void IICAcessMem(IICInstance *iic, uint8_t mem_addr, uint8_t *data, uint16_t size, IIC_Mem_Mode_e mem_mode)
{
    if (mem_mode == IIC_WRITE_MEM)
    {
        HAL_I2C_Mem_Write(iic->handle, iic->dev_address, mem_addr, I2C_MEMADD_SIZE_8BIT, data, size, 1000);
    }
    else if (mem_mode == IIC_READ_MEM)
    {
        HAL_I2C_Mem_Read(iic->handle, iic->dev_address, mem_addr, I2C_MEMADD_SIZE_8BIT, data, size, 1000);
    }
    else
    {
        while (1)
            ; // 未知模式, 程序停止
    }
}