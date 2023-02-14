#ifndef GENERAL_DEF_H
#define GENERAL_DEF_H

// 一些module的通用数值型定义,注意条件macro兼容,一些宏可能在math.h中已经定义过了


#ifndef PI
#define PI 3.1415926535f
#endif                  // !PI
#define PI2 (PI * 2.0f) // 2 pi

#define RAD_2_ANGLE 57.2957795f    // 180/pi
#define ANGLE_2_RAD 0.01745329252f // pi/180

#define RPM_2_ANGLE_PER_SEC 6.0f // ×360°/60sec

#endif // !GENERAL_DEF_H