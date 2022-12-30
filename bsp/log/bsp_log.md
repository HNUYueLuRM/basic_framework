# bsp_log

<p align='right'>neozng1@hnu.edu.cn</p>

> TODO:
>
> 1. 在未接入调试器的时候，将日志写入flash中，并提供接口读取
> 2. 增加日志分级，提供info、warning、error三个等级的日志

## 使用说明

bsp_log是基于segger RTT实现的日志打印模块。

```c
int printf_log(const char *fmt, ...);
void Float2Str(char *str, float va);
```

调用第一个函数，可以通过jlink或dap-link向调试器连接的上位机发送信息，格式和printf相同，示例如下：

```c
printf_log("Hello World!\n");
printf_log("Motor %d met some problem, error code %d!\n",3,1);
```

第二个函数可以将浮点类型转换成字符串以方便发送：

```c
float current_feedback=114.514;
char* str_buff[64];
Float2Str(str_buff,current_feedback);
printf_log("Motor %d met some problem, error code %d!\n",3,1);
```

或直接通过`%f`格式符直接使用`printf_log()`发送日志，可以设置小数点位数以降低带宽开销。
