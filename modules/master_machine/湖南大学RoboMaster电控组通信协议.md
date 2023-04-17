# 湖南大学RoboMaster电控组通信协议

<p align='right'>Seasky LIUWei</p>

<p align='right'>modified by neozng1@hnu.edu.cn</p>

可用于视觉，以及其他自研模块（仅限串口通信）

> TODO:
>
> 1. 利用F4自带的硬件CRC模块计算校验码，提高速度
> 2. 增加更多的数据类型支持，使得数据类型也为可配置的

## 一、串口配置

通信方式是串口，配置为波特率 921600，8 位数据位，1 位停止位，无硬件流控，无校验位。

## 二、数据帧说明

以下所有低位在前发送，数据长度可变。

1. 通信协议格式

   | 帧头                    | 数据ID         | 数据          | 帧尾                                |
   | ----------------------- | -------------- | ------------- | :---------------------------------- |
   | protocol_header(4-byte) | cmd_id(2-byte) | data (n-byte) | frame_tail(2-byte，CRC16，整包校验) |

2. 帧头详细定义

   | 帧头        | 偏移位置 | 字节大小 | 内容                          |
   | ----------- | -------- | -------- | ----------------------------- |
   | sof(CMD)    | 0        | 1        | 数据帧起始字节，固定值为 0xA5 |
   | data_length | 1        | 2        | 数据帧中 data 的长度          |
   | crc_check   | 3        | 1        | 帧头CRC校验                   |

3. cmd_id 命令码 ID 说明 **(字节偏移 4，字节大小 2)**

   | 命令码         | 数据段长度      | 功能说明 |
   | -------------- | --------------- | -------- |
   | 0x0001(可修改) | 2 byte (16-bit) | 视觉数据 |
   | .......        |                 |          |

4.  数据段data (n-byte)

   | 数据           | 偏移位置 | 字节大小        | 内容                                  |
   | -------------- | -------- | --------------- | ------------------------------------- |
   | flags_register | 6        | 2               | 16位标志置位寄存器                    |
   | float_data     | 8        | 4 * data_length | float数据内容（4 * data_length-byte） |

5. frame_tail(CRC16，整包校验)

## 三、协议接口说明

```c
/*更新发送数据帧，并计算发送数据帧长度*/
void get_protocol_send_data(uint16_t send_id,		 //信号id
							uint16_t flags_register, // 16位寄存器
							float *tx_data,			 //待发送的float数据
							uint8_t float_length,	 // float的数据长度
							uint8_t *tx_buf,		 //待发送的数据帧
							uint16_t *tx_buf_len);	 //待发送的数据帧长度
```

将float_data字段准备好放入`tx_data[]`并设置好标志位`flags_register`之后，调用此函数。它将计算循环冗余校验码，并把数据转换成串口发送的格式，放在`tx_buf[]`中。

```c
/*接收数据处理*/
uint16_t get_protocol_info(uint8_t *rx_buf,			 //接收到的原始数据
						   uint16_t *flags_register, //接收数据的16位寄存器地址
						   float *rx_data);			 //接收的float数据存储地址
```

将收到的一包原始数据buff地址传入，若校验通过，会把收到的标志位和float数据解析出来，保存在`flags_register*`和 `rx_data[]`中。