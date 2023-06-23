#include "bsp_pwm.h"
#include "buzzer.h"
#include "bsp_dwt.h"

static PWMInstance *buzzer;

static alarm_level_e now_alarm_level = ALARM_OFFLINE;

void BuzzerOn(PWMInstance *buzzer );
/**
*
* @brief 蜂鸣器报警函数
* @param alarm_level 报警级别
*/
void BuzzerPlay(alarm_level_e alarm_level)
{
    static alarm_level_e last_alarm_level = ALARM_LEVEL_LOW;
    
    if(((int)DWT_GetTimeline_s() % 5)<1) //每5秒检查一次
    {
        last_alarm_level = ALARM_LEVEL_LOW;
        now_alarm_level = ALARM_OFFLINE;
    }
    
    if(last_alarm_level <= now_alarm_level) //如果当前报警级别大于等于上一次报警级别,则更新报警级别
    {
        now_alarm_level = alarm_level;
    }
    last_alarm_level = alarm_level;

}

/**
 * @brief 蜂鸣器初始化
 * 
 */
void buzzer_init()
{
    PWM_Init_Config_s buzzer_config = {
        .htim = &htim4,
        .channel= TIM_CHANNEL_3,
        .period = 1,
        .pulse = 10000,
        .callback = BuzzerOn,
    };
    buzzer = PWMRegister(&buzzer_config);
}
/**
 * @brief 开启蜂鸣器
 * 
 * @param buzzer 
 */
//*@todo: 优化报警声，应类似D__,DDD,B__,BBB等报警声
void BuzzerOn(PWMInstance *buzzer )
{
    switch (now_alarm_level)
    {
    case ALARM_LEVEL_LOW:
        PWMSetPeriod(buzzer, 1);
        PWMSetPulse(buzzer, 10000);
        break;
    case ALARM_LEVEL_BELOW_MEDIUM:
        PWMSetPeriod(buzzer, 2);
        PWMSetPulse(buzzer, 10000);
        break;
    case ALARM_LEVEL_MEDIUM:
        PWMSetPeriod(buzzer, 3);
        PWMSetPulse(buzzer, 10000);
        break;
    case ALARM_LEVEL_ABOVE_MEDIUM:
        PWMSetPeriod(buzzer, 4);
        PWMSetPulse(buzzer, 10000);
        break;
    case ALARM_LEVEL_HIGH:
        PWMSetPeriod(buzzer, 5);
        PWMSetPulse(buzzer, 10000);
        break;
    
    default:
        PWMSetPulse(buzzer, 0);
        break;
    }
}





