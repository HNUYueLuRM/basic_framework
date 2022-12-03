#include "robot.h"
#include "robot_def.h"

#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
#include "chassis.h"
#include "chassis_cmd.h"
#endif 
#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
#include "gimbal.h"
#include "shoot.h"
#include "gimbal_cmd.h"
#endif 


void RobotInit()
{
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
    ChassisInit();
    ChassisCMDInit();
#endif 

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
    GimbalCMDInit();
    GimbalInit();
    ShootInit();
#endif 
}

void RobotTask()
{
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
    ChassisTask();
    ChassisCMDTask();
#endif 

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
    GimbalCMDTask();
    GimbalTask();
    ShootTask();
#endif 
}

