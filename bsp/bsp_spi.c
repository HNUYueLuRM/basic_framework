#include "bsp_spi.h"
#include "memory.h"
#include "stdlib.h"

/* 所有的spi instance保存于此,用于callback时判断中断来源*/
static SPIInstance *spi_instance[SPI_DEVICE_CNT] = {NULL};
static uint8_t idx = 0; // 配合中断以及初始化

/**
 * @brief 当SPI接收完成,将会调用此回调函数,可以进行协议解析或其他必须的数据处理等
 *
 * @param hspi spi handle
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    for (size_t i = 0; i < idx; i++)
    {
        if (spi_instance[i]->spi_handle == hspi && spi_instance[i]->callback)
        {
            // 拉高片选(关闭传输),调用解析回调函数
            HAL_GPIO_WritePin(spi_instance[i]->GPIO_cs, spi_instance[i]->cs_pin, GPIO_PIN_SET);
            spi_instance[i]->callback(spi_instance[i]);
            break;
        }
    }
}

/**
 * @brief 和RxCpltCallback共用解析即可,这里只是形式上封装一下,不用重复写
 *
 * @param hspi spi handle
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    HAL_SPI_RxCpltCallback(hspi);
}

SPIInstance *SPIRegister(SPI_Init_Config_s *conf)
{
    spi_instance[idx] = (SPIInstance *)malloc(sizeof(SPIInstance));
    spi_instance[idx]->callback = conf->callback;
    spi_instance[idx]->spi_work_mode = conf->spi_work_mode;
    spi_instance[idx]->spi_handle = conf->spi_handle;
    return spi_instance[idx++];
}

void SPITransmit(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len)
{
    // 拉低片选,开始传输
    HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_Transmit_DMA(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_Transmit_IT(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_Transmit(spi_ins->spi_handle, ptr_data, len, 10);
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPIRecv(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len)
{
    // 拉低片选,开始传输
    HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_Receive_DMA(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_Receive_IT(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_Receive(spi_ins->spi_handle, ptr_data, len, 10);
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPITransRecv(SPIInstance *spi_ins, uint8_t *ptr_data_rx, uint8_t *ptr_data_tx, uint8_t len)
{
    // 拉低片选,开始传输
    HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_TransmitReceive_DMA(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_TransmitReceive_IT(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_TransmitReceive(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len, 10);
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPISetMode(SPIInstance *spi_ins, SPI_TXRX_MODE_e spi_mode)
{
    if (spi_ins->spi_work_mode != spi_mode)
    {
        switch (spi_ins->spi_work_mode)
        {
        case SPI_IT_MODE:
        case SPI_DMA_MODE:
            // IT和DMA处理相同,都是先终止传输,防止传输未完成直接切换导致spi死机
            HAL_SPI_Abort_IT(spi_ins->spi_handle);
            HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_SET); // 关闭后拉高片选
            break;
        case SPI_BLOCK_MODE:
            // 阻塞模式仍然有可能在多线程的情况下出现传输到一半切换,因此先终止
            HAL_SPI_Abort(spi_ins->spi_handle);
            HAL_GPIO_WritePin(spi_ins->GPIO_cs, spi_ins->cs_pin, GPIO_PIN_SET); // 关闭后拉高片选
            break;
        default:
            while (1)
                ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
            break;
        }
        spi_ins->spi_work_mode = spi_mode;
    }
}
