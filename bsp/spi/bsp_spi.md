# bsp spi

> 预计增加模拟spi

初始化传入参数中的GPIOx（GPIOA，GPIOB，...）和cs_pin（GPIO_PIN_1,GPIO_PIN_2, ...）都是HAL库内建的宏，在CubeMX初始化的时候若有给gpio分配标签则填入对应名字即可，否则填入原本的宏。

注意，如果你没有在CubeMX中为spi分配dma通道，请不要使用dma模式

（后续添加安全检查，通过判断hspi的dma handler是否为空来选择模式，如果为空，则自动将DMA转为IT模式以继续传输，并通过log warning 提醒用户）