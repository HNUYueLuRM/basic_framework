/**
 * @file dmmotor.c
 * @author MSC1309(msc1309@hnu.edu.cn)
 * @brief 达妙电机驱动，模仿dji_motor.c，对老代码进行优化而产出的统一，稳定的版本
 * @version 0.1
 * @date 2026-07-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "dmmotor.h"
#include "memory.h"
#include "general_def.h"
#include "user_lib.h"
#include "string.h"
#include "bsp_log.h"

static uint8_t idx_dm = 0;//用于记录注册的电机数量
static DMMotorInstance dm_motor_instance[DM_MOTOR_CNT];//提前定义好空白电机实例
static uint8_t dm_cycle_cnt = 0;//用于分频计数，控制DM任务周期

/* 两个用于将uint值和float值进行映射的函数,在设定发送值和解析反馈值时使用 */
static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (uint16_t)((x - offset) * ((float)((1 << bits) - 1)) / span);
}
static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

//用于给电机发送设置性质的指令，包含使能，失能，重设零点，清除错误
static void DMMotorSetMode(DMMotor_Mode_e cmd, DMMotorInstance *motor)
{
    memset(motor->motor_can_instance->tx_buff, 0xff, 7);  // 发送电机指令的时候前面7bytes都是0xff
    motor->motor_can_instance->tx_buff[7] = (uint8_t)cmd; // 最后一位是命令id
    CANTransmit(motor->motor_can_instance, 8);
}

//DM电机接收回调函数，用于处理接收到的数据
static void DMMotorDecode(CANInstance *motor_can)
{
    uint16_t tmp; // 用于暂存解析值,稍后转换成float数据,避免多次创建临时变量
    uint8_t *rxbuff = motor_can->rx_buff;
    uint8_t ERR;
    DMMotorInstance *motor = (DMMotorInstance *)motor_can->id;
    DM_Motor_Measure_s *measure = &(motor->measure); // 将can实例中保存的id转换成电机实例的指针

    motor->rx_refresh_dt = 0;//一旦接收到反馈重置计数

    ERR = (rxbuff[0] >> 4) & 0x0F;
    measure->state = ERR;//解析状态位

    measure->last_position = measure->position;//保存上一次的角度，用于多圈计算

    tmp = (uint16_t)((rxbuff[1] << 8) | rxbuff[2]);
    measure->position = uint_to_float(tmp, DM_P_MIN, DM_P_MAX, 16);//解析角度

    if (motor->rx_init_flag == 0){
        measure->last_position = measure->position;//防止因为第一次接受到数据时measure->last_position默认为0导致计数错误
        motor->rx_init_flag = 1;
    }

    tmp = (uint16_t)((rxbuff[3] << 4) | rxbuff[4] >> 4);
    measure->velocity = uint_to_float(tmp, DM_V_MIN, DM_V_MAX, 12);//解析速度

    tmp = (uint16_t)(((rxbuff[4] & 0x0f) << 8) | rxbuff[5]);
    measure->torque = uint_to_float(tmp, DM_T_MIN, DM_T_MAX, 12);//解析力矩

    measure->T_Mos = (float)rxbuff[6];
    measure->T_Rotor = (float)rxbuff[7];//解析电机温度

    // 多圈角度计数算法，前提是两次采样周期间隔内电机转动的角度不超过DM_P_MAX
    if (motor->measure.position - motor->measure.last_position > DM_P_MAX)
        motor->measure.total_round -= 1;
    else if (motor->measure.position - motor->measure.last_position < -DM_P_MAX)
        motor->measure.total_round += 1;
    motor->measure.total_position = motor->measure.position + motor->measure.total_round * 2 * DM_P_MAX;
}

//用于自动恢复通信（清除timeout错误），自动使能意外失能的电机
static void DMAutoRecoverComm(DMMotorInstance *motor)
{
    if (motor->measure.state == DM_STATE_DISABLE)
        DMMotorSetMode(DM_CMD_MOTOR_MODE, motor);
    if (motor->measure.state == DM_STATE_COMM_LOST)
        DMMotorSetMode(DM_CMD_CLEAR_ERROR, motor);
}

//普通模式初始化函数
DMMotorInstance *DMMotorInit(DMMotor_Init_Config_s *config)
{
    if (idx_dm >= DM_MOTOR_CNT){//注册电机数量超过最大值，报错
        LOGERROR("[dmmotor] Too many motors!");
        return NULL;
    }

    if(config->can_init_config.tx_id > 0x0F){//电机的txid大于0xF，会使电机状态位解析值不可预测，报错
        LOGERROR("[dmmotor] txID is greater than 0x0F.");
        return NULL;
    }

    DMMotorInstance *motor = &dm_motor_instance[idx_dm];
    idx_dm++;

    //将注册信息填入实例中
    motor->motor_settings = config->controller_setting_init_config;
    PIDInit(&motor->speed_PID, &config->controller_param_init_config.speed_PID);
    PIDInit(&motor->angle_PID, &config->controller_param_init_config.angle_PID);
    motor->other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    motor->other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;
    motor->speed_feedforward_ptr = config->controller_param_init_config.speed_feedforward_ptr;
    motor->force_feedforward_ptr = config->controller_param_init_config.force_feedforward_ptr;

    //为电机实例注册CAN总线
    config->can_init_config.can_module_callback = DMMotorDecode;
    config->can_init_config.id = motor;
    motor->motor_can_instance = CANRegister(&config->can_init_config);

    //模式为普通模式
    motor->working_type = NORMAL_MODE;

    //开启并使能电机
    DMMotorEnable(motor);
    DMMotorSetMode(DM_CMD_MOTOR_MODE, motor);
    
    return motor;
}

//MIT模式初始化函数
DMMotorInstance *MITMotorInit(MIT_Init_Config_s *config)
{
    if (idx_dm >= DM_MOTOR_CNT) {//注册电机数量超过最大值，报错
        LOGERROR("[dmmotor] Too many motors!");
        return NULL;
    }

    if(config->can_init_config.tx_id > 0x0F){//电机的txid大于0xF，会使电机状态位解析值不可预测，报错
        LOGERROR("[dmmotor] txID is greater than 0x0F.");
        return NULL;
    }

    DMMotorInstance *motor = &dm_motor_instance[idx_dm];
    idx_dm++;

    //将注册信息填入实例中
    motor->motor_settings.motor_reverse_flag = config->motor_reverse_flag;

    motor->MIT_Kp = config->controller_param_init_config.Kp;
    motor->MIT_Kd = config->controller_param_init_config.Kd;

    //为电机实例注册CAN总线
    config->can_init_config.can_module_callback = DMMotorDecode;
    config->can_init_config.id = motor;
    motor->motor_can_instance = CANRegister(&config->can_init_config);

    //模式为MIT模式
    motor->working_type = MIT_MODE;

    //开启并使能电机
    DMMotorEnable(motor);
    DMMotorSetMode(DM_CMD_MOTOR_MODE, motor);
    
    return motor;
}

/// @brief 两种模式下均可使用的通用函数
//开启电机（注意不是使能）
void DMMotorEnable(DMMotorInstance *motor)
{
    motor->stop_flag = MOTOR_ENABLED;
}

//关闭电机，使电机的输出为0，清空PID（注意不是失能）
void DMMotorStop(DMMotorInstance *motor) 
{
    motor->stop_flag = MOTOR_STOP;
}

//用于重设零点
void DMMotorCaliEncoder(DMMotorInstance *motor)
{
    DMMotorSetMode(DM_CMD_ZERO_POSITION, motor);
    DWT_Delay(0.01);//重设零点会使电机短时间内无法使用，需要延时一段时间
}

/// @brief 普通模式下设置各种参数
//切换电机最外层闭环
void DMMotorOuterLoop(DMMotorInstance *motor, Closeloop_Type_e type)
{
    motor->motor_settings.outer_loop_type = type;
}

//切换电机反馈模式
void DMMotorChangeFeed(DMMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type)
{
    if (loop == ANGLE_LOOP)
        motor->motor_settings.angle_feedback_source = type;
    else if (loop == SPEED_LOOP)
        motor->motor_settings.speed_feedback_source = type;
}

//设置最外环参考值，内环参考值请通过前馈指针设置
void DMMotorSetRef(DMMotorInstance *motor, float ref)
{
    motor->pid_ref = ref;
}

/// @brief MIT模式下设置各种参数
void DMMotorSetMITAngle(DMMotorInstance *motor, float angle)
{
    motor->MIT_ref_angle = angle;
}

void DMMotorSetMITSpeed(DMMotorInstance *motor, float speed)
{
    motor->MIT_ref_speed = speed;
}

void DMMotorSetMITTorque(DMMotorInstance *motor, float torque)
{
    motor->MIT_ref_torque = torque;
}

//DM任务函数，以一定频率执行（取决于DM_TASK_CYCLE），用于PID计算，发送控制帧，以及维护通信
void DMMotorTask()
{
    //用于控制函数执行周期
    if (idx_dm == 0)
        return;
    dm_cycle_cnt++;
    if (dm_cycle_cnt != DM_TASK_CYCLE)
        return;
    else
        dm_cycle_cnt = 0;

    DMMotorInstance *motor;
    DM_Motor_Measure_s *measure;
    DMMotor_Control_Setting_s *motor_setting;
    DMMotor_Send_s motor_send_mailbox;

    for(int i = 0;i < idx_dm; i++){//遍历所有电机实例，分别执行任务

        motor = &dm_motor_instance[i];
        motor_setting = &motor->motor_settings;
        measure = &motor->measure;

        motor->rx_refresh_dt++;//每遍历到这个电机一次，计数就自增一次

        DMAutoRecoverComm(motor);//自动恢复通信，将通信丢失和意外失能的电机重新拉回到使能状态

        if (motor->working_type == MIT_MODE)//MIT模式需要发送的控制帧
        {
            //将参数从浮点数转换为uint形式
            motor_send_mailbox.position_des = float_to_uint(motor->MIT_ref_angle, DM_P_MIN, DM_P_MAX, 16);
            motor_send_mailbox.velocity_des = float_to_uint(motor->MIT_ref_speed, DM_V_MIN, DM_V_MAX, 12);
            motor_send_mailbox.torque_des   = float_to_uint(motor->MIT_ref_torque, DM_T_MIN, DM_T_MAX, 12);
            motor_send_mailbox.Kp           = float_to_uint(motor->MIT_Kp, KP_MIN, KP_MAX, 12);
            motor_send_mailbox.Kd           = float_to_uint(motor->MIT_Kd, KD_MIN, KD_MAX, 12);

            //有反向标志位的情况
            if (motor_setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE){
                motor_send_mailbox.position_des = float_to_uint(- motor->MIT_ref_angle, DM_P_MIN, DM_P_MAX, 16);
                motor_send_mailbox.velocity_des = float_to_uint(- motor->MIT_ref_speed, DM_V_MIN, DM_V_MAX, 12);
                motor_send_mailbox.torque_des   = float_to_uint(- motor->MIT_ref_torque, DM_T_MIN, DM_T_MAX, 12);
            }

            //如果电机被关闭，则清空其PID以及力矩输出
            if (motor->stop_flag == MOTOR_STOP)
            {
                motor_send_mailbox.torque_des = float_to_uint(0, DM_T_MIN, DM_T_MAX, 12);
                motor_send_mailbox.Kp         = float_to_uint(0, KP_MIN, KP_MAX, 12);
                motor_send_mailbox.Kd         = float_to_uint(0, KD_MIN, KD_MAX, 12);
            }

            //将参数按通信协议中的格式填入到tx_buff中
            motor->motor_can_instance->tx_buff[0] = (uint8_t)(motor_send_mailbox.position_des >> 8);
            motor->motor_can_instance->tx_buff[1] = (uint8_t)(motor_send_mailbox.position_des);
            motor->motor_can_instance->tx_buff[2] = (uint8_t)(motor_send_mailbox.velocity_des >> 4);
            motor->motor_can_instance->tx_buff[3] = (uint8_t)(((motor_send_mailbox.velocity_des & 0xF) << 4) | (motor_send_mailbox.Kp >> 8));
            motor->motor_can_instance->tx_buff[4] = (uint8_t)(motor_send_mailbox.Kp);
            motor->motor_can_instance->tx_buff[5] = (uint8_t)(motor_send_mailbox.Kd >> 4);
            motor->motor_can_instance->tx_buff[6] = (uint8_t)(((motor_send_mailbox.Kd & 0xF) << 4) | (motor_send_mailbox.torque_des >> 8));
            motor->motor_can_instance->tx_buff[7] = (uint8_t)(motor_send_mailbox.torque_des);

            //发送控制帧
            CANTransmit(motor->motor_can_instance, 8);
        }
        else if (motor->working_type == NORMAL_MODE)//普通模式需要发送的控制帧
        {
            float pid_ref, pid_measure, set;

            pid_ref = motor->pid_ref;

            if (motor_setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)//如果设置反向，直接将参考值反转符号
                pid_ref *= -1;

            if ((motor_setting->close_loop_type & ANGLE_LOOP) && motor_setting->outer_loop_type == ANGLE_LOOP)//如果启用了角度环且最外层闭环为角度环，则计算角度环
            {
                if (motor_setting->angle_feedback_source == OTHER_FEED)//判断是否使用电机反馈
                    pid_measure = *motor->other_angle_feedback_ptr;
                else
                    pid_measure = measure->total_position;
                
                pid_ref = PIDCalculate(&motor->angle_PID, pid_measure, pid_ref); //对总角度进行闭环，而不是单圈角度
            }

            if ((motor_setting->close_loop_type & SPEED_LOOP) && (motor_setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP)))//如果启用了速度环且最外层闭环为角度环或速度环，则计算速度环
            {
                if (motor_setting->feedforward_flag & SPEED_FEEDFORWARD)
                    pid_ref += *motor->speed_feedforward_ptr; //速度前馈

                if (motor_setting->speed_feedback_source == OTHER_FEED)//判断是否使用电机反馈
                    pid_measure = *motor->other_speed_feedback_ptr;
                else
                    pid_measure = measure->velocity;
                
                pid_ref = PIDCalculate(&motor->speed_PID, pid_measure, pid_ref);//对速度环闭环
            }

            
            if (motor_setting->feedforward_flag & FORCE_FEEDFORWARD)
                pid_ref += *motor->force_feedforward_ptr;//力矩前馈

            set = pid_ref;//得到最终的控制值

            LIMIT_MIN_MAX(set, DM_T_MIN, DM_T_MAX);//进行限幅防止溢出

            //只对力矩（控制量）进行设置，其他均为0
            motor_send_mailbox.position_des = float_to_uint(0, DM_P_MIN, DM_P_MAX, 16);
            motor_send_mailbox.velocity_des = float_to_uint(0, DM_V_MIN, DM_V_MAX, 12);
            motor_send_mailbox.torque_des   = float_to_uint(set, DM_T_MIN, DM_T_MAX, 12);
            motor_send_mailbox.Kp           = float_to_uint(0, KP_MIN, KP_MAX, 12);
            motor_send_mailbox.Kd           = float_to_uint(0, KD_MIN, KD_MAX, 12);

            //如果电机被关闭，则清空其PID以及力矩输出
            if (motor->stop_flag == MOTOR_STOP){
                motor_send_mailbox.torque_des = float_to_uint(0, DM_T_MIN, DM_T_MAX, 12);
                PIDClear(&motor->angle_PID);
                PIDClear(&motor->speed_PID);
            }

            //将参数按通信协议中的格式填入到tx_buff中
            motor->motor_can_instance->tx_buff[0] = (uint8_t)(motor_send_mailbox.position_des >> 8);
            motor->motor_can_instance->tx_buff[1] = (uint8_t)(motor_send_mailbox.position_des);
            motor->motor_can_instance->tx_buff[2] = (uint8_t)(motor_send_mailbox.velocity_des >> 4);
            motor->motor_can_instance->tx_buff[3] = (uint8_t)(((motor_send_mailbox.velocity_des & 0xF) << 4) | (motor_send_mailbox.Kp >> 8));
            motor->motor_can_instance->tx_buff[4] = (uint8_t)(motor_send_mailbox.Kp);
            motor->motor_can_instance->tx_buff[5] = (uint8_t)(motor_send_mailbox.Kd >> 4);
            motor->motor_can_instance->tx_buff[6] = (uint8_t)(((motor_send_mailbox.Kd & 0xF) << 4) | (motor_send_mailbox.torque_des >> 8));
            motor->motor_can_instance->tx_buff[7] = (uint8_t)(motor_send_mailbox.torque_des);

            //发送控制帧
            CANTransmit(motor->motor_can_instance, 8);
        }
    }
}
