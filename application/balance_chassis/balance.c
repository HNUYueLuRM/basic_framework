// app
#include "balance.h"
#include "gain_table.h"
#include "robot_def.h"
#include "general_def.h"
// module
#include "HT04.h"
#include "LK9025.h"
#include "bmi088.h"
#include "referee.h"
#include "super_cap.h"
#include "controller.h"
#include "can_comm.h"
#include "user_lib.h"
// standard
#include "stdint.h"
#include "arm_math.h" // 需要用到较多三角函数

#include "bsp_dwt.h"
static uint32_t balance_dwt_cnt;
static float balance_dt;

/* 底盘拥有的模块实例 */
static attitude_t *imu_data;
// static BMI088Instance *imu;
static SuperCapInstance *super_cap;
static referee_info_t *referee_data; // 裁判系统的数据会被不断更新
// 电机
static HTMotorInstance *lf;
static HTMotorInstance *rf;
static HTMotorInstance *lb;
static HTMotorInstance *rb;
static LKMotorInstance *l_driven;
static LKMotorInstance *r_driven;
// 底盘板和云台板通信
static CANCommInstance *chassis_comm; // 由于采用多板架构,即使使用C板也有空余串口,可以使用串口通信以获得更高的通信速率并降低阻塞
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Chassis_Upload_Data_s chassis_feed_send;
// 两个腿的参数,0为左腿,1为右腿
static LinkNPodParam left_side, right_side;
static PIDInstance swerving_pid;        // 转向PID,有转向指令时使用IMU的加速度反馈积分以获取速度和位置状态量
static PIDInstance anti_crash_pid;      // 抗劈叉,将输出以相反的方向叠加到左右腿的上
static PIDInstance leg_length_pid;      // 用PD模拟弹簧的传递函数,不需要积分项(弹簧是一个无积分的二阶系统),增益不可过大否则抗外界冲击响应时太"硬"
static PIDInstance roll_compensate_pid; // roll轴补偿,用于保持机体水平

/* ↓↓↓分割出这些函数是为了提高可读性,使得阅读者的逻辑更加顺畅;但实际上这些函数都不长,可以以注释的形式直接加到BalanceTask里↓↓↓*/

/**
 * @brief 将电机和imu的数据组装为LinkNPodParam结构体
 *
 */
static void ParamAssemble()
{
}

/**
 * @brief 根据关节角度和角速度,计算单杆长度和角度以及变化率
 *
 * @param p 5连杆和腿的参数
 */
static void Link2Pod(LinkNPodParam *p)
{ // 拟将功能封装到vmc_project.h中
    float xD, yD, xB, yB, BD, A0, B0, xC, yC, phi2;
    xD = JOINT_DISTANCE + THIGH_LEN * arm_cos_f32(p->phi4);
    yD = THIGH_LEN * arm_sin_f32(p->phi4);
    xB = 0 + THIGH_LEN * arm_cos_f32(p->phi1);
    yB = THIGH_LEN * arm_sin_f32(p->phi1);
    BD = powf(xD - xB, 2) + powf(yD - yB, 2);
    A0 = 2 * CALF_LEN * (xD - xB);
    B0 = 2 * CALF_LEN * (yD - yB);
    p->phi2 = 2 * atan2f(B0 + Sqrt(powf(A0, 2) + powf(B0, 2) - powf(BD, 2)), A0 + BD);
    xC = xB + CALF_LEN * arm_cos_f32(p->phi2);
    yC = yB + CALF_LEN * arm_sin_f32(p->phi2);
#ifdef ANGLE_DIFF_VMC
    float p5t = atan2f(yC, xC - JOINT_DISTANCE / 2); // 避免重复计算
    float plt = Sqrt(powf(xC - JOINT_DISTANCE / 2, 2) + powf(yC, 2));
    p->phi5_w = (p5t - p->phi5) / balance_dt;
    p->pod_len_w = (plt - p->pod_len) / balance_dt;
    p->phi5 = p5t;
    p->pod_len = plt;
#endif
    p->phi5 = atan2f(yC, xC - JOINT_DISTANCE / 2);
    p->pod_len = Sqrt(powf(xC - JOINT_DISTANCE / 2, 2) + powf(yC, 2));
    p->phi3 = atan2(yC - yD, xC - xD); // 稍后用于计算VMC

#ifdef VELOCITY_DIFF_VMC
    float phi1_pred = p->phi1 + p->phi1_w * balance_dt; // 预测下一时刻的关节角度(利用关节角速度)
    float phi4_pred = p->phi4 + p->phi4_w * balance_dt;
    // 重新计算腿长和腿角度
    xD = JOINT_DISTANCE + THIGH_LEN * arm_cos_f32(phi4_pred);
    yD = THIGH_LEN * arm_sin_f32(phi4_pred);
    xB = 0 + THIGH_LEN * arm_cos_f32(phi1_pred);
    yB = THIGH_LEN * arm_sin_f32(phi1_pred);
    BD = powf(xD - xB, 2) + powf(yD - yB, 2);
    A0 = 2 * CALF_LEN * (xD - xB);
    B0 = 2 * CALF_LEN * (yD - yB);
    phi2 = 2 * atan2f(B0 + Sqrt(powf(A0, 2) + powf(B0, 2) - powf(BD, 2)), A0 + BD); // 不要用link->phi2,因为这里是预测的
    xC = xB + CALF_LEN * arm_cos_f32(phi2);
    yC = yB + CALF_LEN * arm_sin_f32(phi2);
    // 差分计算腿长变化率和腿角速度
    p->pod_w = (atan2f(yC, xC - JOINT_DISTANCE / 2) - p->phi5) / balance_dt;
    p->pod_v = (Sqrt(powf(xC - JOINT_DISTANCE / 2, 2) + powf(yC, 2)) - p->pod_len) / balance_dt;
#endif // VELOCITY_DIFF_VMC
}

/**
 * @brief 根据状态反馈计算当前腿长,查表获得LQR的反馈增益,并列式计算LQR的输出
 *        由于反馈矩阵和控制矩阵都比较稀疏,故不使用矩阵库,避免非零项计算
 * @note 计算得到的腿部力矩输出还要再综合运动控制系统补偿后映射为两个关节电机
 *       而轮子的输出则只经过转向PID的反馈增益计算
 *
 */
static void CalcLQR(LinkNPodParam *p)
{
}

/**
 * @brief 腿部控制:抗劈叉; 轮子控制:转向
 * @todo 确定方向
 */
static void SynthesizeMotion()
{
    PIDCalculate(&anti_crash_pid, left_side.phi5 - right_side.phi5, 0);
    left_side.T_pod += anti_crash_pid.Output;
    right_side.T_pod -= anti_crash_pid.Output;

    // PIDCalculate(&swerving_pid, imu_data->Yaw, 0); // 对速度闭环还是使用角度增量闭环?
    left_side.T_wheel -= swerving_pid.Output;
    right_side.T_wheel += swerving_pid.Output;
}

/**
 * @brief 腿部控制:长度.用PD模拟弹簧的传递函数
 *
 */
static void LegControl(LinkNPodParam *p, float target_length)
{
    p->F_pod += PIDCalculate(&leg_length_pid, p->pod_len, target_length);
}

/**
 * @brief roll轴补偿(保持机体水平)
 *
 */
static void RollCompensate()
{
    PIDCalculate(&roll_compensate_pid, imu_data->Roll, 0);
    left_side.F_pod += roll_compensate_pid.Output;
    right_side.F_pod -= roll_compensate_pid.Output;
}

/**
 * @brief 将前面计算的T和F映射为关节电机输出
 *
 */
static void VMCProject(LinkNPodParam *p)
{
    float s23 = arm_sin_f32(p->phi2 - p->phi3);
    float phi25 = p->phi2 - p->phi5;
    float phi35 = p->phi3 - p->phi5;
    float F_m_L = p->F_pod * p->pod_len;
    p->T_back = -(THIGH_LEN * arm_sin_f32(p->phi1 - p->phi2) * (p->T_pod * arm_cos_f32(phi35) - F_m_L * arm_sin_f32(phi35))) / (p->pod_len * s23);
    p->T_front = -(THIGH_LEN * arm_sin_f32(p->phi3 - p->phi4) * (p->T_pod * arm_cos_f32(phi25) - F_m_L * arm_sin_f32(phi25))) / (p->pod_len * s23);
}

/**
 * @brief 离地监测和?跳跃控制?
 *        通过模型解算地面的支持力完成离地检测
 *
 */
static uint8_t air_flag;
static void FlyDetect()
{
}

/**
 * @brief 功率限制,一般不需要
 *
 */
static void WattLimit(LinkNPodParam *p)
{
}

void BalanceInit()
{ // IMU初始化
    BMI088_Init_Config_s imu_config = {
        .spi_acc_config = {
            .GPIOx = GPIOC,
            .cs_pin = GPIO_PIN_4,
            .spi_handle = &hspi1,
        },
        .spi_gyro_config = {
            .GPIOx = GPIOC,
            .cs_pin = GPIO_PIN_4,
            .spi_handle = &hspi1,
        },
        .heat_pid_config = {
            .Kp = 0.0f,
            .Kd = 0.0f,
            .Ki = 0.0f,
            .MaxOut = 0.0f,
            .DeadBand = 0.0f,
        },
        .heat_pwm_config = {
            .channel = TIM_CHANNEL_1,
            .htim = &htim1,
        },
        .cali_mode = BMI088_CALIBRATE_ONLINE_MODE,
        .work_mode = BMI088_BLOCK_PERIODIC_MODE,
    };
    // imu = BMI088Register(&imu_config);
    imu_data = INS_Init();

    SuperCap_Init_Config_s cap_conf = {
        // 超级电容初始化
        .can_config.can_handle = &hcan1,
        .can_config.rx_id = 0x311,
        .can_config.tx_id = 0x312,
    };
    super_cap = SuperCapInit(&cap_conf);

    // ↓↓↓---------------关节电机初始化----------------↓↓↓

    Motor_Init_Config_s joint_conf = {
        // 写一个,剩下的修改方向和id即可

    };
    lf = HTMotorInit(&joint_conf);

    rf = HTMotorInit(&joint_conf);

    lb = HTMotorInit(&joint_conf);

    rb = HTMotorInit(&joint_conf);

    // ↓↓↓---------------驱动电机初始化----------------↓↓↓
    Motor_Init_Config_s driven_conf = {
        // 写一个,剩下的修改方向和id即可
        .can_init_config.can_handle = &hcan1,
        .controller_param_init_config = {
            .current_PID = {
                .Kp = 1,
                .Ki = 0,
                .Kd = 0,
                .MaxOut = 500,
            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .outer_loop_type = CURRENT_LOOP,
            .close_loop_type = CURRENT_LOOP,
        },
        .motor_type = LK9025,
    };
    driven_conf.can_init_config.tx_id = 1;
    l_driven = LKMotorInit(&driven_conf);

    driven_conf.can_init_config.tx_id = 2;
    r_driven = LKMotorInit(&driven_conf);

    CANComm_Init_Config_s chassis_comm_conf = {
        // 底盘板和云台板通信
        .can_config = {
            .can_handle = &hcan1,
            .rx_id = 0x201,
            .tx_id = 0x200,
        },
        .send_data_len = sizeof(Chassis_Upload_Data_s),
        .recv_data_len = sizeof(Chassis_Ctrl_Cmd_s),
    };
    chassis_comm = CANCommInit(&chassis_comm_conf);

    // referee_data = RefereeInit(&huart6); // 裁判系统串口

    // ↓↓↓---------------综合运动控制----------------↓↓↓
    PID_Init_Config_s swerving_pid_conf = {
        .Kp = 0.0f,
        .Kd = 0.0f,
        .Ki = 0.0f,
        .MaxOut = 0.0f,
        .DeadBand = 0.0f,
        .Improve = PID_IMPROVE_NONE,
    };
    PIDInit(&swerving_pid, &swerving_pid_conf);

    PID_Init_Config_s anti_crash_pid_conf = {
        .Kp = 0.0f,
        .Kd = 0.0f,
        .Ki = 0.0f,
        .MaxOut = 0.0f,
        .DeadBand = 0.0f,
        .Improve = PID_IMPROVE_NONE,
    };
    PIDInit(&swerving_pid, &swerving_pid_conf);

    PID_Init_Config_s leg_length_pid_conf = {
        .Kp = 0.0f,
        .Kd = 0.0f,
        .Ki = 0.0f,
        .MaxOut = 0.0f,
        .DeadBand = 0.0f,
        .Improve = PID_IMPROVE_NONE,
    };
    PIDInit(&leg_length_pid, &leg_length_pid_conf);

    PID_Init_Config_s roll_compensate_pid_conf = {
        .Kp = 0.0f,
        .Kd = 0.0f,
        .Ki = 0.0f,
        .MaxOut = 0.0f,
        .DeadBand = 0.0f,
        .Improve = PID_IMPROVE_NONE,
    };
    PIDInit(&roll_compensate_pid, &roll_compensate_pid_conf);
}

/* balanceTask可能需要以更高频率运行,以提高线性化的精确程度 */
void BalanceTask()
{
    chassis_cmd_recv = *(Chassis_Ctrl_Cmd_s *)CANCommGet(chassis_comm);

    ParamAssemble(); // 参数组装,将电机和IMU的参数组装到一起
    // 将五连杆映射成单杆
    Link2Pod(&left_side);
    Link2Pod(&right_side);
    // 根据单杆计算处的角度和杆长,计算反馈增益
    CalcLQR(&left_side);
    CalcLQR(&right_side);
    // 腿长控制
    LegControl(&left_side, 0);
    LegControl(&right_side, 0);

    SynthesizeMotion(); // 综合运动控制,转向+抗劈叉

    RollCompensate(); // 俯仰角补偿,保持机体水平
    // VMC映射成关节输出
    VMCProject(&left_side);
    VMCProject(&right_side);

    FlyDetect(); // 滞空检测
    // 电机输出限幅
    WattLimit(&left_side);
    WattLimit(&right_side);

    // code to go here... 裁判系统,UI,多机通信
    
    CANCommSend(chassis_comm, (uint8_t*)&chassis_feed_send);
}
