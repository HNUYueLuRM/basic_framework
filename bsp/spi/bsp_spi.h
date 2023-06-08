#include "spi.h"
#include "stdint.h"
#include "gpio.h"

/* 根据开发板引出的spi引脚以及CubeMX中的初始化配置设定 */
#define SPI_DEVICE_CNT 2       // C型开发板引出两路spi,分别连接BMI088/作为扩展IO在8pin牛角座引出
#define MX_SPI_BUS_SLAVE_CNT 4 // 单个spi总线上挂载的从机数目

/* spi transmit recv mode enumerate*/
typedef enum
{
    SPI_BLOCK_MODE = 0, // 默认使用阻塞模式
    SPI_IT_MODE,
    SPI_DMA_MODE,
} SPI_TXRX_MODE_e;

/* SPI实例结构体定义 */
typedef struct spi_ins_temp
{
    SPI_HandleTypeDef *spi_handle; // SPI外设handle
    GPIO_TypeDef *GPIOx;           // 片选信号对应的GPIO,如GPIOA,GPIOB等等
    uint16_t cs_pin;               // 片选信号对应的引脚号,GPIO_PIN_1,GPIO_PIN_2等等

    SPI_TXRX_MODE_e spi_work_mode; // 传输工作模式
    uint8_t rx_size;               // 本次接收的数据长度
    uint8_t *rx_buffer;            // 本次接收的数据缓冲区

    void (*callback)(struct spi_ins_temp *); // 接收回调函数
    void *id;                                // 模块指针
} SPIInstance;

/* 接收回调函数定义,包含SPI的module按照此格式构建回调函数 */
typedef void (*spi_rx_callback)(SPIInstance *);

// @todo: 这里可以将GPIO_TypeDef *GPIOx; uint16_t cs_pin合并为bsp_gpio以简化代码实现
/* SPI初始化配置,其实基本和SPIIstance一模一样,为了代码风格统一因此再次定义 */
typedef struct
{
    SPI_HandleTypeDef *spi_handle; // SPI外设handle
    GPIO_TypeDef *GPIOx;           // 片选信号对应的GPIO,如GPIOA,GPIOB等等
    uint16_t cs_pin;               // 片选信号对应的引脚号,GPIO_PIN_1,GPIO_PIN_2等等

    SPI_TXRX_MODE_e spi_work_mode; // 传输工作模式

    spi_rx_callback callback; // 接收回调函数
    void *id;                 // 模块指针
} SPI_Init_Config_s;

/**
 * @brief 注册一个spi instance
 *
 * @param conf 传入spi配置
 * @return SPIInstance* 返回一个spi实例指针,之后通过该指针操作spi外设
 */
SPIInstance *SPIRegister(SPI_Init_Config_s *conf);

/**
 * @brief 通过spi向对应从机发送数据
 * @todo  后续加入阻塞模式下的timeout参数
 *
 * @param spi_ins spi实例指针
 * @param ptr_data 要发送的数据
 * @param len 待发送的数据长度
 */
void SPITransmit(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len);

/**
 * @brief 通过spi从从机获取数据
 * @attention 特别注意:请保证ptr_data在回调函数被调用之前仍然在作用域内,否则析构之后的行为是未定义的!!!
 * 
 * @param spi_ins spi实例指针
 * @param ptr_data 接受数据buffer的首地址
 * @param len 待接收的长度
 */
void SPIRecv(SPIInstance *spi_ins, uint8_t *ptr_data, uint8_t len);

/**
 * @brief 通过spi利用移位寄存器同时收发数据
 * @todo  后续加入阻塞模式下的timeout参数
 * @attention 特别注意:请保证ptr_data_rx在回调函数被调用之前仍然在作用域内,否则析构之后的行为是未定义的!!!
 * 
 * @param spi_ins spi实例指针
 * @param ptr_data_rx 接收数据地址
 * @param ptr_data_tx 发送数据地址
 * @param len 接收&发送的长度
 */
void SPITransRecv(SPIInstance *spi_ins, uint8_t *ptr_data_rx, uint8_t *ptr_data_tx, uint8_t len);

/**
 * @brief 设定spi收发的工作模式
 *
 * @param spi_ins spi实例指针
 * @param spi_mode 工作模式,包括阻塞模式(block),中断模式(IT),DMA模式.详见SPI_TXRX_MODE_e的定义
 * 
 * @todo 是否直接将mode作为transmit/recv的参数,而不是作为spi实例的属性?两者各有优劣
 */
void SPISetMode(SPIInstance *spi_ins, SPI_TXRX_MODE_e spi_mode);
