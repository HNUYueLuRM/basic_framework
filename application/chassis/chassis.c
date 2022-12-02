#include "chassis.h"
#include "robot_def.h"
#include "dji_motor.h"
#include "super_cap.h"

#ifdef CHASSIS_BOARD //使用板载IMU获取底盘转动角速度
#include "ins_task.h"
IMU_Data_t* Chassis_IMU_data;
#endif // CHASSIS_BOARD

// SuperCAP cap;
static dji_motor_instance* lf; //left right forward back
static dji_motor_instance* rf;
static dji_motor_instance* lb;
static dji_motor_instance* rb;
static Chassis_Ctrl_Cmd_s chassis_cmd_recv;
static Chassis_Upload_Data_s chassis_feedback_data;

static void mecanum_calculate()
{

}



void ChassisInit()
{

}

void ChassisTask() 
{

}