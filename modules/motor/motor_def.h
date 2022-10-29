#ifndef MOTOR_DEF_H
#define MOTOR_DEF_H

typedef enum 
{
    CURRENT=0,
    SPEED=1,
    ANGLE=2
} closeloop_e;

typedef enum
{
    MOTOR=0,
    OTHER=1
} feedback_source_e;

typedef enum
{
    CLOCKWISE=0,
    COUNTER_CLOCKWISE=1
} reverse_flag_e;




#endif // !MOTOR_DEF_H
