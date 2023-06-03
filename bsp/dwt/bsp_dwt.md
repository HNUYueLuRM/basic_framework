# bsp_dwt

DWT是stm32内部的一个"隐藏资源",他的用途是给下载器提供准确的定时,从而为调试信息加上时间戳.并在固定的时间间隔将调试数据发送到你的xxlink上.



## 常用功能

### 计算两次进入同一个函数的时间间隔

```c
static uint32_t cnt;
float deltaT;

deltaT=DWT_GetDeltaT(&cnt);
```

### 计算执行某部分代码的耗时

```c
float start,end;
start=DWT_DetTimeline_ms();

// some proc to go... 
for(uint8_t i=0;i<10;i++)
	foo();

end = DWT_DetTimeline_ms()-start;
```

