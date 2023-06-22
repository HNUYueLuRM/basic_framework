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

我们还提供了一个宏用于调试计时:

```c
#define TIME_ELAPSE(dt, code)                    \
    do                                           \
    {                                            \
        float tstart = DWT_GetTimeline_s();      \
        code;                                    \
        dt = DWT_GetTimeline_s() - tstart;       \
        LOGINFO("[DWT] " #dt " = %f s\r\n", dt); \
    } while (0)

```

传入一个float类型的变量,并将你要执行的代码写入第二个参数:

```c
    static float my_func_dt;
    TIME_ELAPSE(my_func_dt,
                Function1(vara);
                Function2(some, var);
                Function3(your,param);
                 // something more
                 );
    // my_func_dt can be used for other purpose then;
```
