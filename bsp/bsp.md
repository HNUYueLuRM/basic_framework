BSP

这是BSP层的说明文档。

> TODO:
> 1. 增加软件中断支持
> 2. 增加硬件CRC支持

bsp的功能是提供对片上外设的封装，即单片机芯片内部拥有的功能的封装。在开发板pcb上集成的模块应该放在module层而不是这里。

bsp应该提供几种接口。包括初始化接口，一般命名为`XXXRegister()`(对于只有一个instance的可以叫`XXXInit()`,但建议统一风格都叫register)；调用此模块实现的必要功能，如通信型外设（CubeMX下的connectivity）提供接收和发送的接口，以及接收完成的数据回调函数。