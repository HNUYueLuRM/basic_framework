#include "robot.h"
#include "robot_def.h"

#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)
#include "chassis.h"
#endif 
#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)
#include "gimbal.h"
#include "shoot.h"
#endif 

void RobotInit()
{
    
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)

#endif 

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)

#endif 
}

void RobotTask()
{
#if defined(ONE_BOARD) || defined(CHASSIS_BOARD)

#endif 

#if defined(ONE_BOARD) || defined(GIMBAL_BOARD)

#endif 
}

