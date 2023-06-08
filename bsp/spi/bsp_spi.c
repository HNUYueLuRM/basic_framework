#include "bsp_spi.h"
#include "memory.h"
#include "stdlib.h"

/* 所有的spi instance保存于此,用于callback时判断中断来源*/
static SPIInstance *spi_instance[SPI_DEVICE_CNT] = {NULL};
static uint8_t idx = 0; // 配合中断以及初始化

SPIInstance *SPIRegister(SPI_Init_Config_s *conf)
{
    if (idx >= MX_SPI_BUS_SLAVE_CNT) // 超过最大实例数
        while (1)
            ;
    SPIInstance *instance = (SPIInstance *)malloc(sizeof(SPIInstance));
    memset(instance, 0, sizeof(SPIInstance));

    instance->spi_handle = conf->spi_handle;
    instance->GPIOx = conf->GPIOx;
    instance->cs_pin = conf->cs_pin;
    instance->spi_work_mode = conf->spi_work_mode;
    instance->callback = conf->callback;
    instance->id = conf->id;

    spi_instance[idx++] = instance;
    return instance;
}

void SPITransmit(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len)
{
    // 拉低片选,开始传输(选中从机)
    HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_Transmit_DMA(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_Transmit_IT(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_Transmit(spi_ins->spi_handle, ptr_data, len, 1000); // 默认50ms超时
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPIRecv(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len)
{
    // 用于稍后回调使用
    spi_ins->rx_size = len;
    spi_ins->rx_buffer = ptr_data;
    // 拉低片选,开始传输
    HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_Receive_DMA(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_Receive_IT(spi_ins->spi_handle, ptr_data, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_Receive(spi_ins->spi_handle, ptr_data, len, 1000);
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPITransRecv(SPIInstance *spi_ins, uint8_t *ptr_data_rx, uint8_t *ptr_data_tx, uint8_t len)
{
    // 用于稍后回调使用,请保证ptr_data_rx在回调函数被调用之前仍然在作用域内,否则析构之后的行为是未定义的!!!
    spi_ins->rx_size = len;
    spi_ins->rx_buffer = ptr_data_rx;
    // 拉低片选,开始传输
    HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_RESET);
    switch (spi_ins->spi_work_mode)
    {
    case SPI_DMA_MODE:
        HAL_SPI_TransmitReceive_DMA(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len);
        break;
    case SPI_IT_MODE:
        HAL_SPI_TransmitReceive_IT(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len);
        break;
    case SPI_BLOCK_MODE:
        HAL_SPI_TransmitReceive(spi_ins->spi_handle, ptr_data_tx, ptr_data_rx, len, 1000); // 默认50ms超时
        // 阻塞模式不会调用回调函数,传输完成后直接拉高片选结束
        HAL_GPIO_WritePin(spi_ins->GPIOx, spi_ins->cs_pin, GPIO_PIN_SET);
        break;
    default:
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况
        break;
    }
}

void SPISetMode(SPIInstance *spi_ins, SPI_TXRX_MODE_e spi_mode)
{
    if (spi_mode != SPI_DMA_MODE && spi_mode != SPI_IT_MODE && spi_mode != SPI_BLOCK_MODE)
        while (1)
            ; // error mode! 请查看是否正确设置模式，或出现指针越界导致模式被异常修改的情况

    if (spi_ins->spi_work_mode != spi_mode)
    {
        spi_ins->spi_work_mode = spi_mode;
    }
}

/**
 * @brief 当SPI接收完成,将会调用此回调函数,可以进行协议解析或其他必须的数据处理等
 *
 * @param hspi spi handle
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    for (size_t i = 0; i < idx; i++)
    {
        // 如果是当前spi硬件发出的complete,且cs_pin为低电平(说明正在传输),则尝试调用回调函数
        if (spi_instance[i]->spi_handle == hspi && // 显然同一时间一条总线只能有一个从机在接收数据
            HAL_GPIO_ReadPin(spi_instance[i]->GPIOx, spi_instance[i]->cs_pin) == GPIO_PIN_RESET)
        {
            // 先拉高片选,结束传输,在判断是否有回调函数,如果有则调用回调函数
            HAL_GPIO_WritePin(spi_instance[i]->GPIOx, spi_instance[i]->cs_pin, GPIO_PIN_SET);
            // @todo 后续添加holdon模式,由用户自行决定何时释放片选,允许进行连续传输
            if (spi_instance[i]->callback != NULL) // 回调函数不为空, 则调用回调函数
                spi_instance[i]->callback(spi_instance[i]);
            return;
        }
    }
}

/**
 * @brief 和RxCpltCallback共用解析即可,这里只是形式上封装一下,不用重复写
 *        这是对HAL库的__weak函数的重写,传输使用IT或DMA模式,在传输完成时会调用此函数
 * @param hspi spi handle
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    HAL_SPI_RxCpltCallback(hspi); // 直接调用接收完成的回调函数
}
