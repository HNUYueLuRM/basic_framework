#include "dji_motor.h"
#include "general_def.h"

static uint8_t idx = 0; // register idx,是该文件的全局电机索引,在注册时使用

/* DJI电机的实例,此处仅保存指针,内存的分配将通过电机实例初始化时通过malloc()进行 */
static DJIMotorInstance *dji_motor_info[DJI_MOTOR_CNT] = {NULL};

/**
 * @brief 由于DJI电机发送以四个一组的形式进行,故对其进行特殊处理,用6个(2can*3group)can_instance专门负责发送
 *        该变量将在 DJIMotorControl() 中使用,分组在 MotorSenderGrouping()中进行
 *
 * C610(m2006)/C620(m3508):0x1ff,0x200; GM6020:0x1ff,0x2ff
 * 反馈: GM6020: 0x204+id ; C610/C620: 0x200+id
 * can1: [0]:0x1FF,[1]:0x200,[2]:0x2FF
 * can2: [3]:0x1FF,[4]:0x200,[5]:0x2FF
 */
static CANInstance sender_assignment[6] =
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
 * @brief 当注册的电机id冲突时,会进入这个函数并提示冲突的ID
 * @todo 通过segger jlink 发送日志
 */
static void IDcrash_Handler(uint8_t conflict_motor_idx, uint8_t temp_motor_idx)
{
    while (1)
    {
    };
}

/**
 * @brief 根据电调/拨码开关上的ID,计算发送ID和接收ID,并对电机进行分组以便处理多电机控制命令
 *
 * @param config
 */
static void MotorSenderGrouping(CAN_Init_Config_s *config)
{
    uint8_t motor_id = config->tx_id - 1; // 下标从零开始,先减一方便赋值
    uint8_t motor_send_num;
    uint8_t motor_grouping;

    switch (dji_motor_info[idx]->motor_type)
    {
    case M2006:
    case M3508:
        if (motor_id < 4) // 根据ID分组
        {
            motor_send_num = motor_id;
            motor_grouping = config->can_handle == &hcan1 ? 1 : 4;
        }
        else
        {
            motor_send_num = motor_id - 4;
            motor_grouping = config->can_handle == &hcan1 ? 0 : 3;
        }

        // 计算接收id并设置分组发送id
        config->rx_id = 0x200 + motor_id + 1;
        sender_enable_flag[motor_grouping] = 1;
        dji_motor_info[idx]->message_num = motor_send_num;
        dji_motor_info[idx]->sender_group = motor_grouping;

        // 检查是否发生id冲突
        for (size_t i = 0; i < idx; i++)
        {
            if (dji_motor_info[i]->motor_can_instance->can_handle == config->can_handle && dji_motor_info[i]->motor_can_instance->rx_id == config->rx_id)
                IDcrash_Handler(i, idx);
        }
        break;

    case GM6020:
        if (motor_id < 4)
        {
            motor_send_num = motor_id;
            motor_grouping = config->can_handle == &hcan1 ? 0 : 3;
        }
        else
        {
            motor_send_num = motor_id - 4;
            motor_grouping = config->can_handle == &hcan1 ? 2 : 5;
        }

        config->rx_id = 0x204 + motor_id + 1;
        sender_enable_flag[motor_grouping] = 1;
        dji_motor_info[idx]->message_num = motor_send_num;
        dji_motor_info[idx]->sender_group = motor_grouping;

        for (size_t i = 0; i < idx; i++)
        {
            if (dji_motor_info[i]->motor_can_instance->can_handle == config->can_handle && dji_motor_info[i]->motor_can_instance->rx_id == config->rx_id)
                IDcrash_Handler(i, idx);
        }
        break;

    default: // other motors should not be registered here
        break;
    }
}

/**
 * @todo  是否可以简化多圈角度的计算？
 * @brief 根据返回的can_instance对反馈报文进行解析
 *
 * @param _instance 收到数据的instance,通过遍历与所有电机进行对比以选择正确的实例
 */
static void DecodeDJIMotor(CANInstance *_instance)
{
    // 由于需要多次变址访存,直接将buff和measure地址保存在寄存器里避免多次存取
    static uint8_t *rxbuff;
    static DJI_Motor_Measure_s *measure;

    for (size_t i = 0; i < idx; i++)
    {
        if (dji_motor_info[i]->motor_can_instance == _instance)
        {
            rxbuff = _instance->rx_buff;
            measure = &dji_motor_info[i]->motor_measure; // measure要多次使用,保存指针减小访存开销

            // resolve data and apply filter to current and speed
            measure->last_ecd = measure->ecd;
            measure->ecd = (uint16_t)(rxbuff[0] << 8 | rxbuff[1]);
            measure->angle_single_round = ECD_ANGLE_COEF * measure->ecd;
            measure->speed_angle_per_sec = (1 - SPEED_SMOOTH_COEF) * measure->speed_angle_per_sec +
                                           RPM_2_ANGLE_PER_SEC * SPEED_SMOOTH_COEF * (int16_t)(rxbuff[2] << 8 | rxbuff[3]);
            measure->given_current = (1 - CURRENT_SMOOTH_COEF) * measure->given_current +
                                     RPM_2_ANGLE_PER_SEC * CURRENT_SMOOTH_COEF * (uint16_t)(rxbuff[4] << 8 | rxbuff[5]);
            measure->temperate = rxbuff[6];

            // multi rounds calc,计算的前提是两次采样间电机转过的角度小于180°
            if (measure->ecd - measure->last_ecd > 4096)
                measure->total_round--;
            else if (measure->ecd - measure->last_ecd < -4096)
                measure->total_round++;
            measure->total_angle = measure->total_round * 360 + measure->angle_single_round;
            break;
        }
    }
}

// 电机初始化,返回一个电机实例
DJIMotorInstance *DJIMotorInit(Motor_Init_Config_s *config)
{
    dji_motor_info[idx] = (DJIMotorInstance *)malloc(sizeof(DJIMotorInstance));
    memset(dji_motor_info[idx], 0, sizeof(DJIMotorInstance));

    // motor basic setting
    dji_motor_info[idx]->motor_type = config->motor_type;
    dji_motor_info[idx]->motor_settings = config->controller_setting_init_config;

    // motor controller init
    PID_Init(&dji_motor_info[idx]->motor_controller.current_PID, &config->controller_param_init_config.current_PID);
    PID_Init(&dji_motor_info[idx]->motor_controller.speed_PID, &config->controller_param_init_config.speed_PID);
    PID_Init(&dji_motor_info[idx]->motor_controller.angle_PID, &config->controller_param_init_config.angle_PID);
    dji_motor_info[idx]->motor_controller.other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    dji_motor_info[idx]->motor_controller.other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;

    // group motors, because 4 motors share the same CAN control message
    MotorSenderGrouping(&config->can_init_config);

    // register motor to CAN bus
    config->can_init_config.can_module_callback = DecodeDJIMotor; // set callback
    dji_motor_info[idx]->motor_can_instance = CANRegister(&config->can_init_config);

    return dji_motor_info[idx++];
}

void DJIMotorChangeFeed(DJIMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type)
{
    if (loop == ANGLE_LOOP)
    {
        motor->motor_settings.angle_feedback_source = type;
    }
    if (loop == SPEED_LOOP)
    {
        motor->motor_settings.speed_feedback_source = type;
    }
}

void DJIMotorStop(DJIMotorInstance *motor)
{
    motor->stop_flag = MOTOR_STOP;
}

void DJIMotorEnable(DJIMotorInstance *motor)
{
    motor->stop_flag = MOTOR_ENALBED;
}

void DJIMotorOuterLoop(DJIMotorInstance *motor, Closeloop_Type_e outer_loop)
{
    motor->motor_settings.outer_loop_type = outer_loop;
}

// 设置参考值
void DJIMotorSetRef(DJIMotorInstance *motor, float ref)
{
    motor->motor_controller.pid_ref = ref;
}

// 为所有电机实例计算三环PID,发送控制报文
void DJIMotorControl()
{
    // 预先通过静态变量定义避免反复释放分配栈空间,直接保存一次指针引用从而减小访存的开销
    // 同样可以提高可读性
    static uint8_t group, num;
    static int16_t set;
    static DJIMotorInstance *motor;
    static Motor_Control_Setting_s *motor_setting;
    static Motor_Controller_s *motor_controller;
    static DJI_Motor_Measure_s *motor_measure;
    static float pid_measure,pid_ref;
    // 遍历所有电机实例,进行串级PID的计算并设置发送报文的值
    for (size_t i = 0; i < idx; i++)
    {
        if (dji_motor_info[i])
        {
            motor = dji_motor_info[i];
            motor_setting = &motor->motor_settings;
            motor_controller = &motor->motor_controller;
            motor_measure = &motor->motor_measure;
            pid_ref=motor_controller->pid_ref; //保存设定值,防止motor_controller->pid_ref在计算过程中被修改

            // pid_ref会顺次通过被启用的闭环充当数据的载体
            // 计算位置环,只有启用位置环且外层闭环为位置时会计算速度环输出
            if ((motor_setting->close_loop_type & ANGLE_LOOP) && motor_setting->outer_loop_type == ANGLE_LOOP)
            {
                if (motor_setting->angle_feedback_source == OTHER_FEED)
                    pid_measure = *motor_controller->other_angle_feedback_ptr;
                else                                          // MOTOR_FEED
                    pid_measure = motor_measure->total_angle; // 对total angle闭环,防止在边界处出现突跃
                // 更新pid_ref进入下一个环
                pid_ref = PID_Calculate(&motor_controller->angle_PID, pid_measure, pid_ref);
            }

            // 计算速度环,(外层闭环为速度或位置)且(启用速度环)时会计算速度环
            if ((motor_setting->close_loop_type & SPEED_LOOP) && (motor_setting->outer_loop_type | (ANGLE_LOOP | SPEED_LOOP)))
            {
                if (motor_setting->speed_feedback_source == OTHER_FEED)
                    pid_measure = *motor_controller->other_speed_feedback_ptr;
                else // MOTOR_FEED
                    pid_measure = motor_measure->speed_angle_per_sec;
                // 更新pid_ref进入下一个环
                pid_ref = PID_Calculate(&motor_controller->speed_PID, pid_measure, pid_ref);
            }

            // 计算电流环,只要启用了电流环就计算,不管外层闭环是什么,并且电流只有电机自身传感器的反馈
            if (motor_setting->close_loop_type & CURRENT_LOOP)
            {
                pid_ref = PID_Calculate(&motor_controller->current_PID, motor_measure->given_current, pid_ref);
            }

            // 获取最终输出
            set = (int16_t)pid_ref;
            if (motor_setting->reverse_flag == MOTOR_DIRECTION_REVERSE)
                set *= -1; // 设置反转

            // 分组填入发送数据
            group = motor->sender_group;
            num = motor->message_num;
            sender_assignment[group].tx_buff[2 * num] = 0xff & set >> 8;
            sender_assignment[group].tx_buff[2 * num + 1] = 0xff & set;

            // 电机是否停止运行
            // if (motor->stop_flag == MOTOR_STOP)
            // { // 若该电机处于停止状态,直接将buff置零
            //     memset(sender_assignment[group].tx_buff + 2 * num, 0, 16u);
            // }
        }
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
