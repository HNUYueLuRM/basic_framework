#ifndef BUZZER_H
#define BUZZER_H
#include "bsp_pwm.h"
#define BUZZER_DEVICE_CNT 5

#define  DoFreq  523
#define  ReFreq  587
#define  MiFreq  659
#define  FaFreq  698
#define  SoFreq  784
#define  LaFreq  880
#define  SiFreq  988

typedef enum
{
    OCTAVE_1 = 0,
    OCTAVE_2,
    OCTAVE_3,
    OCTAVE_4,
    OCTAVE_5,
    OCTAVE_6,
    OCTAVE_7,
    OCTAVE_8,
}octave_e;

typedef enum
{
    ALARM_LEVEL_HIGH = 0,
    ALARM_LEVEL_ABOVE_MEDIUM,
    ALARM_LEVEL_MEDIUM,
    ALARM_LEVEL_BELOW_MEDIUM,
    ALARM_LEVEL_LOW,
}AlarmLevel_e;

typedef enum
{
    ALARM_OFF = 0,
    ALARM_ON,
}AlarmState_e;
typedef struct
{
    AlarmLevel_e alarm_level;
    octave_e octave;
    float loudness;
}Buzzer_config_s;

typedef struct
{
    float loudness;
    octave_e octave;
    AlarmLevel_e alarm_level;
    AlarmState_e alarm_state;
}BuzzzerInstance;


void BuzzerInit();
void BuzzerTask();
BuzzzerInstance *BuzzerRegister(Buzzer_config_s *config);
void AlarmSetStatus(BuzzzerInstance *buzzer, AlarmState_e state);
#endif // !BUZZER_H
