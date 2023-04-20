#pragma once

// 底盘参数
#define CALF_LEN 0.245f           // 小腿
#define THIGH_LEN 0.14f           // 大腿
#define JOINT_DISTANCE 0.108f     // 关节间距
#define WHEEL_RADIUS 0.069f       // 轮子半径
#define LIMIT_LINK_RAD 0.15149458 // 初始限位角度,见ParamAssemble

// 计算速度的方式(五连杆到单杆的映射)
#define VELOCITY_DIFF_VMC // 通过速度计算增量,然后通过差分计算腿长变化率和腿角速度
// #define ANGLE_DIFF_VMC //直接保存上一次的值,通过差分计算腿长变化率和腿角速度

typedef struct
{
    // joint
    float phi1_w, phi4_w, phi2_w; // phi2_w used for calc real wheel speed
    float T_back, T_front;
    // link angle,phi1-ph5, phi5 is pod angle
    float phi1, phi2, phi3, phi4, phi5;
    // wheel
    float wheel_dist;
    float wheel_w;
    float T_wheel;
    // pod
    float theta, theta_w; // 杆和垂直方向的夹角,为控制状态之一
    float pod_len;
    float height, height_v;
    float pod_v, pod_w;
    float F_pod, T_pod;
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
