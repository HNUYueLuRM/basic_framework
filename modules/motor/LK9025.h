#ifndef LK9025_H
#define LK9025_H

#include <stdint-gcc.h>
#include "bsp_can.h"
#include "controller.h"
#include "motor_def.h"

#define LK_MOTOR_CNT 2
#define I_MIN -2000
#define I_MAX 2000

typedef struct // 9025
{
    uint16_t last_ecd;
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;

    PID_t *pid;
    can_instance *motor_can_instance;

} driven_instance;

typedef enum
{
    unused = 0,
} driven_mode;

driven_instance *LKMotroInit(can_instance_config_s config);

void DrivenControl(int16_t motor1_current, int16_t motor2_current);

void SetDrivenMode(driven_mode cmd, uint16_t motor_id);

#endif // LK9025_H
