#include "can_comm.h"
#include "memory.h"
#include "stdlib.h"
#include "crc8.h" 

/* can_comm用于保存每个实例的指针数组,用于回调函数区分实例 */
static CANCommInstance *can_comm_instance[MX_CAN_COMM_COUNT] = {NULL};
static uint8_t idx; //配合can_comm_instance的初始化使用,标识当前初始化的是哪一个实例

/**
 * @brief 
 * 
 * @param _instance 
 */
static void CANCommRxCallback(can_instance *_instance)
{
    for (size_t i = 0; i < idx; i++)
    {
        if (&can_comm_instance[i]->can_ins == _instance) // 遍历,找到对应的接收CAN COMM实例
        {
            if(_instance->rx_buff[0]==CAN_COMM_HEADER && can_comm_instance[i]->recv_state==0) //尚未开始接收且新的一包里有帧头
            {
                if(_instance->rx_buff[1]==can_comm_instance[i]->recv_data_len) // 接收长度等于设定接收长度
                {
                    can_comm_instance[i]->recv_state=1;
                }
                else
                    return ; //直接跳过即可
            }
            if(can_comm_instance[i]->recv_state) //已经开始接收
            {   // 直接拷贝到当前的接收buffer后面
                memcpy(can_comm_instance[i]->raw_recvbuf+can_comm_instance[i]->cur_recv_len,_instance->rx_buff,8);
                can_comm_instance[i]->cur_recv_len+=8;
                // 当前已经收满
                if(can_comm_instance[i]->cur_recv_len>=can_comm_instance[i]->recv_buf_len) 
                {   // buff里本该是tail的位置不等于CAN_COMM_TAIL
                    if(can_comm_instance[i]->raw_recvbuf[can_comm_instance[i]->recv_buf_len-1]!=CAN_COMM_TAIL) 
                    {   
                        memset(can_comm_instance[i]->raw_recvbuf,0,can_comm_instance[i]->recv_buf_len);
                        can_comm_instance[i]->recv_state=0;
                        can_comm_instance[i]->cur_recv_len=0;
                        return ; // 重置状态然后返回
                    }
                    else // tail正确, 对数据进行crc8校验
                    {
                        if(can_comm_instance[i]->raw_recvbuf[can_comm_instance[i]->recv_buf_len-2] == 
                           crc_8(can_comm_instance[i]->raw_recvbuf+2,can_comm_instance[i]->recv_data_len))
                        {   // 通过校验,复制数据到unpack_data中
                            memcpy(can_comm_instance[i]->raw_recvbuf+2,can_comm_instance[i]->unpacked_recv_data,can_comm_instance[i]->recv_data_len);
                            can_comm_instance[i]->update_flag=1; //数据更新flag置为1
                        }
                        memset(can_comm_instance[i]->raw_recvbuf,0,can_comm_instance[i]->recv_buf_len); //整个buff置零
                        can_comm_instance[i]->recv_state=0;
                        can_comm_instance[i]->cur_recv_len=0;
                        return ; // 重置状态然后返回
                    }
                }
            }
            return ;
        }
    }
}

CANCommInstance *CANCommInit(CANComm_Init_Config_s config)
{
    can_comm_instance[idx] = (CANCommInstance *)malloc(sizeof(CANCommInstance));
    memset(can_comm_instance[idx], 0, sizeof(CANCommInstance));
    can_comm_instance[idx]->recv_data_len = config.recv_data_len;
    can_comm_instance[idx]->send_data_len = config.send_data_len;
    can_comm_instance[idx]->send_buf_len = config.send_data_len + CAN_COMM_OFFSET_BYTES;
    can_comm_instance[idx]->raw_sendbuf[0] = CAN_COMM_HEADER;
    can_comm_instance[idx]->raw_sendbuf[1] = config.send_data_len;
    can_comm_instance[idx]->raw_sendbuf[config.send_data_len + CAN_COMM_OFFSET_BYTES - 1] = CAN_COMM_TAIL;

    config.can_config.can_module_callback = CANCommRxCallback;
    CANRegister(&can_comm_instance[idx]->can_ins, config.can_config);
    return can_comm_instance[idx++];
}

void CANCommSend(CANCommInstance *instance, uint8_t *data)
{
    static uint8_t crc8;
    memcpy(instance->raw_sendbuf + 2, data, instance->send_data_len);
    crc8 = crc_8(data, instance->send_data_len);
    instance->raw_sendbuf[2 + instance->send_data_len] = crc8;

    for (size_t i = 0; i < instance->send_buf_len; i += 8)
    {
        memcpy(instance->can_ins.tx_buff, instance->raw_sendbuf[i], 8);
        CANTransmit(&instance->can_ins);
    }
}

void *CANCommGet(CANCommInstance *instance)
{
    instance->update_flag = 0;
    return instance->unpacked_recv_data;
}