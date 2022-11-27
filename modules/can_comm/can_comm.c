#include "can_comm.h"
#include "memory.h"
#include "stdlib.h"

static CANCommInstance *can_comm_instance[MX_CAN_COMM_COUNT] = {NULL};
static uint8_t idx;

static void CANCommRxCallback(can_instance *_instance)
{
    for (size_t i = 0; i < idx; i++)
    {
        if (&can_comm_instance[i]->can_ins == _instance)
        {
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