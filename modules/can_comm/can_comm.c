#include "can_comm.h"
#include "memory.h"
#include "stdlib.h"
#include "crc8.h"

/**
 * @brief 重置CAN comm的接收状态和buffer
 *
 * @param ins 需要重置的实例
 */
static void CANCommResetRx(CANCommInstance *ins)
{
    memset(ins->raw_recvbuf, 0, ins->cur_recv_len);
    ins->recv_state = 0;
    ins->cur_recv_len = 0;
}

/**
 * @brief cancomm的接收回调函数
 *
 * @param _instance
 */
static void CANCommRxCallback(CANInstance *_instance)
{
    static CANCommInstance *comm;
    comm = (CANCommInstance *)_instance->id;

    /* 接收状态判断 */
    if (_instance->rx_buff[0] == CAN_COMM_HEADER && comm->recv_state == 0) // 尚未开始接收且新的一包里有帧头
    {
        if (_instance->rx_buff[1] == comm->recv_data_len) // 接收长度等于设定接收长度
        {
            comm->recv_state = 1;
        }
        else
            return; // 直接跳过即可
    }
    if (comm->recv_state) // 已经收到过帧头
    {                     // 如果已经接收到的长度加上当前一包的长度大于总buf len,说明接收错误
        if (comm->cur_recv_len + _instance->rx_len > comm->recv_buf_len)
        {
            CANCommResetRx(comm);
            return; // 重置状态然后返回
        }

        // 直接拷贝到当前的接收buffer后面
        memcpy(comm->raw_recvbuf + comm->cur_recv_len, _instance->rx_buff, _instance->rx_len);
        comm->cur_recv_len += _instance->rx_len;

        // 当前已经收满
        if (comm->cur_recv_len == comm->recv_buf_len)
        { // buff里本该是tail的位置不等于CAN_COMM_TAIL
            if (comm->raw_recvbuf[comm->recv_buf_len - 1] != CAN_COMM_TAIL)
            {
                CANCommResetRx(comm);
                return; // 重置状态然后返回
            }
            else // tail正确, 对数据进行crc8校验
            {
                if (comm->raw_recvbuf[comm->recv_buf_len - 2] ==
                    crc_8(comm->raw_recvbuf + 2, comm->recv_data_len))
                { // 通过校验,复制数据到unpack_data中
                    memcpy(comm->unpacked_recv_data, comm->raw_recvbuf + 2, comm->recv_data_len);
                    comm->update_flag = 1; // 数据更新flag置为1
                }
                CANCommResetRx(comm);
                return; // 重置状态然后返回
            }
            return; // 访问完一个can comm直接退出,一次中断只处理一个实例的回调
        }
    }
}

CANCommInstance *CANCommInit(CANComm_Init_Config_s *comm_config)
{
    CANCommInstance* ins = (CANCommInstance *)malloc(sizeof(CANCommInstance));
    memset(ins, 0, sizeof(CANCommInstance));
    ins->recv_data_len = comm_config->recv_data_len;
    ins->recv_buf_len = comm_config->recv_data_len + CAN_COMM_OFFSET_BYTES;
    ins->send_data_len = comm_config->send_data_len;
    ins->send_buf_len = comm_config->send_data_len + CAN_COMM_OFFSET_BYTES;
    ins->raw_sendbuf[0] = CAN_COMM_HEADER;
    ins->raw_sendbuf[1] = comm_config->send_data_len;
    ins->raw_sendbuf[comm_config->send_data_len + CAN_COMM_OFFSET_BYTES - 1] = CAN_COMM_TAIL;

    comm_config->can_config.id = ins;
    comm_config->can_config.can_module_callback = CANCommRxCallback;
    ins->can_ins = CANRegister(&comm_config->can_config);
    return ins;
}

void CANCommSend(CANCommInstance *instance, uint8_t *data)
{
    static uint8_t crc8;
    static uint8_t send_len;
    memcpy(instance->raw_sendbuf + 2, data, instance->send_data_len);
    crc8 = crc_8(data, instance->send_data_len);
    instance->raw_sendbuf[2 + instance->send_data_len] = crc8;

    for (size_t i = 0; i < instance->send_buf_len; i += 8)
    { // 如果是最后一包,send len将会小于8,要修改CAN的txconf中的DLC位,调用bsp_can提供的接口即可
        send_len = instance->send_buf_len - i >= 8 ? 8 : instance->send_buf_len - i;
        CANSetDLC(instance->can_ins, send_len);
        memcpy(instance->can_ins->tx_buff, instance->raw_sendbuf + i, send_len);
        CANTransmit(instance->can_ins);
    }
}

void *CANCommGet(CANCommInstance *instance)
{
    instance->update_flag = 0;
    return instance->unpacked_recv_data;
}