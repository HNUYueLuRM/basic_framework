// app
#include "balance.h"
#include "gain_table.h"
#include "robot_def.h"
#include "general_def.h"
// module
#include "HT04.h"
#include "LK9025.h"
#include "bmi088.h"
#include "referee_task.h"
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
static referee_info_t *referee_data;         // 裁判系统的数据会被不断更新
static Referee_Interactive_info_t my_uidata; // UI绘制数据,各种模式和状态/功率的显示
// 电机
static HTMotorInstance *lf, *rf, *lb, *rb;
static LKMotorInstance *l_driven, *r_driven;
// 底盘板和云台板通信
static CANCommInstance *chassis_comm; // 由于采用多板架构,即使使用C板也有空余串口,可以使用串口通信以获得更高的通信速率并降低阻塞
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Chassis_Upload_Data_s chassis_feed_send;
// 两个腿的参数,0为左腿,1为右腿
static LinkNPodParam left_side, right_side;
static PIDInstance swerving_pid;        // 转向PID,有转向指令时使用IMU的加速度反馈积分以获取速度和位置状态量
static PIDInstance anti_crash_pid;      // 抗劈叉,将输出以相反的方向叠加到左右腿的上
static PIDInstance leg_length_pid;      // 用PD模拟弹簧,不要积分(弹簧是无积分二阶系统),增益不可过大否则抗外界冲击响应时太"硬"
static PIDInstance roll_compensate_pid; // roll轴补偿,用于保持机体水平

/* ↓↓↓分割出这些函数是为了提高可读性,使得阅读者的逻辑更加顺畅;但实际上这些函数都不长,可以以注释的形式直接加到BalanceTask里↓↓↓*/

/**
 * @brief 将电机和imu的数据组装为LinkNPodParam结构体
 * @note 由于两个连杆和轮子连接时,只有一个使用了轴承,另外一个直接和电机的定子连接,
 *       所以轮子的转速和电机的转速不一致,因此和地面的速度应为电机转速减去杆的转速,得到的才是轮子的转速
 * @note HT04电机上电的编码器位置为零,请看Link2Pod()的note,确定限位LIMIT_LINK_RAD的正负号
 *
 * @todo angle direction to be verified
 */
static void ParamAssemble()
{
    left_side.phi1 = PI2 - LIMIT_LINK_RAD + lb->measure.total_angle;
    left_side.phi4 = lf->measure.total_angle + LIMIT_LINK_RAD;
    left_side.phi1_w = lb->measure.speed_rads;
    left_side.phi4_w = lf->measure.speed_rads;
    left_side.wheel_dist = l_driven->measure.total_angle / 360 * WHEEL_RADIUS * PI;
    left_side.wheel_w = l_driven->measure.speed_rads;
    // 电机的角度是逆时针为正,右侧全部取反
    right_side.phi1 = PI2 - LIMIT_LINK_RAD - rb->measure.total_angle;
    right_side.phi4 = -rf->measure.total_angle + LIMIT_LINK_RAD;
    right_side.phi1_w = -rb->measure.speed_rads;
    right_side.phi4_w = -rf->measure.speed_rads;
    right_side.wheel_dist = -r_driven->measure.total_angle / 360 * WHEEL_RADIUS * PI;
    right_side.wheel_w = -r_driven->measure.speed_rads;
    // 若有转向指令,则使用IMU积分得到的速度,否则左右轮相反会产生阻碍转向的力矩
    if (chassis_cmd_recv.wz != 0) // 此时相当于lqr的平衡环只对机体的速度响应
    {
        float vel_divide_R = imu_data->Accel[3] * balance_dt / WHEEL_RADIUS; // todo: 确定速度方向
        left_side.wheel_w = vel_divide_R;
        right_side.wheel_w = vel_divide_R;
    } // 转向的时候不需要修正轮速
}

/**
 * @brief 根据关节角度和角速度,计算单杆长度和角度以及变化率
 * @todo 测试两种的效果,留下其中一种;
 *       若差分的效果好则不需要VELOCITY_DIFF_VMC内的代码
 *
 * @note 右侧视图
 *  ___x          anti-clockwise is positive for motor
 * |            _____
 * |y          /     \
 *             \     /
 *              \   /
 *
 * @param p 5连杆和腿的参数
 */
static void Link2Pod(LinkNPodParam *p)
{ // 拟将功能封装到vmc_project.h中
    float xD, yD, xB, yB, BD, A0, B0, xC, yC, phi2t, phi5t;
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

    p->phi5 = atan2f(yC, xC - JOINT_DISTANCE / 2);
    p->pod_len = Sqrt(powf(xC - JOINT_DISTANCE / 2, 2) + powf(yC, 2));
    p->phi3 = atan2(yC - yD, xC - xD);                              // 稍后用于计算VMC
    p->theta = p->phi5 - 0.5 * PI - imu_data->Pitch * DEGREE_2_RAD; // 确定方向
    p->height = p->pod_len * arm_cos_f32(p->theta);

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
    phi2t = 2 * atan2f(B0 + Sqrt(powf(A0, 2) + powf(B0, 2) - powf(BD, 2)), A0 + BD); // 不要用link->phi2,因为这里是预测的
    xC = xB + CALF_LEN * arm_cos_f32(phi2t);
    yC = yB + CALF_LEN * arm_sin_f32(phi2t);
    phi5t = atan2f(yC, xC - JOINT_DISTANCE / 2);
    // 差分计算腿长变化率和腿角速度
    p->phi2_w = (phi2t - p->phi2) / balance_dt; // 稍后用于修正轮速
    p->pod_w = (phi5t - p->phi5) / balance_dt;
    p->pod_v = (Sqrt(powf(xC - JOINT_DISTANCE / 2, 2) + powf(yC, 2)) - p->pod_len) / balance_dt;
    p->theta_w = (phi5t - 0.5 * PI - imu_data->Pitch * DEGREE_2_RAD - balance_dt * imu_data->Gyro[3] - p->theta) / balance_dt; //@todo 确定c板安装方向(BMI088);
    p->height_v = p->pod_v * arm_cos_f32(p->theta) - p->pod_len * arm_sin_f32(p->theta) * p->theta_w;                          // 这很酷!PDE!
#endif
    if (chassis_cmd_recv.wz == 0)                                  // 没有转向控制指令,修正轮速. @todo 确定方向
        p->wheel_w = (p->wheel_w - p->phi2_w + imu_data->Gyro[3]); // 注意此时单位仍然是rad/s, @todo 确定c板安装方向(BMI088);
    // 此时使用的是电机编码器反馈的速度
}

/**
 * @brief 根据状态反馈计算当前腿长,查表获得LQR的反馈增益,并列式计算LQR的输出
 *        由于反馈矩阵和控制矩阵都比较稀疏,故不使用矩阵库,避免非零项计算
 *
 * @note 计算得到的腿部力矩输出还要再综合运动控制系统补偿后映射为两个关节电机
 *       而轮子的输出则只经过转向PID的反馈增益计算
 *
 * @todo 确定使用查表还是多项式拟合
 *
 */
static void CalcLQR(LinkNPodParam *p, float target_x)
{
    float *gain_list = LookUpKgain(p->pod_len); // K11,K12... K21,K22... K26
    float T[2];                                 // 0 T_wheel, 1 T_pod;
    for (uint8_t i = 0; i < 2; i++)
    {
        T[i] = gain_list[i * 6 + 0] * -p->theta +
               gain_list[i * 6 + 1] * -p->theta_w +
               gain_list[i * 6 + 4] * -imu_data->Pitch * DEGREE_2_RAD +
               gain_list[i * 6 + 5] * -imu_data->Gyro[1];              // @todo 待确定c板安装方向
        if (chassis_cmd_recv.vx == 0 && chassis_cmd_recv.wz == 0)      // 位置闭环仅在目标转向速度和水平速度都为零的情况下起作用
            T[i] += gain_list[i * 6 + 2] * (target_x - p->wheel_dist); // 若速度为0,则加入位置闭环,以提供对外力更好的抵抗和复位
        else
            T[i] += gain_list[i * 6 + 3] * (chassis_cmd_recv.vx - p->wheel_w * WHEEL_RADIUS); // 若目标速度|角速度不为0,则只对速度闭环
    }                                                                                         // 有转向指令的时候,轮速的来源是IMU,而不是编码器
    p->T_wheel = T[0];
    p->T_pod = T[1];
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

    PIDCalculate(&swerving_pid, imu_data->Yaw, chassis_cmd_recv.wz); // 对速度闭环还是使用角度增量闭环?
    left_side.T_wheel -= swerving_pid.Output;
    right_side.T_wheel += swerving_pid.Output;
}

/**
 * @brief 腿部控制:长度.用PD模拟弹簧的传递函数
 *
 * @note 实际使用的是机体当前的高度,若使用腿长计算则会导致加速和减速时因腿倾斜故而机体高度变化
 */
static void LegControl(LinkNPodParam *p, float target_length)
{
    p->F_pod += PIDCalculate(&leg_length_pid, p->height, target_length);
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
 * @brief 功率限制,一般不需要;限制完成之后设定电机输出
 *
 */
static void WattLimitSet()
{
    // code to limit 9025's output
    // ...

    // 设定电机电流
    HTMotorSetRef(lf, left_side.T_front);
    HTMotorSetRef(lb, left_side.T_back);
    HTMotorSetRef(rf, right_side.T_front);
    HTMotorSetRef(rb, right_side.T_back);
    LKMotorSetRef(l_driven, left_side.T_wheel);
    LKMotorSetRef(r_driven, right_side.T_wheel);
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
        .can_init_config = {
            .can_handle = &hcan1,
        },
        .controller_param_init_config = {
            .current_PID = {
                .Kp = 1,
            },
        },
        .controller_setting_init_config = {
            .close_loop_type = CURRENT_LOOP,
            .outer_loop_type = CURRENT_LOOP,
            .motor_reverse_flag = FEEDBACK_DIRECTION_NORMAL,
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
        },
        .motor_type = HT04,
    };

    joint_conf.can_init_config.tx_id = 1;
    joint_conf.can_init_config.rx_id = 11;
    lf = HTMotorInit(&joint_conf);
    joint_conf.can_init_config.tx_id = 2;
    joint_conf.can_init_config.rx_id = 12;
    lb = HTMotorInit(&joint_conf);
    joint_conf.can_init_config.tx_id = 3;
    joint_conf.can_init_config.rx_id = 13;
    rf = HTMotorInit(&joint_conf);
    joint_conf.can_init_config.tx_id = 4;
    joint_conf.can_init_config.rx_id = 14;
    rb = HTMotorInit(&joint_conf);

    // ↓↓↓---------------驱动电机初始化----------------↓↓↓
    Motor_Init_Config_s driven_conf = {
        // 写一个,剩下的修改方向和id即可
        .can_init_config.can_handle = &hcan1,
        .controller_param_init_config = {
            .current_PID = {
                .Kp = 274.348,
            },
        },
        .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED,
            .speed_feedback_source = MOTOR_FEED,
            .outer_loop_type = CURRENT_LOOP,
            .close_loop_type = CURRENT_LOOP,
            .motor_reverse_flag = MOTOR_DIRECTION_NORMAL,
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

    referee_data = Referee_Interactive_init(&huart6, &my_uidata); // 裁判系统串口

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

    if (chassis_cmd_recv.chassis_mode == CHASSIS_ZERO_FORCE)
    {
        HTMotorStop(lf);
        HTMotorStop(rf);
        HTMotorStop(lb);
        HTMotorStop(rb);
        LKMotorStop(l_driven);
        LKMotorStop(r_driven);
    }
    else
    {
        HTMotorEnable(lf);
        HTMotorEnable(rf);
        HTMotorEnable(lb);
        HTMotorEnable(rb);
        LKMotorEnable(l_driven);
        LKMotorEnable(r_driven);
    }

    ParamAssemble(); // 参数组装,将电机和IMU的参数组装到一起
    // 将五连杆映射成单杆
    Link2Pod(&left_side);
    Link2Pod(&right_side);
    // 根据单杆计算处的角度和杆长,计算反馈增益
    CalcLQR(&left_side, chassis_cmd_recv.vx); // @todo,需要确定速度or位置闭环
    CalcLQR(&right_side, chassis_cmd_recv.vx);
    // 腿长控制
    LegControl(&left_side, 0);
    LegControl(&right_side, 0);
    // 综合运动控制,转向+抗劈叉
    SynthesizeMotion(); // 两边要一起
    // 俯仰角补偿,保持机体水平
    RollCompensate(); // 两边也要一起
    // VMC映射成关节输出
    VMCProject(&left_side);
    VMCProject(&right_side);

    FlyDetect(); // 滞空检测,尝试使用加速度计自由落体时量测值小?

    WattLimitSet(); // 电机输出限幅

    // code to go here... 裁判系统,UI,多机通信

    CANCommSend(chassis_comm, (uint8_t *)&chassis_feed_send);
}