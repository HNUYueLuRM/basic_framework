##舵机的使用

<p align='left' >panrui@hnu.edu.cn</p>

> todo: 由于新增了bsp_pwm的支持,舵机模块需要部分重构
### 舵机基础知识

已最常见的SG90舵机为例，SG90舵机要求工作在频率为50HZ——周期为20ms的PWM波，且对应信号的高低电平在0.5ms - 2.5ms之间，对应的舵机转动角度如下表所示（当然也可以按照这个线性的对应关系去达到转动自己想要的角度，如想要转动60°，则高电平脉宽为大概为1.2ms，具体能不能转到特定的角度还和舵机的精度有关）

>0.5ms-------------0度； 2.5%
>1.0ms------------45度； 5.0%
>1.5ms------------90度； 7.5%
>2.0ms-----------135度； 10.0%
>2.5ms-----------180度； 12.5%

根据`<font color=black size=3>Tout = (PSC+1)* (ARR+1)/Tclk</font>`公式
则我们需要产生50Hz的PWM波，则预分频的系数为 Prescaler = 168-1，自动重装载值 Counter Period = 20000-1，此时定时器产生的频率为 168Mhz/168/20000 = 50Hz。  当然这个值也可以自己设置，只要满足产生的频率为50Hz即可.
`__HAL_TIM_SET_COMPARE(htim, Channel, compare_value);`
这是设置占空比的函数
eg：当初始占空比为1200/20000则为6%，根据20*6%=1.2ms （1.2-0.5）/(2.5-0.5)*180=63° 故舵机会转动63° 
为了方便通过上述eg我们将所需要的角度与PWM计数值对应关系封装成函数。需要在初始化的時候输入我们所需要的角度和相关定时器参数即可。这样我们就可以设置SG90为参数范围内(0~180°)任意度数。

---

## 如何注册一个舵机实例
!!!

**注意！由于舵机为开环控制，无论选择舵机为何种类型，舵机都能够正常运行，但是运行的角度可能会与设定不同，请务必正确选择舵机型号！且最多添加7个舵机！**
我们可以像这样注册一个舵机实例
```c

static ServoInstance *leftservomoto;
//初始化参数
Servo_Init_Config_s config={
    //舵机安装选择的定时器及通道
    //C板有常用的7路PWM输出:TIM1-1,2,3,4 TIM8-1,2,3
    .htim=&htim1,
    .Channel=TIM_CHANNEL_1,
    //舵机的初始化模式和类型
    .Servo_Angle_Type=Start_mode,
    .Servo_type=Servo180,
};
// 设置好参数后进行初始化并保留返回的指针
leftservomoto = ServoInit(&config);
```
>要控制一个舵机 我们提供了以下三个接口
```c
//自由模式下，写入自由角度数值
void Servo_Motor_FreeAngle_Set(ServoInstance *Servo_Motor, int16_t S_angle);
//起止模式下，写入起始，终止角度数值(防止反复写入起始和终止角度)
void Servo_Motor_StartSTOP_Angle_Set(ServoInstance *Servo_Motor, int16_t Start_angle, int16_t Final_angle);
/*
    Free_Angle_mode, // 任意角度模式
    Start_mode,      // 起始角度模式
    Final_mode,      // 终止角度模式
*/
void Servo_Motor_Type_Select(ServoInstance *Servo_Motor,int16_t mode);
//比如我们要使用舵机，并更改一个舵机的模式
void ServoTask()
{
    //更改leftservomoto为Free_Angle_mode模式
    Servo_Motor_Type_Select(leftservomoto,Free_Angle_mode);
    //设置转到0角度
    Servo_Motor_FreeAngle_Set(leftservomoto, 0);
    //调用函数，控制电机
    Servo_Motor_Control();
}


```