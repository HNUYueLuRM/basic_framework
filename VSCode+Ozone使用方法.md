# VSCode+Ozone开发STM32的方法

<center><b><font face="楷体">neozng1@hnu.edu.cn</font></b></center>

[TOC]

## 前言

了解过嵌入式开发的你一定接触过Keil，这款20世纪风格UI的IDE伴随很多人度过了学习单片机的岁月。然而由于其缺少代码补全、高亮和静态检查的支持，以及为人诟病的一系列逆天的设置、极慢的编译速度（特别是在开发HAL库时），很多开发者开始转向其他IDE。

IAR、CubeIDE等都是广为使用的“其他”IDE，但是他们也有各自的缺点，不能让笔者满意。作为IDE界的艺术家，JetBrains推出的Clion也在相当程度上完善了对嵌入式开发的支持。不过，在体验过多款IDE后，还是**VSCode**这款高度定制化的编辑器最让人满意。

而Ozone则是SEGGER(做jilnk的)推出的调试应用，支持变量实时更新，变量曲线可视化，SEGGER RTT日志，DBG虚拟串口等功能，大大扩展了调试的功能。很多人习惯使用串口进行可视化调试，如vofa，串口调试助手等。然而通过这些方式进行调试，都是对内核有**侵入性**的，会占有内核资源并且导致定时器的时间错乱。由于DBG有单独连接到FLASH和CPU寄存器的高速总线（类似于DMA），可以在不影响程序正常运行的情况下以极高的频率直接获取变量值。

下面，将从工具链介绍、环境配置以及调试工作流三个方面介绍以VSCode为编辑器，Ozone为调试接口的开发环境。

开发的大致流程为：

~~~mermaid
graph LR
CubeMX进行初始化 --> VSCode编写代/进行编译/简单调试 --> Ozone变量可视化调试+log
~~~

## 前置知识

1. 计算机速成课：[Crash Course Computer Science](https://www.bilibili.com/video/av21376839/?vd_source=ddae2b7332590050afe28928f52f0bda)

2. 从零到一打造一台计算机：

   [编程前你最好了解的基本硬件和计算机基础知识（模拟电路）](https://www.bilibili.com/video/BV1774114798/?spm_id_from=333.788.recommend_more_video.11&vd_source=ddae2b7332590050afe28928f52f0bda)

   [编程前你最好了解的基本硬件和计算机基础知识（数字电路）](https://www.bilibili.com/video/BV1Hi4y1t7zY/?spm_id_from=333.788.recommend_more_video.0)

   [从0到1设计一台计算机](https://www.bilibili.com/video/BV1wi4y157D3/?spm_id_from=333.788.recommend_more_video.0&vd_source=ddae2b7332590050afe28928f52f0bda)

3. C语言基础：[程序设计入门——C语言](https://www.icourse163.org/course/ZJU-199001?from=searchPage&outVendor=zw_mooc_pcssjg_)

***务必学完以上课程再开始本教程的学习。***

## 预备知识

1. 软件安装(队伍NAS和资料硬盘内提供了所有必要的依赖,安装包和插件)
2. C语言从源代码到.bin和.hex等机器代码的编译和链接过程
3. C语言的内存模型
4. C语言标准，动态链接库和静态编译的区别，一些编译器的常用选项
5. STM32F4系列的DBG外设工作原理

### 编译全过程

C语言代码由固定的词汇（关键字）按照固定的格式（语法）组织起来，简单直观，程序员容易识别和理解，但是CPU只能识别二进制形式的指令，并且这些指令是和硬件相关的（感兴趣的同学可以搜索**指令集**相关内容）。这就需要一个工具，将C语言代码转换成CPU能够识别的二进制指令，对于我们的x86平台windows下的程序就是.exe后缀的文件；对于单片机，一般来说是.bin或.hex等格式的文件（调试文件包括axf和elf）。

能够完成这个转化过程的工具是一个特殊的软件，叫做**编译器（Compiler）**。常见的编译器包括开源的GNU GCC，windows下微软开发的visual C++，以及apple主导的llvm/clang。编译器能够识别代码中的关键字、表达式以及各种特定的格式，并将他们转换成特定的符号，也就是**汇编语言**（再次注意汇编语言是平台特定的），这个过程称为**编译（Compile）**。

对于单个.c文件，从C语言开始到单片机可识别的.bin文件，一般要经历以下几步：

![img](https://pic3.zhimg.com/80/v2-2797ea99d0d38eb9996993bb0ad77ab2_720w.webp)

首先是编译**预处理**Preprocessing，这一步会展开宏并删除注释，将多余的空格去除。预处理之后会生成.i文件。

然后，开始**编译**Compilation的工作。编译器会将源代码进行语法分析、词法分析、语义分析等，根据编译设置进行性能优化，然后生成汇编代码.s文件。汇编代码仍然是以助记符的形式记录的文本，比如将某个地址的数据加载到CPU寄存器等，还需要进一步翻译成二进制代码。

下一步就是进行**汇编**Assemble，编译器会根据汇编助记符和机器代码的查找表将所有符号进行替换，生成.o .obj等文件。但请注意，这些文件并不能直接使用（烧录），我们在编写代码的时候，都会包含一些**库**，因此编译结果应当有多个.o文件。我们还需要一种方法将这些目标文件缝合在一起，使得在遇到函数调用的时候，程序可以正确地跳转到对应的地方执行。

最后一步就由链接器Linker（也称LD）完成，称为**链接**Linking。比如你编写了一个motor.c文件和.h文件，并在main.c中包含了motor.h，使用了后者提供的`MotorControl()`函数。那么，链接器会根据编译器生成.obj文件时留下的函数入口地址，将main.o里的调用映射到生成的motor.o中。链接完成后，就生成了单片机可以识别的可执行文件，通过支持的串口或下载器烧录，便可以运行。

> 另外，上图可以看到左侧的**静态库**，包括`.lib .a`，比如我们在STM32中使用的DSP运算库就是这种文件。他在本质上和.o文件相同，只要你在你编写的源文件中包含了这些库的头文件，链接器就可以根据映射关系找到头文件中声明的函数在库文件的地址。（直接提供库而不是.c文件，就可以防止源代码泄露，因此一些不开源的程序会提供函数调用的头文件和接口具体实现的库；你也可以编写自己的库，感兴趣自行搜索）

链接之后，实际上还要进行不同代码片段的重组、地址重映射，详细的内容请参看：[C/C++语言编译链接过程](https://zhuanlan.zhihu.com/p/88255667)，这篇教程还提供了以GCC为例的代码编译示例。

### C语言内存模型

<img src="C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112160213066.png" alt="image-20221112160213066" style="zoom:80%;" />

以上是C语言常见的内存模型，即C语言的代码块以及运行时使用的内存（包括函数、变量等）的组织方式。

> 有些平台的图与此相反，栈在最下面（内存低地址），其他区域都倒置，不影响我们理解

**代码段**即我们编写的代码，也就是前面说的编译和链接之后最终生成的可执行文件占据的空间。一些常量，包括字符串和使用`const`关键字修饰的变量被放在常量存储区。`static`修饰的静态变量（包括函数静态变量和文件静态变量）以及全局变量放在常量区上面一点的全局区（也称静态区）。

然后就是最重要的**堆**和**栈**。在一个代码块内定义的变量会被放在栈区，一旦离开作用域（出了它被定义的`{}`的区域），就会立刻被销毁。在调用函数或进入一个用户自定义的`{}`块，都会在栈上开辟一块新的空间，空间的大小和内存分配由操作系统或C库自动管理。**一般来说，直接通过变量访问栈内存，速度最快**（对于单片机）。而堆则是存储程序员自行分配的变量的地方，即使用`malloc(),realloc() ,new`等方法获取的空间，都被分配在这里。

>  在CubeMX初始化的时候，Project mananger标签页下有一个Linker Setting的选项，这里是设置最小堆内存和栈内存的地方。如果你的程序里写了大规模的数组，或使用`malloc()`等分配了大量的空间，可能出现栈溢出或堆挤占栈空间的情况。需要根据MCU的资源大小，设置合适的stack size和heap size。

### C language标准和编译器

不同的C语言标准（一般以年份作代号）支持的语法特性和关键字不同，拥有的功能也不同。一般来说语言标准都是向前兼容的，在更新之后仍然会保存前代的基本功能支持（legacy support）。不过，为了程序能够正常运行，我们还需要一些硬件或平台支持的组件。比如`malloc()`这个函数，在linux平台和windows平台上的具体实现就相去甚远，跟单片机更是差了不止一点。前两者一般和对应的操作系统有关，后者在裸机上则是直接通过硬件或ST公司提供的硬件抽象层代码实现。

然而，不同编译器提供的代码实现也不尽相同，比如使用clang和gcc这两种c语言编译器，他们对于一些标准库（也称C库，包括stdio，stdlib，string等在内的实现）的函数的实现就不太一样。再如`__packed`是arm-cc提供的一个字节不对齐关键字，在一些其他编译器中就不支持这种实现。

以前大家常用的KEIL使用的是ARM提供的arm-cc工具链（非常蛋疼，甚至不支持uint8_t=0b00001111这种二进制定义法），而该教程选用的是开源的**Arm GNU Toolchain**。在非目标机且和目标机平台不同的平台上进行开发被成为**跨平台开发**，进行的编译也被成为**交叉编译**（在一个平台上生成另一个平台上的 可执行代码）。

> 工具链包含了编译器，链接器以及调试器等开发常用组件。我们使用的Arm GNU toolchain中，编译器是`arm-none-eabi-gcc.exe`，链接器是`arm-none-eabi-ld.exe`，调试器则是`arm-none-eabi-gdb.exe`。通过跨平台调试器和j-link/st-link/dap-link，我们就可以在自己的电脑上对异构平台（即单片机）的运行进行调试了。

### Debug外设工作原理

![image-20221112145717063](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112145717063.png)

DBG支持模块（红框标注部分，也可以看作一个外设）通过一条专用的AHB-AP总线和调试接口相连（Jtag或swd），并且有与**数据**和**外设**总线直接相连的桥接器。它还同时连接了中断嵌套管理器（因此同样可以捕获中断并进行debug）和ITM、DWT、FPB这些调试支持模块。因此DBG可以直接获取内存或片上外设内的数据而不需要占用CPU的资源，并将这些数据通过专用外设总线发送给调试器，进而在上位机中读取。

FPB是flash patch breakpoint闪存指令断点的缩写，用于提供代码断点插入的支持，当CPU的指令寄存器读取到某一条指令时，FPB会监测到它的动作，并通知TPIU暂停CPU进行现场保护。

DWT是data watch trace数据观察与追踪单元的缩写，用于比较debug变量的大小，并追踪变量值的变化。当你设定了比较断点规则（当某个数据大于/小于某个值时暂停程序）或将变量加入watch进行查看，DWT就会开始工作。DWT还提供了一个额外的计时器，即所有可见的TIM资源之外的另一个硬件计时器（因为调试其他硬件定时器的计时由于时钟变化可能定时不准，而DWT定时器是始终正常运行的）。它用于给自身和其他调试器模块产生的信息打上时间戳。我们的bsp中也封装了dwt计时器，你可以使用它来计时。

ITM是instrument trace macrocell指令追踪宏单元的缩写，它用于提供非阻塞式的日志发送支持（相当于大家常用的串口调试），SEGGER RTT就可以利用这个模块，向上位机发送日志和信息。

以上三个模块都需要通过TPIU（trace port interface unit）和外部调试器（j-link等）进行连接，TPIU会将三个模块发来的数据进行封装并通过DWT记录时间，发送给上位机。

## 环境配置

- 安装STM32CubeMX，并安装F4支持包和DSP库支持包-

- 安装VSCode，并安装C/C++，Cortex-Debug，Cortex-Debug: Device Support Pack - STM32F4，Better C++ Syntax，IntelliCode，Makfile Tools，C/C++ Snippets插件

  ![image-20221112172157533](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172157533.png)

  ![image-20221112172208749](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172208749.png)

  ![image-20221112172221756](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172221756.png)

  ![image-20221112172239386](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172239386.png)

  ![image-20221112172254809](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172254809.png)

- 安装MinGW，等待界面如下：

  ![image-20221112172051589](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172051589.png)

  安装好后，打开MinGW后将所有的支持包勾选，然后安装：

  ![image-20221112172348408](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172348408.png)

  ![image-20221112172420037](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172420037.png)

  安装完以后，将MinGW的bin文件夹添加到环境变量中的path下，按下菜单键搜索**编辑系统环境变量**打开之后：

  ![image-20221112172716320](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172716320.png)

  验证安装：

  打开命令行（win+R，cmd，回车），输入`gcc -v`，如果没有报错，并输出了一堆路径和参数说明安装成功。

- 配置gcc-arm-none-eabi环境变量

  同上，将工具链的bin添加到PATH：

  ![image-20221112172858593](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112172858593.png)

  <center>安装路径可能不一样，这里要使用你自己的路径而不是直接抄</center>

  验证安装：

  打开命令行，输入`arm-none-eabi-gcc -v`，如果没有报错，并输出了一堆路径和参数说明安装成功。

> 添加到环境变量PATH的意思是，当一些程序需要某些依赖或者要打开某些程序时，系统会自动前往PATH下寻找对应项。**一般需要重启使环境变量生效。**

- **将OpenOCD解压到一个文件夹里**，稍后需要在VSCode中设置这个路径。

- CubeMX生成代码的时候工具链选择makefile

  ![image-20221112173534670](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112173534670.png)

  生成的目录结构如下：

  ![image-20221112174211802](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112174211802.png)

  Makefile就是我们要使用的构建规则文件。

## VSCode编译和调试配置

### 编译

用VSCode打开创建的项目文件夹，**Makefile Tools插件会询问你是否帮助配置intellisense，选择是。**











### 简单调试

>  在VSCode中调试不能像Keil一样查看变量动态变化，但是支持以外的所有操作，如查看外设和反汇编代码，设置断点触发方式等。

用于调试的配置参考这篇博客：[Cortex-debug 调试器使用介绍](https://blog.csdn.net/qq_40833810/article/details/106713462)

你需要配置**arm gnu工具链的路径**，**OpenOCD的路径**（使得GDB调试器可以找到OpenOCD并调用它，从而连接硬件调试器如j-link等），该工作区（文件夹）的**launch.json文件**（用于启动vscode的调试任务）。

如果教程看不懂，请看`.vscode`下的`launch.json`，照葫芦画瓢。

根目录下已经提供了C板所需的.svd和使用无线调试器时所用的openocd.cfg配置文件。

然后选择run and debug标签页，在选项中选择你配置好的选项，开始调试。

![image-20221112180103750](C:\Users\Neo\AppData\Roaming\Typora\typora-user-images\image-20221112180103750.png)

