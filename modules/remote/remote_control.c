#include "remote_control.h"
#include "string.h"
#include "bsp_usart.h"
#include "memory.h"

#define REMOTE_CONTROL_FRAME_SIZE 18u // 遥控器接收的buffer大小
// 遥控器数据
static RC_ctrl_t rc_ctrl[2]; //[0]:当前数据,[1]:上一次的数据.用于按键判断
// 遥控器拥有的串口实例
static USARTInstance *rc_usart_instance;

/**
 * @brief          remote control protocol resolution
 * @param[in]      sbus_buf: raw data point
 * @param[out]     rc_ctrl: remote control data struct point
 * @retval         none
 */
static void sbus_to_rc(volatile const uint8_t *sbus_buf)
{
    memcpy(&rc_ctrl[1], &rc_ctrl[0], sizeof(RC_ctrl_t)); // 保存上一次的数据
    // 摇杆
    rc_ctrl[0].joy_stick.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;                              //!< Channel 0
    rc_ctrl[0].joy_stick.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff;                       //!< Channel 1
    rc_ctrl[0].joy_stick.ch[2] = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) | (sbus_buf[4] << 10)) & 0x07ff; //!< Channel 2
    rc_ctrl[0].joy_stick.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff;                       //!< Channel 3
    rc_ctrl[0].joy_stick.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);                                       // 拨轮
    // 开关,0左1右
    rc_ctrl[0].joy_stick.s[0] = ((sbus_buf[5] >> 4) & 0x0003);      //!< Switch left
    rc_ctrl[0].joy_stick.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2; //!< Switch right

    // 鼠标解析
    rc_ctrl[0].mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);   //!< Mouse X axis
    rc_ctrl[0].mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);   //!< Mouse Y axis
    rc_ctrl[0].mouse.z = sbus_buf[10] | (sbus_buf[11] << 8); //!< Mouse Z axis
    rc_ctrl[0].mouse.press_l = sbus_buf[12];                 //!< Mouse Left Is Press ?
    rc_ctrl[0].mouse.press_r = sbus_buf[13];                 //!< Mouse Right Is Press ?

    // 按键值,每个键1bit,key_temp共16位;按键顺序在remote_control.h的宏定义中可见
    rc_ctrl[0].key_temp = sbus_buf[14] | (sbus_buf[15] << 8); //!< KeyBoard value

    // @todo 似乎可以直接用位域操作进行,把key_temp通过强制类型转换变成key类型? 方案见remote_control.md
    // 按键值解算,利用宏+循环减少代码长度
    for (uint16_t i = 0x0001, j = 0; i < 0x8001; i *= 2, j++) // 依次查看每一个键
    {
        // 如果键按下,对应键的key press状态置1,否则为0
        rc_ctrl[0].key[KEY_PRESS][j] = rc_ctrl[0].key_temp & i;
        // 如果当前按下且上一次没按下,切换按键状态.一些工作要通过按键状态而不是按键是否按下来确定(实际上是大部分)
        rc_ctrl[0].key[KEY_STATE][j] = rc_ctrl[0].key[KEY_PRESS][j] && !rc_ctrl[1].key[KEY_PRESS][j];
        // 检查是否有组合键按下
        if (rc_ctrl[0].key_temp & 0x0001u << Key_Shift) // 按下ctrl
            rc_ctrl[0].key[KEY_PRESS_WITH_SHIFT][j] = rc_ctrl[0].key_temp & i;
        if (rc_ctrl[0].key_temp & 0x0001u << Key_Ctrl) //  按下shift
            rc_ctrl[0].key[KEY_PRESS_WITH_CTRL][j] = rc_ctrl[0].key_temp & i;
    }

    // 减去偏置值
    rc_ctrl[0].joy_stick.ch[0] -= RC_CH_VALUE_OFFSET;
    rc_ctrl[0].joy_stick.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl[0].joy_stick.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl[0].joy_stick.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl[0].joy_stick.ch[4] -= RC_CH_VALUE_OFFSET;
}

/**
 * @brief protocol resolve callback
 *        this func would be called when usart3 idle interrupt happens
 *        对sbus_to_rc的简单封装
 *
 */
static void RefereeRxCallback()
{
    sbus_to_rc(rc_usart_instance->recv_buff);
}

RC_ctrl_t *RemoteControlInit(UART_HandleTypeDef *rc_usart_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = RefereeRxCallback;
    conf.usart_handle = rc_usart_handle;
    conf.recv_buff_size = REMOTE_CONTROL_FRAME_SIZE;
    rc_usart_instance = USARTRegister(&conf);
    return &rc_ctrl;
}