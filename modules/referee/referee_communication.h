#ifndef REFEREE_COMMUNICATION_H
#define REFEREE_COMMUNICATION_H

#include "stdint.h"
#include "referee_def.h"
#include "rm_referee.h"

void Communicate_SendData(referee_id_t *_id,robot_interactive_data_t *_data);

#endif