# bsp iic

> 预计增加模拟iic

## 注意事项

使用时写入地址，不需要左移！！！

cubemx未配置dma时请勿使用dma传输，其行为是未定义的。

## 总线机制详解

关于I2C的序列传输,Restart condition和总线仲裁,请看:

https://blog.csdn.net/NeoZng/article/details/128496694

https://blog.csdn.net/NeoZng/article/details/128486366

使用序列通信则在单次通信后不会释放总线,继续占用直到调用传输函数时传入`IIC_RELEASE`参数. 这个功能只在一条总线上挂载多个主机的时候有用.
