# daemon

<p align='right'>neozng1@hnu.edu.cn</p>

用于监测模块和应用运行情况的module(和官方代码中的deteck task作用相同)

## 使用范例

要使用该module，则包含`daemon.h`的头文件，并在使用daemon的文件中保留一个daemon的指针。

初始化的时候，要传入以下参数：

```c
typedef struct
{
    uint16_t reload_count;     // 实际上这是app唯一需要设置的值?
    offline_callback callback; // 异常处理函数,当模块发生异常时会被调用
    void *owner_id;            // id取拥有daemon的实例的地址,如DJIMotorInstance*,cast成void*类型
} Daemon_Init_Config_s;
```

`reload_count`是”喂狗“时的重载值，一般根据你希望的离线容许时间和模块接收数据/访问数据的频率确定。daemonTask递减计数器的频率是100hz(在HAL_N_Middlewares/Src/freertos.c中查看任务),你可以据此以及模块收到数据/操作的频率设置reload_count。

`daemon_task()`会在实时系统中以1kHz的频率运行，每次运行该任务，都会将所有daemon实例当前的count进行自减操作，当count减为零，则说明模块已经很久没有上线（处于deactivated状态，即没有收到数据，也没有进行其他读写操作）。

`offline_callback`是模块离线的回调函数，即当包含daemon的模块发生离线情况时，该函数会被调用以应对离线情况，如重启模块，重新初始化等。如果没有则传入`NULL`即可。

`owner_id`即模块取自身地址并通过强制类型转换化为`void*`类型，用于拥有多个实例的模块在`offline_callback`中区分自身。如多个电机都使用一个相同的`offline_callback`，那么在调用回调函数的时候就可以通过该指针来访问某个特定的电机。

> 这种方法也称作“parent pointer”，即**保存拥有指向自身的指针对象的地址**。这样就可以在特定的情况下通过自身来访问自己的父对象。



## 具体实现

即`DaemonTask()`，在操作系统中以1kHz运行。注释详细，请参见`daemon.c`。