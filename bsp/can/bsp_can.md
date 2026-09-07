# bsp_can

<p align='right'>neozng1@hnu.edu.cn</p>

# 请注意使用CAN设备的时候务必保证总线只接入了2个终端电阻！开发板一般都有一个，6020电机、c620/c610电调、LK电机也都有终端电阻，注意把多于2个的全部断开（通过拨码）



## 使用说明

若你希望新增一个基于CAN的module，首先在该模块下应该有一个包含`can_instance`指针的module结构体（或当功能简单的时候，可以是单独存在的`can_instance`，但不推荐这样做）。

## 代码结构

.h文件内包括了外部接口和类型定义,以及模块对应的宏。c文件内为私有函数和外部接口的定义。

## 类型定义

```c

#define CAN_MX_REGISTER_CNT 16     // 这个数量取决于CAN总线的负载
#define MX_CAN_FILTER_CNT (2 * 14) // 最多可以使用的CAN过滤器数量,目前远不会用到这么多
#define DEVICE_CAN_CNT 2           // 根据板子设定,F407IG有CAN1,CAN2,因此为2;F334只有一个,则设为1
#define CAN_Send_FIFO_SIZE 16      // 发送消息队列的长度

/* can instance typedef, every module registered to CAN should have this variable */
typedef struct _
{
    CAN_HandleTypeDef *can_handle; // can句柄
    uint32_t tx_id;                // 发送id
    uint32_t tx_mailbox;           // CAN消息填入的邮箱号
    uint8_t tx_buff[8];            // 发送缓存,最大为8
    uint8_t rx_buff[8];            // 接收缓存,最大消息长度为8
    uint32_t rx_id;                // 接收id
    uint8_t rx_len;                // 接收长度,可能为0-8
    // 接收的回调函数,用于解析接收到的数据
    void (*can_module_callback)(struct _ *); // callback needs an instance to tell among registered ones
    void *id;                                // 使用can外设的模块指针(即id指向的模块拥有此can实例,是父子关系)
} CANInstance;

/* CAN实例初始化结构体,将此结构体指针传入注册函数 */
typedef struct
{
    CAN_HandleTypeDef *can_handle;              // can句柄
    uint32_t tx_id;                             // 发送id
    uint32_t rx_id;                             // 接收id
    void (*can_module_callback)(CANInstance *); // 处理接收数据的回调函数
    void *id;                                   // 拥有can实例的模块地址,用于区分不同的模块(如果有需要的话),如果不需要可以不传入
} CAN_Init_Config_s;

typedef struct{
    uint8_t data_buff[8];      //数据包
    uint8_t DLC_length;        //数据长度
    CANInstance *can_instance; //对应的CAN实例指针
} CAN_FIFO_Data_s;

typedef struct{
    CAN_FIFO_Data_s queue[CAN_Send_FIFO_SIZE]; // 队列
    uint8_t front_idx;             // 队列头索引
    uint8_t back_idx;              // 队列尾索引
    uint8_t current_size;          // 当前队列长度
} CAN_Send_FIFO_s;
```

- `CAN_MX_REGISTER_CNT`是最大的CAN设备注册数量，当每个设备的发送频率都较高时，设备过多会产生总线拥塞从而出现丢包和数据错误的情况。
- `MX_CAN_FILTER_CNT`是最大的CAN接收过滤器数量，两个CAN共享标号0~27共28个过滤器。这部分内容比较繁杂，暂时不用理解，有兴趣自行参考MCU的数据手册。当前为简单起见，每个过滤器只设置一组规则用于控制一个id的过滤。
- `DEVICE_CAN_CNT`是MCU拥有的CAN硬件数量。
- `CAN_Send_FIFO_SIZE`是一个CAN硬件对应的发送队列长度（每个hcan一个队列）。队列的作用是解决邮箱忙导致发送阻塞的问题（见下文外部接口部分）。

- `can_instance`是一个CAN实例。注意，CAN作为一个总线设备，一条总线上可以挂载多个设备，因此多个设备可以共享同一个CAN硬件。其成员变量包括发送id，发送邮箱（不需要管，只是一个32位变量，CAN收发器会自动设置其值），发送buff以及接收buff，还有接收id和接收协议解析回调函数。**由于目前使用的设备每个数据帧的长度都是8，因此收发buff长度暂时固定为8**。定义该结构体的时候使用了一个技巧，使得在结构体内部可以用结构体自身的指针作为成员，即`can_module_callback`的定义。

- `CAN_Init_Config_s`是用于初始化CAN实例的结构，在调用CAN实例的初始化函数时传入（下面介绍函数时详细介绍）。注意其成员中没有发送相关的配置，发送id依然通过`tx_id`指定。

- `CAN_FIFO_Data_s`和`CAN_Send_FIFO_s`是发送队列相关的结构体，队列成员存放一帧待发送的数据包、数据长度以及发送方实例指针。**这两个结构体是bsp层的私有实现，module代码不需要关心，只在理解bsp_can.c内部逻辑时使用。**

- `can_module_callback()`是模块提供给CAN接收中断回调函数使用的协议解析函数指针。对于每个需要CAN的模块，需要定义一个这样的函数用于解包数据。
- 每个使用CAN外设的module，都需要在其内部定义一个`can_instance*`。


## 外部接口

```c
CANInstance *CANRegister(CAN_Init_Config_s *config);
void CANTransmit(CANInstance *_instance, uint8_t length); // 发送一帧数据,length为数据长度
```

`CANRegister`是用于初始化CAN实例的接口，module层的模块对象（也应当为一个结构体）内要包含一个`can_instance`。调用时传入实例指针，以及用于初始化的config。`CANRegister`应当在module的初始化函数内被调用，推荐config采用以下的方式定义，更加直观明了：

```c
CAN_Init_Config_s config={.can_handle=&hcan1,
                          .tx_id=0x005,
                          .rx_id=0x200,
                          .can_module_callback=MotorCallback}
```

`CANTransmit()`是模块通过其拥有的CAN实例发送数据的接口，调用时传入对应的instance和本次发送的数据长度（1~8，非法长度会被日志指出）。在发送之前，应当给instance内的`tx_buff`赋值。

与旧版不同的是，**`CANTransmit()`不再忙等邮箱空闲而是非阻塞的**：消息先被拷贝到对应CAN硬件的发送队列中（`CANSendFIFOPut`），随后立即触发一次发送尝试（`CANSendFIFOTransmit`）。如果邮箱当前是空的，消息被直接填入邮箱发送；如果邮箱已满，消息留在队列中等待。当邮箱发送完成释放后（总线上有该id的设备才会发送完成），`HAL_CAN_TxMailboxXCompleteCallback()`会被触发，继续从队列取出下一条消息发送，即"入队→发送→发送完成中断→继续出队发送"的链式驱动。当队列满时，会丢弃队头最旧的一条消息并打印错误日志。

## 私有函数和变量

在.c文件内设为static的函数和变量

```c
static CANInstance *can_instance[CAN_MX_REGISTER_CNT] = {NULL};
static uint8_t idx; // 全局CAN实例索引,每次有新的模块注册会自增
static CAN_Send_FIFO_s CAN1_Send_FIFO, CAN2_Send_FIFO; // CAN发送队列,每个hcan对应一个队列
```

这是bsp层管理所有CAN实例的入口。

```c
static void CANServiceInit()
static void CANAddFilter(CANInstance *_instance)
static void CANSendFIFOPut(CANInstance *_instance, CAN_Send_FIFO_s *CAN_Send_FIFO, uint8_t length)
static void CANSendFIFOTransmit(CAN_Send_FIFO_s *CAN_Send_FIFO)
static void CANFIFOxCallback(CAN_HandleTypeDef *_hcan, uint32_t fifox)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
```

- `CANServiceInit()`会被`CANRegister()`调用，对CAN外设进行硬件初始化并开启接收中断、消息提醒以及发送邮箱空闲中断（`CAN_IT_TX_MAILBOX_EMPTY`,发送完成链式驱动的前提）。

- `CANAddFilter()`在每次使用`CANRegister()`的时候被调用，用于给当前注册的实例添加过滤器规则并设定处理对应`rx_id`的接收FIFO。过滤器的作用是减小CAN收发器的压力，只接收符合过滤器规则的报文（否则不会产生接收中断）。注意FIFO的分配规则是各CAN内部按过滤器编号的奇偶分配：奇数编号的模块被分配到FIFO0，偶数编号的模块被分配到FIFO1（先前版本是按id奇偶分配，已修改）。

- `CANSendFIFOPut()`和`CANSendFIFOTransmit()`是发送队列的入队和出队发送函数。二者都在临界区内操作队列：入队发生在任务上下文，出队发生在发送完成中断上下文，存在竞态，因此用BASEPRI关闭优先级5及以上的中断来保护（对外部中断实时性有轻微影响，后续可能改为无锁方案）。`CANSendFIFOTransmit()`会在邮箱满时直接返回，消息留在队列中等待下一次发送中断的触发。

- `HAL_CAN_RxFifo0MsgPendingCallback()`和`HAL_CAN_RxFifo1MsgPendingCallback()`都是对HAL的CAN回调函数的重定义（原本的callback是`__weak`修饰的弱定义），当发生FIFO0或FIFO1有新消息到达的时候，对应的callback会被调用。`CANFIFOxCallback()`随后被前两者调用，并根据接收id和硬件中断来源（哪一个CAN硬件，CAN1还是CAN2）调用对应的instance的回调函数进行协议解析。

- `HAL_CAN_TxMailbox0/1/2CompleteCallback()`是发送完成的回调。每当一个邮箱发送完成被释放，就调用`CANSendFIFOTransmit()`将对应CAN硬件的队列中的下一条消息填入邮箱，形成上面的链式驱动。

- 当有一个模块注册了多个can实例时，通过`CANInstance.id`,使用强制类型转换将其转换成对应模块的实例指针，就可以对不同的模块实例进行回调处理了。

## 注意事项

- 发送流程是非阻塞的：即使总线上没有挂载目标设备（接收id与发送报文id相同的设备），发送队列也不会把任务卡死。但此时邮箱永远不会发送完成、队列会不断累积，最终丢弃最旧的消息并打印`CAN Send FIFO full`错误日志。建议没有连接CAN进行调试时，按需注释掉有关CAN发送的代码部分，或仔细检查总线上是否挂载了对应该id的设备。
- 由于发送占用了三个发送完成中断回调，请确保这些回调没有被其他代码覆盖（弱定义，只有本模块重定义）。
- 队列满时的丢帧策略是丢弃最旧的报文，若某模块的发送频率超过总线吞吐，其发送消息可能被降级，请注意控制总线负载
`CAN_Send_FIFO_SIZE`。
