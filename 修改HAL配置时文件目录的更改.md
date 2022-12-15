# 修改STM HAL配置时文件目录的更改

> 仅修改引脚位置和外设配置时

1. 在generate code之前，保存Makefile的副本 (因为修改了makefile的内容,添加了注释和伪构建目标,HAL无法正确识别格式导致直接其将整个文件清空)
2. 生成之后，将原makefile的内容替换到新的makefile中
3. 将原main.c和freertos.c以及stm32f4xx_it.c替换到新的Src文件夹下 (Src的内容不用修改,**除非你新增了功能,那么需要将新增的初始化头文件`xxx.h`包含到main函数中,然后调用其对应的`MX_xxx_Init()`函数对硬件进行初始化**)
4. 将新生成的Src,Inc,Driver/STM32F4xx_HAL_Driver放入HAL_N_Middlewares下的对应位置(第三个目录只有新增了功能的时候才需要添加)

> Q:为什么这么做?

> A:CubeMX生成的依赖库和文件有很大的冗余,占用空间较多.为了方便pull&push已经将Driver和Middlewares中不需要的文件全部删除. 修改HAL的配置(在没有新增其他库和依赖的情况下,大部分我们的更改都属于这种情况)大多是对初始化配置,即放在Inc和Src下的文件进行修改;若新增了功能,可能还会新增Driver/STM32F4xx_HAL_Driver的内容(比如新添加了flash或iic,原来没有这两个功能).