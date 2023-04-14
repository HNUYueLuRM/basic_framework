#pragma once

// 底盘参数
#define CALF_LEN 0.25f        // 小腿
#define THIGH_LEN 0.15f       // 大腿
#define JOINT_DISTANCE 0.108f // 关节间距

#define VELOCITY_DIFF_VMC //通过速度计算增量,然后通过差分计算腿长变化率和腿角速度
// #define ANGLE_DIFF_VMC //直接保存上一次的值,通过差分计算腿长变化率和腿角速度


typedef struct
{
    // joint
    float phi1_w, phi4_w;
    float T_back, T_front;

    // link angle,phi1-ph5, phi5 is pod angle
    float phi1, phi2, phi3, phi4, phi5;

    // wheel
    float wheel_angle;
    float wheel_w;
    float T_wheel;

    // pod
    float pod_len;
    float pod_w;
    float pod_v;
    float F_pod;
    float T_pod;
} LinkNPodParam;

/**
 * @brief 平衡底盘初始化
 *
 */
void BalanceInit();

/**
 * @brief 平衡底盘任务
 *
 */
void BalanceTask();
