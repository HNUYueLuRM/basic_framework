/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ins_task.h"
#include "motor_task.h"
#include "led_task.h"
#include "referee_task.h"
#include "master_process.h"
#include "daemon.h"
#include "robot.h"
#include "HT04.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId insTaskHandle;
osThreadId ledTaskHandle;
osThreadId robotTaskHandle;
osThreadId motorTaskHandle;
osThreadId daemonTaskHandle;
osThreadId uiTaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartINSTASK(void const *argument);

void StartMOTORTASK(void const *argument);

void StartDAEMONTASK(void const *argument);

void StartROBOTTASK(void const *argument);

void StartUITASK(void const *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(instask, StartINSTASK, osPriorityAboveNormal, 0, 1024);
  insTaskHandle = osThreadCreate(osThread(instask), NULL); // 由于是阻塞读取传感器,为姿态解算设置较高优先级,确保以1khz的频率执行
  // 后续修改为读取传感器数据准备好的中断处理,

  osThreadDef(motortask, StartMOTORTASK, osPriorityNormal, 0, 256);
  motorTaskHandle = osThreadCreate(osThread(motortask), NULL);

  osThreadDef(daemontask, StartDAEMONTASK, osPriorityNormal, 0, 128);
  daemonTaskHandle = osThreadCreate(osThread(daemontask), NULL);

  osThreadDef(robottask, StartROBOTTASK, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(robottask), NULL);

  osThreadDef(uitask, StartUITASK, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(uitask), NULL);

  /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  vTaskDelete(NULL); // 删除默认任务,防止占用CPU
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartINSTASK(void const *argument)
{
  while (1)
  {
    // 1kHz
    INS_Task();
    VisionSend(); // 解算完成后发送视觉数据,但是当前的实现不太优雅,后续若添加硬件触发需要重新考虑结构的组织
    osDelay(1);
  }
}

void StartMOTORTASK(void const *argument)
{
  // 若使用HT电机则取消本行注释,该接口会为注册了的电机设备创建线程
  // HTMotorControlInit();
  while (1)
  {
    // 500Hz
    MotorControlTask();
    osDelay(2);
  }
}

void StartDAEMONTASK(void const *argument)
{
  while (1)
  {
    // 100Hz
    DaemonTask();
    osDelay(10);
  }
}

void StartROBOTTASK(void const *argument)
{
  while (1)
  {
    // 200Hz-500Hz,若有额外的控制任务如平衡步兵可能需要提升至1kHz
    RobotTask();
    osDelay(5);
  }
}

void StartUITASK(void const *argument)
{
  My_UI_init();
  while (1)
  {
    Referee_Interactive_task(); // 每次给裁判系统发送完一包数据后,挂起一次,防止卡在裁判系统发送中,详见Referee_Interactive_task函数的refereeSend();
    osDelay(1); // 即使没有任何UI需要刷新,也挂起一次,防止卡在UITask中无法切换
  }
}
/* USER CODE END Application */
