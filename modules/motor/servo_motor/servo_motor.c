#include "servo_motor.h"
#include "stdlib.h"
#include "memory.h"

extern TIM_HandleTypeDef htim1;
/*第二版*/
static ServoInstance *servo_motor_instance[SERVO_MOTOR_CNT] = {NULL};
static int16_t compare_value[SERVO_MOTOR_CNT] = {0};
static uint8_t servo_idx = 0; // register servo_idx,是该文件的全局舵机索引,在注册时使用

// 通过此函数注册一个舵机
ServoInstance *ServoInit(Servo_Init_Config_s *Servo_Init_Config)
{
    ServoInstance *servo = (ServoInstance *)malloc(sizeof(ServoInstance));
    memset(servo, 0, sizeof(ServoInstance));

    servo->Servo_type = Servo_Init_Config->Servo_type;
    servo->htim = Servo_Init_Config->htim;
    servo->Channel = Servo_Init_Config->Channel;

    HAL_TIM_PWM_Start(Servo_Init_Config->htim, Servo_Init_Config->Channel);
    servo_motor_instance[servo_idx++] = servo;
    return servo;
}

/**
 * @brief 写入自由角度数值
 *
 * @param Servo_Motor 注册的舵机实例
 * @param S_angle  改变自由模式设定的角度
 */
void Servo_Motor_FreeAngle_Set(ServoInstance *Servo_Motor, int16_t S_angle)
{
    switch (Servo_Motor->Servo_type)
    {
    case Servo180:
        if (S_angle > 180)
            S_angle = 180;
        break;
    case Servo270:
        if (S_angle > 270)
            S_angle = 270;
        break;
    case Servo360:
        if (S_angle > 100)
            S_angle = 100;
        break;
    default:
        break;
    }
    if (S_angle < 0)
        S_angle = 0;
    Servo_Motor->Servo_Angle.free_angle = S_angle;
}

/**
 * @brief 写入起始，终止角度数值
 *
 * @param Servo_Motor 注册的舵机实例
 * @param Start_angle 起始角度
 * @param Final_angle 终止角度
 */
void Servo_Motor_StartSTOP_Angle_Set(ServoInstance *Servo_Motor, int16_t Start_angle, int16_t Final_angle)
{
    Servo_Motor->Servo_Angle.Init_angle = Start_angle;
    Servo_Motor->Servo_Angle.Final_angle = Final_angle;
}
/**
 * @brief 舵机模式选择
 *
 * @param Servo_Motor 注册的舵机实例
 * @param mode 需要选择的模式
 */
void Servo_Motor_Type_Select(ServoInstance *Servo_Motor, int16_t mode)
{
    Servo_Motor->Servo_Angle_Type = mode;
}

/**
 * @brief 舵机输出控制
 *
 */
void ServeoMotorControl()
{
    ServoInstance *Servo_Motor;

    for (size_t i = 0; i < servo_idx; i++)
    {
        if (servo_motor_instance[i])
        {
            Servo_Motor = servo_motor_instance[i];

            switch (Servo_Motor->Servo_type)
            {
            case Servo180:
                if (Servo_Motor->Servo_Angle_Type == Start_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.Init_angle * 20000 / 20 / 90;
                if (Servo_Motor->Servo_Angle_Type == Final_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.Final_angle * 20000 / 20 / 90;
                if (Servo_Motor->Servo_Angle_Type == Free_Angle_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.free_angle * 20000 / 20 / 90;
                __HAL_TIM_SET_COMPARE(Servo_Motor->htim, Servo_Motor->Channel, compare_value[i]);
                break;
            case Servo270:
                if (Servo_Motor->Servo_Angle_Type == Start_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.Init_angle * 20000 / 20 / 135;
                if (Servo_Motor->Servo_Angle_Type == Final_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.Final_angle * 20000 / 20 / 135;
                if (Servo_Motor->Servo_Angle_Type == Free_Angle_mode)
                    compare_value[i] = 0.5 * 20000 / 20 + Servo_Motor->Servo_Angle.free_angle * 20000 / 20 / 135;
                __HAL_TIM_SET_COMPARE(Servo_Motor->htim, Servo_Motor->Channel, compare_value[i]);
                break;
            case Servo360:
                /*500-2500的占空比 500-1500对应正向转速 1500-2500对于反向转速*/
                compare_value[i] = 500 + 20 * Servo_Motor->Servo_Angle.servo360speed;
                __HAL_TIM_SET_COMPARE(Servo_Motor->htim, Servo_Motor->Channel, compare_value[i]);
                break;
            default:
                break;
            }
        }
    }
}
