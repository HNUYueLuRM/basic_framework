#include "bsp_init.h"
#include "robot.h"
#include "robot_def.h"

#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
#include "chassis.h"
#endif
#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
#include "gimbal.h"
#include "shoot.h"
#include "robot_cmd.h"
#endif

void RobotInit()
{
    BSPInit();
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
    ChassisInit();
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
#endif

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
    GimbalCMDTask();
    GimbalTask();
    ShootTask();
#endif
}
