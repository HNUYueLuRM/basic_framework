/**
 ******************************************************************************
 * @file    ins_task.h
 * @author  Wang Hongxi
 * @version V2.0.0
 * @date    2022/2/23
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef __INS_TASK_H
#define __INS_TASK_H

#include "stdint.h"
#include "BMI088driver.h"
#include "QuaternionEKF.h"

#define X 0
#define Y 1
#define Z 2

#define INS_TASK_PERIOD 1

typedef struct
{
    float Roll;
    float Pitch;
    float Yaw;
    float YawTotalAngle;
} attitude_t;

typedef struct
{
    float q[4]; // 四元数估计值

    float Gyro[3];          // 角速度
    float Accel[3];         // 加速度
    float MotionAccel_b[3]; // 机体坐标加速度
    float MotionAccel_n[3]; // 绝对系加速度

    float AccelLPF; // 加速度低通滤波系数

    // 加速度在绝对系的向量表示
    float xn[3];
    float yn[3];
    float zn[3];

    float atanxz;
    float atanyz;

    // 位姿
    float Roll;
    float Pitch;
    float Yaw;
    float YawTotalAngle;
} INS_t;

/**
 * @brief 用于修正安装误差的参数
 *
 */
typedef struct
{
    uint8_t flag;

    float scale[3];

    float Yaw;
    float Pitch;
    float Roll;
} IMU_Param_t;

/**
 * @brief 反馈位姿结构体指针,apps通过初始化时保存该指针以实现数据访问
 *
 * @return attitude_t*
 */
attitude_t *GetINSptr();

/**
 * @brief 初始化惯导解算系统
 *
 */
void INS_Init(void);

/**
 * @brief 此函数放入实时系统中,以1kHz频率运行
 *        p.s. osDelay(1);
 *
 */
void INS_Task(void);

/**
 * @brief 四元数更新函数,即实现dq/dt=0.5Ωq
 *
 * @param q  四元数
 * @param gx
 * @param gy
 * @param gz
 * @param dt 距离上次调用的时间间隔
 */
void QuaternionUpdate(float *q, float gx, float gy, float gz, float dt);

/**
 * @brief 四元数转换成欧拉角 ZYX
 *
 * @param q
 * @param Yaw
 * @param Pitch
 * @param Roll
 */
void QuaternionToEularAngle(float *q, float *Yaw, float *Pitch, float *Roll);

/**
 * @brief ZYX欧拉角转换为四元数
 *
 * @param Yaw
 * @param Pitch
 * @param Roll
 * @param q
 */
void EularAngleToQuaternion(float Yaw, float Pitch, float Roll, float *q);

/**
 * @brief 机体系到惯性系的变换函数
 *
 * @param vecBF body frame
 * @param vecEF earth frame
 * @param q
 */
void BodyFrameToEarthFrame(const float *vecBF, float *vecEF, float *q);

/**
 * @brief 惯性系转换到机体系
 *
 * @param vecEF
 * @param vecBF
 * @param q
 */
void EarthFrameToBodyFrame(const float *vecEF, float *vecBF, float *q);

#endif
