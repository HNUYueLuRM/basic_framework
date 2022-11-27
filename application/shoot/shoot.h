#ifndef SHOOT_H
#define SHOOT_H

#include "robot_def.h"
#include "dji_motor.h"

typedef struct 
{
    dji_motor_instance friction_l; 
    dji_motor_instance friction_r;
    dji_motor_instance loader;

} shoot;

void ShootInit();

void ShootTask();

#endif //SHOOT_H