#include "dji_motor.h"

static uint8_t idx = 0; // register idx
/* DJI电机的实例,此处仅保存指针,内存的分配将通过电机实例初始化时通过malloc()进行 */
static dji_motor_instance *dji_motor_info[DJI_MOTOR_CNT] = {NULL};

/**
 * @brief 由于DJI电机发送以四个一组的形式进行,故对其进行特殊处理,用6个(2can*3group)can_instance专门负责发送
 *        该变量将在 DJIMotorControl() 中使用,分组在 MotorSenderGrouping()中进行
 *
 * can1: [0]:0x1FF,[1]:0x200,[2]:0x2FF
 * can2: [0]:0x1FF,[1]:0x200,[2]:0x2FF
 */
static can_instance sender_assignment[6] =
    {
        [0] = {.can_handle = &hcan1, .txconf.StdId = 0x1ff, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
        [1] = {.can_handle = &hcan1, .txconf.StdId = 0x200, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
        [2] = {.can_handle = &hcan1, .txconf.StdId = 0x2ff, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
        [3] = {.can_handle = &hcan2, .txconf.StdId = 0x1ff, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
        [4] = {.can_handle = &hcan2, .txconf.StdId = 0x200, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
        [5] = {.can_handle = &hcan2, .txconf.StdId = 0x2ff, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
};

/**
 * @brief 6个用于确认是否有电机注册到sender_assignment中的标志位,防止发送空帧,此变量将在 DJIMotorControl() 使用
 *        flag的初始化在 MotorSenderGrouping()中进行
 *
 */
static uint8_t sender_enable_flag[6] = {0};

/**
 * @brief 根据电调/拨码开关上的ID,计算发送ID和接收ID,并对电机进行分组以便处理多电机控制命令
 *
 * @param config
 */
static void MotorSenderGrouping(can_instance_config *config)
{
    uint8_t motor_id = config->tx_id - 1;
    uint8_t motor_rx_id;
    uint8_t motor_send_num;
    uint8_t motor_grouping;

    switch (dji_motor_info[idx]->motor_type)
    {
    case M2006:
    case M3508:
        if (motor_id < 4)
        {
            dji_motor_info[idx]->message_num = motor_id;
            dji_motor_info[idx]->sender_group = config->can_handle == &hcan1 ? 1 : 4;
        }
        else
        {
            dji_motor_info[idx]->message_num = motor_id - 4;
            dji_motor_info[idx]->sender_group = config->can_handle == &hcan1 ? 0 : 3;
        }
        config->rx_id = 0x200 + motor_id;
        sender_enable_flag[dji_motor_info[idx]->sender_group] = 1;
        break;

    case GM6020:
        if (motor_id < 4)
        {
            dji_motor_info[idx]->message_num = motor_id;
            dji_motor_info[idx]->sender_group = config->can_handle == &hcan1 ? 0 : 3;
        }
        else
        {
            dji_motor_info[idx]->message_num = motor_id - 4;
            dji_motor_info[idx]->sender_group = config->can_handle == &hcan1 ? 2 : 5;
        }
        config->rx_id = 0x204 + motor_id;
        sender_enable_flag[dji_motor_info[idx]->sender_group] = 1;
        break;
    // other motors should not be registered here
    default:
        break;
    }
}

/**
 * @todo 待添加此功能.
 *
 * @brief 当注册的电机id冲突时,会进入这个函数并提示冲突的ID
 *
 */
static void IDcrash_Handler()
{
    while (1)
    {
    };
}

/**
 * @brief 根据返回的can_instance对反馈报文进行解析
 *
 * @param _instance 收到数据的instance,通过遍历与所有电机进行对比
 */
static void DecodeDJIMotor(can_instance *_instance)
{
    uint8_t *rxbuff = _instance->rx_buff;
    for (size_t i = 0; i < DJI_MOTOR_CNT; i++)
    {
        if (dji_motor_info[i]->motor_can_instance == _instance)
        {
            dji_motor_info[i]->motor_measure.last_ecd = dji_motor_info[i]->motor_measure.ecd;
            dji_motor_info[i]->motor_measure.ecd = (uint16_t)(rxbuff[0] << 8 | rxbuff[1]);
            dji_motor_info[i]->motor_measure.speed_rpm = (uint16_t)(rxbuff[2] << 8 | rxbuff[3]);
            dji_motor_info[i]->motor_measure.given_current = (uint16_t)(rxbuff[4] << 8 | rxbuff[5]);
            dji_motor_info[i]->motor_measure.temperate = rxbuff[6];
            break;
        }
    }
}

dji_motor_instance *DJIMotorInit(can_instance_config config,
                                 Motor_Control_Setting_s motor_setting,
                                 Motor_Controller_Init_s controller_init,
                                 Motor_Type_e type)
{
    dji_motor_info[idx] = (dji_motor_instance *)malloc(sizeof(dji_motor_instance));
    // motor setting
    dji_motor_info[idx]->motor_type = type;
    dji_motor_info[idx]->motor_settings = motor_setting;

    // motor controller init
    // @todo : PID init
    dji_motor_info[idx]->motor_controller.other_angle_feedback_ptr = controller_init.other_angle_feedback_ptr;
    dji_motor_info[idx]->motor_controller.other_speed_feedback_ptr = controller_init.other_speed_feedback_ptr;
    // group motors, because 4 motors share the same CAN control message
    MotorSenderGrouping(&config);
    // register motor to CAN bus
    config.can_module_callback = DecodeDJIMotor;
    dji_motor_info[idx]->motor_can_instance = CANRegister(config);

    return dji_motor_info[idx++];
}

void DJIMotorSetRef(dji_motor_instance *motor, float ref)
{
    motor->motor_controller.pid_ref = ref;
}

void DJIMotorControl()
{
    static uint8_t group, num, set;
    // 遍历所有电机实例,进行串级PID的计算并设置发送报文的值
    for (size_t i = 0; i < DJI_MOTOR_CNT; i++)
    {
        if (dji_motor_info[i])
        {
            // @todo: 计算PID
            // calculate pid output
            // ...
            group = dji_motor_info[i]->sender_group;
            num = dji_motor_info[i]->message_num;
            set = (int16_t)dji_motor_info[i]->motor_controller.pid_output;
            // sender_assignment[group].rx_buff[num]= 0xff & PIDoutPIDoutput>>8;
            // sender_assignment[group].rx_buff[num]= 0xff & PIDoutput;
        }
        else
            break;
    }

    // 遍历flag,检查是否要发送这一帧报文
    for (size_t i = 0; i < 6; i++)
    {
        if (sender_enable_flag[i])
        {
            CANTransmit(&sender_assignment[i]);
        }
    }
}