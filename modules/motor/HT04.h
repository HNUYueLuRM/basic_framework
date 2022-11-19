#ifndef HT04_H
#define HT04_H

#include <stdint-gcc.h>
#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

#define HT_MOTOR_CNT 4

#define P_MIN -95.5f // Radians
#define P_MAX 95.5f
#define V_MIN -45.0f // Rad/s
#define V_MAX 45.0f
#define T_MIN -18.0f
#define T_MAX 18.0f

typedef struct // HT04
{
    float last_ecd;
    float ecd;
    float speed_rpm;
    float given_current;

    PID_t pid;
    can_instance *motor_can_instace;
} joint_instance;

typedef enum
{
    CMD_MOTOR_MODE = 0xfc,
    CMD_RESET_MODE = 0xfd,
    CMD_ZERO_POSITION = 0xfe
} joint_mode;

joint_instance *HTMotorInit(can_instance_config_s config);

void JointControl(joint_instance *_instance, float current);

void SetJointMode(joint_mode cmd, joint_instance *_instance);

#endif // !HT04_H#define HT04_H