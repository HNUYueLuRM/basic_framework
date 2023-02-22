#include "remote_control.h"
#include "string.h"
#include "bsp_usart.h"
#include "memory.h"
#include "stdlib.h"
#include "daemon.h"

#define REMOTE_CONTROL_FRAME_SIZE 18u // 遥控器接收的buffer大小
// 遥控器数据
static RC_ctrl_t rc_ctrl[2];     //[0]:当前数据TEMP,[1]:上一次的数据LAST.用于按键持续按下和切换的判断
static uint8_t rc_init_flag = 0; // 遥控器初始化标志位

// 遥控器拥有的串口实例,因为遥控器是单例,所以这里只有一个,就不封装了
static USARTInstance *rc_usart_instance;
static DaemonInstance *rc_daemon_instance;
/**
 * @brief 矫正遥控器摇杆的值,超过660或者小于-660的值都认为是无效值,置0
 *
 */
static void RectifyRCjoystick()
{
    for (uint8_t i = 0; i < 5; ++i)
    {
        if (abs(*(&rc_ctrl[TEMP].rc.rocker_l_ + i)) > 660)
            *(&rc_ctrl[TEMP].rc.rocker_l_ + i) = 0;
    }
}

/**
 * @brief          remote control protocol resolution
 * @param[in]      sbus_buf: raw data point
 * @param[out]     rc_ctrl: remote control data struct point
 * @retval         none
 */
static void sbus_to_rc(const uint8_t *sbus_buf)
{
    memcpy(&rc_ctrl[1], &rc_ctrl[TEMP], sizeof(RC_ctrl_t)); // 保存上一次的数据,用于按键持续按下和切换的判断
    // 摇杆,直接解算时减去偏置
    rc_ctrl[TEMP].rc.rocker_r_ = ((sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff) - RC_CH_VALUE_OFFSET;                              //!< Channel 0
    rc_ctrl[TEMP].rc.rocker_r1 = (((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff) - RC_CH_VALUE_OFFSET;                       //!< Channel 1
    rc_ctrl[TEMP].rc.rocker_l_ = (((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) | (sbus_buf[4] << 10)) & 0x07ff) - RC_CH_VALUE_OFFSET; //!< Channel 2
    rc_ctrl[TEMP].rc.rocker_l1 = (((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff) - RC_CH_VALUE_OFFSET;                       //!< Channel 3
    rc_ctrl[TEMP].rc.dial = ((sbus_buf[16] | (sbus_buf[17] << 8)) & 0x07FF) - RC_CH_VALUE_OFFSET;                                 // 左侧拨轮
    RectifyRCjoystick();
    // 开关,0左1右
    rc_ctrl[TEMP].rc.switch_right = ((sbus_buf[5] >> 4) & 0x0003);     //!< Switch right
    rc_ctrl[TEMP].rc.switch_left = ((sbus_buf[5] >> 4) & 0x000C) >> 2; //!< Switch left

    // 鼠标解析
    rc_ctrl[TEMP].mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);   //!< Mouse X axis
    rc_ctrl[TEMP].mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);   //!< Mouse Y axis
    rc_ctrl[TEMP].mouse.z = sbus_buf[10] | (sbus_buf[11] << 8); //!< Mouse Z axis
    rc_ctrl[TEMP].mouse.press_l = sbus_buf[12];                 //!< Mouse Left Is Press ?
    rc_ctrl[TEMP].mouse.press_r = sbus_buf[13];                 //!< Mouse Right Is Press ?

    // 按键值,每个键1bit,key_temp共16位;按键顺序在remote_control.h的宏定义中可见
    // 使用位域后不再需要这一中间操作
    rc_ctrl[TEMP].key_temp = sbus_buf[14] | (sbus_buf[15] << 8); //!< KeyBoard value

    // @todo 似乎可以直接用位域操作进行,把key_temp通过强制类型转换变成key类型? 位域方案在下面,尚未测试
    // 按键值解算,利用宏+循环减少代码长度
    for (uint16_t i = 0x0001, j = 0; i != 0x8000; i *= 2, j++) // 依次查看每一个键
    {
        // 如果键按下,对应键的key press状态置1,否则为0
        rc_ctrl[TEMP].key[KEY_PRESS][j] = rc_ctrl[TEMP].key_temp & i;
        // 如果当前按下且上一次没按下,切换按键状态.一些模式要通过按键状态而不是按键是否按下来确定(实际上是大部分)
        rc_ctrl[TEMP].key[KEY_STATE][j] = rc_ctrl[TEMP].key[KEY_PRESS][j] && !rc_ctrl[1].key[KEY_PRESS][j];
        // 检查是否有组合键按下
        if (rc_ctrl[TEMP].key_temp & 0x0001u << Key_Shift) // 按下ctrl
            rc_ctrl[TEMP].key[KEY_PRESS_WITH_SHIFT][j] = rc_ctrl[TEMP].key_temp & i;
        if (rc_ctrl[TEMP].key_temp & 0x0001u << Key_Ctrl) //  按下shift
            rc_ctrl[TEMP].key[KEY_PRESS_WITH_CTRL][j] = rc_ctrl[TEMP].key_temp & i;
    }

    // 位域的按键值解算,直接memcpy即可,注意小端低字节在前,即lsb在第一位,msb在最后. 尚未测试
    // *(uint16_t *)&rc_ctrl[TEMP].key_test[KEY_PRESS] = (uint16_t)(sbus_buf[14] | (sbus_buf[15] << 8));
    // *(uint16_t *)&rc_ctrl[TEMP].key_test[KEY_STATE] = *(uint16_t *)&rc_ctrl[TEMP].key_test[KEY_PRESS] & ~(*(uint16_t *)&(rc_ctrl[1].key_test[KEY_PRESS]));
    // if (rc_ctrl[TEMP].key_test[KEY_PRESS].ctrl)
    //     rc_ctrl[TEMP].key_test[KEY_PRESS_WITH_CTRL] = rc_ctrl[TEMP].key_test[KEY_PRESS];
    // if (rc_ctrl[TEMP].key_test[KEY_PRESS].shift)
    //     rc_ctrl[TEMP].key_test[Key_Shift] = rc_ctrl[TEMP].key_test[KEY_PRESS];
}

/**
 * @brief 对sbus_to_rc的简单封装,用于注册到bsp_usart的回调函数中
 *
 */
static void RemoteControlRxCallback()
{
    DaemonReload(rc_daemon_instance);         // 先喂狗
    sbus_to_rc(rc_usart_instance->recv_buff); // 进行协议解析
}

/**
 * @brief
 *
 */
static void RCLostCallback()
{
    // @todo 遥控器丢失的处理
}

RC_ctrl_t *RemoteControlInit(UART_HandleTypeDef *rc_usart_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = RemoteControlRxCallback;
    conf.usart_handle = rc_usart_handle;
    conf.recv_buff_size = REMOTE_CONTROL_FRAME_SIZE;
    rc_usart_instance = USARTRegister(&conf);

    // 进行守护进程的注册,用于定时检查遥控器是否正常工作
    // @todo 当前守护进程直接在这里注册,后续考虑将其封装到遥控器的初始化函数中,即可以让用户决定reload_count的值(是否有必要?)
    Daemon_Init_Config_s daemon_conf = {
        .reload_count = 100, // 100ms,遥控器的接收频率实际上是1000/14Hz(大约70)
        .callback = NULL,    // 后续考虑重新启动遥控器对应串口的传输
        .owner_id = NULL,    // 只有1个遥控器,不需要owner_id
    };
    rc_daemon_instance = DaemonRegister(&daemon_conf);

    rc_init_flag = 1;
    return (RC_ctrl_t *)&rc_ctrl;
}

uint8_t RemotecontrolIsOnline()
{
    if (rc_init_flag)
        return DaemonIsOnline(rc_daemon_instance);
    return 0;
    
}