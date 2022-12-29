#include "bsp_can.h"
#include "main.h"
#include "memory.h"
#include "stdlib.h"

/* can instance ptrs storage, used for recv callback */
// 在CAN产生接收中断会遍历数组,选出hcan和rxid与发生中断的实例相同的那个,调用其回调函数
static CANInstance *instance[MX_REGISTER_DEVICE_CNT] = {NULL};

/* ----------------two static function called by CANRegister()-------------------- */

/**
 * @brief add filter to receive mesg with specific ID,called by CANRegister()
 *        给CAN添加过滤器后,BxCAN会根据接收到的报文的id进行消息过滤,符合规则的id会被填入FIFO触发中断
 *
 * @note there are total 28 filter and 2 FIFO in bxCAN of STM32F4 series product.
 *       here, we assign the former 14 to CAN1 and the rest for CAN2
 *       when initializing, module with odd ID will be assigned to FIFO0 while even one to FIFO1
 *       those modules which registered in CAN1 would use Filter0-13, while CAN2 use Filter14-27
 *
 * @attention you don't have to fully understand what this function done, cause it is basically
 *            for initialization.Enjoy developing without caring about the infrastructure!
 *            if you really want to know what is happeng, contact author.
 *
 * @param _instance can instance owned by specific module
 */
static void CANAddFilter(CANInstance *_instance)
{
    CAN_FilterTypeDef can_filter_conf;
    static uint8_t can1_filter_idx = 0, can2_filter_idx = 14;

    can_filter_conf.FilterMode = CAN_FILTERMODE_IDLIST;
    can_filter_conf.FilterScale = CAN_FILTERSCALE_16BIT;
    can_filter_conf.FilterFIFOAssignment = (_instance->tx_id & 1) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    can_filter_conf.SlaveStartFilterBank = 14;
    can_filter_conf.FilterIdLow = _instance->rx_id << 5;
    can_filter_conf.FilterBank = _instance->can_handle == &hcan1 ? (can1_filter_idx++) : (can2_filter_idx++);
    can_filter_conf.FilterActivation = CAN_FILTER_ENABLE;

    HAL_CAN_ConfigFilter(_instance->can_handle, &can_filter_conf);
}

/**
 * @brief called by CANRegister before the first module being registered
 *        在第一个CAN实例初始化的时候会自动调用此函数,启动CAN服务
 *
 * @note this func will handle all these thing automatically
 *       there is no need to worry about hardware initialization, we do these for you!
 *
 */
static void CANServiceInit()
{
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_Start(&hcan2);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);
}

/* ----------------------- two extern callable function -----------------------*/

CANInstance *CANRegister(CAN_Init_Config_s *config)
{
    static uint8_t idx; // 全局CAN实例索引,每次有新的模块注册会自增
    if (!idx)
    {
        CANServiceInit(); // 第一次注册,先进行硬件初始化
    }
    instance[idx] = (CANInstance *)malloc(sizeof(CANInstance)); // 分配空间
    memset(instance[idx], 0, sizeof(CANInstance));
    // 进行发送报文的配置
    instance[idx]->txconf.StdId = config->tx_id;
    instance[idx]->txconf.IDE = CAN_ID_STD;
    instance[idx]->txconf.RTR = CAN_RTR_DATA;
    instance[idx]->txconf.DLC = 0x08; // 默认发送长度为8
    // 设置回调函数和接收发送id
    instance[idx]->can_handle = config->can_handle;
    instance[idx]->tx_id = config->tx_id; // 好像没用,可以删掉
    instance[idx]->rx_id = config->rx_id;
    instance[idx]->can_module_callback = config->can_module_callback;
    instance[idx]->id=config->id;

    CANAddFilter(instance[idx]); // 添加CAN过滤器规则
    return instance[idx++];      // 返回指针
}

/* TODO:目前似乎封装过度,应该添加一个指向tx_buff的指针,tx_buff不应该由CAN instance保存 */
void CANTransmit(CANInstance *_instance)
{
    while (HAL_CAN_GetTxMailboxesFreeLevel(_instance->can_handle) == 0)
        ;
    // tx_mailbox会保存实际填入了这一帧消息的邮箱,但是知道是哪个邮箱发的似乎也没啥用
    HAL_CAN_AddTxMessage(_instance->can_handle, &_instance->txconf, _instance->tx_buff, &_instance->tx_mailbox);
}

void CANSetDLC(CANInstance *_instance, uint8_t length)
{
    if (length > 8) // 安全检查
        while (1)
            ;
    _instance->txconf.DLC = length;
}

/* -----------------------belows are callback definitions--------------------------*/

/**
 * @brief this func will recv data from @param:fifox to a tmp can_rx_buff
 *        then, all the instances will be polling to check which should recv this pack of data
 *
 * @param _hcan
 * @param fifox passed to HAL_CAN_GetRxMessage() to get mesg from a specific fifo
 */
static void CANFIFOxCallback(CAN_HandleTypeDef *_hcan, uint32_t fifox)
{
    uint8_t can_rx_buff[8];
    CAN_RxHeaderTypeDef rxconf;
    HAL_CAN_GetRxMessage(_hcan, fifox, &rxconf, can_rx_buff);
    for (size_t i = 0; i < MX_REGISTER_DEVICE_CNT; ++i)
    {
        if (instance[i] != NULL) // 碰到NULL说明已经遍历完所有实例
        {                        // 两者相等说明这是要找的实例
            if (_hcan == instance[i]->can_handle && rxconf.StdId == instance[i]->rx_id)
            {
                instance[i]->rx_len = rxconf.DLC;
                memcpy(instance[i]->rx_buff, can_rx_buff, rxconf.DLC); // 消息拷贝到对应实例
                instance[i]->can_module_callback(instance[i]);         // 触发回调进行数据解析和处理
                break;
            }
        }
        else
            break;
    }
}

/* ATTENTION: two CAN devices in STM32 share two FIFOs */
/* functions below will call CANFIFOxCallback() to further process message from a specific CAN device */
/**
 * @brief rx fifo callback. Once FIFO_0 is full,this func would be called
 *
 * @param hcan CAN handle indicate which device the oddest mesg in FIFO_0 comes from
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CANFIFOxCallback(hcan, CAN_RX_FIFO0);
}

/**
 * @brief rx fifo callback. Once FIFO_1 is full,this func would be called
 *
 * @param hcan CAN handle indicate which device the oddest mesg in FIFO_1 comes from
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CANFIFOxCallback(hcan, CAN_RX_FIFO1);
}