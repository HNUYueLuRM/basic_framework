# YueLu2022/2023 EC basic_framework-dev

> ***可能是最完整最详细最好的电控开源！***

[TOC]

本框架设计参考了哈尔滨工业大学（深圳）南工骁🦅战队的EC_framework以及RoboMaster官方的RoboRTS-firmware。

- 若无法访问github，戳[gitee仓库](https://gitee.com/hnuyuelurm/basic_framework)
- 若gitee内容被屏蔽，戳[github仓库](https://github.com/HNUYueLuRM/basic_framework)

> 基于basic_framework打造的C++进阶重构版本[***powerful_framework***](https://gitee.com/hnuyuelurm/powerful_framework)现已发布！增加全新的消息交互机制和严格的跨任务数据读写保护，采用了现代构建系统CMake+Ninja以追求极致的编译速度，各种针对嵌入式的编译优化全开，DIY程度进一步提升，更有自定义CMSIS-DSP和Eigen等扩展库支持！快来加入试用/和我们一起开发吧😋



## 架构

### 软件栈



### 多任务协作



### 设计思想

框架在结构上分为三层：bsp/module/app。整体使用的设计模式是**结构层级模式**，即每个“类”包含需要使用的底层“类”，通过组装不同的基础模实现更强大的功能。而最顶层的app之间则通过**pub-sub消息机制**进行解耦，使得在编写代码时不会出现相互包含的情况。

**pub-sub机制的体现**：以本仓库的app层为例，包含了chassis，gimbal，shoot，cmd四个应用，每个应用都对应了机器人上的不同模组。cmd应用负责从机器人控制信号来源（遥控器/上位机/环境传感器）处获取信息并解析成各个**执行单元的实际动作**（电机/舵机/气缸/阀门等的扭矩/速度/位置/角度/开度等），并将此信息**发布**出去。chassis、gimbal、shoot等包含了执行单元的应用则**订阅**这些消息，并通过自己包含的子模块，调用它们的接口实现动作。

**结构层级模式的体现**：以chassis应用为例，chassis中包含了4个底盘电机模块。当chassis收到cmd应用的信息，希望让底盘以1m/s的速度前进。chassis首先根据底盘的类型（舵轮/麦克纳姆轮/全向轮/平衡底盘）以及对应的动力学/运动学解算函数，计算得到每个电机的输目标输入值，此时chassis将输入通过电机模块（motor module）的接口将设定值告知电机。而每个电机模块又有各自的PID计算模块和自身电流&速度&角度传感器的信息，可以计算出最终需要的电流设定值。假设该电机使用CAN协议与电调通信，则电机通过自身包含的CANInstance（bsp_can提供）用于和实际硬件交互，电机模块将设定值电流值或其他指令按照通信协议组织在CAN报文中，通过CANInstance提供的接口，把最终控制数据发送给电调，实现控制闭环。可以看到，**包含关系为chassis∈motor∈bspcan**。

有了上面的大体认知，我们分别介绍框架的三层结构。

- **bsp**即板级支持包，提供对开发板外设的软件抽象，让module层能使用和硬件无关的接口（由bsp提供）进行数据处理与交互。bsp层和ST的HAL为强耦合，与硬件直接绑定。若要向其他的ST芯片移植，基本不需要修改bsp层；若是其他单片机则建议保留**接口设计**，对接口调用进行重现实现。每一种外设的头文件中都定义了一个**XXXInstance**（xxx为外设名），其中包含了使用该外设所需要的所有数据，如发送/接收的数据，长度，id（如果有），父指针（指向module实例的指针，用于回调）等。由于C没有`class`，因此所有bsp的接口都需要传入一个额外的参数：XXXInstance*，用于实现c++的`this`指针以区分具体是哪一个实例调用了接口。
- **module**即模块层，包括了需要开发板硬件外设支持的（一般用于通信）真实**硬件模组**如电机、舵机、imu、测距传感器，以及通过



## 执行顺序与数据流





## 开发工具

介绍完整的工作流。

### 工具链

强烈推荐使用arm-gnu工具链进行编译（arm-none-eabi-xxx）。

官方下载地址：[Arm GNU Toolchain Downloads – Arm Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

我们更推荐使用Msys2进行库和开发工具管理，关于如何使用Msys2请参考：[如何使用本框架](##如何使用本框架)

> 仍然支持使用arm-cc工具链（即keil默认的工具链）进行开发，在cubemx初始化时生成MDK项目即可，然后再手动添加basic_framework的所有头文件和源文件。但非常不建议这样做，arm-cc仅支持单线程编译且编译优化选项远不如arm-gnu多，自定义程度同样不比。~~若你一定要这样做，则可以在VSCode中安装keil assistant插件。~~

### IDE?

使用VSCode作为“IDE”。需要的插件支持均已经在[VSCode+Ozone使用方法.md](VSCode+Ozone使用方法.md)中给出。通过VSCode强大的插件系统、language server以及代码补全高亮助力效率倍增。编译则使用集成的task进行，还可以将开发环境终端加入VSCode进一步提升体验。基本的调试如变量&寄存器查看均已在插件中提供支持，`launch.json`可以进行高自由度的自定义。

`Git`集成与额外插件补充让版本管理和协作从未如此简单，`live share`把你的伙伴们聚在一起集思广益，一同对抗困难的bug。更多特性和开发技巧请参考"如何使用本框架"章节。

> **不论如何，请不要使用KEIL作为你的代码编辑器。**

### 调试和性能分析

- 基础的调试可以在VSCode中完成。cortex-debug插件的最新版本已经支持多个gdb-server（jlink/stlink/openocd/pyocd）的live watch（动态变量监视）和变量示波器（可视化）。若不是有特别的需求，**请勿使用串口调试器**。

- 有高速变量查看和数据记录、多路数据可视化的需求（如进行pid参数整定、查找难以定位的bug）时，使用**Segger Ozone**。
- FreeMaster也可以作为调试的备选项。
- 基本的、日常性能分析可以通过`bsp_dwt`完成。若要分析关于任务运行和每个函数执行的详细信息和时间，推荐使用**Segger Systemviewer**。

## 如何使用本框架






## 后续计划
