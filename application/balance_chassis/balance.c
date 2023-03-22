// app
#include "balance.h"
#include "vmc_project.h"
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
// standard
#include "stdint.h"
#include "arm_math.h" // 需要用到较多三角函数

/* 底盘拥有的模块实例 */
static BMI088Instance *imu;
static SuperCapInstance *super_cap;
static referee_info_t *referee_data; // 裁判系统的数据会被不断更新

static HTMotorInstance *lf;
static HTMotorInstance *rf;
static HTMotorInstance *lb;
static HTMotorInstance *rb;
static LKMotorInstance *l_driven;
static LKMotorInstance *r_driven;

static CANCommInstance *chassis_comm; // 底盘板和云台板通信
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Chassis_Upload_Data_s chassis_feed_send;
// 由于采用多板架构,即使使用C板也有空余串口,可以使用串口通信以获得更高的通信速率

/* 方便函数间无开销传递参数的中间变量 */
// 若将下面的封装函数取缔,则可以将这些变量放入BalanceTask函数体内.
// static ...
static float leg_len_l, leg_len_r;     // 左右腿长(虚拟)
static float leg_angle_l, leg_angle_r; // 左右腿角度(虚拟)
// 倒立摆的虚拟力和虚拟力矩
static float F_virtual_left, T_virtual_left, F_virtual_right, T_virtual_right;
// 左前,左后,右前,右后关节力矩
static float T_joint_lf, T_joint_lr, T_joint_rf, T_joint_rb;
static float T_leg_left, T_leg_right; // 左右驱动电机力矩

/* ↓↓↓分割出这些函数是为了提高可读性,使得阅读者的逻辑更加顺畅;但实际上这些函数都不长,可以以注释的形式直接加到BalanceTask里↓↓↓*/

/**
 * @brief 根据状态反馈计算当前腿长,查表获得LQR的反馈增益,并列式计算LQR的输出
 *        由于反馈矩阵和控制矩阵都比较稀疏,故不使用矩阵库,避免非零项计算
 *
 */
static void CalcLQR()
{ 
}

/**
 * @brief 将LQR的输出映射到关节和驱动电机的输出
 *
 */
static void VMCProject()
{ // 拟将功能封装到vmc_project.h中
}

/**
 * @brief 腿部角度控制:转向和抗劈叉
 *
 */
static PIDInstance swerving_pid; // 转向PID,有转向指令时使用IMU的加速度反馈积分以获取速度和位置状态量
static PIDInstance anti_crash_pid; // 抗劈叉,将输出以相反的方向叠加到左右腿的上

static void SynthesizeMotion()
{
}

/**
 * @brief 腿长控制:长度 + roll轴补偿(保持机体水平),用PD模拟弹簧的传递函数
 *
 */
static PIDInstance leg_length_pid; // 用PD模拟弹簧的传递函数,不需要积分项(弹簧是一个无积分的二阶系统),增益不可过大否则抗外界冲击响应时太"硬"
static PIDInstance roll_compensate_pid; // roll轴补偿,用于保持机体水平

static void LegControl()
{
}

/**
 * @brief 离地监测和?跳跃控制?
 *        通过模型解算地面的支持力完成离地检测
 *
 */
static void FlyDetect()
{
}

/**
 * @brief 功率限制,一般不需要
 *
 */
static void WattLimit()
{
}

void BalanceInit()
{
    BMI088_Init_Config_s imu_config = {
        // IMU初始化
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
        .acc_int_config = {
            .exti_mode = EXTI_TRIGGER_FALLING,
            .GPIO_Pin = GPIO_PIN_10,
            .GPIOx = GPIOA,
        },
        .gyro_int_config = {
            .exti_mode = EXTI_TRIGGER_FALLING,
            .GPIO_Pin = GPIO_PIN_11,
            .GPIOx = GPIOA,
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
    driven_conf.can_init_config.tx_id=1;
    l_driven = LKMotorInit(&driven_conf);

    driven_conf.can_init_config.tx_id=2;
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

    referee_data = RefereeInit(&huart6); // 裁判系统串口

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
}
