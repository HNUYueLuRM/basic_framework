# dmmotor

---

## 总览和封装说明



dmmotor 模块对达妙（DM）系列电机进行了完整的驱动封装，提供**普通模式**（串级 PID 控制）与 **MIT 模式**（直接位置‑速度‑力矩前馈 \+ 阻抗控制）两种控制方式。和dji\_motor模块一致，你不再需要关心 CAN 报文的发送接收、协议解析以及 PID 的具体计算，只需通过对应接口设定参考值，模块会自动完成所有底层工作。



**==设定值的单位==**



1. 位置环为**弧度制**

2. 速度环为角速度，单位为**弧度/每秒**（rad/s）==

3. 力矩的单位为N \* m



## 快速开始



### 电机和头文件的配置



请使用达妙调试助手连接你的电机，重点关注以下参数：

- PMAX, VMAX, TMAX：应该与dmmotor\.h中的宏定义一致

- id：代表txid，由于通信协议的限制，txid请不要超过0x0F

- Master id：代表rxid

- timeout：达妙电机内置硬件看门狗，在未收到控制帧一段时间后会自动进入通讯丢失状态并失能，框架内会自动尝试恢复通信并使能电机，推荐设置为2000\-10000（代表0\.1\-0\.5s），若设置为0则关闭该功能



前往dmmotor\.h，重点关注以下宏定义：

- `DM_MOTOR_CNT`：达妙电机最大注册数量，如果你预期注册的达妙电机超过这个值，请进行修改

- `DM_TASK_CYCLE`：达妙任务运行周期，决定控制频率，PID计算频率以及电机反馈频率，必须为整数，单位为ms，推荐值为2

- `DM_P_MIN DM_P_MAX DM_V_MIN DM_V_MAX DM_T_MIN DM_T_MAX `：务必和电机内置参数保持一致



前往motor\_task\.c，观察MotorControlTask\(\)函数中是否有`DMMotorTask();` ，如果没有，请主动将其加入



### 初始化并注册电机



初始化时你需要提供的参数因模式而异：



#### 普通模式 \(`DMMotorInit`\)



- **CAN 总线设置**：CAN 句柄及电机 tx\_id和rx\_id

- **控制设置**（`DMMotor_Control_Setting_s`）：

    - 外层闭环类型 `outer_loop_type`（位置、速度等）

    - 内层闭环组合 `close_loop_type`（使用位或组合，如 `SPEED_LOOP | ANGLE_LOOP`）

    - 是否反转 `motor_reverse_flag`

    - 反馈来源 `angle_feedback_source`、`speed_feedback_source`（若为 `OTHER_FEED`，需提供外部数据指针）

    - 前馈类型 `feedforward_flag` \(使用位或组合\)

- **控制器参数**（`DMMotor_Controller_Init_s`）：

    - 角度环、速度环的 PID 参数

    - 外部反馈数据指针 `other_angle_feedback_ptr`、`other_speed_feedback_ptr`

    - 前馈数据指针 `speed_feedforward_ptr`、`force_feedforward_ptr`

        

#### MIT 模式 \(`MITMotorInit`\)



- **CAN 总线设置**：CAN 句柄及电机 tx\_id和rx\_id

- **MIT 参数**（`MIT_Controller_Init_s`）：`Kp` `Kd` 参数

- **是否反转**：`motor_reverse_flag`

    

初始化后电机自动进入使能状态，可立即控制。



### 电机控制以及外部接口说明



电机实例注册完毕后，在application调用以下函数，即可实现电机控制



- **通用接口**

```C
/**
 * @brief 软件开启电机，电机初始化后默认就是开启状态
 * @param motor 对应的电机实例
 */
void DMMotorEnable(DMMotorInstance *motor);

/**
 * @brief 软件关闭电机，使电机输出为0并清空PID参数，请调用该接口来对电机进行急停而不是直接发送失能帧
 * @param motor 对应的电机实例
 */
void DMMotorStop(DMMotorInstance *motor);

/**
 * @brief 将电机当前位置设置为电机的零点，不推荐调用
 * @param motor 对应的电机实例
 */
void DMMotorCaliEncoder(DMMotorInstance *motor);
```



- **普通模式接口**

```C
/**
 * @brief DM普通模式下设置最外环参考值，内环参考值请通过前馈指针设置
 * @param motor 对应的电机实例
 * @param ref 设置的参考值
 */
void DMMotorSetRef(DMMotorInstance *motor, float ref);

/**
 * @brief DM普通模式下设置最外环的闭环类型
 * @param motor 对应的电机实例
 * @param closeloop_type 闭环类型
 */
void DMMotorOuterLoop(DMMotorInstance *motor, Closeloop_Type_e closeloop_type);

/**
 * @brief DM普通模式下切换反馈类型
 * @param motor 对应的电机实例
 * @param loop 需要切换反馈类型的闭环
 * @param type 反馈类型
 */
void DMMotorChangeFeed(DMMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type);
```



- **MIT模式接口**

```C
/**
 * @brief MIT模式下设置位置参考值
 * @param motor 对应的电机实例
 * @param angle 设置的参考值
 */
void DMMotorSetMITAngle(DMMotorInstance *motor, float angle);

/**
 * @brief MIT模式下设置速度参考值
 * @param motor 对应的电机实例
 * @param speed 设置的参考值
 */
void DMMotorSetMITSpeed(DMMotorInstance *motor, float speed);

/**
 * @brief MIT模式下设置力矩参考值
 * @param motor 对应的电机实例
 * @param torque 设置的参考值
 */
void DMMotorSetMITTorque(DMMotorInstance *motor, float torque);
```

### 获取电机反馈数据



直接访问电机实例的 `measure` 成员：



```C
float angle = motor->measure.total_position;
float speed = motor->measure.velocity;
float torque = motor->measure.torque;
```

---

## 代码结构



`.h` 文件内包含外部接口、类型定义及模块配置宏。  

`.c` 文件内包含私有函数和接口实现。  

`motor_def.h` 提供闭环类型、反馈源等通用定义。



## 类型定义



```C
#define DM_MOTOR_CNT 6          // 最大支持的达妙电机数量，避免总线拥塞
#define DM_TASK_CYCLE 2         // 控制任务运行周期（ms），决定控制频率

// 以下映射范围请根据实际电机和调试助手修改！
#define DM_P_MIN (-12.5f)       // 位置下限（建议 -π ~ π）
#define DM_P_MAX 12.5f          // 位置上限
#define DM_V_MIN (-45.0f)       // 速度下限
#define DM_V_MAX 45.0f          // 速度上限
#define DM_T_MIN (-54.0f)       // 力矩下限
#define DM_T_MAX 54.0f          // 力矩上限

#define KP_MIN 0.0f             // MIT 模式 Kp 下限（固定）
#define KP_MAX 500.0f           // MIT 模式 Kp 上限
#define KD_MIN 0.0f             // MIT 模式 Kd 下限（固定）
#define KD_MAX 5.0f             // MIT 模式 Kd 上限
```



### 反馈数据结构

```C
typedef struct {
    uint8_t state;             // 状态码，见 DMMotor_Err_state_e
    float velocity;            // 当前速度
    float last_position;       // 上一次角度（用于多圈计算）
    float position;            // 单圈角度
    float total_position;      // 累计多圈角度
    float torque;              // 当前力矩
    float T_Mos;               // MOS 温度
    float T_Rotor;             // 转子温度
    int total_round;           // 累计圈数
} DM_Motor_Measure_s;
```



### 电机实例结构体

```C
typedef struct {
    DM_Motor_Measure_s measure;                // 反馈数据
    DMMotor_Control_Setting_s motor_settings;  // 控制设置（普通模式）
    PIDInstance speed_PID;                     // 速度环 PID
    PIDInstance angle_PID;                     // 角度环 PID
    float *other_angle_feedback_ptr;           // 外部角度反馈
    float *other_speed_feedback_ptr;           // 外部速度反馈
    float *speed_feedforward_ptr;              // 速度前馈
    float *force_feedforward_ptr;              // 力矩前馈
    float pid_ref;                             // 最外层参考输入
    float MIT_ref_angle;                       // MIT 模式角度参考
    float MIT_ref_speed;                       // MIT 模式速度参考
    float MIT_ref_torque;                      // MIT 模式力矩参考
    float MIT_Kp;                              // MIT 模式 Kp
    float MIT_Kd;                              // MIT 模式 Kd
    int rx_refresh_dt;                         // 通信质量监测（自增）
    int rx_init_flag;                          // 首次接收标志
    Motor_Working_Type_e stop_flag;            // 软件启停标志
    CANInstance *motor_can_instance;           // 挂载的 CAN 实例
    Motor_Control_Type_e working_type;         // NORMAL_MODE / MIT_MODE
} DMMotorInstance;
```



### 控制设置（普通模式专用）

```C
typedef struct {
    Closeloop_Type_e outer_loop_type;        // 最外层闭环（如 ANGLE_LOOP）
    Closeloop_Type_e close_loop_type;        // 启用的闭环组合（如 SPEED_LOOP|ANGLE_LOOP）
    Motor_Reverse_Flag_e motor_reverse_flag; // 是否反转
    Feedback_Source_e angle_feedback_source; // 角度反馈源
    Feedback_Source_e speed_feedback_source; // 速度反馈源
    Feedfoward_Type_e feedforward_flag;      // 前馈类型
} DMMotor_Control_Setting_s;
```



### 闭环类型与反馈源（来自 `motor_def.h`）

```C
typedef enum {
    CURRENT_LOOP = 0b0001,
    SPEED_LOOP   = 0b0010,
    ANGLE_LOOP   = 0b0100,
    // 组合
    SPEED_AND_CURRENT_LOOP  = 0b0011,
    ANGLE_AND_SPEED_LOOP    = 0b0110,
    ALL_THREE_LOOP          = 0b0111,
} Closeloop_Type_e;

typedef enum {
    MOTOR_FEED = 0,
    OTHER_FEED = 1,
} Feedback_Source_e;

typedef enum {
    MOTOR_DIRECTION_NORMAL  = 0,
    MOTOR_DIRECTION_REVERSE = 1,
} Motor_Reverse_Flag_e;
```



## 外部接口



```C
// ---- 初始化 ----
DMMotorInstance *DMMotorInit(DMMotor_Init_Config_s *config);  // 普通模式
DMMotorInstance *MITMotorInit(MIT_Init_Config_s *config);     // MIT 模式

// ---- 普通模式控制 ----
void DMMotorSetRef(DMMotorInstance *motor, float ref);        // 设定最外层参考值
void DMMotorOuterLoop(DMMotorInstance *motor, Closeloop_Type_e type); // 切换最外层闭环
void DMMotorChangeFeed(DMMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type); // 切换反馈源

// ---- MIT 模式控制 ----
void DMMotorSetMITAngle(DMMotorInstance *motor, float angle);
void DMMotorSetMITSpeed(DMMotorInstance *motor, float speed);
void DMMotorSetMITTorque(DMMotorInstance *motor, float torque);

// ---- 通用 ----
void DMMotorEnable(DMMotorInstance *motor);   // 软件使能
void DMMotorStop(DMMotorInstance *motor);     // 软件停止，输出清 0，普通模式下清 PID
void DMMotorCaliEncoder(DMMotorInstance *motor); // 将当前位置设为零点（慎用）

// ---- 任务函数 ----
void DMMotorTask();   // 需放入实时任务中周期调用
```



- `DMMotorInit()` 与 `MITMotorInit()` 分别用于注册普通模式与 MIT 模式的电机，**返回实例指针，失败返回 NULL**。调用前需填充对应的初始化配置结构体，其中 CAN 的 `tx_id` 必须 ≤ 0x0F。

- `DMMotorSetRef()` 设置普通模式最外环的参考值。如果有多级闭环，内环参考值由外环 PID 输出自动产生；也可以通过前馈指针直接注入内环给定。

- `DMMotorOuterLoop()` 动态改变最外层闭环，例如将拨盘电机在角度闭环和速度闭环之间切换。

- `DMMotorChangeFeed()` 改变指定闭环的反馈数据源（角度或速度），需预先在初始化时提供外部数据指针。

- `DMMotorSetMITAngle/Speed/Torque()` 分别设定 MIT 模式下的位置、速度、力矩参考值，直接通过 CAN 帧发送给电机，由电机内部阻抗控制器执行。反向标志会自动处理符号。

- `DMMotorEnable()` / `DMMotorStop()` 控制电机输出启停。

- `DMMotorCaliEncoder()` 发送零点设置命令，调用后会阻塞 10 ms，不推荐运行中调用。

- `DMMotorTask()` 是核心任务，完成反馈解析、自动恢复通信、PID 计算（普通模式）以及控制帧发送。**必须在 ****`MotorControlTask()`**** 或类似实时任务中以固定周期调用**，周期由 `DM_TASK_CYCLE` 决定（例如每 2 ms 执行一次）。

    

## 私有函数和变量



```C
static uint8_t idx_dm = 0;                              // 已注册电机计数
static DMMotorInstance dm_motor_instance[DM_MOTOR_CNT];  // 电机实例池
static uint8_t dm_cycle_cnt = 0;                         // 任务分频计数器

static uint16_t float_to_uint(...);   // 浮点转 CAN 报文整型
static float uint_to_float(...);      // CAN 报文整型转浮点
static void DMMotorSetMode(...);      // 发送使能/失能/清错/零点命令帧
static void DMMotorDecode(...);       // 反馈帧解析（CAN 接收回调）
static void DMAutoRecoverComm(...);   // 自动使能、清通信丢失错误
```



- `dm_motor_instance` 为静态分配的电机实例池，最大数量由 `DM_MOTOR_CNT` 决定。

- `DMMotorDecode()` 注册为 CAN 接收回调，完成状态解析、多圈累计、位置/速度/力矩/温度的提取。**多圈计数基于两次采样间转动不超过 ****`DM_P_MAX`**** 的假设。**

- `DMAutoRecoverComm()` 在任务中每次都会检查，若电机处于失能或通信丢失状态，自动发送使能或清除错误命令。

- 普通模式的 PID 计算在 `DMMotorTask()` 内部完成，顺序为角度环 → 速度环 →（可由前馈注入力矩）。PID 输出经过限幅并转换为 CAN 报文后发送。

    

## 使用范例



### 1\. 普通模式



```C
#include "dmmotor.h"

/* 定义外部反馈源（如有） */
float imu_yaw_angle = 0.0f;      // 由 IMU 更新
float feedforward_speed = 0.0f;  // 速度前馈值
float feedforward_torque = 0.0f; // 力矩前馈值

/* 构建初始化配置 */
DMMotor_Init_Config_s config = {
    .can_init_config = {
        .can_handle = &hcan1,
        .tx_id = 0x01,            // 达妙电机 CAN ID，必须 ≤ 0x0F
        .rx_id = 0x16
    },
    .controller_setting_init_config = {
        .outer_loop_type = ANGLE_LOOP,//最外层为角度环
        .close_loop_type = ANGLE_LOOP | SPEED_LOOP,  // 位置‑速度串级
        .motor_reverse_flag = MOTOR_DIRECTION_NORMAL,// 不反转
        .angle_feedback_source = OTHER_FEED,// 采用imu反馈
        .speed_feedback_source = MOTOR_FEED,// 采用电机反馈
        .feedforward_flag = SPEED_FEEDFORWARD | FORCE_FEEDFORWARD  // 启用速度+力矩前馈
    },
    .controller_param_init_config = {
        .angle_PID = {.Kp=2.0f, .Ki=0.0f, .Kd=0.5f, .MaxOut=30.0f,
                      .Improve=0},
        .speed_PID = {.Kp=5.0f, .Ki=1.0f, .Kd=0.0f, .MaxOut=50.0f,
                      .Improve=0},
        .other_angle_feedback_ptr = &imu_yaw_angle,                 
        .speed_feedforward_ptr = &feedforward_speed,        
        .force_feedforward_ptr = &feedforward_torque//填入需要使用的反馈和前馈的数据源的指针
    }
};

/* 注册电机，保存指针 */
DMMotorInstance *motor1 = DMMotorInit(&config);
```



在应用层控制：

```C
// 设置目标角度（例如转到 3.14 弧度）
DMMotorSetRef(motor1, 3.14f);

// 若需要切换为速度闭环
DMMotorOuterLoop(motor1, SPEED_LOOP);
DMMotorSetRef(motor1, 10.0f);  // 设置目标速度为10 rad/s
```



### 2\. MIT 模式



```C
#include "dmmotor.h"

MIT_Init_Config_s mit_config = {
    .can_init_config = {
        .can_handle = &hcan1,
        .tx_id = 0x02,
        .rx_id = 0x18
    },
    .motor_reverse_flag = MOTOR_DIRECTION_NORMAL,
    .controller_param_init_config = {
        .Kp = 100.0f,
        .Kd = 5.0f
    }
};

DMMotorInstance *motor2 = MITMotorInit(&mit_config);
```



在应用层控制：

```C
// 设定目标位置 0.5 rad，速度 0 rad/s，前馈力矩 1.0 N·m
DMMotorSetMITAngle(motor2, 0.5f);
DMMotorSetMITSpeed(motor2, 0);
DMMotorSetMITTorque(motor2, 1.0f);
```



### 3\. 在任务中调用 `DMMotorTask()`

```C
void MotorControlTask(void)
{
    //此处有其他电机的任务函数
    DMMotorTask();
}
```



