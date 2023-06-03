# univsersal communication

@todo

unicomm旨在为通信提供一套标准的协议接口，屏蔽底层的硬件差异，使得上层应用可以定制通信协议，包括包长度/可变帧长/帧头尾/校验方式等。

不论底层具体使用的是什么硬件接口，实际上每一帧传输完并把数据放在缓冲区之后，就没有任何区别了。 此模块实际上就是对缓冲区的rawdata进行操作，包括查找帧头，计算包长度，校验错误等。

完成之后，可以将module/can_comm、视觉的通信协议seasky_protocol和master_process等移除，把原使用了cancomm的应用迁移到此模块。
