#include "HT04.h"
#include "memory.h"

joint_instance joint_motor_info[HT_MOTOR_CNT];

static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (uint16_t) ((x-offset)*((float)((1<<bits)-1))/span);
}

static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}

void JointControl(joint_instance* _instance,float current)
{
    uint16_t tmp;
    LIMIT_MIN_MAX(current,  T_MIN,  T_MAX);
    tmp = float_to_uint(current, T_MIN, T_MAX, 12);
    _instance->motor_can_instace.rx_buff[6] = tmp>>8;
    _instance->motor_can_instace.rx_buff[7] = tmp&0xff;
    CANTransmit(_instance);
}

void SetJointMode(joint_mode cmd,joint_instance* _instance)
{
    static uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    buf[7]=(uint8_t)cmd;
    memcpy(_instance->motor_can_instace.rx_buff,buf,8*sizeof(uint8_t));
    CANTransmit(&_instance->motor_can_instace);
}

void DecodeJoint(can_instance* motor_instance)
{
    uint16_t tmp;
    for (size_t i = 0; i < HT_MOTOR_CNT; i++)
    {
        if(&joint_motor_info[i].motor_can_instace==motor_instance)
        {
            tmp = (motor_instance->rx_buff[1] << 8) | motor_instance->rx_buff[2];
            joint_motor_info[i].last_ecd=joint_motor_info[i].ecd;
            joint_motor_info[i].ecd=uint_to_float(tmp,P_MAX,P_MIN,16);
            tmp = (motor_instance->rx_buff[3] << 4) | (motor_instance->rx_buff[4] >> 4);
            joint_motor_info[i].speed_rpm= uint_to_float(tmp,V_MAX,V_MIN,12);
            tmp=((motor_instance->rx_buff[4]&0xf)<<8) | motor_instance->rx_buff[5];
            joint_motor_info[i].given_current=uint_to_float(tmp,T_MAX,T_MIN,12);
            break;
        }
    }
}