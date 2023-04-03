# dji_motor

<p align='right'>neozng1@hnu.edu.cn</p>

> TODO:
>
> 1. 给不同的电机设置不同的低通滤波器惯性系数而不是统一使用宏
> 2. 为M2006和M3508增加开环的零位校准函数

---

> 建议将电机的反馈频率通过RoboMaster Assistant统一设置为500Hz。当前默认的`MotorTask()`执行频率为500Hz，若不修改电机反馈频率可能导致单条总线挂载的电机数量有限，且容易出现帧错误和仲裁失败的情况。

## 总览和封装说明

> 如果你不需要理解该模块的工作原理，你只需要查看这一小节。

dji_motor模块对DJI智能电机，包括M2006，M3508以及GM6020进行了详尽的封装。你不再需要关心PID的计算以及CAN报文的发送和接收解析，你只需要专注于根据应用层的需求，设定合理的期望值，并通过`DJIMotorSetRef()`设置对应电机的输入参考即可。

**==设定值的单位==**

1. ==位置环为**角度制**（0-360，total_angle可以为任意值）==

2. ==速度环为角速度，单位为**度/每秒**（deg/sec）==

3. ==电流环为A==

4. ==GM6020的输入设定为**力矩**，待测量（-30000~30000）==

   ==M3508的输入设定为-20A~20A （-16384~16384）==

   ==M2006的输入设定为-10A~10A （-10000~10000）==

如果你希望更改电机的反馈来源，比如进入小陀螺模式/视觉模式（这时候你想要云台保持静止，使用IMU的yaw角度值作为反馈来源），只需要调用`DJIMotorChangeFeed()`，电机便可立刻切换反馈数据来源至IMU。

要获得一个电机，请通过`DJIMotorInit()`并传入一些参数，他就会返回一个电机的指针。你也不再需要查看这些电机和电调的说明书，**只需要设置其电机id**（6020为拨码开关值，2006和3508为电调的闪动次数），该模块会自动为你计算CAN发送和接收ID并搞定所有硬件层的琐事。

初始化电机时，你需要传入的参数包括：

- **电机挂载的CAN总线设置**，CAN1 or CAN2，以及电机的id，使用`can_instance_config_s`封装，只需要设置这两个参数:

  ```c
  CAN_HandleTypeDef *can_handle;
  uint32_t tx_id; // tx_id设置为电机id,不需要查说明书计算，直接为电调的闪动次数或拨码开关值，为1-8
  ```

- **电机类型**，使用`Motor_Type_e`：

  ```c
  GM6020 = 0
  M3508  = 1
  M2006  = 2
  ```

- **电机控制设置**

  - 闭环类型

    ```c
    OPEN_LOOP
    CURRENT_LOOP 
    SPEED_LOOP 
    ANGLE_LOOP 
    CURRENT_LOOP | SPEED_LOOP       // 同时对电流和速度闭环
    SPEED_LOOP   | ANGLE_LOOP       // 同时对速度和位置闭环
    CURRENT_LOOP | SPEED_LOOP |ANGLE_LOOP // 三环全开
    ```

  - 是否反转

    ```c
    MOTOR_DIRECTION_NORMAL 
    MOTOR_DIRECTION_REVERSE
    ```

  - 是否其他反馈来源，以及他们对应的数据指针（如果有的话）

    ```c
    MOTOR_FEED = 0
    OTHER_FEED = 1
    ---
    
    // 电流只能从电机传感器获得所以无法设置其他来源
    ```

  - 每个环的PID参数以及是否使用改进功能，以及其他反馈来源指针（如果在上一步启用了其他数据来源）

    ```c
    typedef struct // config parameter
    {
        float Kp;
        float Ki;
        float Kd;
     
        float MaxOut;        // 输出限幅
        // 以下是优化参数
        float IntegralLimit; // 积分限幅
        float DeadBand;      // 死区
        float CoefA;         // For Changing Integral
        float CoefB;         // ITerm = Err*((A-abs(err)+B)/A)  when B<|err|<A+B
        float Output_LPF_RC; // RC = 1/omegac
        float Derivative_LPF_RC;
        
        PID_Improvement_e Improve; // 优化环节，定义在下一个代码块
    } PIDInit_config_s;
    // 只有当你设启用了对应的优化环节，优化参数才会生效
    ```

    ```c
    typedef enum
    {
        NONE      = 0b00000000,                     
        Integral_Limit     = 0b00000001,           
        Derivative_On_Measurement   = 0b00000010,   
        Trapezoid_Intergral      = 0b00000100,       
        Proportional_On_Measurement = 0b00001000, 
        OutputFilter     = 0b00010000,               
        ChangingIntegrationRate  = 0b00100000,    
        DerivativeFilter    = 0b01000000,            
        ErrorHandle     = 0b10000000,         
    } PID_Improvement_e;
    // 若希望使用多个环节的优化，这样就行：Integral_Limit |Trapezoid_Intergral|...|...
    ```

    ```c
    float *other_angle_feedback_ptr
    float *other_speed_feedback_ptr
    ```

---

推荐的初始化参数编写格式如下：

```c
Motor_Init_Config_s config = {
  .motor_type = M3508,  // 要注册的电机为3508电机
  .can_init_config = {.can_handle = &hcan1, // 挂载在CAN1
       .tx_id = 1},          // C620每隔一段时间闪动1次,设置为1
  // 采用电机编码器角度与速度反馈,启用速度环和电流环,不反转,最外层闭环为速度环
        .controller_setting_init_config = {.angle_feedback_source = MOTOR_FEED, 
             
            .outer_loop_type = SPEED_LOOP,
            .close_loop_type = SPEED_LOOP | CURRENT_LOOP, 
             .speed_feedback_source = MOTOR_FEED, 
             .motor_reverse_flag = MOTOR_DIRECTION_NORMAL},
     // 电流环和速度环PID参数的设置,不采用计算优化则不需要传入Improve参数
        // 不使用其他数据来源(如IMU),不需要传入反馈数据变量指针
  .controller_param_init_config = {.current_PID = {.Improve = 0,
                                                         .Kp = 1,
                                                         .Ki = 0,
                                                         .Kd = 0,
                                                         .DeadBand = 0,
                                                         .MaxOut = 4000},
           .speed_PID = {.Improve = 0,
                                                       .Kp = 1,
                                                       .Ki = 0,
                                                       .Kd = 0,
                                                       .DeadBand = 0,
                                                       .MaxOut = 4000}}};

dji_motor_instance *djimotor = DJIMotorInit(config); // 设置好参数后进行初始化并保留返回的指针
```

---

要控制一个DJI电机，我们提供了2个接口：

```c
void DJIMotorSetRef(dji_motor_instance *motor, float ref);

void DJIMotorChangeFeed(dji_motor_instance *motor, 
                        Closeloop_Type_e loop, 
                        Feedback_Source_e type);
```

调用第一个并传入设定值，它会自动根据你设定的PID参数进行动作。 如果对不同闭环都有参考输入,则设置最外层的闭环(通过此函数)并将剩下的参考输入通过前馈数据指针进行设定

调用第二个并设定要修改的反馈环节和反馈类型，它会将反馈数据指针切换到你设定好的变量（需要在初始化的时候设置反馈指针）。

**如果需要获取电机的反馈数据**（如小陀螺模式需要根据麦克纳姆轮逆运动学解算底盘速度），直接通过你拥有的`dji_motor_instance`访问成员变量：

```c
// LeftForwardMotor是一个dji_motor_instance实例
float speed=LeftForwardMotor->motor_measure->speed_rpm;
...
```

***现在，忘记PID的计算和发送、接收以及协议解析，专注于模块之间的逻辑交互吧。***

---

## 代码结构

.h文件内包括了外部接口和类型定义,以及模块对应的宏。c文件内为私有函数和外部接口的定义。

motor_def.h内包含了一些电机通用的定义。

## 类型定义

```c
#define DJI_MOTOR_CNT 12
#define SPEED_SMOOTH_COEF 0.9f    // better to be greater than 0.85
#define CURRENT_SMOOTH_COEF 0.98f // this coef must be greater than 0.95

typedef struct /* DJI电机CAN反馈信息*/
{
    uint16_t ecd;
    uint16_t last_ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t total_round;
    int32_t total_angle;
} dji_motor_measure;

typedef struct
{
    /* motor measurement recv from CAN feedback */
    dji_motor_measure motor_measure;
    /* basic config of a motor*/
    Motor_Control_Setting_s motor_settings;
    /* controller used in the motor (3 loops)*/
    Motor_Controller_s motor_controller;
    /* the CAN instance own by motor instance*/
    can_instance motor_can_instance;
    /* sender assigment*/
    uint8_t sender_group;
    uint8_t message_num;
 
   uint8_t stop_flag;
    
    Motor_Type_e motor_type;
} dji_motor_instance;
```

- `DJI_MOTOR_CNT`是允许的最大DJI电机数量，根据经验，暂定为每个CAN6个，防止出现拥塞。

- `SPEED_SMOOTH_COEF`和`CURRENT_SMOOTH_COEF`是电机反馈的电流和速度数据低通滤波器惯性系数，数值越小平滑效果越大，但滞后也越大。设定时不应当低于推荐值。
- `dji_motor_measure`是DJI电机的反馈信息，包括当前编码器值、上次测量编码器值、速度、电流、温度、总圈数和单圈角度。

- `Motor_Control_Setting_s`的定义在`motor_def.h`之中，它和`Motor_Controller_s`都是所有电机通用的组件（如M3508，LK9025，HT04，MT6023等），其包含内容如下：

  ```c
  typedef struct /* 电机控制配置 */
  {
      Closeloop_Type_e outer_loop_type;
      Closeloop_Type_e close_loop_type;
      Motor_Reverse_Flag_e motor_reverse_flag;
      Feedback_Source_e angle_feedback_source;
      Feedback_Source_e speed_feedback_source;
  } Motor_Control_Setting_s;
  ```

  `Motor_Control_Setting_s`里包含了电机的闭环类型，反转标志以及额外的反馈来源标志。

  - 闭环类型指示该电机使用的控制器配置，其枚举定义如下：

    ```c
    typedef enum
    {
        CURRENT_LOOP = 0b0001,
        SPEED_LOOP = 0b0010,
        ANGLE_LOOP = 0b0100,
        _ = 0b0011,
        __ = 0b0110,
        ___ = 0b0111
    } Closeloop_Type_e;
    ```

    以M3508为例，假设需要进行**速度闭环**和**电流闭环**，那么在初始化时就将这个变量的值设为`CURRENT_LOOP | SPEED_LOOP`。在`DJIMotorControl()`中，函数将会根据此标志位判断设定的参考值需要经过那些控制器的计算。
    另外,你还需要设置当前电机的最外层闭环，即电机的闭环目标为什么类型的值。初始化时需要设置`outer_loop_type`。以M2006作为拨盘电机时为例，你希望它在单发/双发等固定发射数量的模式下对位置进行闭环（拨盘转过一定角度对应拨出一颗弹丸），但你也有可能希望在连发的时候让拨盘连续的转动，以一定的频率发射弹丸。我们提供了`DJIMotorOuterLoop()`用于修改电机的外层闭环，改变电机的闭环对象。

    > 注意，务必分清串级控制（多环）和外层闭环的区别。前者是为了提高内环的性能，使得其能更好地跟随外环参考值；而后者描述的是系统真实的控制目标（闭环目标）。如3508，没有电流环仍然可以对速度完成闭环，对于高层的应用来说，它们本质上不关心电机内部是否还有电流环，它们只把外层闭环为速度的电机当作一个**速度伺服执行器**，**外层闭环**描述的就是真正的闭环目标。

  - 为了避开恼人的正负号，提高代码的可维护性，在初始化电机时设定`motor_reverse_flag`使得所有电机都按照你想要的方向旋转，其定义如下：

    ```c
    typedef enum
    {
        MOTOR_DIRECTION_NORMAL = 0,
        MOTOR_DIRECTION_REVERSE = 1
    } Motor_Reverse_Flag_e;
    ```

  - `speed_feedback_source`以及`angle_feedback_source`是指示电机反馈来源的标志位。一般情况下电机使用自身的编码器作为控制反馈量。但在某些时候，如小陀螺模式，云台电机会使用IMU的姿态数据作为反馈数据来源。其定义如下：

    ```c
    typedef enum
    {
        MOTOR_FEED = 0,
        OTHER_FEED = 1
    } Feedback_Source_e;
    ```

    **注意，如果启用其他数据来源，你需要在电机的控制器配置`Motor_Controller_s`下的`other_xxx_feedback_ptr`中指定其他数据来源。**

    你可以在`DJIMotorChangeFeed()`中修改电机的数据来源。

- `Motor_Controller_s`的定义也在`motor_def.h`之中：

  ```c
  /* 电机控制器,包括其他来源的反馈数据指针,3环控制器和电机的参考输入*/
  typedef struct
  {
      float *other_angle_feedback_ptr;
      float *other_speed_feedback_ptr;
      PID_t current_PID;
      PID_t speed_PID;
      PID_t angle_PID;
      
      float pid_ref; // 将会作为每个环的输入和输出顺次通过串级闭环
  } Motor_Controller_s;
  ```

  两个`float*`指针应当指向其他反馈来源数据（如果有的话，需要在`motor_settings`中设定）。

  三个PID分别为三个控制闭环所用，在`DJIMotorControl()`中，该函数会根据`close_loop_type`的设定计算对应的闭环。

  **`pid_ref`是控制的设定值，app层的应用想要更改电机的输出，就要调用`DJIMotorSetRef()`更改此值。**

- `dji_motor_instance`是一个DJI电机实例。一个电机实例内包含电机的反馈信息，电机的控制设置，电机控制器，电机对应的CAN实例以及电机的类型；由于DJI电机支持**一帧报文控制至多4个电机**，该结构体还包含了用于给电机分组发送进行特殊处理的`sender_group`和`message_num`（具体实现细节参考`MotorSenderGrouping()`函数）。

## 外部接口

```c
dji_motor_instance *DJIMotorInit(can_instance_config config,
                                 Motor_Control_Setting_s motor_setting,
                                 Motor_Controller_Init_s controller_init,
                                 Motor_Type_e type);

void DJIMotorSetRef(dji_motor_instance *motor, float ref);

void DJIMotorChangeFeed(dji_motor_instance *motor, 
                        Closeloop_Type_e loop, 
                        Feedback_Source_e type);

void DJIMotorControl();

void DJIMotorStop(dji_motor_instance *motor);

void DJIMotorEnable(dji_motor_instance *motor);

void DJIMotorOuterLoop(dji_motor_instance *motor);
```

- `DJIMotorInit()`是用于初始化电机对象的接口，传入包括电机can配置、电机控制配置、电机控制器配置以及电机类型在内的初始化参数。**它将会返回一个电机实例指针**，你应当在应用层保存这个指针，这样才能操控这个电机。

- `DJIMotorSetRef()`是设定电机输出的接口，**在调用这个函数的时候，你可以认为你的设定值会直接转变为电机的输出**。`DJIMotorControl()`会帮你完成闭环计算，不用担心PID。

- `DJIMotorChangeFeed()`一般在更改云台或底盘的运动模式的时候被调用，传入要修改反馈来源的电机实例指针、要修改的闭环以及反馈来源类型。如希望切换到IMU的yaw值作为云台设定值，传入yaw轴电机实例和`ANGLE_LOOP`（位置环）、`OTHER_FEED`（启用其他数据来源）即可。当然，你需要在初始化的时候设定`motor_controller`中的 `other_angle_feedback_ptr`，使其指向yaw值的变量。

- `DJIMotorControl()`是根据电机的配置计算控制值的函数。该函数在`motor_task.c`中被调用，应当在freeRTOS中以一定频率运行。此函数为PID的计算进行了彻底的封装，要修改电机的参考输入，请在app层的应用中调用`DJIMotorSetRef()`。

  该函数的具体实现请参照代码，注释已经较为清晰。流程大致为：

  1. 根据电机的初始化控制配置，计算各个控制闭环
  2. 根据反转标志位，确定是否将输出反转
  3. 根据每个电机的发送分组，将最终输出值填入对应的分组buff
  4. 检查每一个分组，若该分组有电机，发送报文
  
- `DJIMotorStop()`和`DJIMotorEnable()`用于控制电机的启动和停止。当电机被设为stop的时候，不会响应任何的参考输入。

- `DJIMotorOuterLoop()`用于修改电机的外部闭环类型，即电机的真实闭环目标。

## 私有函数和变量

在.c文件内设为static的函数和变量

```c
static uint8_t idx = 0; // register idx,是该文件的全局电机索引,在注册时使用
static dji_motor_instance *dji_motor_info[DJI_MOTOR_CNT] = {NULL};
```

这是管理所有电机实例的入口。idx用于电机初始化。

```c
#define PI2 (3.141592f * 2)
#define ECD_ANGLE_COEF_DJI 3.835e-4 // ecd/8192*pi
```

这两个宏用于在电机反馈信息中的多圈角度计算，将编码器的0~8192转化为角度表示。

```c
/* @brief 由于DJI电机发送以四个一组的形式进行,故对其进行特殊处理,用6个(2can*3group)can_instance专门负责发送
 *        该变量将在 DJIMotorControl() 中使用,分组在 MotorSenderGrouping()中进行
 *
 * can1: [0]:0x1FF,[1]:0x200,[2]:0x2FF
 * can2: [0]:0x1FF,[1]:0x200,[2]:0x2FF */
static can_instance sender_assignment[6] =
{
        [0] = {.can_handle = &hcan1, .txconf.StdId = 0x1ff, .txconf.IDE = CAN_ID_STD, .txconf.RTR = CAN_RTR_DATA, .txconf.DLC = 0x08, .tx_buff = {0}},
  ...
        ...
};

static uint8_t sender_enable_flag[6] = {0};
```

- 这些是电机分组发送所需的变量。注册电机时，会根据挂载的总线以及发送id，将电机分组。在CAN发送电机控制信息的时候，根据`sender_assignment[]`保存的分组进行发送，而不会使用电机实例自带的`can_instance`。
- DJI电机共有3种分组，分别为0x1FF,0x200,0x2FF。注册电机的时候，`MotorSenderGrouping()`函数会根据发送id计算出CAN的`tx_id`（即上述三个中的一个）和`rx_id`。然后为电机实例分配用于指示其在`sender_assignment[]`中的编号的 `sender_group`和其在该发送组中的位置`message_num`（一帧报文可以发送四条控制指令，`message_num`会指定电机是这四个中的哪一个）。具体的分配请查看`MotorSenderGrouping()`的定义。
- 当某一个分组有电机注册时，该分组的索引将会在`sender_enable_flag`[]中被置1，这样，就可以避免发送没有电机注册的报文，防止总线拥塞。具体的，在`DecodeDJIMotor()`中，该函数会查看`sender_enable_flag[]`的每一个位置，确定这一组是否有电机被注册，若有则发送`sender_assignment[]`中对应位置的`tx_buff`。

```c
static void IDcrash_Handler(uint8_t conflict_motor_idx, uint8_t temp_motor_idx)

static void MotorSenderGrouping(can_instance_config *config)

static void DecodeDJIMotor(can_instance *_instance)
```

- `IDcrash_Handler()`在电机id发生冲突的时候会被`MotorSenderGrouping()`调用，陷入死循环之中，并把冲突的id保存在函数里。这样就可以通过debug确定是否发生冲突以及冲突的编号。

- `MotorSenderGrouping()`被`DJIMotorInit()`调用，他将会根据电机id计算出CAN的发送和接收ID，并根据发送ID对电机进行分组。

- `DecodeDJIMotor()`是解析电机反馈报文的函数，在`DJIMotorInit()`中会将其注册到该电机实例对应的`can_instance`中（即`can_instance`的`can_module_callback()`）。这样，当该电机的反馈报文到达时，`bsp_can.c`中的回调函数会调用解包函数进行反馈数据解析。

  该函数还会对电流和速度反馈值进行滤波，消除高频噪声；同时计算多圈角度和单圈绝对角度。
  
  **电机反馈的电流值为说明书中的映射值，需转换为实际值。**
  
  **反馈的速度单位是rpm（转每分钟），转换为角度每秒。**
  
  **反馈的位置是编码器值（0~8191），转换为角度。**

## 使用范例

```c
//初始化设置
Motor_Init_Config_s config = {
  .motor_type = GM6020,
  .can_init_config = {
   .can_handle = &hcan1,
   .tx_id = 6
        },
  .controller_setting_init_config = {
            .angle_feedback_source = MOTOR_FEED, 
            .outer_loop_type = SPEED_LOOP,
            .close_loop_type = SPEED_LOOP | ANGLE_LOOP, 
            .speed_feedback_source = MOTOR_FEED, 
            .motor_reverse_flag = MOTOR_DIRECTION_NORMAL
        },
  .controller_param_init_config = {
            .angle_PID = {
                .Improve = 0, 
                .Kp = 1, 
                .Ki = 0, 
                .Kd = 0, 
                .DeadBand = 0, 
                .MaxOut = 4000}, 
            .speed_PID = {
                .Improve = 0, 
                .Kp = 1, 
                .Ki = 0, 
                .Kd = 0, 
                .DeadBand = 0, 
                .MaxOut = 4000
            }
        }
};
//注册电机并保存实例指针
dji_motor_instance *djimotor = DJIMotorInit(&config);
```

然后在任务中修改电机设定值即可实现控制：

```
DJIMotorSetRef(djimotor, 10);
```

前提是已经将`DJIMotorControl()`放入实时系统任务当中或以一定d。你也可以单独执行`DJIMotorControl()`。
