<!--
 * @Descripttion: 
 * @version: 
 * @Author: Chenfu
 * @Date: 2022-12-02 21:32:47
 * @LastEditTime: 2022-12-05 15:27:57
-->
# super_can

## 代码结构

.h中放置的是数据定义和外部接口，以及协议的定义和宏，.c中包含一些私有函数。

## 外部接口

```c
SuperCapInstance *SuperCapInit(SuperCap_Init_Config_s* supercap_config);
void SuperCapSend(SuperCapInstance *instance, uint8_t *data);
```
## 私有函数和变量

```c
static SuperCapInstance *super_cap_instance = NULL;
static uint8_t *rxbuff;
static void SuperCapRxCallback(can_instance *_instance)
```

`SuperCapRxCallback()`是super cap初始化can实例时的回调函数，用于can接收中断，进行协议解析。

## 使用范例

初始化时设置如下：

```c
SuperCap_Init_Config_s capconfig = {
		.can_config = {
			.can_handle = &hcan1,
			.rx_id = 0x301,
			.tx_id = 0x302
		},
		.recv_data_len = 4*sizeof(uint16_t),
		.send_data_len = sizeof(uint8_t)
	};
SuperCapInstance *ins =SuperCapInit(&capconfig);
```


发送通过`SuperCapSend()`，建议使用强制类型转换：

```c
uint16_t tx = 0x321;
SuperCapSend(ins, (uint8_t*)&tx);
```

